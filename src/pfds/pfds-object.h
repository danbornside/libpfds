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

#ifndef PFDS_OBJECT_HEADER_INCLUDED
#define PFDS_OBJECT_HEADER_INCLUDED

#include "pfds-intl.h"

#include <stdio.h>

typedef struct pfds_objectvtable pfds_objectvtable;
typedef struct pfds_catenablevtable pfds_catenablevtable;
typedef struct pfds_sequencevtable pfds_sequencevtable;
typedef struct pfds_mappingvtable pfds_mappingvtable;

typedef enum pfds_ordering {
    PFDS_LT = -1,
    PFDS_EQ = 0,
    PFDS_GT = 1,
} pfds_ordering;



/** base "class" for most types in this library.
 *
 * \headerfile pfds/pfds-object.h <pfds/pfds-object.h>
 *
 * fixed header for all values that are managed by the garbage collector.
 * this must always be the first field in any structure that uses it.
 *
 * \class pfds_object
 *
 * All containers require that the elements they contain have pfds_object as
 * first member.  in Normal use types based on object start with a reference
 * count of 1 and must be `retain` / `release`ed as needed, when references are
 * passed around.
 *
 */
struct pfds_object {
    PFDS_GC_HEADER
    const pfds_objectvtable* vtable;
};

/** assortment of general purpose overloadable methods for the types defined in this lib. 
 * all fields may be left null for reasonable default behavior
 */
struct pfds_objectvtable {
    const char* typename;

    /** finalizer for object.
     * \private \memberof pfds_object
     *
     * if not null, called by the garbage collector just before an object is freed
     * use this method to free resources and decrease refcounts of owned members
     *
     * \param self
     * \invariant invalid(self)
     *
     */
    const void (*destroy)(pfds_object*);

    /** convenience function to display an object
     * \public \memberof pfds_object
     *
     * \param stream File pointer open for writing.
     * \param self
     * \invariant borrow(self)
     *
     */
    const int (*debugfputs)(FILE*, pfds_object*);

    /** compare two objects
     *
     * @param l
     * @param r
     * @return zero if both objects are the same, negative if the first is smaller and positive if the first is larger
     * \invariant borrow(l) borrow(r)
     */
    pfds_ordering (*cmp)(pfds_object*, pfds_object*);

    /** object is a catenable */
    const pfds_catenablevtable *catenable;
    const pfds_sequencevtable *sequence;
    const pfds_mappingvtable *mapping;
};

/** convenience function to display an object to a file stream.
 *
 * \param stream File pointer open for writing.
 * \param self
 * \invariant borrow(self)
 *
 */
int pfds_object_debugfputs(FILE* stream, pfds_object* self);

#define pfds_debugfputs(file, self) pfds_object_debugfputs(file, (pfds_object*) self)
#define pfds_debugputs(self) pfds_object_debugfputs(stdout, (pfds_object*) self)

/** compare two objects
 *
 * @param l
 * @param r
 * @return zero if both objects are the same, negative if the first is smaller and positive if the first is larger
 * \invariant borrow(l) borrow(r)
 */
#define pfds_cmp(l, r) pfds_object_cmp((pfds_object*) l, (pfds_object*) r)

/** compare two objects
 *
 * @param l
 * @param r
 * @return zero if both objects are the same, negative if the first is smaller and positive if the first is larger
 * \invariant borrow(l) borrow(r)
 */
pfds_ordering pfds_object_cmp(pfds_object* l, pfds_object* r);

#endif
