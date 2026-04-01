/**
 * @file EventQueue.cpp
 * @author Truong Phan
 * 
 * The implementation of the EventQueue class.
 */
// Include statements
#include "EventQueue.hpp"
#include <algorithm> // For push_heap and pop_heap
#include <cassert> // For assert statements

namespace cse498
{
    bool EventQueue::Push(Event event)
    {
        event.mTiebreaker = mInsertionIndex++;

        mHeap.push_back(std::move(event));
        std::push_heap(mHeap.begin(), mHeap.end(), Comparator{});

        assert(std::is_heap(mHeap.begin(), mHeap.end(), Comparator{}) 
                && "EventQueue: Heap property broken");

        return true;
    }

    bool EventQueue::Push(const std::string& data, int priority)
    {
        return Push(Event(data, priority));
    }

    bool EventQueue::Pop()
    {
        assert(!Empty() && "EventQueue: Pop() called on empty EventQueue");

        if (Empty())
            return false;

        std::pop_heap(mHeap.begin(), mHeap.end(), Comparator{});
        mHeap.pop_back();

        assert(std::is_heap(mHeap.begin(), mHeap.end(), Comparator{}) 
                && "EventQueue: Heap property broken");

        return true;
    }

    std::optional<Event> EventQueue::Top() const 
    { 
        if (Empty())
            return std::nullopt;

        return mHeap.front(); 
    }

    std::size_t EventQueue::Size() const 
    { 
        return mHeap.size(); 
    }

    bool EventQueue::Empty() const 
    { 
        return mHeap.empty(); 
    }

    void EventQueue::Clear()
    {
        mHeap.clear();
        mInsertionIndex = 0;
    }
}