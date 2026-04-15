/** 
 * @file EventQueueTest.cpp
 * @author Truong Phan
 *
 * Tests for the EventQueue class.
 * 
 * Citation: ChatGPT and Claude AI were used to assist in the design and 
 * implementation of the tests. The generated code was then modified and improved
 * by the author to meet the requirements of the project.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/EventQueue.hpp"

using cse498::EventQueue;
using cse498::Event;

TEST_CASE("Test Event Constructor and Getters", "[Event]") 
{
    Event<std::string> e1;

    REQUIRE(e1.GetData() == "");
    REQUIRE(e1.GetPriority() == 0);
    REQUIRE(e1.GetTiebreaker() == 0);

    Event<std::string> e2("Sample Event", 5);

    REQUIRE(e2.GetData() == "Sample Event");
    REQUIRE(e2.GetPriority() == 5);
    REQUIRE(e2.GetTiebreaker() == 0);

    Event<std::string> e3("Another Event", 42);

    REQUIRE(e3.GetData() == "Another Event");
    REQUIRE(e3.GetPriority() == 42);
    REQUIRE(e3.GetTiebreaker() == 0);
}

TEST_CASE("Test EventQueue Default, Copy, and Assignment Constructor", "[EventQueue]") 
{
    EventQueue<std::string> eq;
    REQUIRE(eq.Empty());
    REQUIRE(eq.Size() == 0);

    eq.Push(Event<std::string>("Event 1", 1));

    EventQueue<std::string> copy(eq);
    REQUIRE(copy.Size() == 1);
    REQUIRE(copy.Top()->GetData() == "Event 1");

    EventQueue<std::string> assigned;
    assigned = eq;
    REQUIRE(assigned.Size() == 1);
    REQUIRE(assigned.Top()->GetData() == "Event 1");
}

TEST_CASE("Test Copy and Assignment Correctness", "[EventQueue]") 
{
    EventQueue<std::string> eq;

    eq.Push(Event<std::string>("Event 1", 1));
    eq.Push(Event<std::string>("Event 2", 2));
    eq.Push(Event<std::string>("Event 3", 3));

    EventQueue<std::string> copy(eq);
    REQUIRE(copy.Size() == eq.Size());
    REQUIRE(copy.Top()->GetData() == eq.Top()->GetData());
    REQUIRE(copy.Pop());
    REQUIRE(copy.Size() == eq.Size() - 1);
    REQUIRE(eq.Size() == 3);

    EventQueue<std::string> assigned;
    assigned = eq;
    REQUIRE(assigned.Size() == eq.Size());
    REQUIRE(assigned.Top()->GetData() == eq.Top()->GetData());
    REQUIRE(assigned.Pop());
    REQUIRE(assigned.Size() == eq.Size() - 1);
    REQUIRE(eq.Size() == 3);
}

TEST_CASE("Test Push and Top", "[EventQueue]") 
{
    EventQueue<std::string> eq;

    eq.Push(Event<std::string>("Event 1", 2));
    REQUIRE(eq.Size() == 1);
    REQUIRE(eq.Top()->GetData() == "Event 1");
    REQUIRE(eq.Top()->GetPriority() == 2);
    REQUIRE(eq.Top()->GetTiebreaker() == 0);

    eq.Push(Event<std::string>("Event 2", 1));
    REQUIRE(eq.Size() == 2);
    REQUIRE(eq.Top()->GetData() == "Event 2");
    REQUIRE(eq.Top()->GetPriority() == 1);
    REQUIRE(eq.Top()->GetTiebreaker() == 1);

    eq.Push("Event 3", 0);
    REQUIRE(eq.Size() == 3);
    REQUIRE(eq.Top()->GetData() == "Event 3");
    REQUIRE(eq.Top()->GetPriority() == 0);
    REQUIRE(eq.Top()->GetTiebreaker() == 2);
}

TEST_CASE("Test Top and Pop on Empty Queue", "[EventQueue]")
{
    EventQueue<std::string> eq;
    REQUIRE(eq.Empty());

    REQUIRE(eq.Top() == nullptr);
    REQUIRE_FALSE(eq.Pop());

    REQUIRE(eq.Empty());
    REQUIRE(eq.Size() == 0);

    eq.Push(Event<std::string>("Event 1", 1));
    eq.Clear();
    REQUIRE(eq.Top() == nullptr);
    REQUIRE_FALSE(eq.Pop());
}

TEST_CASE("Test Pop", "[EventQueue]") 
{
    EventQueue<std::string> eq;

    eq.Push(Event<std::string>("Event 1", 2));
    eq.Push(Event<std::string>("Event 2", 1));
    eq.Push(Event<std::string>("Event 3", 3));

    REQUIRE(eq.Pop());
    REQUIRE(eq.Size() == 2);
    REQUIRE(eq.Top()->GetData() == "Event 1");
    REQUIRE(eq.Top()->GetPriority() == 2);

    REQUIRE(eq.Pop());
    REQUIRE(eq.Size() == 1);
    REQUIRE(eq.Top()->GetData() == "Event 3");
    REQUIRE(eq.Top()->GetPriority() == 3);

    REQUIRE(eq.Pop());
    REQUIRE(eq.Empty());
}

TEST_CASE("Test Size, Empty, and Clear", "[EventQueue]") 
{
    EventQueue<std::string> eq;

    REQUIRE(eq.Empty());
    REQUIRE(eq.Size() == 0);

    eq.Push(Event<std::string>("Event 1", 1));
    eq.Push(Event<std::string>("Event 2", 2));

    REQUIRE(!eq.Empty());
    REQUIRE(eq.Size() == 2);

    eq.Clear();
    REQUIRE(eq.Empty());
    REQUIRE(eq.Size() == 0);

    eq.Push(Event<std::string>("Event 3", 3));
    REQUIRE(eq.Size() == 1);
    REQUIRE(eq.Top()->GetData() == "Event 3");
}

TEST_CASE("Test Equal-Priority Ordering", "[EventQueue]") 
{
    EventQueue<std::string> eq;

    eq.Push(Event<std::string>("Event 1", 5));
    eq.Push(Event<std::string>("Event 2", 5));
    eq.Push(Event<std::string>("Event 3", 5));

    REQUIRE(eq.Top()->GetData() == "Event 1");
    REQUIRE(eq.Pop());
    REQUIRE(eq.Top()->GetData() == "Event 2");
    REQUIRE(eq.Pop());
    REQUIRE(eq.Top()->GetData() == "Event 3");
    REQUIRE(eq.Pop());
    REQUIRE(eq.Empty());
}

TEST_CASE("Test Different Priorities and Interleaved Ordering", "[EventQueue]") 
{
    EventQueue<std::string> eq;

    eq.Push(Event<std::string>("A", 2));
    eq.Push(Event<std::string>("B", 1));
    eq.Push(Event<std::string>("C", 2));
    eq.Push(Event<std::string>("D", 1));

    REQUIRE(eq.Top()->GetData() == "B");
    REQUIRE(eq.Top()->GetPriority() == 1);
    REQUIRE(eq.Top()->GetTiebreaker() == 1);
    REQUIRE(eq.Pop());

    REQUIRE(eq.Top()->GetData() == "D");
    REQUIRE(eq.Top()->GetPriority() == 1);
    REQUIRE(eq.Top()->GetTiebreaker() == 3);
    REQUIRE(eq.Pop());

    REQUIRE(eq.Top()->GetData() == "A");
    REQUIRE(eq.Top()->GetPriority() == 2);
    REQUIRE(eq.Top()->GetTiebreaker() == 0);
    REQUIRE(eq.Pop());

    REQUIRE(eq.Top()->GetData() == "C");
    REQUIRE(eq.Top()->GetPriority() == 2);
    REQUIRE(eq.Top()->GetTiebreaker() == 2);
    REQUIRE(eq.Pop());

    REQUIRE(eq.Empty());
}

TEST_CASE("Test Large Scale Push, Pop, and Ordering", "[EventQueue]")
{
    EventQueue<std::string> eq;

    const int NUM_EVENTS = 1000;
    const int NUM_PRIORITIES = 10;

    for (int i = 0; i < NUM_EVENTS; ++i)
    {
        int priority = i % NUM_PRIORITIES;
        eq.Push(Event<std::string>("Event " + std::to_string(i), priority));
    }

    REQUIRE(eq.Size() == NUM_EVENTS);

    int lastPriority = -1;
    std::size_t lastTiebreaker = 0;
    int count = 0;

    while (!eq.Empty())
    {
        const auto* top = eq.Top();
        REQUIRE(top != nullptr);

        int currentPriority = top->GetPriority();
        std::size_t currentTiebreaker = top->GetTiebreaker();

        REQUIRE(currentPriority >= lastPriority);

        if (currentPriority == lastPriority)
            REQUIRE(currentTiebreaker > lastTiebreaker);

        lastPriority = currentPriority;
        lastTiebreaker = currentTiebreaker;
        ++count;

        REQUIRE(eq.Pop());
    }

    REQUIRE(count == NUM_EVENTS);
    REQUIRE(eq.Empty());
    REQUIRE(eq.Size() == 0);

    REQUIRE(eq.Top() == nullptr);
    REQUIRE_FALSE(eq.Pop());
}

TEST_CASE("Test Templated Event and EventQueue", "[EventQueue]")
{
    EventQueue<int> intQueue;

    intQueue.Push(Event<int>(42, 2));
    intQueue.Push(Event<int>(7, 1));
    intQueue.Push(Event<int>(99, 3));

    REQUIRE(intQueue.Top()->GetData() == 7);
    REQUIRE(intQueue.Pop());
    REQUIRE(intQueue.Top()->GetData() == 42);
    REQUIRE(intQueue.Pop());
    REQUIRE(intQueue.Top()->GetData() == 99);
    REQUIRE(intQueue.Pop());
    REQUIRE(intQueue.Empty());

    EventQueue<double> doubleQueue;

    doubleQueue.Push(Event<double>(3.14, 2));
    doubleQueue.Push(Event<double>(2.71, 1));
    doubleQueue.Push(Event<double>(1.41, 3));

    REQUIRE(doubleQueue.Top()->GetData() == 2.71);
    REQUIRE(doubleQueue.Pop());
    REQUIRE(doubleQueue.Top()->GetData() == 3.14);
    REQUIRE(doubleQueue.Pop());
    REQUIRE(doubleQueue.Top()->GetData() == 1.41);
    REQUIRE(doubleQueue.Pop());
    REQUIRE(doubleQueue.Empty());

    struct Point { int x, y; };
    EventQueue<Point> pointQueue;

    pointQueue.Push(Event<Point>({1, 2}, 3));
    pointQueue.Push(Event<Point>({3, 4}, 1));
    pointQueue.Push(Event<Point>({5, 6}, 2));

    REQUIRE(pointQueue.Top()->GetData().x == 3);
    REQUIRE(pointQueue.Top()->GetData().y == 4);
    REQUIRE(pointQueue.Pop());
    REQUIRE(pointQueue.Top()->GetData().x == 5);
    REQUIRE(pointQueue.Pop());
    REQUIRE(pointQueue.Top()->GetData().x == 1);
    REQUIRE(pointQueue.Pop());
    REQUIRE(pointQueue.Empty());
}

TEST_CASE("Test PopTop", "[EventQueue]")
{
    EventQueue<std::string> eq;

    auto empty = eq.PopTop();
    REQUIRE(empty.has_value() == false);

    eq.Push(Event<std::string>("Event 1", 2));
    eq.Push(Event<std::string>("Event 2", 1));
    eq.Push(Event<std::string>("Event 3", 3));

    REQUIRE(eq.Size() == 3);

    auto e1 = eq.PopTop();
    REQUIRE(e1.has_value());
    REQUIRE(e1->GetData() == "Event 2");

    auto e2 = eq.PopTop();
    REQUIRE(e2.has_value());
    REQUIRE(e2->GetData() == "Event 1");

    auto e3 = eq.PopTop();
    REQUIRE(e3.has_value());
    REQUIRE(e3->GetData() == "Event 3");

    auto e4 = eq.PopTop();
    REQUIRE(e4.has_value() == false);
}