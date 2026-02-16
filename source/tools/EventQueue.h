/**
 * @file EventQueue.h
 * @author Truong Phan
 * 
 * Class for an EventQueue
 */
#pragma once

// Include statements
#include <algorithm> // For push_heap and pop_heap
#include <vector>   // For heap container
#include <string>  // For event data type

/*
 * Struct for an Event
 */
struct Event
{
    // Event data
    std::string mData;

    // Priority of the event
    int mPriority;

    // Insertion index used for tiebreaker 
    size_t mTiebreaker;

    Event();

    Event(const std::string& data, int priority);
};

/*
 * Class for an EventQueue
 */
class EventQueue 
{
private:
    // Heap container holding all events
    std::vector<Event> mHeap;
    
    // Insertion index
    size_t mInsertionIndex = 0;

    // Comparator
    struct Comparator
    {
        bool operator() (const Event& a, const Event& b) const;
    };

public:
    EventQueue();
    
    void Push(const Event& event);

    void Pop();

    const Event& Top() const;

    size_t Size() const;

    bool Empty() const;
};