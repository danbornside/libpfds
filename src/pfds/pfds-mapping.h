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

#ifndef PFDS_MAPPING_HEADER_INCLUDED
#define PFDS_MAPPING_HEADER_INCLUDED

#include <stdbool.h>
#include "pfds-object.h"

typedef struct pfds_mapping {
    pfds_object object;
} pfds_mapping;

typedef struct pfds_object_pair {
    pfds_object* key;
    pfds_object* value;
} pfds_object_pair ;

typedef struct pfds_mappingvtable {

    /* construct a new mapping from an array of pairs
     *
     * @param n size of the array
     * @param items array of n items
     * @returns a new mapping
     * \invariant give(return) take(items[0..n].{key,value})
     *
     */
    pfds_mapping* (*fromArray)(size_t n, pfds_object_pair items[]);

    pfds_mapping* (*singleton)(pfds_object* key, pfds_object* value);

    /* construct a new empty mapping with no elements
     *
     * @returns a new mapping
     * \invariant give(return) take(items[0..n].{key,value})
     *
     */
    pfds_mapping* (*empty)();

    /** test if a mapping has elements
     *
     * @param self
     * @returns true if the mapping is the empty mapping
     * \invariant borrow(self)
     *
     */
    bool (*isEmpty)(pfds_mapping* self);

    /** test if a mapping has elements
     *
     * @param self
     * @returns the number of key-value pairs in the mapping
     * \invariant borrow(self)
     */
    size_t (*size)(pfds_mapping*);


    pfds_mapping* (*insert)(pfds_mapping*, pfds_object* key, pfds_object* value);

    /** find the value associated with a particular key in a mapping, or NULL if the key is not present.
     *
     * @param self
     * @param key
     * @returns a value or NULL
     * \invariant lend(return, self) borrow(self) borrow(key)
     */
    pfds_object* (*lookup)(pfds_mapping* self, pfds_object* key);
    pfds_mapping* (*erase)(pfds_mapping*, pfds_object*);

    // pfds_mapping* (*unionFirst)(pfds_mapping*, pfds_mapping*);
    // pfds_mapping* (*intersectionFirst)(pfds_mapping*, pfds_mapping*);

    /** split the smallest key and value from a mapping
     *
     * @param [out] item the smallest item in self, or 0 if self is the empty mapping
     * @param [out] rest every item from self larger than the smallest key, or NULL
     * @param [in] self
     * @returns true if a smallest key was found and false if the mapping is empty.
     * \invariant give(item.key) give(item.value) give(rest) take(self)
     *
     */
    bool (*popMin)(pfds_object_pair* item, pfds_mapping** rest, pfds_mapping* self);

    // bool (*popMax)(pfds_object_pair*, pfds_mapping**, pfds_mapping*);

    // pfds_object* (*minKey)(pfds_mapping*);
    // pfds_object* (*maxKey)(pfds_mapping*);

} pfds_mappingvtable;


bool pfds_mapping_isEmpty(pfds_mapping* self);
size_t pfds_mapping_size(pfds_mapping* self);


pfds_object* pfds_mapping_lookup(pfds_mapping* self, pfds_object* key);
bool pfds_mapping_popMin(pfds_object_pair* item, pfds_mapping** rest, pfds_mapping* self);

int pfds_mapping_defaultDebugfputs(FILE* stream, pfds_mapping* self);
pfds_ordering pfds_mapping_defaultCmp(pfds_mapping* l, pfds_mapping* r);

pfds_mapping* pfds_mapping_insert(pfds_mapping* self, pfds_object* key, pfds_object* value);
pfds_mapping* pfds_mapping_erase(pfds_mapping* self, pfds_object* key);

#endif
