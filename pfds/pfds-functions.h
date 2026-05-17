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

#ifndef PFDS_FUNCTIONS_HEADER_INCLUDED
#define PFDS_FUNCTIONS_HEADER_INCLUDED

#include "pfds-object.h"

/** generic placeholder for a function with 2 arguments
 *
 * \param userData arbitrary user data to be passed to the function.
 * \param l
 * \param r
 * \returns result of the selected 2 argument closure.
 * \invariant give(return) take(l) take(r)
 *
 */
typedef pfds_object* (*binop)(void* userData, pfds_object* l, pfds_object* r);

#endif


