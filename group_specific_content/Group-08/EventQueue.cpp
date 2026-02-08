/**
 * @file EventQueue.cpp
 * @author Truong Phan
 * 
 */

// Include statements
#include "EventQueue.h"
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <cassert>

/**
 * Default Constructor
 */
template<typename T>
Event<T>::Event()
{
};

/**
 * Constructor
 * @param data The event data
 * @param priority The priority of the event
 * @param index The insertion index of the event
 */
template<typename T>
Event<T>::Event(const T& data, int priority, int index) : mData(data), mPriority(priority), mTiebreaker(index)
{
}

/**
 * Comparison operator
 */
template<typename T>
bool EventQueue<T>::Compare::operator()(const Event<T>& a, const Event<T>& b) const
{
    // Same priorities --> use tiebreaker (smaller wins)
    if (a.mPriority == b.mPriority)
        return a.mTiebreaker > b.mTiebreaker;
    // Different priorities --> use prioritiy (smaller wins)
    return a.mPriority > b.mPriority;
}

/**
 * Default Constructor
 */
template<typename T>
EventQueue<T>::EventQueue()
{
};

/** 
 * Constructor
 * @param events A vector holding events
 */
template<typename T>
EventQueue<T>::EventQueue(const std::vector<std::pair<T, int>>& events)
{
    // Iterate through events
    for (const auto& e : events)
    {
        // Push to heap container
        mHeap.push_back(Event(e.first, e.second, mInsertionIndex++));
    }
    // Make a heap
    std::make_heap(mHeap.begin(), mHeap.end(), Compare{});
};

/**
 * Push an event onto the EventQueue
 * @param event The event to be added
 */
template<typename T>
void EventQueue<T>::Push(const Event<T>& event)
{
    // Add event
    mHeap.push_back(event);
    std::push_heap(mHeap.begin(), mHeap.end(), Compare{});
}

/**
 * Pop an event from the EventQueue
 */
template<typename T>
void EventQueue<T>::Pop()
{
    assert(!Empty() && "EventQueue: Pop() called on empty EventQueue");

    // Remove top event
    std::pop_heap(mHeap.begin(), mHeap.end(), Compare{});
    mHeap.pop_back();
}

/** 
 * Return the top event in the EventQueue
 */
template<typename T>
const T& EventQueue<T>::Top() const 
{ 
    assert(!Empty() && "EventQueue: Top() called on empty EventQueue");

    // Return top event of the EventQueue
    return mHeap.front().mData; 
}

/**
 * Return the current size of the EventQueue
 */
template<typename T>
size_t EventQueue<T>::Size() const 
{ 
    // Return the current size of the EventQueue
    return mHeap.size(); 
}

/** 
 * Whether or not the EventQueue is empty
 */
template<typename T>
bool EventQueue<T>::Empty() const 
{ 
    // Return if its empty or not
    return mHeap.empty(); 
};

// Explicit instantiations
template struct Event<std::string>;
template class EventQueue<std::string>;
