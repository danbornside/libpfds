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

#ifndef PFDS_TREELIST_HEADER_INCLUDED
#define PFDS_TREELIST_HEADER_INCLUDED

#include "pfds-object.h"

typedef struct pfds_TreeList pfds_TreeList;
extern const pfds_objectvtable pfds_TreeList_vtable;

pfds_TreeList* pfds_TreeList_fromArray(size_t n, pfds_object** xs);
int pfds_TreeList_debugfputs(FILE* stream, pfds_TreeList* self);
extern pfds_TreeList* pfds_TreeList_singleton(pfds_object* x);
extern pfds_TreeList* pfds_TreeList_mempty(void);

#endif
