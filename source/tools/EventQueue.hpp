/**
 * @file EventQueue.hpp
 * @author Truong Phan
 * 
 * Class for an EventQueue.
 * 
 * This file defines the Event and EventQueue classes. The Event class represents an 
 * individual event with associated data and priority, while the EventQueue class 
 * manages a collection of events in a priority queue (min-heap) structure. 
 * The EventQueue supports operations to add events, remove the top event, access the top event, 
 * check the size of the queue, and clear all events from the queue.
 * 
 * Priority rules:
 * 1. Events with lower priority values are processed before events with higher priority values.
 * 2. If two events have the same priority, the event that was inserted earlier 
 *    (with a smaller tiebreaker value) is processed first.
 * 
 * Citation: Claude AI and ChatGPT were used to assist in the design and 
 * implementation of the both classes. The generated code was then modified and improved 
 * by the author to meet the requirements of the project.
 */
#pragma once

// Include statements
#include <vector>   // For heap container
#include <string>  // For event data type
#include <cstddef> // For size_t
#include <utility> // For std::move
#include <optional> // For std::optional
#include <algorithm> // For push_heap and pop_heap
#include <cassert> // For assert statements

namespace cse498
{
    /*
    * Class for an Event
    */
    template<typename T>
    class Event
    {
    private:
        // Event data
        T mData;

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
        Event(T data, int priority)
            : mData(std::move(data)), mPriority(priority) {}

        // Event Getters
        const T& GetData() const { return mData; }

        int GetPriority() const { return mPriority; }

        std::size_t GetTiebreaker() const { return mTiebreaker; }

        // Allow EventQueue to access private members of Event
        template<typename U>
        friend class EventQueue;
    };

    /*
    * Class for an EventQueue
    */
    template<typename T>
    class EventQueue 
    {
    private:
        // Heap container holding all events
        std::vector<Event<T>> mHeap;
        
        // Insertion index
        std::size_t mInsertionIndex = 0;

        /**
         * Comparison operator
         * 
         * 1. Lower priority values are processed first.
         * 2. If priorities are equal, the event inserted earlier 
         *    (smaller mTiebreaker) is processed first.
         * 
         * @param a The first event
         * @param b The second event
         * @return Whether or not a should come after b in the heap
         */ 
        struct Comparator
        {
            constexpr bool operator()(const Event<T>& a, const Event<T>& b) const
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

        /**
         * Push an event onto the EventQueue.
         * 
         * The event is copied to modify the tiebreaker, which is set to the current insertion index. 
         * The event is then added to the heap and the heap property is maintained using std::push_heap.
         * 
         * @param event The event to be added
         * @return This will return false if the event could not be added to the EventQueue and true otherwise.
         */
        [[nodiscard]] bool Push(Event<T> event)
        {
            event.mTiebreaker = mInsertionIndex++;

            mHeap.push_back(std::move(event));
            std::push_heap(mHeap.begin(), mHeap.end(), Comparator{});

            assert(std::is_heap(mHeap.begin(), mHeap.end(), Comparator{}) 
                    && "EventQueue: Heap property broken");

            return true;
        }

        /**
         * Push an event onto the EventQueue.
         * 
         * This is an overload of the Push function that takes in the event data and priority directly.
         * 
         * @param data The event data
         * @param priority The priority of the event
         * @return This will return false if the event could not be added to the EventQueue and true otherwise.
         */
        [[nodiscard]] bool Push(const T& data, int priority)
        {
            return Push(Event<T>(data, priority));
        }

        /**
         * Removes the top event from the EventQueue.
         * 
         * The top event is popped from the heap using std::pop_heap, which moves the 
         * top event to the end of the vector. The last event is then removed from the vector 
         * and the heap property is maintained.
         * 
         * Precondition: The EventQueue should not be empty. 
         * 
         * @return Whether or not the pop was successful. This will return false if the EventQueue 
         * is empty and true otherwise.
         * 
         */
        [[nodiscard]] bool Pop()
        {
            if (Empty())
                return false;

            std::pop_heap(mHeap.begin(), mHeap.end(), Comparator{});
            mHeap.pop_back();

            assert(std::is_heap(mHeap.begin(), mHeap.end(), Comparator{}) 
                    && "EventQueue: Heap property broken");

            return true;
        }

        /** 
         * Return the top event in the EventQueue.
         * 
         * This function returns a pointer to the top event in the EventQueue, which 
         * is the event with the lowest priority value.
         * 
        * Precondition: The EventQueue should not be empty. 
         * 
         * @return A pointer to the top event in the EventQueue. This will return nullptr if the EventQueue is empty.
         */
        [[nodiscard]] const Event<T>* Top() const
        { 
            if (Empty())
                return nullptr;

            return &mHeap.front(); 
        }

        /**
         * Return the current size of the EventQueue.
         * 
         * This function returns the number of events currently in the EventQueue 
         * by returning the size of the underlying vector.
         * 
         * @return The number of events currently in the EventQueue.
         */
        [[nodiscard]] std::size_t Size() const
        {
            return mHeap.size(); 
        }

        /** 
         * Whether the EventQueue is empty or not.
         * 
         * This function checks if the EventQueue is empty by checking if the underlying vector is empty.
         * 
         * @return This will return true if the EventQueue is empty and false otherwise.
         */
        [[nodiscard]] bool Empty() const
        {
            return mHeap.empty(); 
        }

        /**
         * Clear all events from the EventQueue.
         * 
         * This function clears the EventQueue by clearing the underlying vector and resetting the insertion index.
         */
        void Clear()
        {
            mHeap.clear();
            mInsertionIndex = 0;
        }
    };
}