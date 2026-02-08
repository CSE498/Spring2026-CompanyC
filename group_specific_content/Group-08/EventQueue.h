/**
 * @file EventQueue.cpp
 * @author Truong Phan
 * 
 * Class for an EventQueue
 */
#pragma once

// Include statements
#include <algorithm>
#include <vector>

/*
 * Struct for an Event
 */
template<typename T>
struct Event
{
    // Event data
    T mData;

    // Priority of the event
    int mPriority;

    // Insertion index used for tiebreaker 
    size_t mTiebreaker;

    // Default Constructor
    Event();

    // Constructor
    Event(const T& data, int priority, int index);
};

template<typename T>
/*
 * Class for an EventQueue
 */
class EventQueue 
{
private:
    // Heap container holding all events
    std::vector<Event<T>> mHeap;
    // Insertion index
    size_t mInsertionIndex = 0;

    // Comparator
    struct Compare
    {
        bool operator() (const Event<T>& a, const Event<T>& b) const;
    };

public:
    // Default Constructor
    EventQueue();
    
    // Constructor
    EventQueue(const std::vector<std::pair<T, int>>& events);   
    
    void Push(const Event<T>& event);

    void Pop();

    const T& Top() const;

    size_t Size() const;

    bool Empty() const;

};