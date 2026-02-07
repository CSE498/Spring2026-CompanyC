/**
 * @file Memofunction.cpp
 * @author Jose Antonio Hernandez- Martinez
 */

#include "Memofunction.h"
#include <cassert>
#include <iostream>


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
            return pair.second; // cache hit
            std::cout << " Hit " << std::endl;
        }
        else{
            std::cout << " Miss  " << std::endl;
        }
    }
    /// If not a cache hit
    if (cache.size() >= capacity)
    {
        std::cout << " Full" << std::endl;
        int oldestKey = order.front(); // stemp stopes the kley to
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
    return cache.size();
}