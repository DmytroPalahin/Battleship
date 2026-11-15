/* SPDX-License-Identifier: MIT */
/**
 * @file rng.h
 * @brief Small, seedable pseudo-random number generator.
 *
 * The engine never calls rand(): every source of randomness is an explicit
 * bs_rng owned by the caller. A given seed therefore reproduces a whole game,
 * which is what makes the test-suite and the benchmarks deterministic.
 *
 * Algorithm: xoshiro256** (Blackman & Vigna), seeded through SplitMix64.
 * Not suitable for cryptography.
 */
#ifndef BATTLESHIP_RNG_H
#define BATTLESHIP_RNG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Generator state. Treat as opaque; initialise with bs_rng_seed(). */
typedef struct bs_rng {
    uint64_t s[4]; /**< The 256 bits of state. */
} bs_rng;

/** Initialises the generator. Any seed is valid, including zero. */
void bs_rng_seed(bs_rng *rng, uint64_t seed);

/** Next 64 uniformly distributed bits. */
uint64_t bs_rng_next(bs_rng *rng);

/**
 * Uniform integer in [0, bound), free of modulo bias.
 * @param rng   generator to draw from.
 * @param bound must be greater than zero; a bound of zero yields zero.
 */
uint32_t bs_rng_below(bs_rng *rng, uint32_t bound);

#ifdef __cplusplus
}
#endif

#endif /* BATTLESHIP_RNG_H */
