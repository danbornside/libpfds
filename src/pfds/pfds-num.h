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

#ifndef PFDS_NUM_HEADER_INCLUDED
#define PFDS_NUM_HEADER_INCLUDED

#include "pfds-object.h"

/** boxed double
 *
 * \class pfds_Double
 * \extends pfds_object
 */
typedef struct pfds_Double pfds_Double;

/** boxed uint64_t
 *
 * \class pfds_UInt64
 * \extends pfds_object
 */
typedef struct pfds_UInt64 pfds_UInt64;

extern const pfds_objectvtable pfds_UInt64_vtable;
extern const pfds_objectvtable pfds_Double_vtable;

/** A boxed double
 *
 * \invariant give(return)
 */
pfds_Double* pfds_Double_new(double);
double pfds_Double_get(pfds_Double*);

/** A boxed unsigned long
 *
 * \invariant give(return)
 */
pfds_UInt64* pfds_UInt64_new(unsigned long);
unsigned long pfds_UInt64_get(pfds_UInt64*);

extern pfds_catenablevtable pfds_catenable_sum;

pfds_ordering pfds_Double_cmp(pfds_Double *l, pfds_Double *r);
pfds_ordering pfds_UInt64_cmp(pfds_UInt64 *l, pfds_UInt64 *r);

#endif
