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

#ifndef PFDS_ARRAYMAP_HEADER_INCLUDED
#define PFDS_ARRAYMAP_HEADER_INCLUDED

#include "pfds-mapping.h"
#include "pfds-object.h"

typedef struct pfds_ArrayMap pfds_ArrayMap;
extern pfds_objectvtable pfds_ArrayMap_vtable;

pfds_ArrayMap * pfds_ArrayMap_empty();
pfds_ArrayMap * pfds_ArrayMap_singleton(pfds_object *key, pfds_object* value);
pfds_ArrayMap * pfds_ArrayMap_fromArray(size_t size, pfds_object_pair items[]);
pfds_ordering pfds_ArrayMap_cmp(pfds_ArrayMap* l, pfds_ArrayMap* r);
int pfds_ArrayMap_debugfputs(FILE* stream, pfds_ArrayMap* self);

enum pfds_ArrayMapCopyFlags {
    AMCF_NONE = 0, // there are no available speedups
    AMCF_OWNBUFFER = 1<<1, // item buffer was allocated for map
    AMCF_SORTED = 1<<2, // items are already sorted
    AMCF_UNIQUE = 1<<3, // items are already unique
};

pfds_ArrayMap * pfds_ArrayMap_fromArray_ex(size_t size, pfds_object_pair items[], enum pfds_ArrayMapCopyFlags flags);

/** construct a new mapping from an ArrayMap.
 *
 * \param dict
 * \param items
 * \invariant give(return) take(lst)
 */
pfds_mapping* pfds_mapping_fromArrayMap(const pfds_objectvtable *vtable, pfds_ArrayMap* arrayMap);

/** retrieve an item from an ArrayMap by ordinal position
 * \param self
 * \param i
 * \returns item at i'th position
 * \invariant lend(return.{key,value}, self), borrow(self)
 */
pfds_object_pair pfds_ArrayMap_getItem(pfds_ArrayMap* self, size_t i);

#endif
