/**
 * @file EventQueue.cpp
 * @author Truong Phan
 * 
 * The implementation of the EventQueue class
 */

// Include statements
#include "EventQueue.hpp"
#include <algorithm> // For push_heap and pop_heap
#include <cassert> // For assert statements

namespace cse498
{
    /**
     * Comparison operator
     * 
     * @param a The first event
     * @param b The second event
     * @return Whether or not a should come after b in the heap
     */
    bool EventQueue::Comparator::operator()(const Event& a, const Event& b) const
    {
        if (a.mPriority == b.mPriority)
            return a.mTiebreaker > b.mTiebreaker;
        return a.mPriority > b.mPriority;
    }

    /**
     * Push an event onto the EventQueue, this function was written with the help of ChatGPT.
     * 
     * The event is copied to modify the tiebreaker, which is set to the current insertion index. 
     * The event is then added to the heap and the heap property is maintained using std::push_heap.
     * 
     * @param event The event to be added
     */
    void EventQueue::Push(const Event& event)
    {
        Event temp = event;
        temp.mTiebreaker = mInsertionIndex++;

        mHeap.push_back(temp);
        std::push_heap(mHeap.begin(), mHeap.end(), Comparator{});

        assert(std::is_heap(mHeap.begin(), mHeap.end(), Comparator{}) && " EventQueue: Heap property broken");
    }

    /**
     * Push an event onto the EventQueue.
     * 
     * This is an overload of the Push function that takes in the event data and priority directly.
     * 
     * @param data The event data
     * @param priority The priority of the event
     */
    void EventQueue::Push(const std::string& data, int priority)
    {
        Push(Event(data, priority));
    }

    /**
     * Removes the top event from the EventQueue. This function was written with the help of ChatGPT.
     * 
     * The top event is popped from the heap using std::pop_heap, which moves the 
     * top event to the end of the vector. The last event is then removed from the vector 
     * and the heap property is maintained.
     * 
     */
    void EventQueue::Pop()
    {
        assert(!Empty() && "EventQueue: Pop() called on empty EventQueue");

        std::pop_heap(mHeap.begin(), mHeap.end(), Comparator{});
        mHeap.pop_back();

        assert(std::is_heap(mHeap.begin(), mHeap.end(), Comparator{}) && " EventQueue: Heap property broken");
    }

    /** 
     * Return the top event in the EventQueue.
     * 
     * This function returns a reference to the top event in the heap, which is the event 
     * with the highest priority (lowest priority value).
     * 
     */
    const Event& EventQueue::Top() const 
    { 
        assert(!Empty() && "EventQueue: Top() called on empty EventQueue");

        return mHeap.front(); 
    }

    /**
     * Return the current size of the EventQueue.
     * 
     * This function returns the number of events currently in the EventQueue 
     * by returning the size of the underlying vector.
     */
    std::size_t EventQueue::Size() const 
    { 
        return mHeap.size(); 
    }

    /** 
     * Whether the EventQueue is empty or not.
     * 
     * This function checks if the EventQueue is empty by checking if the underlying vector is empty.
     */
    bool EventQueue::Empty() const 
    { 
        return mHeap.empty(); 
    };

    /**
     * Clear all events from the EventQueue.
     * 
     * This function clears the EventQueue by clearing the underlying vector and resetting the insertion index.
     */
    void EventQueue::Clear()
    {
        mHeap.clear();
        mInsertionIndex = 0;
    }
}