/**
 * @file Memofunction.h
 * @author Jose Antonio Hernandez- Martinez
 *
 *
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

    /// Constructor
    Memofunction( std::function <int(int)> func);

    /// Call wrapper (this is what lets you do: memo(5); )
    int operator()(int x);

    /// helpers
    void Clear();
    std::size_t Size() const;

};


#endif //CAPSTONE_MEMOFUNCTION_H