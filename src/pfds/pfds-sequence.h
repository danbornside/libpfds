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

#ifndef PFDS_SEQUENCE_HEADER_INCLUDED
#define PFDS_SEQUENCE_HEADER_INCLUDED

#include <stdbool.h>

#include "pfds-object.h"
#include "pfds-functions.h"

typedef struct pfds_sequence {
    pfds_object object;
}
pfds_sequence;

struct pfds_sequencevtable {
    /** a sequence is required to be a monoid in a standard way. */
    const pfds_catenablevtable *catenable;

    /** construct a new sequence of the selected type from an array of the given objects.
     *
     * \param n
     * \param xs
     * \invariant give(return) take(xs[0..n])
     */
    pfds_sequence* (*fromArray)(size_t, pfds_object**);

    /** construct a new sequence of one element
     *
     * \param x
     * \invariant give(return) take(x)
     */
    pfds_sequence* (*singleton)(pfds_object*);

    /** test if a sequence has elements
     *
     * \param self
     * \return true if self is the empty sequence, false otherwise.
     * \invariant borrow(self)
     *
     */
    bool (*isEmpty)(pfds_sequence*);

    /** return the number of elementst in a sequence
     *
     * \param self
     * \return number of elements in self
     * \invariant borrow(self)
     */
    size_t (*size)(pfds_sequence*);

    /** split sequence into first element and remaing sequence. outputs NULL and returns false on empty sequence
     *
     * \param head[out] first element of a sequence
     * \param tail[out] remaining elements of a sequence
     * \param self[in] sequence to be examined.
     * \return true if there was an element to be popped.
     * \invariant when(return == true) give(head) give(tail) take(self)
     *      | when(return == false) borrow(self)
     */
    bool (*popFront)(pfds_object**, pfds_sequence**, pfds_sequence*);
    /** split sequence into intial sequence and last element.  outputs NULL and returns false on empty sequence
     *
     * \param init[out] remaining elements of a sequence
     * \param last[out] first element of a sequence
     * \param self[in] sequence to be examined.
     * \return true if there was an element to be popped.
     * \invariant when(return == true) give(init) give(last) take(self)
     *      | when(return == false) borrow(self)
     */
    bool (*popBack)(pfds_sequence**, pfds_object**, pfds_sequence*);

    /** split sequence into intial sequence, selected element, and remaining sequence.  outputs NULL and returns false if sequence does not contain selected element
     *
     * \param init[out] first n-1 elements
     * \param link[out] nth element
     * \param last[out] n+1 thru last elements
     * \param self[in] sequence to be examined.
     * \return true if sequence is large enough to contain nth element.
     * \invariant when(return == true) give(init) give(link) give(tail) take(self)
     *      | when(return == false) borrow(self)
     */
    bool (*split)(pfds_sequence**, pfds_object**, pfds_sequence**, pfds_sequence*, size_t);

    /** add an element to the front of a sequence.
     *
     * \param head
     * \param tail
     * \invariant give(return) take(head) take(tail)
     */
    pfds_sequence* (*pushFront)(pfds_object* head, pfds_sequence* tail);

    /** add an element to the back of a sequence.
     *
     * \param last
     * \param init
     * \invariant give(return) take(init) take(last)
     */
    pfds_sequence* (*pushBack)(pfds_sequence* init, pfds_object* last);

    /** return the nth element or NULL if the container does not contain enough elements
     *
     * \param self
     * \param n
     * \returns the nth element of self if it exists, NULL otherwise
     * \invariant when(return != NULL) lend(return, self)
     *
     */
    pfds_object* (*get)(pfds_sequence* self, size_t n);

    /** add an element x in position n.
     *
     * \param self
     * \param n
     * \param x
     * \returns a copy of the sequence with the element added at the requested location.
     * \invariant give(return) take(self) (take x)
     *
     */
    pfds_sequence* (*insertBefore)(pfds_sequence* self, size_t n, pfds_object* x);

    /** add an element x immediately after the element in position n.
     *
     * \param self
     * \param n
     * \param x
     * \returns a copy of the sequence with the element added at the requested location.
     * \invariant give(return) take(self) (take x)
     *
     */
    pfds_sequence* (*insertAfter)(pfds_sequence* self, size_t n, pfds_object* x);

