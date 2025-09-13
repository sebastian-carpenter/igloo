#include <stdint.h>

#include "igloo_random.h"

/* PRNG (xorshiro256_pp) implementation copied from
 * https://en.wikipedia.org/wiki/Xorshift#xoshiro256++
 *
 * seed (splitmix64) implementation copied from
 * https://en.wikipedia.org/wiki/Xorshift#Initialization
 */

uint64_t splitmix64(struct splitmix64_state *state)
{
    uint64_t result = (state->s += 0x9E3779B97F4A7C15);
    result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
    result = (result ^ (result >> 27)) * 0x94D049BB133111EB;
    return result ^ (result >> 31);
}

uint64_t rol64(uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

void xoshiro256pp_init(struct xoshiro256pp_state *state, uint64_t seed)
{
    struct splitmix64_state smstate = {seed};

    state->s[0] = splitmix64(&smstate);
    state->s[1] = splitmix64(&smstate);
    state->s[2] = splitmix64(&smstate);
    state->s[3] = splitmix64(&smstate);
}

uint64_t xoshiro256pp(struct xoshiro256pp_state *state)
{
    uint64_t *s = state->s;
    uint64_t const result = rol64(s[0] + s[3], 23) + s[0];
    uint64_t const t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = rol64(s[3], 45);

    return result;
}
