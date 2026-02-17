/**
 * @file Memofunction.cpp
 * @author Jose Antonio Hernandez- Martinez
 * @brief This Memofunction class is a function wrapper that simulates like a cache.
 *
 */
#include "Memofunction.h"
#include <cassert>
#include <iostream>
#include <functional>
#include <utility>
/**
 * @important
 * IMPROVEMENTS./a
 * - instead of std::queue, I can make a double linked list
 *      + DLL + map lets you evict/move/remove a key in O(1).
 *      + remove/update a slot in the middle
 * - unordered_map::find() makes lookup average O(1) vs iterating O(n).
 * - pass capacity into the constructor instead of hard-coding 3.
 * - Generalize to templates so it can memoize other types
 * - helper functions can be used
 *      + optional stats cache hits, misses, evictions (for debugging/performance insight).
 *      + sometype fo visual that helps understand what its containing
 * - Can switch from LRU to FIFO??!!!
 */

 namespace cse498{
    template <typename Key, typename Func>
    Memofunction<Key, Func>::Memofunction(Func func)
        : mfunc(std::move(func))
    {
        // assert(mfunc && "Memofunction: wrapped function must be valid");
    }

    template <typename Key, typename Func>
    typename Memofunction<Key, Func>::result_type
    Memofunction<Key, Func>::operator()(const Key &x)
    {
        /* Loops to check for cache hit */
        for (const auto &pair : cache)
        {
            if (pair.first == x)
            {
                // std::cout << " Hit " << std::endl;
                return pair.second; // cache hit
            }
            // else
            // {
            //     std::cout << " Miss  " << std::endl;
            // }
        }
        /// If not a cache hit
        if (cache.size() >= capacity)
        {
            // std::cout << " Full" << std::endl;
            Key oldest = order.front(); // temp stores the key to variable
            order.pop();                // rmeoves the fron(oldest key)
            cache.erase(oldest);        // erases the pair from cache
        }

        result_type result = mfunc(x);
        cache[x] = result; //  we WANT to insert/update
        order.push(x);
        return result;
    }

    template <typename Key, typename Func>
    void Memofunction<Key, Func>::Clear()
    {
        cache.clear();
        while (!order.empty())
            order.pop();
    }

    template <typename Key, typename Func>
    std::size_t Memofunction<Key, Func>::Size() const
    {
        return cache.size(); /// gets size of cache
    }
 }