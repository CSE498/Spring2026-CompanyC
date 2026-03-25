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
#include <utility>

namespace cse498
{

    template <typename Key, typename Func>
    class MemoFunction
    {
    public:
        // Generic type to help with the wrapping
        using result_type = std::invoke_result_t<Func &, const Key &>; /// This helps initialize my cache

    private:
        static constexpr std::size_t DEFAULT_CAPACITY = 3;

        std::unordered_map<Key, result_type> cache; // container
        Func mfunc;                                 /// saves function
        std::size_t capacity = DEFAULT_CAPACITY;    // just 3 slots for now
        std::queue<Key> eviction_queue;             // helps in FIFO system
        std::size_t hit_count = 0;                  /// hit counter
        std::size_t miss_count = 0;                 /// miss counter

    public:
        /**
         *@brief Creates Memofunction around a function (as of right just a int(int type of functions))
         *@param func Function to memorize
         * Starts empty and has fixed capacity
         */
        explicit MemoFunction(Func func)
            : mfunc(std::move(func))
        {
        }

        /**
         *@brief Creates Memofunction around a function with a custom capacity
         *@param func Function to memorize
         *@param cap Max cache size
         */
        explicit MemoFunction(Func func, std::size_t cap)
            : mfunc(std::move(func)), capacity(cap)
        {
        }

        /**
         * @brief Compute f(x) using the cache when possible.
         * @param x Input to the wrapped function.
         * @return f(x), either from cache (hit) or newly computed (miss).
         */
        result_type operator()(const Key &x)
        {
            auto it = cache.find(x);
            if (it != cache.end())
            {
                ++hit_count;
                return it->second; // cache hit
            }
            ++miss_count;
            if (capacity == 0)
            {
                return mfunc(x);
            }

            if (cache.size() >= capacity)
            {
                if (!eviction_queue.empty())
                {
                    Key oldest = eviction_queue.front();
                    eviction_queue.pop();
                    cache.erase(oldest);
                }
            }

            result_type result = mfunc(x);
            cache.emplace(x, result);
            eviction_queue.push(x);
            return result;
        }

        /*HELPERS - more to come */
        /**
         * @brief Clear all cached values and reset insertion order.
         */
        void clear()
        {
            cache.clear();
            std::queue<Key> empty;
            eviction_queue.swap(empty);
        }

        /**
         * @brief Current number of cached entries.
         * @return Number of keys stored in the cache.
         */
        std::size_t size() const
        {
            return cache.size(); /// gets size of cache
        }

        void set_capacity(std::size_t cap)
        {
            capacity = cap;

            if (capacity == 0)
            {
                clear();
                return;
            }

            while (cache.size() > capacity && !eviction_queue.empty())
            {
                Key oldest = eviction_queue.front();
                eviction_queue.pop();
                cache.erase(oldest);
            }
        }

        /**
         *  @brief Get the current cache capacity.
         * @return Maximum number of entries the cache can hold.
         */
        std::size_t get_capacity() const
        {
            return capacity;
        }

        std::size_t get_hits() const
        {
            return hit_count;
        }

        std::size_t get_misses() const
        {
            return miss_count;
        }

        double hit_rate() const
        {
            std::size_t total = hit_count + miss_count;
            if (total == 0)
            {
                return 0.0;
            }
            return static_cast<double>(hit_count) / total;
        }
    };

    template <typename Key, typename Func>
    using Memofunction = MemoFunction<Key, Func>;

}

#endif // CAPSTONE_MEMOFUNCTION_H