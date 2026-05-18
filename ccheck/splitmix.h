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

#ifndef SPLITMAX_HEADER_DEFINED
#define SPLITMAX_HEADER_DEFINED

/** pure C implementation of SplitMix.

This library offers a pure c implementation of the SplitMix algorithm of
PseudoRandom number generators.

> [The] algorithm SPLITMIX for an object- oriented and splittable pseudorandom
> number generator (PRNG) that is quite fast: 9 64-bit arithmetic/logical opera-
> tions per 64 bits generated. A conventional linear PRNG ob- ject provides a
> generate method that returns one pseudoran- dom value and updates the state of
> the PRNG, but a splittable PRNG object also has a second operation, split,
> that replaces the original PRNG object with two (seemingly) independent PRNG
> objects, by creating and returning a new such object and updating the state of
> the original object. Splittable PRNG objects make it easy to organize the use
> of pseudorandom numbers in multithreaded programs structured using fork- join
> parallelism. No locking or synchronization is required (other than the usual
> memory fence immediately after ob- ject creation).
    -- [Steel, Lea, Flood ;2014]

The PRNG emphasizes determinism even when used in a parallel processing
context, and is emphatically not cryptographically secure, even if used with
secure seeds.  If you need a PRNG for a cryptographic application, look
elsewhere.

 */


#include <stdint.h>

/// random generator state.
typedef struct SplitMix64 {
    int64_t seed;
    int64_t gamma;
    int64_t nextSplit;
} SplitMix64;


/** Set the global default seed value.
 *
 * This function should be called only once before any invocation of
 * SplitMix64_initDefault.  if not called, this library will behave as if
 * `SplitMix64_setInitialSeed(0)` had been called.
 */
extern void SplitMix64_setInitialSeed(uint64_t seed);

/** Set the global default seed value.
 *
 * This function should be called only once before any invocation of
 * SplitMix64_initDefault.  if not called, this library will behave as if
 * `SplitMix64_setInitialSeed(0)` had been called.
 */
extern void SplitMix64_initialSeed();

/** initialize a random generator with the default seed
 *
 * @param self pointer to a region of memmory where the generator will be set up.
 */
extern void SplitMix64_initDefault(SplitMix64* self);

/** initialize a random generator with the given seed value)
 *
 * @param self pointer to a region of memmory where the generator will be set up.
 * @param seed the initial seed value for the generator.
 */
extern void SplitMix64_init(SplitMix64* self, int64_t seed);

/** split a random generator into two that may be used in parallel.
 *
 * @param dest a new uninitialized region of memory for the child generator.
 * @param src a valid generator to act as parent of the split.
 */
extern void SplitMix64_split(SplitMix64* dest, SplitMix64* src);

#define SplitMix64_nextLong(gen) ((long) SplitMix64_nextInt64(gen))
#define SplitMix64_nextInt(gen) ((int) SplitMix64_nextInt64(gen))
#define SplitMix64_nextLongRange(gen, lb, ub) ((long) SplitMix64_nextInt64Range(gen, lb, ub))
#define SplitMix64_nextIntRange(gen, lb, ub) ((int) SplitMix64_nextInt64Range(gen, lb, ub))

/** generate a Pseudorandom 64bit value */
extern int64_t SplitMix64_nextInt64(SplitMix64* self);

/** generate a Pseudorandom double value in the range [0,1) */
extern double SplitMix64_nextDouble(SplitMix64* self);

/** generate a Pseudorandom double value in the range [lb, ub) */
extern double SplitMix64_nextDoubleRange(SplitMix64* self, double lb, double ub);

/** generate a Pseudorandom int value in the range [lb, ub) */
extern int64_t SplitMix64_nextInt64Range(SplitMix64* self, int64_t lb, int64_t ub);

#endif
