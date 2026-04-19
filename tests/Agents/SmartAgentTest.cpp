#include <deque>
#include <future>
#include <string>

#include "catch2/catch.hpp"

#include "Agents/SmartAgent.hpp"
#include "Worlds/MazeWorld.hpp"

namespace {

std::string stub_reply;
std::deque<std::string> stub_replies;
bool callback_called = false;
size_t callback_count = 0;
std::string last_prompt;

void ResetStubState()
{
  stub_reply.clear();
  stub_replies.clear();
  callback_called = false;
  callback_count = 0;
  last_prompt.clear();
}

std::future<std::string> ReadyReplyFuture(std::string text)
{
  std::promise<std::string> promise;
  std::future<std::string> future = promise.get_future();
  promise.set_value(std::move(text));
  return future;
}


std::future<std::string> StubNpcRequest(const cse498::SmartAgent &,
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

struct ScopedStubCallback {
  ScopedStubCallback()
  {
    ResetStubState();
    cse498::SmartAgent::SetNpcRequestCallback(&StubNpcRequest);
  }

  ~ScopedStubCallback()
  {
    cse498::SmartAgent::SetNpcRequestCallback(nullptr);
  }
};

}

TEST_CASE("SmartAgent uses a stubbed callback for plain move replies", "[SmartAgent]")
{
  ScopedStubCallback stubbed_callback;
  stub_replies = {"right"};

  cse498::MazeWorld world;
  cse498::SmartAgent & agent = world.AddAgent<cse498::SmartAgent>("Smart");
  agent.SetLocation(cse498::WorldPosition{3,1});

  const size_t action = StepAgent(world, agent);

  REQUIRE(callback_called == true);
  REQUIRE(callback_count >= 1);
  REQUIRE(action == agent.GetActionID("right"));
  REQUIRE(agent.GetLocation().AsWorldPosition() == cse498::WorldPosition{4,1});
  REQUIRE(agent.GetLastNpcLine() == "right");
  REQUIRE(last_prompt.find("PRIMARY GOAL:") != std::string::npos);
  REQUIRE(last_prompt.find("LEGAL MOVES:") != std::string::npos);
  REQUIRE(last_prompt.find("MOVE: <one legal move word") != std::string::npos);
}

TEST_CASE("SmartAgent can parse formatted stub replies", "[SmartAgent]")
{
  ScopedStubCallback stubbed_callback;
  const std::string formatted_reply = "PLAN: head for the opening\nMOVE: down\n";
  stub_replies = {formatted_reply};

  cse498::MazeWorld world;
  cse498::SmartAgent & agent = world.AddAgent<cse498::SmartAgent>("Smart");
  agent.SetLocation(cse498::WorldPosition{3,1});

  const size_t action = StepAgent(world, agent);

  REQUIRE(callback_called == true);
  REQUIRE(action == agent.GetActionID("down"));
  REQUIRE(agent.GetLastNpcLine() == formatted_reply);
}

TEST_CASE("SmartAgent ignores invalid stub replies", "[SmartAgent]")
{
  ScopedStubCallback stubbed_callback;
  stub_reply = "banana";

  cse498::MazeWorld world;
  cse498::SmartAgent & agent = world.AddAgent<cse498::SmartAgent>("Smart");
  agent.SetLocation(cse498::WorldPosition{3,1});

  const size_t action = agent.SelectAction(world.GetGrid());

  REQUIRE(callback_called == true);
  REQUIRE(action == 0);
  REQUIRE(agent.GetQueuedMoveCount() == 0);
  REQUIRE(agent.GetBufferedMoveCountIncludingInflight() == 0);
  REQUIRE(agent.IsNpcRequestInFlight() == false);
  REQUIRE(last_prompt.find("PRIMARY GOAL:") != std::string::npos);
  REQUIRE(last_prompt.find("LEGAL MOVES:") != std::string::npos);
  REQUIRE(last_prompt.find("MOVE: <one legal move word") != std::string::npos);
}

TEST_CASE("SmartAgent includes notified goals in stub prompts", "[SmartAgent]")
{
  ScopedStubCallback stubbed_callback;
  stub_replies = {"down"};

  cse498::MazeWorld world;
  cse498::SmartAgent & agent = world.AddAgent<cse498::SmartAgent>("Smart");
  agent.SetLocation(cse498::WorldPosition{3,1});
  agent.Notify("Reach the exit", "goal");

  const size_t action = StepAgent(world, agent);

  REQUIRE(action == agent.GetActionID("down"));
  REQUIRE(last_prompt.find("PRIMARY GOAL:\nReach the exit") != std::string::npos);
  REQUIRE(last_prompt.find("No explicit goal was provided") == std::string::npos);
}

TEST_CASE("SmartAgent keeps another stubbed move ready after the first turn", "[SmartAgent]")
{
  ScopedStubCallback stubbed_callback;
  stub_replies = {"right", "down", "down"};

  cse498::MazeWorld world;
  cse498::SmartAgent & agent = world.AddAgent<cse498::SmartAgent>("Smart");
  agent.SetLocation(cse498::WorldPosition{3,1});

  const size_t first_action = StepAgent(world, agent);
  REQUIRE(first_action == agent.GetActionID("right"));
  REQUIRE(agent.GetBufferedMoveCountIncludingInflight() >= 1);

  const size_t second_action = StepAgent(world, agent);

  REQUIRE(second_action == agent.GetActionID("down"));
  REQUIRE(agent.GetLocation().AsWorldPosition() == cse498::WorldPosition{4,2});
}

TEST_CASE("SmartAgent exposes a shared system prompt", "[SmartAgent]")
{
  const std::string_view system_prompt = cse498::SmartAgent::GetSystemPrompt();

  REQUIRE(system_prompt.find("PRIMARY GOAL matters most") != std::string_view::npos);
  REQUIRE(system_prompt.find("best next legal move") != std::string_view::npos);
  REQUIRE(system_prompt.find("MOVE: <one legal move word>") != std::string_view::npos);
}
