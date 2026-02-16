/**
 * @file EventQueue.cpp
 * @author Truong Phan
 * 
 */

// Include statements
#include "EventQueue.h"
#include <vector> // For heap container
#include <algorithm> // For push_heap and pop_heap
#include <utility> // For std::pair
#include <string> // For event data type
#include <cassert> // For assert statements

namespace cse498
{
    /**
     * Default Constructor
     */
    Event::Event() : mData(""), mPriority(0), mTiebreaker(0)
    {

    }

    /**
     * Constructor
     * @param data The event data
     * @param priority The priority of the event
     * @param index The insertion index of the event
     */
    Event::Event(const std::string& data, int priority) : mData(data), mPriority(priority), mTiebreaker(0)
    {

    }

    /**
     * Comparison operator
     * @param a The first event
     * @param b The second event
     * @return Whether or not a should come after b in the heap
     * References: 
     * https://stackoverflow.com/questions/63331546/how-to-customize-stdmake-heap-comparison-function-based-on-some-data-structure
     * "ChatGPT - "How do I change the comparison for a heap in c++?",
     * "ChatGPT - "How should I tiebreak a priority queue if both of them are the same priority? Should it be the time it was added?"
     */
    bool EventQueue::Comparator::operator()(const Event& a, const Event& b) const
    {
        // Same priorities: use tiebreaker (smaller wins)
        if (a.mPriority == b.mPriority)
            return a.mTiebreaker > b.mTiebreaker;
        // Different priorities: use prioritiy (smaller wins)
        return a.mPriority > b.mPriority;
    }

    /**
     * Default Constructor
     * References:
     * https://www.geeksforgeeks.org/cpp/cpp-stl-heap/
     */
    EventQueue::EventQueue()
    {

    }

    /**
     * Push an event onto the EventQueue
     * @param event The event to be added
     * References:
     * ChatGPT - "Can you check the const correctness of this function? void Push(const Event& event)"
     */
    void EventQueue::Push(const Event& event)
    {
        // Create a copy of the event to modify the tiebreaker
        Event temp = event;
        temp.mTiebreaker = mInsertionIndex++;

        // Add event
        mHeap.push_back(temp);
        std::push_heap(mHeap.begin(), mHeap.end(), Comparator{});

    }

    /**
     * Pop an event from the EventQueue
     */
    void EventQueue::Pop()
    {
        assert(!Empty() && "EventQueue: Pop() called on empty EventQueue");

        // Remove top event
        std::pop_heap(mHeap.begin(), mHeap.end(), Comparator{});
        mHeap.pop_back();
    }

    /** 
     * Return the top event in the EventQueue
     * References:
     * ChatGPT - "Can you check the const correctness of this function? const Event& EventQueue::Top() const"
     */
    const Event& EventQueue::Top() const 
    { 
        assert(!Empty() && "EventQueue: Top() called on empty EventQueue");

        // Return top event of the EventQueue
        return mHeap.front(); 
    }

    /**
     * Return the current size of the EventQueue
     */
    size_t EventQueue::Size() const 
    { 
        // Return the current size of the EventQueue
        return mHeap.size(); 
    }

    /** 
     * Whether or not the EventQueue is empty
     */
    bool EventQueue::Empty() const 
    { 
        // Return if its empty or not
        return mHeap.empty(); 
    };
}