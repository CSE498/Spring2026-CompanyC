/**
 * @file Memofunction.cpp
 * @author Jose Antonio Hernandez- Martinez
 * @brief This Memofunction class is a function wrapper that simulates like a cache. 
 * 
 */

#include "Memofunction.h"
#include <cassert>
#include <iostream>



/**
 * @important
 * IMPROVEMENTS 
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

Memofunction::Memofunction(std::function<int(int)> func)
    : mfunc(std::move(func))
{
    assert(mfunc && "Memofunction: wrapped function must be valid");
}

int Memofunction::operator()(int x)
{
    /* Loops to check for cache hit */
    for (const auto& pair : cache)
    {
        if (pair.first == x)
        {
            std::cout << " Hit " << std::endl;
            return pair.second; // cache hit
            
        }
        else{
            std::cout << " Miss  " << std::endl;
        }
    }
    /// If not a cache hit
    if (cache.size() >= capacity)
    {
        std::cout << " Full" << std::endl;
        int oldestKey = order.front(); // temp stores the key to variable
        order.pop(); //rmeoves the fron(oldest key)
        cache.erase(oldestKey); //erases the pair from cache
    }

    int result = mfunc(x);
    cache[x] = result;     // OK here because we WANT to insert/update
    order.push(x);
    return result;
}

void Memofunction::Clear()
{
    ///More
    cache.clear();
    while (!order.empty()) order.pop();
}

std::size_t Memofunction::Size() const
{
    return cache.size(); ///gets size of cache
}