/**
 * @file EventQueue.hpp
 * @author Truong Phan
 * 
 * Class for an EventQueue
 */
#pragma once

// Include statements
#include <vector>   // For heap container
#include <string>  // For event data type
#include <cstddef> // For size_t
#include <utility> // For std::move

namespace cse498
{
    /*
    * Class for an Event
    */
    class Event
    {
    private:
        // Event data
        std::string mData;

        // Priority of the event
        int mPriority = 0;

        // Insertion index used for tiebreaker 
        std::size_t mTiebreaker = 0;

    public:
        /**
        * Default Constructor
        */
        Event() = default;

        /**
         * Constructor
         * @param data The event data
         * @param priority The priority of the event
         */
        Event(std::string data, int priority)
            : mData(std::move(data)), mPriority(priority) {}

        // Event Getters
        const std::string& GetData() const { return mData; }

        int GetPriority() const { return mPriority; }

        std::size_t GetTiebreaker() const { return mTiebreaker; }

        // Allow EventQueue to access private members of Event
        friend class EventQueue;
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

        /**
         * Comparison operator
         * 
         * 1. Lower priority values are processed first.
         * 2. If priorities are equal, the event inserted earlier (smaller mTiebreaker) is processed first.
         * 
         * @param a The first event
         * @param b The second event
         * @return Whether or not a should come after b in the heap
         */ 
        struct Comparator
        {
            constexpr bool operator()(const Event& a, const Event& b) const
            {
                if (a.mPriority == b.mPriority)
                    return a.mTiebreaker > b.mTiebreaker;
                return a.mPriority > b.mPriority;
            }
        };

    public:
        /**
         * Default Constructor
         */
        EventQueue() = default;

        /**
         * Default Destructor
         */
        ~EventQueue() = default;

        // Copy constructor
        EventQueue(const EventQueue&) = default;
        
        // Assignment operator
        EventQueue& operator=(const EventQueue&) = default;

        void Push(Event event);

        void Push(const std::string& data, int priority);

        void Pop();

        [[nodiscard]] const Event& Top() const;

        [[nodiscard]] std::size_t Size() const;

        [[nodiscard]] bool Empty() const;

        void Clear();
    };
}