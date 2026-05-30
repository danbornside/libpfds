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

#ifndef PFDS_STRING_HEADER_INCLUDED
#define PFDS_STRING_HEADER_INCLUDED

#include "pfds-object.h"

#include <stdbool.h>

/** boxed array of char
 *
 * \class pfds_String
 * \extends pfds_object
 * \implements pfds_catenable
 *
 */
typedef struct pfds_String pfds_String;

extern const pfds_objectvtable pfds_String_vtable;

/** Conveninece method to turn any object into its string representation.
 *
 * \param self
 * \invariant give(return) borrow(self)
 */
pfds_String* pfds_object_toString(pfds_object* self);
#define pfds_toString(self) pfds_object_toString((pfds_object*) self)

/** construct a new String from an existing buffer.  the buffer must be nul
 * terminated, size is as returned by strlen. if owner is true the buffer will
 * be freed when the string is destroyed
 */
pfds_String* pfds_String_new(const char* buf, size_t size, bool owner);

pfds_String* pfds_String_fromCstring(char* buf);
pfds_String* pfds_String_fromConstCstring(char* buf);

pfds_String* pfds_String_concat(size_t n, pfds_String* chunks[]);

/** format an object to a pfds_String.
 *
 * \param self
 * \invariant borrow(self)
 */
const char* pfds_String_toCstring(pfds_String* self);

pfds_ordering pfds_String_cmp(pfds_String *l, pfds_String *r);

extern pfds_String* pfds_String_empty(void);

#endif
