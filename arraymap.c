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

#include <string.h>

#include "pfds.h"
#include "pfds/pfds-arraymap.h"
#include "pfds/pfds-object-intl.h"


void ArrayMap_destroy(pfds_ArrayMap* self);

struct pfds_ArrayMap {
    pfds_object object;
    size_t size;
    struct pfds_object_pair* items;
};

pfds_mappingvtable ArrayMap_mapping = {
    .empty = (pfds_mapping* (*)())
        pfds_ArrayMap_empty,
    .singleton = (pfds_mapping* (*)(struct pfds_object_pair))
        pfds_ArrayMap_singleton,
    .fromArray = (pfds_mapping* (*)(size_t, struct pfds_object_pair*))
        pfds_ArrayMap_fromArray,
};

pfds_objectvtable pfds_ArrayMap_vtable = {
    .typename = "ArrayMap",
    .destroy = (void (*)(pfds_object*))
        ArrayMap_destroy,
    .cmp = (pfds_ordering (*)(pfds_object*, pfds_object*))
        pfds_ArrayMap_cmp,
    .debugfputs = (void (*)(FILE*, pfds_object*))
        pfds_ArrayMap_debugfputs,
    .mapping = &ArrayMap_mapping,
};

void ArrayMap_destroy(pfds_ArrayMap* self) {
    for (size_t i = 0; i < self->size; i++) {
        pfds_release(self->items[i].key);
        pfds_release(self->items[i].value);
    }
}


pfds_ArrayMap * pfds_ArrayMap_empty() {
    pfds_ArrayMap * self = (pfds_ArrayMap *) pfds_object_new(
            sizeof(pfds_ArrayMap),
            &pfds_ArrayMap_vtable);
    self->size = 0;
    return self;
}

pfds_ArrayMap * pfds_ArrayMap_singleton(struct pfds_object_pair item) {
    panic("TODO");
}

enum ArrayMapCopyFlags {
    AMCF_NONE = 0, // there are no available speedups
    AMCF_OWNBUFFER = 1<<1, // item buffer was allocated for map
    AMCF_SORTED = 1<<2, // items are already sorted
    AMCF_UNIQUE = 1<<3, // items are already unique

};

pfds_ordering pfds_objectpair_cmp(struct pfds_object_pair* l, struct pfds_object_pair* r) {
    return pfds_cmp(l->key, r->key);
}

pfds_ArrayMap * pfds_ArrayMap_fromArray_ex(
        size_t size,
        struct pfds_object_pair items[],
        enum ArrayMapCopyFlags flags) {

    // first a couple of special cases
    if (size == 0) {
        if (items != NULL && flags & AMCF_OWNBUFFER) {
            free(items);
        }
        return pfds_ArrayMap_empty();
    }
    if (size == 1) {
        pfds_ArrayMap* self = pfds_ArrayMap_singleton(items[0]);
        if (flags & AMCF_OWNBUFFER) {
            free(items);
        }
        return self;
    }

    // size is at least 2, there is some sorting work to potentially do.

    struct pfds_object_pair* tmpItems;
    // copy items to temporary work buffer
    if (flags & AMCF_OWNBUFFER) {
        tmpItems = items;
    } else {
        tmpItems = (struct pfds_object_pair*) calloc(sizeof(struct pfds_object_pair), size);
        memcpy(tmpItems, items, sizeof(struct pfds_object_pair) * size);
    }
    // sort items with qsort
    if (! (flags & AMCF_SORTED)) {
        qsort(tmpItems,
                size,
                sizeof(struct pfds_object_pair),
                (int (*)(const void*, const void*)) pfds_objectpair_cmp);
    }

    // remove duplicates
    size_t uniqSize;
    if (flags & AMCF_UNIQUE) {
        uniqSize = size;
    } else {
        uniqSize = 1;
        for (long i = size - 2; i >= 0; i--) {
            if (pfds_objectpair_cmp(&tmpItems[i], &tmpItems[i+1]) == PFDS_EQ) {
                pfds_release(tmpItems[i+1].key);
                tmpItems[i+1].key = NULL;
                pfds_release(tmpItems[i+1].value);
                tmpItems[i+1].value = NULL;
            } else {
                uniqSize++;
            }
        }
        // copy items to final buffer
        if (uniqSize < size) {
            struct pfds_object_pair* tmpUniq = (struct pfds_object_pair*)
                calloc(sizeof(struct pfds_object_pair), uniqSize);
            int iUniq, iTmp;
            for (iUniq = iTmp = 0; iTmp < size; iTmp++) {
                if (tmpItems[iTmp].key != NULL) {
                    tmpUniq[iUniq] = tmpItems[iTmp];
                    iUniq++;
                }
            }
            free(tmpItems);
            tmpItems = tmpUniq;
        }
    }
    pfds_ArrayMap* self = (pfds_ArrayMap*) pfds_object_new(
            sizeof(pfds_ArrayMap),
            &pfds_ArrayMap_vtable);
    self->size = uniqSize;
    self->items = tmpItems;
    return self;
}

pfds_ArrayMap * pfds_ArrayMap_fromArray(size_t size, struct pfds_object_pair items[]) {
    return pfds_ArrayMap_fromArray_ex(size, items, AMCF_NONE);
}


pfds_ordering pfds_ArrayMap_cmp(pfds_ArrayMap* l, pfds_ArrayMap* r) {
    panic("TODO");
}

void pfds_ArrayMap_debugfputs(FILE* stream, pfds_ArrayMap* self) {
    panic("TODO");
}
