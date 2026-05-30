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

#ifndef PFDS_TREEMAP_HEADER_INCLUDED
#define PFDS_TREEMAP_HEADER_INCLUDED

#include "pfds-object.h"
#include "pfds-mapping.h"

#include <stdbool.h>

typedef struct pfds_TreeMap pfds_TreeMap;
extern const pfds_objectvtable pfds_TreeMap_vtable;

pfds_TreeMap* pfds_TreeMap_fromArray(size_t n, pfds_object_pair items[]);
extern pfds_TreeMap* pfds_TreeMap_singleton(pfds_object* key, pfds_object* value);
extern pfds_TreeMap* pfds_TreeMap_empty(void);
bool pfds_TreeMap_isEmpty(pfds_TreeMap* self);
size_t pfds_TreeMap_size(pfds_TreeMap* self);
pfds_object* pfds_TreeMap_lookup(pfds_TreeMap* self, pfds_object* key);
bool pfds_TreeMap_popMin(pfds_object_pair*, pfds_TreeMap**, pfds_TreeMap*);
pfds_TreeMap* pfds_TreeMap_erase(pfds_TreeMap* self, pfds_object* key);
pfds_TreeMap* pfds_TreeMap_insert(pfds_TreeMap* self, pfds_object* key, pfds_object* value);

#endif
