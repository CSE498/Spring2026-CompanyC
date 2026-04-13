/** 
 * @file EventQueue.cpp
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
    // Test default constructor  
    Event<std::string> e1;

    // Test getters for default event
    REQUIRE(e1.GetData() == "");
    REQUIRE(e1.GetPriority() == 0);
    REQUIRE(e1.GetTiebreaker() == 0);

    // Test parameterized constructor
    Event<std::string> e2("Sample Event", 5);

    // Test getters for parameterized event
    REQUIRE(e2.GetData() == "Sample Event");
    REQUIRE(e2.GetPriority() == 5);
    REQUIRE(e2.GetTiebreaker() == 0);

    // Test move constructor
    Event<std::string> e3("Another Event", 42);

    // Test getters for moved event
    REQUIRE(e3.GetData() == "Another Event");
    REQUIRE(e3.GetPriority() == 42);
    REQUIRE(e3.GetTiebreaker() == 0);
}

TEST_CASE("Test EventQueue Default, Copy, and Assignment Constructor", "[EventQueue]") 
{
    // Test default constructor
    EventQueue<std::string> eq;
    REQUIRE(eq.Empty());
    REQUIRE(eq.Size() == 0);

    REQUIRE(eq.Push(Event<std::string>("Event 1", 1)));

    // Test copy constructor
    EventQueue<std::string> copy(eq);
    REQUIRE(copy.Size() == 1);
    REQUIRE(copy.Top()->GetData() == "Event 1");

    // Test assignment operator
    EventQueue<std::string> assigned;
    assigned = eq;
    REQUIRE(assigned.Size() == 1);
    REQUIRE(assigned.Top()->GetData() == "Event 1");
}

TEST_CASE("Test Copy and Assignment Correctness", "[EventQueue]") 
{
    // Test that copy and assignment create independent copies of the EventQueue
    EventQueue<std::string> eq;
    REQUIRE(eq.Push(Event<std::string>("Event 1", 1)));
    REQUIRE(eq.Push(Event<std::string>("Event 2", 2)));
    REQUIRE(eq.Push(Event<std::string>("Event 3", 3)));

    // Copy constructor
    EventQueue<std::string> copy(eq);
    REQUIRE(copy.Size() == eq.Size());
    REQUIRE(copy.Top()->GetData() == eq.Top()->GetData());
    REQUIRE(copy.Pop());
    REQUIRE(copy.Size() == eq.Size() - 1);
    REQUIRE(eq.Size() == 3);

    // Assignment operator
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
    // Test that events are pushed correctly and that the top event is returned correctly
    EventQueue<std::string> eq;

    REQUIRE(eq.Push(Event<std::string>("Event 1", 2)));
    REQUIRE(eq.Size() == 1);
    REQUIRE(eq.Top()->GetData() == "Event 1");
    REQUIRE(eq.Top()->GetPriority() == 2);
    REQUIRE(eq.Top()->GetTiebreaker() == 0);

    REQUIRE(eq.Push(Event<std::string>("Event 2", 1)));
    REQUIRE(eq.Size() == 2);
    REQUIRE(eq.Top()->GetData() == "Event 2");
    REQUIRE(eq.Top()->GetPriority() == 1);
    REQUIRE(eq.Top()->GetTiebreaker() == 1);

    REQUIRE(eq.Push("Event 3", 0));
    REQUIRE(eq.Size() == 3);
    REQUIRE(eq.Top()->GetData() == "Event 3");
    REQUIRE(eq.Top()->GetPriority() == 0);
    REQUIRE(eq.Top()->GetTiebreaker() == 2);
}

TEST_CASE("Test Top and Pop on Empty Queue", "[EventQueue]")
{
    // Test that Top() returns nullptr and Pop() returns false on an empty queue
    EventQueue<std::string> eq;
    REQUIRE(eq.Empty());

    // Top() should return nullptr when the queue is empty
    REQUIRE(eq.Top() == nullptr);

    // Pop() should return false when the queue is empty
    REQUIRE_FALSE(eq.Pop());

    // Queue should still be empty and well-behaved after failed operations
    REQUIRE(eq.Empty());
    REQUIRE(eq.Size() == 0);

    // Top() should return nullptr after Clear() empties a non-empty queue
    REQUIRE(eq.Push(Event<std::string>("Event 1", 1)));
    eq.Clear();
    REQUIRE(eq.Top() == nullptr);
    REQUIRE_FALSE(eq.Pop());
}

TEST_CASE("Test Pop", "[EventQueue]") 
{
    // Test that events are popped correctly and that the heap property is maintained
    EventQueue<std::string> eq;
    REQUIRE(eq.Push(Event<std::string>("Event 1", 2)));
    REQUIRE(eq.Push(Event<std::string>("Event 2", 1)));
    REQUIRE(eq.Push(Event<std::string>("Event 3", 3)));

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
    // Test that Size, Empty, and Clear functions work correctly
    EventQueue<std::string> eq;
    REQUIRE(eq.Empty());
    REQUIRE(eq.Size() == 0);

    REQUIRE(eq.Push(Event<std::string>("Event 1", 1)));
    REQUIRE(eq.Push(Event<std::string>("Event 2", 2)));
    REQUIRE(!eq.Empty());
    REQUIRE(eq.Size() == 2);

    eq.Clear();
    REQUIRE(eq.Empty());
    REQUIRE(eq.Size() == 0);

    REQUIRE(eq.Push(Event<std::string>("Event 3", 3)));
    REQUIRE(eq.Size() == 1);
    REQUIRE(eq.Top()->GetData() == "Event 3");
}

TEST_CASE("Test Equal-Priority Ordering", "[EventQueue]") 
{
    // Test that events with the same priority are ordered 
    // based on their insertion order (tiebreaker).

    // All events have the same priority, so they should be 
    // processed in the order they were inserted
    EventQueue<std::string> eq;
    REQUIRE(eq.Push(Event<std::string>("Event 1", 5)));
    REQUIRE(eq.Push(Event<std::string>("Event 2", 5)));
    REQUIRE(eq.Push(Event<std::string>("Event 3", 5)));

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
    // Test that events with different priorities are ordered correctly and that
    // events with the same priority are ordered based on their insertion order (tiebreaker)

    // All events have different priorities, so they should be processed in 
    // order of their priority (lowest priority value first)
    EventQueue<std::string> eq;
    REQUIRE(eq.Push(Event<std::string>("A", 2)));
    REQUIRE(eq.Push(Event<std::string>("B", 1)));
    REQUIRE(eq.Push(Event<std::string>("C", 2)));
    REQUIRE(eq.Push(Event<std::string>("D", 1)));

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
    // Test that the EventQueue correctly orders a large number of events
    // and maintains the heap property throughout
    EventQueue<std::string> eq;

    const int NUM_EVENTS = 1000;
    const int NUM_PRIORITIES = 10;

    // Push 1000 events with priorities cycling 0-9
    for (int i = 0; i < NUM_EVENTS; ++i)
    {
        int priority = i % NUM_PRIORITIES;
        REQUIRE(eq.Push(Event<std::string>("Event " + std::to_string(i), priority)));
    }

    REQUIRE(eq.Size() == NUM_EVENTS);

    // Pop all events and verify they come out in non-decreasing priority order,
    // and that events with the same priority are ordered by insertion (tiebreaker)
    int lastPriority = -1;
    std::size_t lastTiebreaker = 0;
    int count = 0;

    while (!eq.Empty())
    {
        const auto* top = eq.Top();
        REQUIRE(top != nullptr);

        int currentPriority = top->GetPriority();
        std::size_t currentTiebreaker = top->GetTiebreaker();

        // Priority must be non-decreasing
        REQUIRE(currentPriority >= lastPriority);

        // Within the same priority, tiebreaker must be increasing
        if (currentPriority == lastPriority)
            REQUIRE(currentTiebreaker > lastTiebreaker);

        lastPriority = currentPriority;
        lastTiebreaker = currentTiebreaker;
        ++count;

        REQUIRE(eq.Pop());
    }

    // All 1000 events should have been popped
    REQUIRE(count == NUM_EVENTS);
    REQUIRE(eq.Empty());
    REQUIRE(eq.Size() == 0);

    // Top() and Pop() should behave correctly after draining the large queue
    REQUIRE(eq.Top() == nullptr);
    REQUIRE_FALSE(eq.Pop());
}

TEST_CASE("Test Templated Event and EventQueue", "[EventQueue]")
{
    // Test with int data
    EventQueue<int> intQueue;
    REQUIRE(intQueue.Push(Event<int>(42, 2)));
    REQUIRE(intQueue.Push(Event<int>(7, 1)));
    REQUIRE(intQueue.Push(Event<int>(99, 3)));

    REQUIRE(intQueue.Top()->GetData() == 7);
    REQUIRE(intQueue.Top()->GetPriority() == 1);
    REQUIRE(intQueue.Pop());
    REQUIRE(intQueue.Top()->GetData() == 42);
    REQUIRE(intQueue.Pop());
    REQUIRE(intQueue.Top()->GetData() == 99);
    REQUIRE(intQueue.Pop());
    REQUIRE(intQueue.Empty());

    // Test with double data
    EventQueue<double> doubleQueue;
    REQUIRE(doubleQueue.Push(Event<double>(3.14, 2)));
    REQUIRE(doubleQueue.Push(Event<double>(2.71, 1)));
    REQUIRE(doubleQueue.Push(Event<double>(1.41, 3)));

    REQUIRE(doubleQueue.Top()->GetData() == 2.71);
    REQUIRE(doubleQueue.Top()->GetPriority() == 1);
    REQUIRE(doubleQueue.Pop());
    REQUIRE(doubleQueue.Top()->GetData() == 3.14);
    REQUIRE(doubleQueue.Pop());
    REQUIRE(doubleQueue.Top()->GetData() == 1.41);
    REQUIRE(doubleQueue.Pop());
    REQUIRE(doubleQueue.Empty());

    // Test with a custom struct
    struct Point { int x, y; };
    EventQueue<Point> pointQueue;
    REQUIRE(pointQueue.Push(Event<Point>({1, 2}, 3)));
    REQUIRE(pointQueue.Push(Event<Point>({3, 4}, 1)));
    REQUIRE(pointQueue.Push(Event<Point>({5, 6}, 2)));

    REQUIRE(pointQueue.Top()->GetData().x == 3);
    REQUIRE(pointQueue.Top()->GetData().y == 4);
    REQUIRE(pointQueue.Top()->GetPriority() == 1);
    REQUIRE(pointQueue.Pop());
    REQUIRE(pointQueue.Top()->GetData().x == 5);
    REQUIRE(pointQueue.Top()->GetData().y == 6);
    REQUIRE(pointQueue.Pop());
    REQUIRE(pointQueue.Top()->GetData().x == 1);
    REQUIRE(pointQueue.Top()->GetData().y == 2);
    REQUIRE(pointQueue.Pop());
    REQUIRE(pointQueue.Empty());
}