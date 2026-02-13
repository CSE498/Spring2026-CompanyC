/**
 * @file Random.h
 * @author Benjamin Forbes
 */

#ifndef RANDOM_H
#define RANDOM_H

#pragma once

#include <cstdint>
#include <random>

class Random {
public:
    Random();
    // Prefer this constructor when deterministic behavior is required.
    explicit Random(uint64_t seed);

    void Seed(uint64_t seed);

    double GetDouble(double min, double max);
    int GetInt(int min, int max);
    bool P(double probability);

private:
    // Mersenne Twister provides fast, high-quality pseudo-random numbers
    // suitable for simulation and testing.
    using Engine = std::mt19937;

    Engine mEngine;
    bool mSeeded = false;
};



#endif //RANDOM_H
