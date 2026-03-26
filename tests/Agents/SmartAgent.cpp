#include <deque>
#include <future>

#include "catch2/catch.hpp"

#include "Agents/SmartAgent.hpp"
#include "Worlds/MazeWorld.hpp"

namespace {

std::string stub_reply;
std::deque<std::string> stub_replies;
bool callback_called = false;
size_t callback_count = 0;
std::string last_prompt;

std::future<std::string> ReadyReplyFuture(std::string text)
{
  std::promise<std::string> promise;
  std::future<std::string> future = promise.get_future();
  promise.set_value(std::move(text));
  return future;
}

std::future<std::string> StubNpcRequest(const cse498::SmartAgent & /*agent*/,
                                        const std::string & prompt)
{
  callback_called = true;
  ++callback_count;
  last_prompt = prompt;

  std::string reply = stub_reply;
  if (!stub_replies.empty()) {
    reply = stub_replies.front();
    stub_replies.pop_front();
  }

  return ReadyReplyFuture(std::move(reply));
}

size_t StepAgent(cse498::MazeWorld & world, cse498::SmartAgent & agent)
{
  const size_t action = agent.SelectAction(world.GetGrid());
  const int result = world.DoAction(agent, action);
  agent.SetActionResult(result);
  return action;
}

struct CallbackReset {
  ~CallbackReset()
  {
    cse498::SmartAgent::SetNpcRequestCallback(nullptr);
  }
};

} // namespace

TEST_CASE("SmartAgent returns a valid move and prefetches the next one", "[SmartAgent]")
{
  CallbackReset reset;
  cse498::SmartAgent::SetNpcRequestCallback(&StubNpcRequest);

  stub_reply.clear();
  stub_replies = {"right", "down"};
  callback_called = false;
  callback_count = 0;
  last_prompt.clear();

  cse498::MazeWorld world;
  cse498::SmartAgent & agent = world.AddAgent<cse498::SmartAgent>("Smart");
  agent.SetLocation(cse498::WorldPosition{3,1});

  const size_t action = StepAgent(world, agent);

  REQUIRE(action == agent.GetActionID("right"));
  REQUIRE(agent.GetLocation().AsWorldPosition() == cse498::WorldPosition{4,1});
  REQUIRE(agent.GetLastNpcLine() == "right");
  REQUIRE(callback_called == true);
  REQUIRE(callback_count == 2);
  REQUIRE(agent.GetQueuedMoveCount() == 0);
  REQUIRE(agent.GetBufferedMoveCountIncludingInflight() == 1);
  // These prompt checks are intentionally picky; the prompt wording is part of
  // the behavior we are relying on when this talks to a model.
  REQUIRE(last_prompt.find("PRIMARY GOAL:\nNo explicit goal was provided. Make legal progress.")
          != std::string::npos);
  REQUIRE(last_prompt.find("LEGAL MOVES:\n- down -> (4, 2)\n- left -> (3, 1)\n- right -> (5, 1)\n")
          != std::string::npos);
  REQUIRE(last_prompt.find("GRID (# = wall, space = floor, * = you):\n")
          != std::string::npos);
  REQUIRE(last_prompt.find("Reply in exactly 2 lines:\nPLAN: <very short reasoning>\nMOVE: <one legal move word that best advances the PRIMARY GOAL>")
          != std::string::npos);
}

TEST_CASE("SmartAgent extracts a move from formatted callback text", "[SmartAgent]")
{
  CallbackReset reset;
  cse498::SmartAgent::SetNpcRequestCallback(&StubNpcRequest);

  stub_reply.clear();
  stub_replies = {"PLAN: left loses ground, so move toward the opening\nMOVE: right\n",
                  "PLAN: keep descending\nMOVE: down\n"};
  callback_called = false;
  callback_count = 0;
  last_prompt.clear();

  cse498::MazeWorld world;
  cse498::SmartAgent & agent = world.AddAgent<cse498::SmartAgent>("Smart");
  agent.SetLocation(cse498::WorldPosition{3,1});

  const size_t action = StepAgent(world, agent);

  REQUIRE(action == agent.GetActionID("right"));
  REQUIRE(agent.GetLastNpcLine()
          == "PLAN: left loses ground, so move toward the opening\nMOVE: right\n");
  REQUIRE(callback_called == true);
  REQUIRE(callback_count == 2);
  REQUIRE(last_prompt.find("LEGAL MOVES:\n- down -> (4, 2)\n- left -> (3, 1)\n- right -> (5, 1)\n")
          != std::string::npos);
}

