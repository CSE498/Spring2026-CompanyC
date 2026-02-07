/**
 * @file Memofunction.h
 * @author Jose Antonio Hernandez- Martinez
 * @brief This Memofunction class is a wraps a function and acts like a cache. 
 * It has a FIFO eviction policy and fixed slot size.
 */

#ifndef CAPSTONE_MEMOFUNCTION_H
#define CAPSTONE_MEMOFUNCTION_H



#include <unordered_map>
#include <functional>
#include <queue>

class Memofunction
{
private:

    std::unordered_map<int, int> cache; // container
    std::function<int(int)> mfunc; /// saves function
    size_t capacity = 3; // just 3 slots for now
    std::queue<int> order; // helps in FIFO system
public:

    /** 
    *@brief Creates Memofunction around a function (as of right just a int(int type of functions))
    *@param func Function to memorize
    * Starts empty and has fixed capacity
    */
    Memofunction( std::function <int(int)> func); ///< Constructor

    /**
     * @brief Compute f(x) using the cache when possible.
     * @param x Input to the wrapped function.
     * @return f(x), either from cache (hit) or newly computed (miss).
     */
    int operator()(int x); ///< Call wrapper (this is what lets you do: memo(5); )


    /*HELPERS - more to come */
    /**
     * @brief Clear all cached values and reset insertion order.
     */
    void Clear();
    
    /**
     * @brief Current number of cached entries.
     * @return Number of keys stored in the cache.
     */
    std::size_t Size() const;

};


#endif //CAPSTONE_MEMOFUNCTION_H