    /** replace the element in position n by x.
     *
     * \param self
     * \param n
     * \param x
     * \returns a copy of the sequence with the element added at the requested location.
     * \invariant give(return) take(self) (take x)
     *
     */
    pfds_sequence* (*updateAt)(pfds_sequence* self, size_t n, pfds_object* x);

    /** remove the element at position n.
     *
     * \param self
     * \param n
     * \returns a copy of the sequence with the element added at the requested location.
     * \invariant give(return) take(self)
     *
     */
    pfds_sequence* (*deleteAt)(pfds_sequence* self, size_t n);

    /** return the first element or NULL if the container is empty
     *
     * \param self
     * \returns the first element of self if it exists, NULL otherwise
     * \invariant when(return != NULL) lend(return, self)
     *
     */
    pfds_object* (*front)(pfds_sequence* self);

    /** return the last element or NULL if the container is empty
     *
     * \param self
     * \returns the last element of self if it exists, NULL otherwise
     * \invariant when(return != NULL) lend(return, self)
     *
     */
    pfds_object* (*back)(pfds_sequence* self);

    /** return a duplicate of the sequence with elements in reverse order
     *
     * \param self
     * \invariant give(return) take(self)
     */
    pfds_sequence* (*reverse)(pfds_sequence*);

    /** collect all elements of a sequence into a right associative running
     * total with user provided function.
     *
     * \param fn
     * \param ud
     * \param self
     * \param initial
     * \invariant give(return) take(self) take(initial)
     *
     * */
    pfds_object* (*reduceRight)(binop fn, void* ud, pfds_sequence* self, pfds_object* initial);

    /** collect all elements of a sequence into a left associative running
     * total with user provided function.
     *
     * \param fn
     * \param ud
     * \param initial
     * \param self
     * \invariant give(return) take(self) take(initial)
     *
     * */
    pfds_object* (*reduceLeft)(binop, void*, pfds_object*, pfds_sequence*);
};

/** convenient default implementation of isEmpty that uses size.
 *
 * \param self
 * \return true if self is the empty sequence, false otherwise.
 * \invariant borrow(self)
 *
 */
bool pfds_sequence_defaultIsEmpty(pfds_sequence* self);

/** convenient default implementation of popFront that uses split
 *
 * \param[out] head first element of a sequence
 * \param[out] tail remaining elements of a sequence
 * \param[in] self sequence to be examined.
 * \return true if there was an element to be popped.
 * \invariant when(return == true) give(head) give(tail) take(self)
 *      | when(return == false) borrow(self)
 */
bool pfds_sequence_defaultPopFront(pfds_object** head, pfds_sequence** tail, pfds_sequence* self);

/** convenient default implementation of popBack that uses split
 *
 * \param[out] init remaining elements of a sequence
 * \param[out] last first element of a sequence
 * \param[in] self sequence to be examined.
 * \return true if there was an element to be popped.
 * \invariant when(return == true) give(init) give(last) take(self)
 *      | when(return == false) borrow(self)
 */
bool pfds_sequence_defaultPopBack(pfds_sequence** init, pfds_object** last, pfds_sequence* self);

/** convenient default implementation of front that uses get
 *
 * \param self
 * \returns the first element of self if it exists, NULL otherwise
 * \invariant when(return != NULL) lend(return, self)
 *
 */
pfds_object* pfds_sequence_defaultFront(pfds_sequence* self);

/** convenient default implementation of back that uses size and get
 *
 * \param self
 * \returns the last element of self if it exists, NULL otherwise
 * \invariant when(return != NULL) lend(return, self)
 *
 */
pfds_object* pfds_sequence_defaultBack(pfds_sequence* self);


/** convenience default implementation of debugfputs based on popFront
 * \protected \memberof pfds_sequence
 *
 * \param stream File pointer open for writing.
 * \param self sequence.  must implement popFront
 * \invariant borrow(self)
 *
 */
void pfds_sequence_defaultDebugfputs (FILE* stream, pfds_sequence* self);

/** convenience default implementation of reduceLeft based on popFront
 *
 * \param fn
 * \param ud
 * \param initial
 * \param self
 * \invariant give(return) take(self) take(initial)
 *
 */
pfds_object* pfds_sequence_defaultReduceLeft(binop fn, void* ud, pfds_object* initial, pfds_sequence* self);

