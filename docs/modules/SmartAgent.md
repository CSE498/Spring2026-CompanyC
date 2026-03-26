# SmartAgent and Native Bridge

## 0. Introduction

This document describes how `SmartAgent` currently works in the codebase, along with how the native callback bridge talks to the local llama-server process.

This is a description of the current implementation, not an idealized design. If the code changes later, this doc should be updated to match the code.

## 1. What `SmartAgent` Is

`SmartAgent` is an `AgentBase` subclass that asks an external text-producing callback for its next move. The callback does not return an action ID directly. Instead, it returns text, and `SmartAgent` tries to extract one legal move from that text.

Right now, `SmartAgent` is built around a short-horizon control loop:

1. Build a compact prompt from the current world state.
2. Ask the callback for a reply asynchronously.
3. Parse one move out of that reply.
4. Buffer the move locally.
5. Prefetch the next move when there is room in the buffer.

The current implementation buffers at most `2` actions and tracks at most `4` legal moves for any one request.

## 2. Core State Inside `SmartAgent`

The important internal state is:

- `buffered_actions`
  Ready-to-use action IDs that have already been chosen from previous replies.

- `buffered_action_count`
  How many entries in `buffered_actions` are currently valid.

- `requested_moves`
  The legal move names shown in the most recent outstanding prompt.

- `requested_move_positions`
  The destination cell for each move listed in `requested_moves`.

- `requested_move_count`
  How many legal moves were included in the current pending request.

- `npc_request_in_flight`
  Whether a callback request is currently still running.

- `has_npc_response_future` and `npc_response_future`
  The future returned by the callback, if one exists and is still being tracked.

- `pending_request_grid`
  A snapshot of the grid that was used when the current request was created.

- `pending_request_start_position`
  The position the agent was planning from when the current request was created.

- `last_notification_message` and `last_notification_type`
  The last goal or other message passed in through `Notify()`.

- `last_npc_line`
  The raw text from the most recently handled NPC reply, or a local status message such as `"No valid moves."`

## 3. Initialization and Required Actions

`SmartAgent::Initialize()` succeeds only if the agent has all four movement actions:

- `up`
- `down`
- `left`
- `right`

In `MazeWorld`, those actions are provided automatically when the agent is added to the world.

## 4. The System Prompt

`SmartAgent` exposes a shared system prompt through `GetSystemPrompt()`. The current text is:

```text
The PRIMARY GOAL matters most. Use the LEGAL MOVES and the GRID to choose the best next legal move. Think briefly, then reply with exactly two lines: PLAN: <very short reasoning> and MOVE: <one legal move word>.
```

This is not just documentation text. The native bridge uses this exact string when it calls the local model.

## 5. How the User Prompt Is Built

Each request also gets a user prompt built by `BuildUserPrompt()`. It always uses the same overall shape:

```text
PRIMARY GOAL:
<goal text>

LEGAL MOVES:
- <move> -> (x, y)
...

GRID (# = wall, space = floor, <agent symbol> = you):
+-----------------------+
|...|
...
+-----------------------+

Reply in exactly 2 lines:
PLAN: <very short reasoning>
MOVE: <one legal move word that best advances the PRIMARY GOAL>
```

The pieces come from the following places:

- `PRIMARY GOAL`
  If `Notify()` has previously been called with a message, that message is used here.
  Otherwise the fallback text is:
  `No explicit goal was provided. Make legal progress.`

- `LEGAL MOVES`
  This section only lists moves that are walkable from the current planning position.

- `GRID`
  The actual world grid is rendered as ASCII.
  Walls are shown as `#`.
  Floor cells are shown as spaces.
  The agent's symbol is drawn at the planning position.

The legal moves are always considered in this fixed order:

1. `up`
2. `down`
3. `left`
4. `right`

Moves that would leave the grid or hit a wall are simply omitted from the prompt.

## 6. What the Planning Position Means

`SmartAgent` does not always plan from its literal current position.

Before building a new prompt, it calls `GetPlanningStartPosition()`. That function starts from the agent's current location and then simulates any already-buffered actions in order. If the buffered moves remain legal, the planning position becomes the future position the agent expects to reach after consuming its buffered plan.

This is why prompts can describe moves from a position the agent has not physically reached yet. The agent is trying to keep one step ahead.

## 7. How Replies Are Parsed

The raw callback reply is processed by `NormalizeReply()`.

That function works like this:

1. The full reply is lowercased.
2. It looks line by line for a labeled choice line whose label is one of:
   - `move`
   - `choice`
   - `final`
