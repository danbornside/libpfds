/* This file is part of libpfds, Persistent Functional Data Structures in C.
 *
 * libpfds is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * libpfds is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * libpfds. If not, see <https://www.gnu.org/licenses/>.
 */

#include "splitmix.h"

#include <stdint.h>
#include <stdio.h>
#include <stdatomic.h>
#include <unistd.h>

static const int64_t GAMMA_PRIME = (1L << 56) - 5; // "Percy"
static const int64_t GAMMA_GAMMA = 0x00281E2DBA6606F3L;
static const int64_t DEFAULT_SEED_GAMMA = 0xBD24B73A95FB84D9L;
static const double DOUBLE_ULP = 1.0 / (1L << 53);

static const int64_t DEFAULT_GEN_UNINITIALIZED = 0L;
volatile static atomic_long defaultGen = 0L;

// shift right unsigned
#define LSR(x, y) ((uint64_t) x >> y)



extern void SplitMix64_initialSeed() {
    int64_t seed;
    if(getentropy(&seed, sizeof(int64_t))) {
        perror("SplitMix64_initialSeed() failed to get enough entropy");
    }
    SplitMix64_setInitialSeed(seed);
}
extern void SplitMix64_setInitialSeed(uint64_t seed) {
    uint64_t expected = DEFAULT_GEN_UNINITIALIZED;
    if(!atomic_compare_exchange_strong(&defaultGen, &expected, seed)) {
        perror("SplitMix64 default seed already used!\n");
    }
}


int64_t update(int64_t s, int64_t g) {
    // Add g to s modulo George.
    int64_t p = s + g;
    return (p >= s) ? p
        : (p >= 0x800000000000000DL) ? p - 13L
        : (p - 13L) + g;
}
int64_t mix64(int64_t z) {
    z = (z ^ LSR(z, 33)) * 0xff51afd7ed558ccdL;
    z = (z ^ LSR(z, 33)) * 0xc4ceb9fe1a85ec53L;
    return z ^ LSR(z, 33);
}
int64_t mix56(int64_t z) {
    z = ((z ^ LSR(z, 33)) * 0xff51afd7ed558ccdL) & 0x00FFFFFFFFFFFFFFL;
    z = ((z ^ LSR(z, 33)) * 0xc4ceb9fe1a85ec53L) & 0x00FFFFFFFFFFFFFFL;
    return z ^ LSR(z, 33);
}
int64_t nextRaw64(SplitMix64 * self) {
    return (self->seed = update(self->seed, self->gamma));
}

void SplitMix64_initImpl(SplitMix64 * self, int64_t seed, int64_t s) {
    // We require 0 <= s < Percy
    self->seed = seed;
    s += GAMMA_GAMMA;
    if (s >= GAMMA_PRIME) s -= GAMMA_PRIME;
    self->gamma = mix56(s) + 13;
    self->nextSplit = s;
}

int64_t nextDefaultSeed() {
    int64_t p, q;
    do {
        p = defaultGen;
        q = update(p, DEFAULT_SEED_GAMMA);
    } while(!atomic_compare_exchange_weak(&defaultGen, &p, q));
    return mix64(q);
}

extern void SplitMix64_init(SplitMix64* self, int64_t seed) {
    SplitMix64_initImpl(self, seed, 0);
}

extern void SplitMix64_initDefault(SplitMix64* self) {
    SplitMix64_initImpl(self, nextDefaultSeed(), GAMMA_GAMMA);
}

extern void SplitMix64_split(SplitMix64* dest, SplitMix64* src) {
    SplitMix64_initImpl(dest, nextRaw64(src), src->nextSplit);
}


extern int64_t SplitMix64_nextInt64(SplitMix64* self) {
    return mix64(nextRaw64(self));
}

extern int64_t SplitMix64_nextInt64Range(SplitMix64* self, int64_t lb, int64_t ub) {
    uint64_t x = (uint64_t) SplitMix64_nextInt64(self);
    int64_t y = x % (uint64_t) (ub - lb);
    y += lb;
    return y;
}

extern double SplitMix64_nextDouble(SplitMix64* self) {
    return LSR((uint64_t) SplitMix64_nextInt64(self), 11) * DOUBLE_ULP;
}

extern double SplitMix64_nextDoubleRange(SplitMix64* self, double lb, double ub) {
    double x = SplitMix64_nextDouble(self);
    x *= (ub - lb);
    x += lb;
    return x;
}

