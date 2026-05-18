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

/** @mainpage libpfds: Persistent Functional Data Structures in C
 *
 * the types in this library follow a particular approach to reference counting
 * garbage collection. in particular, any function that is defined to return
 * the same type as its arguments, will take ownership of those arguments,
 * possibly causing them to be garbage collected earlier than expected.  to
 * prevent this from happening, call pfds_retain on any intermediate values you
 * need to keep.
 *
 * \section Garbage Collection
 *
 * this library makes use of a simple reference counting garbage collection system.
 *
 * most functions in this library will adjust reference counts in a sensible
 * way, taking ownership of arguments when constructing new values out of them,
 * or not if examining them.  the exact details are noted in each function's
 * documentatin under invariants.
 *
 * - `dup(x)` increase ref count by one, allowing one more reference.
 * - `drop(x)` decrease ref count by one, signaling the end of a particular reference.
 * - `borrow(x)` hold a ref without adjusting refcount.  the reference must
 *     stay alive for the duration of the function call.
 * - `lend(x, y)` emit a temporary reference to x who's lifetime is determined by y
 * - `take(x)` consume a reference.  caller must retain the reference
 *      themselves to keep it valid after the function returns.
 * - `give(x)` produce a possibly new value referenced by x.
 *
 */
#ifndef PFDS_HEADER_INCLUDED
#define PFDS_HEADER_INCLUDED

#include "pfds/pfds-mapping.h"
#include "pfds/pfds-catenable.h"
#include "pfds/pfds-functions.h"
#include "pfds/pfds-intl.h"
#include "pfds/pfds-num.h"
#include "pfds/pfds-object.h"
#include "pfds/pfds-sequence.h"
#include "pfds/pfds-string.h"
#include "pfds/pfds-treelist.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define pfds_retain_array(n, elts) { for (int retainElts##__LINE__ = 0; retainElts##__LINE__ < n ; ++retainElts##__LINE__) \
    pfds_retain((elts)[retainElts##__LINE__]); }

#define pfds_release_array(n, elts) { for (int retainElts##__LINE__ = 0; retainElts##__LINE__ < n ; ++retainElts##__LINE__) \
    pfds_release((elts)[retainElts##__LINE__]); }




#define pfds_debugfputs(file, self) pfds_object_debugfputs(file, (pfds_object*) self)
#define pfds_debugputs(self) pfds_object_debugfputs(stdout, (pfds_object*) self)


#endif