/** convenience default implementation of reduceRight based on popBack
 *
 * \param fn
 * \param ud
 * \param self
 * \param initial
 * \invariant give(return) take(self) take(initial)
 *
 */
pfds_object* pfds_sequence_defaultReduceRight(binop fn, void* ud, pfds_sequence* self, pfds_object* initial);

/** convenience implementation of everse based on pushFront, popFront and empty
 *
 * \param self
 * \invariant give(return) take(self)
 */
pfds_sequence* pfds_sequence_defaultReverse(pfds_sequence* self);

/** test if a sequence has elements
 *
 * \param self
 * \return true if self is the empty sequence, false otherwise.
 * \invariant borrow(self)
 *
 */
bool pfds_sequence_isEmpty (pfds_sequence* self);

/** \see pfds_sequence::pushFront
 */
pfds_sequence* pfds_sequence_pushFront (pfds_object* elem,pfds_sequence* self);
bool pfds_sequence_popFront (pfds_object** head, pfds_sequence** tail, pfds_sequence* self);
pfds_object* pfds_sequence_front(pfds_sequence* self);
pfds_object* pfds_sequence_back(pfds_sequence* self);
pfds_object* pfds_sequence_get(pfds_sequence* self, size_t idx);
size_t pfds_sequence_size(pfds_sequence* self);

pfds_sequence* pfds_sequence_pushBack(pfds_sequence* self, pfds_object* elem);

pfds_object* pfds_sequence_reduceRight(binop xy2y, void* ud, pfds_sequence* xs, pfds_object* y);
pfds_object* pfds_sequence_reduceLeft(binop xy2x, void* ud, pfds_object* x, pfds_sequence* ys);

pfds_sequence* pfds_sequence_reverse(pfds_sequence*);
bool pfds_sequence_split(pfds_sequence**, pfds_object**, pfds_sequence**, pfds_sequence*, size_t);

bool pfds_sequence_popBack(pfds_sequence**, pfds_object**, pfds_sequence*);

/** combine two elements of the same type
 *
 * \param l
 * \param r
 * \invariant give(return) take(l) take (r)
 */
pfds_sequence* pfds_sequence_mappend(pfds_sequence* l, pfds_sequence* r);

/** add an element x in position n.
 *
 * \param self
 * \param n
 * \param x
 * \returns a copy of the sequence with the element added at the requested location.
 * \invariant give(return) take(self) (take x)
 *
 */
pfds_sequence* pfds_sequence_insertBefore(pfds_sequence* self, size_t n, pfds_object* x);

pfds_sequence* pfds_sequence_insertAfter(pfds_sequence* self, size_t n, pfds_object* x);

pfds_sequence* pfds_sequence_updateAt(pfds_sequence* self, size_t n, pfds_object* x);

/** convenience implementation of insertBefore based on split and concat
 *
 * \param self
 * \param n
 * \param x
 * \returns a copy of the sequence with the element added at the requested location.
 * \invariant give(return) take(self) (take x)
 *
 */
pfds_sequence* pfds_sequence_defaultInsertBefore(pfds_sequence* self, size_t n, pfds_object* x);

/** convenience implementation of insertAfter based on insertBefore
 *
 * \param self
 * \param n
 * \param x
 * \returns a copy of the sequence with the element added at the requested location.
 * \invariant give(return) take(self) (take x)
 *
 */
pfds_sequence* pfds_sequence_defaultInsertAfter(pfds_sequence* self, size_t n, pfds_object* x);

/** remove the element at position n.
 *
 * \param self
 * \param n
 * \returns a copy of the sequence with the element added at the requested location.
 * \invariant give(return) take(self)
 *
 */
pfds_sequence* pfds_sequence_deleteAt(pfds_sequence* self, size_t n);

/** convenience implementation of deleteAt based on split and mappend
 *
 * \param self
 * \param n
 * \returns a copy of the sequence with the element added at the requested location.
 * \invariant give(return) take(self)
 *
 */
pfds_sequence* pfds_sequence_defaultDeleteAt(pfds_sequence* self, size_t n);

/** convenience implementation of updateAt based on split and concat
 *
 * \param self
 * \param n
 * \param x
 * \returns a copy of the sequence with the element added at the requested location.
 * \invariant give(return) take(self) (take x)
 *
 */
pfds_sequence* pfds_sequence_defaultUpdateAt(pfds_sequence* self, size_t n, pfds_object* x);


#endif