TEST_CASE("SmartAgent ignores invalid callback text", "[SmartAgent]")
{
  CallbackReset reset;
  cse498::SmartAgent::SetNpcRequestCallback(&StubNpcRequest);

  stub_reply = "banana";
  stub_replies.clear();
  callback_called = false;
  callback_count = 0;
  last_prompt.clear();

  cse498::MazeWorld world;
  cse498::SmartAgent & agent = world.AddAgent<cse498::SmartAgent>("Smart");
  agent.SetLocation(cse498::WorldPosition{3,1});

  const size_t action = agent.SelectAction(world.GetGrid());

  REQUIRE(action == 0);
  REQUIRE(agent.GetQueuedMoveCount() == 0);
  REQUIRE(agent.GetBufferedMoveCountIncludingInflight() == 0);
  REQUIRE(agent.IsNpcRequestInFlight() == false);
  REQUIRE(callback_called == true);
  REQUIRE(callback_count == 1);
  REQUIRE(last_prompt.find("PRIMARY GOAL:\nNo explicit goal was provided. Make legal progress.")
          != std::string::npos);
  REQUIRE(last_prompt.find("LEGAL MOVES:\n- down -> (3, 2)\n- left -> (2, 1)\n- right -> (4, 1)\n")
          != std::string::npos);
  REQUIRE(last_prompt.find("MOVE: <one legal move word") != std::string::npos);
}

TEST_CASE("SmartAgent prompt includes motivation and stays simple", "[SmartAgent]")
{
  CallbackReset reset;
  cse498::SmartAgent::SetNpcRequestCallback(&StubNpcRequest);

  stub_reply.clear();
  stub_replies = {"down", "down"};
  callback_called = false;
  callback_count = 0;
  last_prompt.clear();

  cse498::MazeWorld world;
  cse498::SmartAgent & agent = world.AddAgent<cse498::SmartAgent>("Smart");
  agent.SetLocation(cse498::WorldPosition{3,1});
  agent.Notify("Reach the exit", "goal");

  const size_t action = StepAgent(world, agent);

  REQUIRE(action == agent.GetActionID("down"));
  REQUIRE(callback_called == true);
  REQUIRE(callback_count == 2);
  REQUIRE(last_prompt.find("PRIMARY GOAL:\nReach the exit") != std::string::npos);
  REQUIRE(last_prompt.find("LEGAL MOVES:\n- up -> (3, 1)\n- down -> (3, 3)\n- left -> (2, 2)\n- right -> (4, 2)\n")
          != std::string::npos);
  REQUIRE(last_prompt.find("STATE:") == std::string::npos);
  REQUIRE(last_prompt.find("PLAN: <very short reasoning>") != std::string::npos);
}

TEST_CASE("SmartAgent consumes prefetched replies on following turns", "[SmartAgent]")
{
  CallbackReset reset;
  cse498::SmartAgent::SetNpcRequestCallback(&StubNpcRequest);

  stub_reply.clear();
  stub_replies = {"right", "down", "down"};
  callback_called = false;
  callback_count = 0;
  last_prompt.clear();

  cse498::MazeWorld world;
  cse498::SmartAgent & agent = world.AddAgent<cse498::SmartAgent>("Smart");
  agent.SetLocation(cse498::WorldPosition{3,1});

  const size_t first_action = StepAgent(world, agent);
  const size_t second_action = StepAgent(world, agent);

  REQUIRE(first_action == agent.GetActionID("right"));
  REQUIRE(second_action == agent.GetActionID("down"));
  REQUIRE(agent.GetLocation().AsWorldPosition() == cse498::WorldPosition{4,2});
  REQUIRE(callback_count == 3);
  REQUIRE(agent.GetBufferedMoveCountIncludingInflight() == 1);
  REQUIRE(last_prompt.find("LEGAL MOVES:\n- up -> (4, 1)\n- down -> (4, 3)\n- left -> (3, 2)\n- right -> (5, 2)\n")
          != std::string::npos);
}

TEST_CASE("SmartAgent exposes a shared system prompt", "[SmartAgent]")
{
  const std::string_view system_prompt = cse498::SmartAgent::GetSystemPrompt();

  REQUIRE(system_prompt.find("PRIMARY GOAL matters most") != std::string_view::npos);
  REQUIRE(system_prompt.find("best next legal move") != std::string_view::npos);
  REQUIRE(system_prompt.find("exactly two lines") != std::string_view::npos);
  REQUIRE(system_prompt.find("MOVE: <one legal move word>") != std::string_view::npos);
}
