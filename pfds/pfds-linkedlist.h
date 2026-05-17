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

/** inefficient implementation of libpfds interfaces based on singly linked list.
 *
 * This types and methods in this module are mostly intended for illustration
 * and testing; most users will not need to use this header.
 */

#ifndef PFDS_LINKEDLIST_HEADER_INCLUDED
#define PFDS_LINKEDLIST_HEADER_INCLUDED

typedef struct pfds_LinkedList pfds_LinkedList;

extern const pfds_objectvtable pfds_LinkedList_vtable;

/** return a new, empty LinkedList
 *
 * \invariant give(return)
 */
pfds_LinkedList* pfds_LinkedList_empty();

pfds_LinkedList* pfds_LinkedList_singleton(pfds_object* x);

/** construct a new linked list by prepending a given element to a given list.
 *
 * this is the conventional way of building up LinkedList.
 *
 * \param head
 * \param tail
 * \invariant give(return) take(head) take(tail)
 */
pfds_LinkedList* pfds_LinkedList_pushFront(pfds_object* head, pfds_LinkedList* tail);

pfds_ordering pfds_LinkedList_cmp(pfds_LinkedList* l, pfds_LinkedList* r);

#endif

