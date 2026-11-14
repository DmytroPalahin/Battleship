/* SPDX-License-Identifier: MIT */
#include "battleship/rng.h"
#include "bs_test.h"

/*
 * Reference outputs of xoshiro256** seeded through SplitMix64, cross-checked
 * against an independent implementation. Pinning them guarantees that a seed
 * replays the same game on every platform and in every future version.
 */
TEST(matches_reference_sequence)
{
    static const uint64_t expected_seed_0[] = {
        UINT64_C(0x99EC5F36CB75F2B4), UINT64_C(0xBF6E1F784956452A), UINT64_C(0x1A5F849D4933E6E0),
        UINT64_C(0x6AA594F1262D2D2C)};
    static const uint64_t expected_seed_42[] = {
        UINT64_C(0x15780B2E0C2EC716), UINT64_C(0x6104D9866D113A7E), UINT64_C(0xAE17533239E499A1),
        UINT64_C(0xECB8AD4703B360A1)};
    bs_rng rng;

    bs_rng_seed(&rng, 0);
    for (size_t i = 0; i < 4; i++) {
        CHECK(bs_rng_next(&rng) == expected_seed_0[i]);
    }
    bs_rng_seed(&rng, 42);
    for (size_t i = 0; i < 4; i++) {
        CHECK(bs_rng_next(&rng) == expected_seed_42[i]);
    }
}

TEST(same_seed_gives_same_sequence)
{
    bs_rng a;
    bs_rng b;
    bs_rng_seed(&a, 2023);
    bs_rng_seed(&b, 2023);
    for (int i = 0; i < 1000; i++) {
        CHECK(bs_rng_next(&a) == bs_rng_next(&b));
    }
}

TEST(different_seeds_diverge)
{
    bs_rng a;
    bs_rng b;
    bs_rng_seed(&a, 1);
    bs_rng_seed(&b, 2);
    int equal = 0;
    for (int i = 0; i < 100; i++) {
        equal += bs_rng_next(&a) == bs_rng_next(&b);
    }
    CHECK_EQ(0, equal);
}

TEST(below_stays_in_range)
{
    static const uint32_t bounds[] = {1, 2, 3, 7, 10, 100, 200, 1000003, UINT32_MAX};
    bs_rng rng;
    bs_rng_seed(&rng, 7);

    for (size_t b = 0; b < sizeof bounds / sizeof bounds[0]; b++) {
        for (int i = 0; i < 2000; i++) {
            CHECK(bs_rng_below(&rng, bounds[b]) < bounds[b]);
        }
    }
    CHECK_EQ(0, bs_rng_below(&rng, 1));
}

TEST(below_tolerates_degenerate_input)
{
    bs_rng rng;
    bs_rng_seed(&rng, 7);
    CHECK_EQ(0, bs_rng_below(&rng, 0));
    CHECK_EQ(0, bs_rng_below(NULL, 10));
    bs_rng_seed(NULL, 1); /* must not crash */
}

/* Every face of a die should come up about one time in six. */
TEST(below_is_roughly_uniform)
{
    enum {
        FACES = 6,
        DRAWS = 60000
    };
    int histogram[FACES] = {0};
    bs_rng rng;
    bs_rng_seed(&rng, 12345);

    for (int i = 0; i < DRAWS; i++) {
        histogram[bs_rng_below(&rng, FACES)]++;
    }
    for (int face = 0; face < FACES; face++) {
        const int expected = DRAWS / FACES;
        CHECK(histogram[face] > expected * 95 / 100);
        CHECK(histogram[face] < expected * 105 / 100);
    }
}

static const bs_test_case TESTS[] = {
    BS_CASE(matches_reference_sequence),       BS_CASE(same_seed_gives_same_sequence),
    BS_CASE(different_seeds_diverge),          BS_CASE(below_stays_in_range),
    BS_CASE(below_tolerates_degenerate_input), BS_CASE(below_is_roughly_uniform),
};

BS_TEST_MAIN(TESTS)
