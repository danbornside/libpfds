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

/** inefficient implementation of libpfds interfaces based on c arrays.
 *
 * This types and methods in this module are mostly intended for illustration
 * and testing; most users will not need to use this header.
 */

#ifndef PFDS_ARRAYLIST_HEADER_INCLUDED
#define PFDS_ARRAYLIST_HEADER_INCLUDED

#include <stdbool.h>

#include "pfds-object.h"
#include "pfds-sequence.h"

/** a simple, generally suboptimal implementation of a sequential container
 * implemented as a simple array of pointers. */
typedef struct pfds_ArrayList pfds_ArrayList;

extern const pfds_objectvtable pfds_ArrayList_vtable;

/** return a new, empty ArrayList
 *
 * \invariant give(return)
 * */
pfds_ArrayList* pfds_ArrayList_empty();

/** return a new ArrayList with one element
 *
 * \param elem
 * \invariant give(return) take(elem)
 */
pfds_ArrayList* pfds_ArrayList_singleton(pfds_object* elem);


/** append one element to an ArrayList
 *
 * \param init
 * \param last
 * \invariant give(return) take(init) take(last)
 */
pfds_ArrayList* pfds_ArrayList_pushBack(pfds_ArrayList* init, pfds_object* last);


/** concatenate two ArrayLists
 *
 * \param l
 * \param r
 * \invariant give(return) take(l) take (r)
 */
pfds_ArrayList* pfds_ArrayList_mappend(pfds_ArrayList* l, pfds_ArrayList* r);

/** construct a new ArrayList from an array of the given objects.
 *
 * \param size
 * \param elements
 * \invariant give(return) take(xs[0..n])
 */
pfds_ArrayList* pfds_ArrayList_fromArray(size_t size, pfds_object** elements);


/** construct a new ArrayList from an array of the given objects.
 *
 *
 * \param size
 * \param elements
 * \param owner the array supplied is allocated for the use of the new array.
 * \invariant give(return) take(xs[0..n])
 */
pfds_ArrayList* pfds_ArrayList_fromArrayEx(size_t size, pfds_object* elements[], bool owner);

pfds_ordering pfds_ArrayList_cmp(pfds_ArrayList* l, pfds_ArrayList* r);

/** construct a new sequence from an ArrayList
 *
 * \param dict
 * \param elements
 * \invariant give(return) take(lst)
 */
pfds_sequence* pfds_sequence_fromArrayList(const pfds_sequencevtable* dict, pfds_ArrayList* elements);

#endif
