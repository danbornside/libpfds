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

#include "pfds-object.h"

typedef struct pfds_ArrayMap pfds_ArrayMap;
extern pfds_objectvtable pfds_ArrayMap_vtable;

pfds_ArrayMap * pfds_ArrayMap_empty();
pfds_ArrayMap * pfds_ArrayMap_singleton(struct pfds_object_pair item);
pfds_ArrayMap * pfds_ArrayMap_fromArray(size_t size, struct pfds_object_pair items[]);
pfds_ordering pfds_ArrayMap_cmp(pfds_ArrayMap* l, pfds_ArrayMap* r);
int pfds_ArrayMap_debugfputs(FILE* stream, pfds_ArrayMap* self);

#endif
