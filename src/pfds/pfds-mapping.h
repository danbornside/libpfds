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

struct pfds_object_pair {
    pfds_object* key;
    pfds_object* value;
};

typedef struct pfds_mappingvtable {

    pfds_mapping* (*fromArray)(size_t n, struct pfds_object_pair items[]);
    pfds_mapping* (*singleton)(struct pfds_object_pair item);
    pfds_mapping* (*empty)();

    bool (*isEmpty)(pfds_mapping*);
    size_t (*size)(pfds_mapping*);

    pfds_mapping* (*insert)(pfds_mapping*, struct pfds_object_pair);
    pfds_object* (*lookup)(pfds_mapping*, pfds_object*);
    pfds_mapping* (*erase)(pfds_mapping*, pfds_object*);

    // pfds_mapping* (*unionFirst)(pfds_mapping*, pfds_mapping*);
    // pfds_mapping* (*intersectionFirst)(pfds_mapping*, pfds_mapping*);

    // bool (*popMin)(pfds_object_pair*, pfds_mapping**, pfds_mapping*);
    // bool (*popMax)(pfds_object_pair*, pfds_mapping**, pfds_mapping*);

    // pfds_object* (*minKey)(pfds_mapping*);
    // pfds_object* (*maxKey)(pfds_mapping*);

} pfds_mappingvtable;

#endif