3. If one of those labels exists, it extracts the first recognized direction token that appears after the colon on that line.
4. If no labeled line yields a move, it scans the entire reply and keeps the last recognized direction token it sees.
5. The only valid direction tokens are:
   - `up`
   - `down`
   - `left`
   - `right`

Examples:

- `"MOVE: right"` becomes `right`
- `"PLAN: go around\nMOVE: down"` becomes `down`
- `"I think left is bad, so go right"` becomes `right`
- `"banana"` becomes empty

The important detail is that `last_npc_line` stores the original raw text, not the normalized token.

## 8. How Replies Become Buffered Actions

When a reply is ready, `HandleNpcReply()` does the work.

It performs these steps:

1. Save the raw reply text into `last_npc_line`.
2. Mark the request as no longer in flight.
3. If there is no pending request metadata, ignore the reply contents and stop.
4. Normalize the reply text into one of the four move names, or an empty string.
5. Re-check that the decoded move is still legal from `pending_request_start_position` on `pending_request_grid`.
6. Convert the move name into an action ID.
7. Queue the action if it is valid and there is buffer space.
8. Clear `requested_move_count` when done.

If the reply is invalid, empty, or no longer legal, no action is buffered.

## 9. The `SelectAction()` Flow

`SelectAction()` is the main control loop. Right now it works in this exact order:

1. If the previous action failed (`action_result == 0`), clear the current plan.
2. Poll for a ready callback result without blocking.
3. If there is no request in flight and there is room in the buffer, start a new request.
4. Poll again immediately so already-ready futures can be consumed in the same turn.
5. If at least one buffered move exists:
   - try to start one more request to prefetch the following move
   - pop and return the first buffered action
6. If no move is available, return action ID `0`

The second poll matters. It lets tests and immediate futures behave like a synchronous callback without needing a second world step.

## 10. When `SmartAgent` Clears Its Plan

`ClearPlan()` resets:

- `buffered_action_count`
- `requested_move_count`
- `npc_request_in_flight`
- `has_npc_response_future`
- `npc_response_future`

It does not clear the remembered goal message.

`ClearPlan()` is triggered in two main places:

- at the start of `SelectAction()` when the previous action result was failure
- inside `Notify()`

`Notify()` also clears `last_npc_line`, because a new goal invalidates any old conversational context.

## 11. Error and Edge Behavior in `SmartAgent`

Current edge handling looks like this:

- No walkable moves from the planning position
  `last_npc_line` becomes `"No valid moves."` if there is no existing buffered plan.

- No callback installed
  `last_npc_line` becomes `"No NPC requester installed."`

- Callback returns an invalid future
  `last_npc_line` becomes `"NPC requester did not return a response future."`

- Callback reply contains no recognized move
  No action is buffered and `SelectAction()` may return `0`.

- Buffered move becomes illegal during simulation
  It will not be queued from that reply.

## 12. Native Bridge Overview

The native bridge lives in `source/web/SmartAgentNativeBridge.hpp`.

It is only compiled when `__EMSCRIPTEN__` is not defined. In other words, this bridge is the native desktop path, not the web build path.

Its job is to provide the default callback implementation that `SmartAgent` can use outside of Emscripten. It does this with:

- a local HTTP request to `http://127.0.0.1:8080/v1/chat/completions`
- an OpenAI-style `messages` payload
- a detached background thread
- a `std::future<std::string>` returned back to `SmartAgent`

`EnsureSmartAgentDefaultCallback()` installs this bridge by calling:

```cpp
SmartAgent::SetNpcRequestCallback(&smart_agent_bridge_detail::RequestSmartAgentMoveViaLlamaServer);
```

## 13. Native Bridge Request Settings

The bridge currently hardcodes these parameters:

- `llama_url = "http://127.0.0.1:8080/v1/chat/completions"`
- `llama_user_prompt_prefix = ""`
- `llama_user_prompt_suffix = ""`
- `llama_max_tokens = 24`
- `llama_temperature = 0`
- `llama_top_k = 40`
- `llama_top_p = 0.95`
- `llama_seed = -1`

So the bridge is expecting a local llama-server instance that accepts OpenAI-compatible chat completions.

## 14. Native Bridge Async Entry Point

The main callback function is:

`RequestSmartAgentMoveViaLlamaServer(const SmartAgent & agent, const std::string & user_prompt)`

What it does:

1. Reads the system prompt from `SmartAgent::GetSystemPrompt()`.
2. Builds the final user prompt by adding the configured prefix and suffix around the incoming prompt.
3. Logs the prompt and token budget to `stdout`.
4. Creates a `std::promise<std::string>` and matching future.
5. Launches a detached background thread.
6. In that thread, calls the synchronous HTTP helper.
7. Stores the returned text into the promise.
8. If anything throws, stores an empty string instead.
9. Returns the future immediately.

