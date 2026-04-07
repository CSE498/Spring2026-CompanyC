#pragma once

#include <array>
#include <cctype>
#include <chrono>
#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include "../core/AgentBase.hpp"

namespace cse498 {

  class SmartAgent : public AgentBase {
  protected:
    // SmartAgent may discard a stale request during ClearPlan() / ReceiveNpcLine()
    using NpcRequestCallback = std::future<std::string> (*)(const SmartAgent &, const std::string &);
    static constexpr size_t max_buffered_actions = 2;
    static constexpr size_t max_requested_moves = 4;
    static inline NpcRequestCallback npc_request_callback = nullptr;

    std::array<size_t, max_buffered_actions> buffered_actions{}; // Ready moves already chosen.
    size_t buffered_action_count = 0;

    std::array<std::string_view, max_requested_moves> requested_moves{}; // Legal moves in the current prompt.
    std::array<WorldPosition, max_requested_moves> requested_move_positions{}; // Resulting cells for those legal moves.
    size_t requested_move_count = 0;

    bool npc_request_in_flight = false; // True while the async single-move request is still running.
    bool has_npc_response_future = false;
    std::future<std::string> npc_response_future;

    WorldGrid pending_request_grid{};
    WorldPosition pending_request_start_position{};

    std::string last_notification_message;
    std::string last_notification_type = "none";
    std::string last_npc_line;

    [[nodiscard]] bool IsWalkableCell(const WorldGrid & grid, WorldPosition pos) const noexcept
    {
      return grid.IsValid(pos) && grid.GetCellTypeSymbol(grid[pos]) != '#';
    }

    void AddRequestedMove(const WorldGrid & grid,
                          std::string_view move_name,
                          WorldPosition pos) noexcept
    {
      if (IsWalkableCell(grid, pos)) {
        requested_moves[requested_move_count] = move_name;
        requested_move_positions[requested_move_count] = pos;
        ++requested_move_count;
      }
    }

    [[nodiscard]] bool IsKnownAction(size_t action_id) const noexcept
    {
      if (action_id == 0) return true;

      for (const auto & action_entry : action_map) {
        if (action_entry.second == action_id) return true;
      }

      return false;
    }

    [[nodiscard]] static std::string_view CanonicalMoveToken(std::string_view token) noexcept
    {
      if (token == "up") return "up";
      if (token == "down") return "down";
      if (token == "left") return "left";
      if (token == "right") return "right";
      return {};
    }

    [[nodiscard]] static std::string NormalizeReply(std::string text)
    {
      for (char & ch : text) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
      }

      // If the model gives us a clean "MOVE:" line, trust that first.
      // If it rambles, we can still salvage the last direction token it mentioned.
      auto ExtractMoveFromSlice = [&](size_t start, size_t end) -> std::string {
        size_t pos = start;
        while (pos < end) {
          while (pos < end && !std::isalpha(static_cast<unsigned char>(text[pos]))) {
            ++pos;
          }

          size_t token_end = pos;
          while (token_end < end && std::isalpha(static_cast<unsigned char>(text[token_end]))) {
            ++token_end;
          }

          if (token_end > pos) {
            const std::string_view token{text.data() + pos, token_end - pos};
            const std::string_view move = CanonicalMoveToken(token);
            if (!move.empty()) return std::string(move);
          }

          pos = token_end;
        }

        return {};
      };

      size_t line_start = 0;
      std::string labeled_move;
      while (line_start < text.size()) {
        size_t line_end = text.find('\n', line_start);
        if (line_end == std::string::npos) line_end = text.size();

        const size_t colon_pos = text.find(':', line_start);
        if (colon_pos != std::string::npos && colon_pos < line_end) {
          const std::string_view label{text.data() + line_start, colon_pos - line_start};
          if (label == "move" || label == "choice" || label == "final") {
            const std::string move = ExtractMoveFromSlice(colon_pos + 1, line_end);
            if (!move.empty()) labeled_move = move;
          }
        }

        if (line_end == text.size()) break;
        line_start = line_end + 1;
      }

      if (!labeled_move.empty()) return labeled_move;

      std::string last_move;
      size_t pos = 0;
      while (pos < text.size()) {
        while (pos < text.size() && !std::isalpha(static_cast<unsigned char>(text[pos]))) {
          ++pos;
        }

        size_t end = pos;
        while (end < text.size() && std::isalpha(static_cast<unsigned char>(text[end]))) {
          ++end;
        }

        if (end > pos) {
          const std::string_view token{text.data() + pos, end - pos};
          const std::string_view move = CanonicalMoveToken(token);
          if (!move.empty()) last_move = std::string(move);
        }

        pos = end;
      }

      return last_move;
    }

    [[nodiscard]] std::string_view GetMoveName(size_t action_id) const noexcept
    {
      if (action_id == GetActionID("up")) return "up";
      if (action_id == GetActionID("down")) return "down";
      if (action_id == GetActionID("left")) return "left";
      if (action_id == GetActionID("right")) return "right";
      return {};
    }

