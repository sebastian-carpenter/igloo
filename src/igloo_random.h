#ifndef IGLOO_RANDOM
#define IGLOO_RANDOM

#include <stdint.h>

/* PRNG (xorshiro256_pp) implementation copied from
 * https://en.wikipedia.org/wiki/Xorshift#xoshiro256++
 *
 * seed (splitmix64) implementation copied from
 * https://en.wikipedia.org/wiki/Xorshift#Initialization
 */

struct splitmix64_state {
    uint64_t s;
};

struct xoshiro256pp_state {
    uint64_t s[4];
};

/* seed provided to state->s before calling
 * return used as init for shiro state */
uint64_t splitmix64(struct splitmix64_state *state);

uint64_t rol64(uint64_t x, int k);

/* retrieve 64 bit values per call
 *  lower 32 bits are considered 'low entropy' so use upper 32 */
void xoshiro256pp_init(struct xoshiro256pp_state *state, uint64_t seed);
uint64_t xoshiro256pp(struct xoshiro256pp_state *state);

#endif
