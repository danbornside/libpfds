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

#ifndef PFDS_CATENABLE_HEADER_INCLUDED
#define PFDS_CATENABLE_HEADER_INCLUDED

#include "pfds-object.h"

struct pfds_catenablevtable {
    /** the identity element of the group
     *
     * \invariant give(return)
     */
    pfds_object* (*mempty)(void);

    /** combine two elements of the group
     *
     * \param l
     * \param r
     * \invariant give(return) take(l) take (r)
     */
    pfds_object* (*mappend)(pfds_object* l, pfds_object* r);

    /** construct a member of the group concatenating an array of several
     * elements of a suitable type.
     *
     * \param n number of elements
     * \param xs
     * \invariant give(return) take(xs[0..n])
     *
     */
    pfds_object* (*concat)(size_t, pfds_object**);
};

/** combine two elements of the same type
 *
 * \param l
 * \param r
 * \invariant give(return) take(l) take (r)
 */
#define pfds_mappend(l, r) ((typeof(l))pfds_object_mappend((pfds_object*)l, (pfds_object*)r))

/** combine two elements of the same type
 *
 * \param l
 * \param r
 * \invariant give(return) take(l) take (r)
 */
pfds_object* pfds_object_mappend(pfds_object* l, pfds_object* r);


#endif