    [[nodiscard]] size_t GetMoveActionID(std::string_view move) const noexcept
    {
      if (move == "up") return GetActionID("up");
      if (move == "down") return GetActionID("down");
      if (move == "left") return GetActionID("left");
      if (move == "right") return GetActionID("right");
      return 0;
    }

    [[nodiscard]] static WorldPosition GetMovedPosition(WorldPosition pos,
                                                        std::string_view move) noexcept
    {
      if (move == "up") return pos.Up();
      if (move == "down") return pos.Down();
      if (move == "left") return pos.Left();
      if (move == "right") return pos.Right();
      return pos;
    }

    [[nodiscard]] bool TryApplyMove(const WorldGrid & grid,
                                    WorldPosition & pos,
                                    std::string_view move) const noexcept
    {
      const WorldPosition next_pos = GetMovedPosition(pos, move);
      if (!IsWalkableCell(grid, next_pos)) return false;
      pos = next_pos;
      return true;
    }

    [[nodiscard]] bool QueueBufferedAction(size_t action_id) noexcept
    {
      if (action_id == 0 || !IsKnownAction(action_id)) return false;
      if (buffered_action_count >= buffered_actions.size()) return false;

      buffered_actions[buffered_action_count++] = action_id;
      return true;
    }

    [[nodiscard]] WorldPosition GetPlanningStartPosition(const WorldGrid & grid) const noexcept
    {
      WorldPosition pos = GetLocation().AsWorldPosition();

      for (size_t i = 0; i < buffered_action_count; ++i) {
        const std::string_view move = GetMoveName(buffered_actions[i]);
        if (move.empty()) break;
        if (!TryApplyMove(grid, pos, move)) break;
      }

      return pos;
    }

    [[nodiscard]] std::string BuildAvailableMovesPrompt() const
    {
      std::ostringstream out;

      for (size_t i = 0; i < requested_move_count; ++i) {
        out << "- " << requested_moves[i]
            << " -> ("
            << requested_move_positions[i].CellX()
            << ", "
            << requested_move_positions[i].CellY()
            << ")\n";
      }

      return out.str();
    }

    [[nodiscard]] std::string BuildGridPrompt(const WorldGrid & grid,
                                              WorldPosition highlighted_pos) const
    {
      std::ostringstream out;

      out << '+' << std::string(grid.GetWidth(), '-') << "+\n";
      for (size_t y = 0; y < grid.GetHeight(); ++y) {
        out << '|';
        for (size_t x = 0; x < grid.GetWidth(); ++x) {
          WorldPosition pos{x, y};
          char symbol = grid.GetSymbol(pos);
          if (pos == highlighted_pos) symbol = GetSymbol();
          out << symbol;
        }
        out << "|\n";
      }
      out << '+' << std::string(grid.GetWidth(), '-') << "+\n";

      return out.str();
    }

    [[nodiscard]] std::string BuildUserPrompt(const WorldGrid & grid,
                                              WorldPosition planning_start_position) const
    {
      std::ostringstream out;

      out << "PRIMARY GOAL:\n";
      if (!last_notification_message.empty()) out << last_notification_message;
      else out << "No explicit goal was provided. Make legal progress.";
      out << "\n\n";

      out << "LEGAL MOVES:\n";
      out << BuildAvailableMovesPrompt();
      out << "\n";

      out << "GRID (# = wall, space = floor, " << GetSymbol() << " = you):\n";
      out << BuildGridPrompt(grid, planning_start_position);
      out << "\n";

      out << "Reply in exactly 2 lines:\n";
      out << "PLAN: <very short reasoning>\n";
      out << "MOVE: <one legal move word that best advances the PRIMARY GOAL>";

      return out.str();
    }

    void MaybeStartNpcRequest(const WorldGrid & grid)
    {
      if (npc_request_in_flight) return;
      if (buffered_action_count >= buffered_actions.size()) return;

      requested_move_count = 0;
      const WorldPosition planning_start_position = GetPlanningStartPosition(grid);
      pending_request_start_position = planning_start_position;
      pending_request_grid = grid;

      AddRequestedMove(grid, "up", planning_start_position.Up());
      AddRequestedMove(grid, "down", planning_start_position.Down());
      AddRequestedMove(grid, "left", planning_start_position.Left());
      AddRequestedMove(grid, "right", planning_start_position.Right());

      if (requested_move_count == 0) {
        if (!HasPlan()) last_npc_line = "No valid moves.";
        return;
      }

      if (npc_request_callback == nullptr) {
        last_npc_line = "No NPC requester installed.";
        return;
      }

      npc_request_in_flight = true;
      npc_response_future = npc_request_callback(*this,
                                                 BuildUserPrompt(grid,
                                                                 planning_start_position));
      has_npc_response_future = npc_response_future.valid();
      if (!has_npc_response_future) {
        npc_request_in_flight = false;
        last_npc_line = "NPC requester did not return a response future.";
      }
    }

