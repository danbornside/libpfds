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

/** pfds internals used for defining garbage collected types compatible with pfds.
 *
 * users of this library should not need to include this header.
 */

#ifndef PFDS_OBJECTINTL_HEADER_INCLUDED
#define PFDS_OBJECTINTL_HEADER_INCLUDED

/** create a new uninitialised object
 *
 * \protected \memberof pfds_object
 * \param size size of object as returned by sizeof
 * \param vtable method dispatch table
 * \invariant give(return)
 *
 * */
pfds_object* pfds_object_new(size_t size, const pfds_objectvtable* vtable);

#define panic(msg) { fprintf(stderr, "PANIC %s:%d\n\t%s\n", __FILE__, __LINE__, msg); abort(); }

#endif
