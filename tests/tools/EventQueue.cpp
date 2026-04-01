/** 
 * @file EventQueue.cpp
 * @author Truong Phan
 *
 * Tests for the EventQueue class.
 * 
 * Citation: ChatGPT and GitHub Copilot were used to assist in the design and 
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
    Event e1;

    // Test getters for default event
    REQUIRE(e1.GetData() == "");
    REQUIRE(e1.GetPriority() == 0);
    REQUIRE(e1.GetTiebreaker() == 0);

    // Test parameterized constructor
    Event e2("Sample Event", 5);

    // Test getters for parameterized event
    REQUIRE(e2.GetData() == "Sample Event");
    REQUIRE(e2.GetPriority() == 5);
    REQUIRE(e2.GetTiebreaker() == 0);

    // Test move constructor
    Event e3("Another Event", 42);

    // Test getters for moved event
    REQUIRE(e3.GetData() == "Another Event");
    REQUIRE(e3.GetPriority() == 42);
    REQUIRE(e3.GetTiebreaker() == 0);
}

TEST_CASE("Test EventQueue Default, Copy, and Assignment Constructor", "[EventQueue]") 
{
    // Test default constructor
    EventQueue eq;
    REQUIRE(eq.Empty());
    REQUIRE(eq.Size() == 0);

    REQUIRE(eq.Push(Event("Event 1", 1)));

    // Test copy constructor
    EventQueue copy(eq);
    REQUIRE(copy.Size() == 1);
    REQUIRE(copy.Top()->GetData() == "Event 1");

    // Test assignment operator
    EventQueue assigned;
    assigned = eq;
    REQUIRE(assigned.Size() == 1);
    REQUIRE(assigned.Top()->GetData() == "Event 1");
}

TEST_CASE("Test Copy and Assignment Correctness", "[EventQueue]") 
{
    // Test that copy and assignment create independent copies of the EventQueue
    EventQueue eq;
    REQUIRE(eq.Push(Event("Event 1", 1)));
    REQUIRE(eq.Push(Event("Event 2", 2)));
    REQUIRE(eq.Push(Event("Event 3", 3)));

    // Copy constructor
    EventQueue copy(eq);
    REQUIRE(copy.Size() == eq.Size());
    REQUIRE(copy.Top()->GetData() == eq.Top()->GetData());
    REQUIRE(copy.Pop());
    REQUIRE(copy.Size() == eq.Size() - 1);
    REQUIRE(eq.Size() == 3);

    // Assignment operator
    EventQueue assigned;
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
    EventQueue eq;

    REQUIRE(eq.Push(Event("Event 1", 2)));
    REQUIRE(eq.Size() == 1);
    REQUIRE(eq.Top()->GetData() == "Event 1");
    REQUIRE(eq.Top()->GetPriority() == 2);
    REQUIRE(eq.Top()->GetTiebreaker() == 0);

    REQUIRE(eq.Push(Event("Event 2", 1)));
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

TEST_CASE("Test Pop", "[EventQueue]") 
{
    // Test that events are popped correctly and that the heap property is maintained
    EventQueue eq;
    REQUIRE(eq.Push(Event("Event 1", 2)));
    REQUIRE(eq.Push(Event("Event 2", 1)));
    REQUIRE(eq.Push(Event("Event 3", 3)));

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
    EventQueue eq;
    REQUIRE(eq.Empty());
    REQUIRE(eq.Size() == 0);

    REQUIRE(eq.Push(Event("Event 1", 1)));
    REQUIRE(eq.Push(Event("Event 2", 2)));
    REQUIRE(!eq.Empty());
    REQUIRE(eq.Size() == 2);

    eq.Clear();
    REQUIRE(eq.Empty());
    REQUIRE(eq.Size() == 0);

    REQUIRE(eq.Push(Event("Event 3", 3)));
    REQUIRE(eq.Size() == 1);
    REQUIRE(eq.Top()->GetData() == "Event 3");
}

TEST_CASE("Test Equal-Priority Ordering", "[EventQueue]") 
{
    // Test that events with the same priority are ordered 
    // based on their insertion order (tiebreaker).

    // All events have the same priority, so they should be 
    // processed in the order they were inserted
    EventQueue eq;
    REQUIRE(eq.Push(Event("Event 1", 5)));
    REQUIRE(eq.Push(Event("Event 2", 5)));
    REQUIRE(eq.Push(Event("Event 3", 5)));

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
    EventQueue eq;
    REQUIRE(eq.Push(Event("A", 2)));
    REQUIRE(eq.Push(Event("B", 1)));
    REQUIRE(eq.Push(Event("C", 2)));
    REQUIRE(eq.Push(Event("D", 1)));

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