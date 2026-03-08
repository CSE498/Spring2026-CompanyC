/** 
 * @file TestEventQueue.cpp
 * @author Truong Phan
 *
 * This file contains unit tests for the EventQueue class. It was written with the help of
 * ChatGPT and GitHub Copilot. 
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/EventQueue.hpp"

using cse498::EventQueue;
using cse498::Event;

TEST_CASE("Test Event Constructor", "[Event]") 
{
    // Test default constructor
    Event e1;
    REQUIRE(e1.mData == "");
    REQUIRE(e1.mPriority == 0);
    REQUIRE(e1.mTiebreaker == 0);

    
    // Test parameterized constructor
    Event e2("Test Data", 5);
    REQUIRE(e2.mData == "Test Data");
    REQUIRE(e2.mPriority == 5);
    REQUIRE(e2.mTiebreaker == 0);
}

TEST_CASE("Test EventQueue Constructor", "[EventQueue]") 
{
    // Test default constructor
    EventQueue eq;
    REQUIRE(eq.Empty());
    REQUIRE(eq.Size() == 0);

    // Test copy constructor
    eq.Push(Event("Event 1", 1));
    EventQueue copy(eq);
    REQUIRE(!copy.Empty());
    REQUIRE(copy.Size() == 1);

    // Test assignment operator
    EventQueue assigned;
    assigned = eq;
    REQUIRE(!assigned.Empty());
    REQUIRE(assigned.Size() == 1);
}

TEST_CASE("Test Push", "[EventQueue]") 
{
    // Test pushing events onto the EventQueue
    EventQueue eq;

    // Push an event and verify it's added correctly
    eq.Push(Event("Event 1", 2));
    REQUIRE(!eq.Empty());
    REQUIRE(eq.Size() == 1);
    REQUIRE(eq.Top().mData == "Event 1");
    REQUIRE(eq.Top().mPriority == 2);
    REQUIRE(eq.Top().mTiebreaker == 0);

    eq.Push(Event("Event 2", 1));
    REQUIRE(eq.Size() == 2);
    REQUIRE(eq.Top().mData == "Event 2");
    REQUIRE(eq.Top().mPriority == 1);
    REQUIRE(eq.Top().mTiebreaker == 1);

    // Test overload of Push that takes data and priority directly
    eq.Push("Event 3", 0);
    REQUIRE(eq.Size() == 3);
    REQUIRE(eq.Top().mData == "Event 3");
    REQUIRE(eq.Top().mPriority == 0);
    REQUIRE(eq.Top().mTiebreaker == 2);

}

TEST_CASE("Test Pop", "[EventQueue]") 
{
    // Test popping events from the EventQueue
    EventQueue eq;
    eq.Push(Event("Event 1", 2));
    eq.Push(Event("Event 2", 1));
    eq.Push(Event("Event 3", 3));

    // Pop events and verify the order is correct
    eq.Pop();
    REQUIRE(eq.Size() == 2);
    REQUIRE(eq.Top().mData == "Event 1");
    REQUIRE(eq.Top().mPriority == 2);
    REQUIRE(eq.Top().mTiebreaker == 0);
    
    eq.Pop();
    REQUIRE(eq.Size() == 1);
    REQUIRE(eq.Top().mData == "Event 3");
    REQUIRE(eq.Top().mPriority == 3);
    REQUIRE(eq.Top().mTiebreaker == 2);
    
    // Pop last event and verify the EventQueue is empty
    eq.Pop();
    REQUIRE(eq.Empty());
}

TEST_CASE("Test Top", "[EventQueue]") 
{
    // Test accessing the top event of the EventQueue
    EventQueue eq;
    eq.Push(Event("Event 1", 2));
    eq.Push(Event("Event 2", 1));
    eq.Push(Event("Event 3", 3));

    // Verify the top event is correct
    REQUIRE(eq.Top().mData == "Event 2");
    REQUIRE(eq.Top().mPriority == 1);
    REQUIRE(eq.Top().mTiebreaker == 1);
    REQUIRE(eq.Size() == 3);

    eq.Pop();

    // Verify the new top event is correct after popping
    REQUIRE(eq.Top().mData == "Event 1");
    REQUIRE(eq.Top().mPriority == 2);
    REQUIRE(eq.Top().mTiebreaker == 0);
    REQUIRE(eq.Size() == 2);
}

TEST_CASE("Test Size and Empty", "[EventQueue]") 
{
    // Test the Size and Empty functions of the EventQueue
    EventQueue eq;

    // Initially empty
    REQUIRE(eq.Size() == 0);
    REQUIRE(eq.Empty());

    // Push events and check size and empty
    eq.Push(Event("Event 1", 2));
    REQUIRE(eq.Size() == 1);
    REQUIRE(!eq.Empty());

    eq.Push(Event("Event 2", 1));
    REQUIRE(eq.Size() == 2);
    REQUIRE(!eq.Empty());

    eq.Pop();
    REQUIRE(eq.Size() == 1);
    REQUIRE(!eq.Empty());

    // Pop last event and check size and empty
    eq.Pop();
    REQUIRE(eq.Size() == 0);
    REQUIRE(eq.Empty());
}

TEST_CASE("Test EventQueue Comparisons", "[EventQueue]") 
{
    // Test that events with the same priority are ordered by insertion index
    EventQueue eq;
    eq.Push(Event("Event 1", 1)); // Tiebreaker 0
    eq.Push(Event("Event 2", 1)); // Tiebreaker 1
    eq.Push(Event("Event 3", 1)); // Tiebreaker 2

    REQUIRE(eq.Top().mData == "Event 1");
    eq.Pop();
    REQUIRE(eq.Top().mData == "Event 2");
    eq.Pop();
    REQUIRE(eq.Top().mData == "Event 3");

    // Test that events with different priorities are ordered by priority
    EventQueue eq2;
    eq2.Push(Event("Event A", 5)); // Tiebreaker 0
    eq2.Push(Event("Event B", 4)); // Tiebreaker 1
    eq2.Push(Event("Event C", 3)); // Tiebreaker 2

    REQUIRE(eq2.Top().mData == "Event C");
    eq2.Pop();
    REQUIRE(eq2.Top().mData == "Event B");
    eq2.Pop();
    REQUIRE(eq2.Top().mData == "Event A");
}