The `agent` parameter is currently unused.

## 15. Native Bridge Synchronous Request Path

The actual HTTP work happens in:

`RequestSmartAgentMoveViaLlamaServerSync(std::string system_prompt, std::string sent_prompt, int request_max_tokens)`

That function does the following:

1. Manually builds a JSON request body with:
   - `messages`
   - `max_tokens`
   - `temperature`
   - `top_k`
   - `top_p`
   - `seed`
   - `stream: false`
2. Writes that JSON into a temporary file named `smart_agent_request.json` in the system temp directory.
3. Runs `curl` with:
   - `-sS`
   - `--fail`
   - `--max-time 15`
   - `Content-Type: application/json`
   - `--data-binary @<temp file>`
4. Deletes the temp file.
5. If the response is empty, logs an error and returns an empty string.
6. Otherwise, extracts the assistant content from the JSON reply.
7. Trims leading and trailing whitespace from that content.
8. Logs the raw and decoded response.
9. Returns the decoded response text.

## 16. Bridge Helper Functions

The bridge uses a small set of utility helpers:

- `Trim()`
  Removes leading and trailing whitespace.

- `JsonEscape()`
  Escapes quotes, backslashes, and common control characters so prompt text can be embedded safely in JSON.

- `QuoteArg()`
  Wraps a shell argument in double quotes.

- `ReadCommandOutput()`
  Executes a shell command using `popen()` or `_popen()` and captures stdout.

- `RequestPath()`
  Builds the temp file path used for the JSON request body.

- `WriteTextFile()`
  Writes the request body to disk.

- `ParseJsonStringAt()`
  Parses one JSON string literal starting at a known quote character.

- `ExtractChatReply()`
  A minimal parser that looks for the current response shape:
  `choices -> message -> content`

This parser is intentionally narrow. It is only trying to pull the assistant content field out of the expected local llama-server response.

## 17. What the Native Bridge Returns to `SmartAgent`

The bridge returns only the decoded assistant message content string.

For example, if the server returns a JSON object with:

```json
{
  "choices": [
    {
      "message": {
        "role": "assistant",
        "content": "PLAN: head left\nMOVE: left"
      }
    }
  ]
}
```

then the bridge hands back this plain string:

```text
PLAN: head left
MOVE: left
```

`SmartAgent` is then responsible for interpreting that text and extracting a move.

## 18. How It Is Wired Up in `simple_main.cpp`

The current native demo path in `source/simple_main.cpp` does this:

1. Calls `EnsureSmartAgentDefaultCallback()`
2. Builds a `MazeWorld`
3. Adds a `SmartAgent`
4. Sets the agent symbol to `P`
5. Places the agent at `(3, 1)`
6. Calls `Notify("Go to the left of the grid")`
7. Adds a `TrashInterface` agent at `(1, 1)` using symbol `@`
8. Calls `world.Run()`

That means the default native demo currently relies on the bridge being installed and a local llama-server being available.

## 19. What the Current Tests Actually Cover

The `SmartAgent` tests in `tests/Agents/SmartAgent.cpp` cover:

- returning a valid move
- prefetching the next move
- extracting a move from formatted callback text
- ignoring invalid callback text
- prompt shape and prompt simplicity
- consuming prefetched replies across turns
- exposing the shared system prompt

These tests use a stub callback that returns ready futures immediately.

So the tests cover `SmartAgent` behavior itself, but they do not fully integration-test the native bridge, `curl`, or the local llama-server endpoint.

## 20. Current Limitations and Gotchas

These are the main caveats in the current implementation:

- The bridge uses a fixed temp filename.
  Concurrent overlapping native requests could step on each other.

- The bridge uses a tiny hand-rolled JSON extractor.
  It works for the expected local response shape, but it is not a general JSON parser.

- The background thread is detached.
  There is no join step or cancellation path.

- `SmartAgent` only buffers individual moves, not a multi-step plan from the model.

- If a reply is invalid, `SmartAgent` simply fails to queue a move and may return action `0`.

- The prompt format is part of the behavior.
  Several tests intentionally lock down prompt wording and structure.

## 21. Short Version

If you only need the current mental model, it is this:

- `SmartAgent` turns the nearby grid state plus the current goal into a short text prompt.
- A callback returns text asynchronously.
- `SmartAgent` extracts one legal move from that text.
- That move is buffered and used as the next action.
- While one move is buffered, `SmartAgent` tries to prefetch one more.
- The native bridge is the default desktop callback that sends the prompt to a local llama-server over HTTP using `curl`, then returns the assistant text as a future.
