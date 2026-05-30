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

#ifndef PFDS_MISC_HEADER_INCLUDED
#define PFDS_MISC_HEADER_INCLUDED

#include <assert.h>
#include <stdlib.h>

/* miscelaneious macros used throughout the implementation of pfds that are inappropriate to install globally */

/* assign to an outparameter pointer if it is not null and perform a fallback otherwise  */
#define OPTIONAL_OUTPARAM2(var, val, garbage) if (var != NULL) { *var = val; } else { garbage ; }

/* assign to an outparameter pointer if it is not null  */
#define OPTIONAL_OUTPARAM(var, val) if (var != NULL) { *var = val; }

#define panic(msg) { fprintf(stderr, "PANIC %s:%d\n\t%s\n", __FILE__, __LINE__, msg); abort(); }

/* alternative version of bsearch that always returns a pointer into the search list.
 *
 * rather than a compare function, bsearch_alt takes a monotone comparison
 * function, which must satisfy:
 *  -     forall i,j:[0..n). i < j && pred(base[i]) => pred(base[j])
 *  - and forall i,j:[0..n). i < j && !pred(base[j]) => !pred(base[i])
 *
 * that is it must switch from false to true at one point in the list and stay
 * true for all subsequent elements.
 *
 * to recover the behavior of bsearch(3), use a predicate like the following:
 *
 * > bool myPred(void* l, void* r) {
 * >     return myCompare(l, r) >= 0;
 * > }
 *
 * Also, users must beware that this function can return a result "past the
 * end" of the search list, if the predicate is false for the whole search list
 *
 * @param userData an arbitrary value passed to pred's first argument
 * @param base pointer to first entry in the search list
 * @param n number of elements in search list
 * @param size size of each element
 * @param pred a monotonic predicate
 * @returns a pointer to the first element for which pred is true.
 */
void* bsearch_alt(const void *userData, const void *base, size_t n, size_t size, bool (*pred)(const void*, const void*));

#endif
