/**
 * @file EventQueue.h
 * @author Truong Phan
 * 
 * Class for an EventQueue
 */
#pragma once

// Include statements
#include <vector>   // For heap container
#include <string>  // For event data type
#include <cstddef> // For size_t

namespace cse498
{
    /*
    * Struct for an Event
    */
    struct Event
    {
        // Event data
        std::string mData;

        // Priority of the event
        int mPriority = 0;

        // Insertion index used for tiebreaker 
        std::size_t mTiebreaker = 0;

        /**
        * Default Constructor
        */
        Event() = default;

        /**
         * Constructor
         * @param data The event data
         * @param priority The priority of the event
         */
        Event(const std::string& data, int priority) : mData(data), mPriority(priority), mTiebreaker(0) {}
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
        std::size_t mInsertionIndex = 0;

        // Comparator
        struct Comparator
        {
            bool operator() (const Event& a, const Event& b) const;
        };

    public:
        /**
         * Default Constructor
         */
        EventQueue() = default;

        // Copy constructor
        EventQueue(const EventQueue&) = default;
        
        // Assignment operator
        EventQueue& operator=(const EventQueue&) = default;

        void Push(const Event& event);

        void Push(const std::string& data, int priority);

        void Pop();

        const Event& Top() const;

        std::size_t Size() const;

        bool Empty() const;
    };
}