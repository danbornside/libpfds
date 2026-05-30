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

#ifndef PFDS_FINGERTREE_HEADER_INCLUDED
#define PFDS_FINGERTREE_HEADER_INCLUDED

#include "pfds-object-intl.h"

#include <stdbool.h>

typedef struct pfds_FingerTree pfds_FingerTree;
typedef pfds_object* (*pfds_measure)(pfds_object*);

pfds_FingerTree* pfds_FingerTree_pushBack(
        const pfds_catenablevtable* cat, pfds_measure mm,
        pfds_FingerTree* init,
        pfds_object* last);

pfds_FingerTree* pfds_FingerTree_pushFront(
        const pfds_catenablevtable* cat, pfds_measure mm,
        pfds_object* head,
        pfds_FingerTree *tail);

/**
 *
 * \param cat
 * \param mm
 * \param xs
 * \param n
 * \param ts
 * \param ys
 * \returns FingerTree
 * \invariant give(return) take(xs) take(ts[0..n]) take(ys)
 *
 */
pfds_FingerTree* pfds_FingerTree_app3(
        const pfds_catenablevtable* cat, pfds_measure mm,
        pfds_FingerTree* xs,
        size_t n, pfds_object** ts,
        pfds_FingerTree* ys);

/** the empty pfds_FingerTree
 *
 * \invariant give(return)
 */
pfds_FingerTree* pfds_FingerTree_empty(void);

/** construct a single element FingerTree
 *
 * \param head
 * \invariant give(return) take(head)
 */
pfds_FingerTree* pfds_FingerTree_singleton (pfds_object* head);


pfds_FingerTree* pfds_FingerTree_pushBackArray(
        const pfds_catenablevtable* cat, pfds_measure mm,
        pfds_FingerTree* init,
        size_t n, pfds_object* lasts[]);

/** test if a FingerTree is empty
 *
 * \param self
 * \return true if self is the empty sequence, false otherwise.
 * \invariant borrow(self)
 *
 */
extern bool pfds_FingerTree_isEmpty(pfds_FingerTree* self);


bool pfds_FingerTree_split(
        pfds_FingerTree** init,
        pfds_object** pivot,
        pfds_FingerTree** tail,
        const pfds_catenablevtable* cat, pfds_measure mm,
        bool (*p)(void*, pfds_object*), void* ud,
        pfds_FingerTree* self);

pfds_object* pfds_FingerTree_measure(const pfds_catenablevtable* cat, pfds_measure mm, pfds_FingerTree* self);

int pfds_FingerTree_debugfputs(FILE* stream, pfds_FingerTree* self, size_t depth);

bool pfds_FingerTree_popFront(
        pfds_object** head, pfds_FingerTree** tail,
        const pfds_catenablevtable* cat, pfds_measure mm,
        pfds_FingerTree* self);

#endif