    void HandleNpcReply(const std::string & text)
    {
      last_npc_line = text;
      npc_request_in_flight = false;

      if (requested_move_count == 0) {
        std::cout << "[SmartAgent] parsed reply move: <ignored with no pending request>\n";
        return;
      }

      const std::string reply = NormalizeReply(text);
      std::cout << "[SmartAgent] parsed reply move: "
                << (reply.empty() ? "<empty>" : reply)
                << '\n';

      WorldPosition sim_pos = pending_request_start_position;
      if (!reply.empty()
          && TryApplyMove(pending_request_grid, sim_pos, reply)
          && QueueBufferedAction(GetMoveActionID(reply))) {
        std::cout << "[SmartAgent] accepted buffered move: " << reply << '\n';
      }
      else {
        std::cout << "[SmartAgent] accepted buffered move: <none>\n";
      }

      std::cout << "[SmartAgent] buffered moves ready: " << buffered_action_count << '\n';
      std::cout << "[SmartAgent] buffered moves including in-flight request: "
                << GetBufferedMoveCountIncludingInflight()
                << '/'
                << max_buffered_actions
                << '\n';

      requested_move_count = 0;
    }

    [[nodiscard]] bool TryConsumeReadyNpcReply()
    {
      if (!npc_request_in_flight || !has_npc_response_future) return false;

      using namespace std::chrono_literals;
      if (npc_response_future.wait_for(0ms) != std::future_status::ready) return false;

      const std::string reply = npc_response_future.get();
      has_npc_response_future = false;
      HandleNpcReply(reply);
      return true;
    }

    void PollNpcReply()
    {
      [[maybe_unused]] const bool consumed_reply = TryConsumeReadyNpcReply();
    }

    void DiscardNpcResponseFuture() noexcept
    {
      has_npc_response_future = false;
      npc_response_future = {};
    }

    [[nodiscard]] size_t PopNextAction() noexcept
    {
      const size_t next_action = buffered_actions[0];

      for (size_t i = 1; i < buffered_action_count; ++i) {
        buffered_actions[i - 1] = buffered_actions[i];
      }

      --buffered_action_count;
      return next_action;
    }

  public:
    SmartAgent(size_t id, const std::string & name, const WorldBase & world)
      : AgentBase(id, name, world) { }
    ~SmartAgent() = default;

    [[nodiscard]] static std::string_view GetSystemPrompt() noexcept
    {
      return
        "The PRIMARY GOAL matters most. "
        "Use the LEGAL MOVES and the GRID to choose the best next legal move. "
        "Think briefly, then reply with exactly two lines: "
        "PLAN: <very short reasoning> and MOVE: <one legal move word>.";
    }

    [[nodiscard]] bool Initialize() override
    {
      return HasAction("up") && HasAction("down") && HasAction("left") && HasAction("right");
    }

    [[nodiscard]] bool HasPlan() const noexcept { return buffered_action_count != 0; }
    [[nodiscard]] size_t GetPlannedActionCount() const noexcept { return buffered_action_count; }
    [[nodiscard]] size_t GetQueuedMoveCount() const noexcept { return buffered_action_count; }
    [[nodiscard]] size_t GetBufferedMoveCountIncludingInflight() const noexcept
    {
      return buffered_action_count + static_cast<size_t>(npc_request_in_flight);
    }

    [[nodiscard]] const std::string & GetLastNotificationMessage() const noexcept
    {
      return last_notification_message;
    }

    [[nodiscard]] const std::string & GetLastNotificationType() const noexcept
    {
      return last_notification_type;
    }

    [[nodiscard]] const std::string & GetLastNpcLine() const noexcept
    {
      return last_npc_line;
    }

    [[nodiscard]] bool IsNpcRequestInFlight() const noexcept
    {
      return npc_request_in_flight;
    }

    void ClearPlan() noexcept
    {
      buffered_action_count = 0;
      requested_move_count = 0;
      npc_request_in_flight = false;
      DiscardNpcResponseFuture();
    }

    // Injects a completed NPC reply. Use the callback path for agent-managed async
    // requests; if one is in flight, its stored future is discarded as stale.
    void ReceiveNpcLine(const std::string & text)
    {
      DiscardNpcResponseFuture();
      HandleNpcReply(text);
    }

    // Installs the async NPC request hook. Resolve to "" on failure.
    static void SetNpcRequestCallback(NpcRequestCallback callback)
    {
      npc_request_callback = callback;
    }

    [[nodiscard]] size_t SelectAction(const WorldGrid & grid) override
    {
      if (action_result == 0) ClearPlan();

      // Use any finished reply first, then try to
      // queue up the next one while there's still buffer space.
      PollNpcReply();
      MaybeStartNpcRequest(grid);
      PollNpcReply();

      if (HasPlan()) {
        // If we already have a move ready, prefetch the following move asynchronously.
        MaybeStartNpcRequest(grid);
        return PopNextAction();
      }

      return 0;
    }

    void Notify(const std::string & message, const std::string & msg_type="none") override
    {
      last_notification_message = message;
      last_notification_type = msg_type;
      last_npc_line.clear();
      ClearPlan();
    }
  };

}
