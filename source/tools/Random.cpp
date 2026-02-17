/**
 * @file Random.cpp
 * @author Benjamin Forbes
 *
 * CITATION: LLM use for syntax, behavior is according to specifications.
 */

#include "Random.h"

#include <cassert>
#include <limits>


/**
 * Default constructor intentionally does not seed the engine.
 * Forces callers to explicitly control determinism.
 */
Random::Random() : mEngine(), mSeeded(false) {}

/**
 * Constructs a seeded generator to provide deterministic random sequences.
 * Preferred for tests, simulations, and any scenario requiring reproducibility.
 */
Random::Random(uint64_t seed) : mEngine(static_cast<Engine::result_type>(seed)), mSeeded(true) {}

/**
 * Reseeds the engine and marks the generator as ready for use.
 * Required before any random values are requested
 */
void Random::Seed(uint64_t seed)
{
    mEngine.seed(static_cast<Engine::result_type>(seed));
    mSeeded = true;
}

double Random::GetDouble(double min, double max)
{
    assert(mSeeded && "Random generator must be seeded before use");
    assert(min <= max && "GetDouble: invalid range (min > max)");

    // uniform_real_distribution typically produces values in [min, max).
    // Half-open ranges are preferred in simulations to avoid boundary overlap.
    std::uniform_real_distribution<double> dist(min, max);
    return dist(mEngine);
}

int Random::GetInt(int min, int max)
{
    assert(mSeeded && "Random generator must be seeded before use");
    assert(min <= max && "GetInt: invalid range (min > max)");

    // uniform_int_distribution avoids modulo bias.
    std::uniform_int_distribution<int> dist(min, max);
    return dist(mEngine);
}

bool Random::P(double probability)
{
    assert(mSeeded && "Random generator must be seeded before use");
    assert(probability >= 0.0 && probability <= 1.0 && "P: probability must be in [0.0, 1.0]");

    // Bernoulli distribution maps directly to a probability check.
    std::bernoulli_distribution dist(probability);
    return dist(mEngine);
}

