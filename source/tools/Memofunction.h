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
#include <type_traits>
#include <ostream>
#include <cstddef>


template <typename Key, typename Func>

class Memofunction
{
public:
    //Generic type to help with the wrapping 
    using result_type = std::invoke_result_t<Func&, const Key&>;  /// This helps intilaize my cache

private:

    std::unordered_map<Key, result_type> cache; // container
    Func mfunc; /// saves function
    std::size_t capacity = 3; // just 3 slots for now
    std::queue<Key> order; // helps in FIFO system
public:

    /** 
    *@brief Creates Memofunction around a function (as of right just a int(int type of functions))
    *@param func Function to memorize
    * Starts empty and has fixed capacity
    */
    Memofunction( Func func); ///< Constructor

    /**
     * @brief Compute f(x) using the cache when possible.
     * @param x Input to the wrapped function.
     * @return f(x), either from cache (hit) or newly computed (miss).
     */
    result_type operator()(const Key& x); ///< Call wrapper (this is what lets you do: memo(5); )


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

    // /**
    //  * @brief Helps visual container and help with debugging purposes
    //  * @param os stream to print out the keys AND values , among other stuff
    //  */

    // void Container(std::ostream& os) const;

};
#ifndef MEMOFUNCTION_IMPL_INCLUDED
#define MEMOFUNCTION_IMPL_INCLUDED
#include "Memofunction.cpp"
#endif

#endif //CAPSTONE_MEMOFUNCTION_H