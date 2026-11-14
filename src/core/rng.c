/* SPDX-License-Identifier: MIT */
#include "battleship/rng.h"

#include <stddef.h>

/* SplitMix64: spreads a 64-bit seed over the 256 bits of xoshiro state. */
static uint64_t splitmix64(uint64_t *state)
{
    uint64_t z = (*state += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

static uint64_t rotl(uint64_t x, unsigned k)
{
    return (x << k) | (x >> (64U - k));
}

void bs_rng_seed(bs_rng *rng, uint64_t seed)
{
    if (rng == NULL) {
        return;
    }
    for (size_t i = 0; i < sizeof rng->s / sizeof rng->s[0]; i++) {
        rng->s[i] = splitmix64(&seed);
    }
}

/* xoshiro256** 1.0 */
uint64_t bs_rng_next(bs_rng *rng)
{
    uint64_t *s = rng->s;
    const uint64_t result = rotl(s[1] * 5U, 7) * 9U;
    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];
    s[2] ^= t;
    s[3] = rotl(s[3], 45);

    return result;
}

static uint32_t next_u32(bs_rng *rng)
{
    return (uint32_t)(bs_rng_next(rng) >> 32);
}

/*
 * Lemire's multiply-and-reject method. The naive `next() % bound` favours
 * small values whenever bound does not divide 2^32; here the few products
 * that would cause the bias are rejected and redrawn.
 */
uint32_t bs_rng_below(bs_rng *rng, uint32_t bound)
{
    if (rng == NULL || bound == 0U) {
        return 0U;
    }

    uint64_t product = (uint64_t)next_u32(rng) * bound;
    uint32_t low = (uint32_t)product;
    if (low < bound) {
        const uint32_t threshold = (0U - bound) % bound;
        while (low < threshold) {
            product = (uint64_t)next_u32(rng) * bound;
            low = (uint32_t)product;
        }
    }
    return (uint32_t)(product >> 32);
}
