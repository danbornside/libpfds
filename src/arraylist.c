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

#define PFDS_INTERNAL

#include <string.h>
#include "pfds.h"
#include "pfds/pfds-object-intl.h"
#include "pfds/pfds-arraylist.h"

struct pfds_ArrayList {
    pfds_object object;
    size_t size;
    pfds_object **elements;
};

pfds_ArrayList* pfds_ArrayList_new(size_t size, pfds_object* elements[], bool owner);
pfds_ArrayList* ArrayList_insertBefore(pfds_ArrayList* self, size_t n, pfds_object* x);
pfds_ArrayList* ArrayList_deleteAt(pfds_ArrayList* self, size_t n);
pfds_ArrayList* ArrayList_updateAt(pfds_ArrayList* self, size_t n, pfds_object* x);

extern pfds_ArrayList* pfds_ArrayList_fromArray(size_t size, pfds_object** elements) {
    pfds_object** myElements = (pfds_object**) calloc(sizeof(pfds_object*), size);
    memcpy(myElements, elements, sizeof(pfds_object*)*size);
    return pfds_ArrayList_new(size, myElements, true);
}


pfds_ArrayList* pfds_ArrayList_concat(size_t n, pfds_ArrayList** selves) {
    size_t size = 0;
    for (size_t i = 0; i < n ; i++) {
        size += selves[i]->size;
    }
    pfds_object** elements = (pfds_object**) calloc(sizeof(pfds_object*), size);

    size_t pos = 0;
    for (size_t i = 0; i < n ; i++) {
        for (size_t j = 0; j < selves[i]->size ; j++) {
            elements[pos] = selves[i]->elements[j];
            pfds_retain(elements[pos]);
            pos++;
        }
        pfds_release(selves[i]);
    }
    return pfds_ArrayList_new(size, elements, true);
}

size_t pfds_ArrayList_size(pfds_ArrayList* self) { return self->size; }


void ArrayList_destroy(pfds_object* self_obj) {
    pfds_ArrayList* self = (pfds_ArrayList*) self_obj;
    if (self->elements) {
        for(size_t i = 0; i < self->size; ++i) {
            pfds_release(self->elements[i]);
        }
        free(self->elements);
    }
}

int ArrayList_debugfputs(FILE* stream, pfds_object* self_obj) {
    pfds_ArrayList* self = (pfds_ArrayList*) self_obj;
    int n = fprintf(stream, "[");
    for(size_t i = 0; i < self->size; ++i) {
        n += fprintf(stream, "%s", i ? ", " : "");
        n += pfds_debugfputs(stream, self->elements[i]);
    }
    n += fprintf(stream, "]");
    return n;
}

pfds_ordering pfds_ArrayList_cmp(pfds_ArrayList* l, pfds_ArrayList* r) {
    size_t size = l->size > r->size ? r->size : l->size;
    for(size_t i = 0; i < size; ++i) {
        pfds_ordering o = pfds_cmp(l->elements[i], r->elements[i]);
        if (o != 0) {
            return o;
        }
    }
    if (l->size < r->size) {
        return PFDS_LT;
    } else if (l->size > r->size) {
        return PFDS_GT;
    } else {
        return PFDS_EQ;
    }
}


bool pfds_ArrayList_isEmpty (pfds_sequence* self_obj) {
    pfds_ArrayList* self = (pfds_ArrayList*) self_obj;
    return self->size == 0;
}

bool pfds_ArrayList_split(pfds_ArrayList** init, pfds_object** link, pfds_ArrayList** tail, pfds_ArrayList* self, size_t pos) {
    if (self->size <= pos) {
        if (init) *init = NULL;
        if (link) *link = NULL;
        if (tail) *tail = NULL;
        pfds_release(self);
        return false;
    }
    if (init != NULL) {
        *init = pfds_ArrayList_new(pos, self->elements, false);
        for(size_t i = 0; i < pos; i++) {
            pfds_retain((*init)->elements[i]);
        }
    }
    if (link != NULL) {
        *link = self->elements[pos];
        pfds_retain(*link);
    }
    if (tail != NULL) {
        size_t tail_sz = self->size - pos - 1;
        *tail = pfds_ArrayList_new(tail_sz, self->elements + (pos + 1), false);
        for(size_t i = 0; i < tail_sz; i++) {
            pfds_retain((*tail)->elements[i]);
        }
    }
    pfds_release(self);
    return true;
}

pfds_ArrayList* pfds_ArrayList_pushFront(pfds_object* head, pfds_ArrayList* tail) {
    // TODO: emit warnings for slow code
    // also todo, optimistic array reuse.
    //
    size_t size = tail->size + 1;
    pfds_object** elements = (pfds_object**) calloc(sizeof(pfds_object*), size);
    elements[0] = head;
    for(size_t i = 0 ; i < tail->size ; ++i) {
        elements[i+1] = tail->elements[i];
        pfds_retain(elements[i+1]);
    }
    pfds_ArrayList* self = pfds_ArrayList_new(size, elements, true);
    pfds_release(tail);
    return self;
}

extern pfds_ArrayList* pfds_ArrayList_pushBack(pfds_ArrayList* init, pfds_object* last) {
    size_t size = init->size + 1;
    pfds_object** elements = (pfds_object**) calloc(sizeof(pfds_object*), size);
    for(size_t i = 0 ; i < init->size ; ++i) {
        elements[i] = init->elements[i];
        pfds_retain(elements[i]);
    }
    elements[init->size] = last;
    pfds_ArrayList* self = pfds_ArrayList_new(size, elements, true);
    pfds_release(init);
    return self;

}

pfds_object* pfds_ArrayList_get(pfds_sequence* self_obj, size_t idx) {
    pfds_ArrayList* self = (pfds_ArrayList*) self_obj;
    if (idx < self->size) {
        pfds_object* elem = self->elements[idx];
        pfds_retain(elem);
        return elem;
    } else {
        return NULL;
    }
}

pfds_ArrayList* ArrayList_reverse(pfds_ArrayList* self) {
    pfds_object** elements = (pfds_object**) calloc(sizeof(pfds_object*), self->size);
    for(size_t i = 0; i < self->size ; i++) {
        elements[i] = self->elements[self->size - 1 - i];
        pfds_retain(elements[i]);
    }
    pfds_release(self);
    return pfds_ArrayList_new(self->size, elements, true);
}

pfds_object* pfds_ArrayList_reduceRight(binop op, void* ud, pfds_ArrayList* self, pfds_object* seed) {
    for (size_t i = 0; i < self->size; i++) {
        pfds_object* elt = self->elements[self->size - 1 - i];
        pfds_retain(elt);
        seed = op(ud, elt, seed);
    }
    pfds_release(self);
    return seed;
}

pfds_object* pfds_ArrayList_reduceLeft(binop op, void* ud, pfds_object* seed, pfds_ArrayList* self) {
    for(size_t i = 0; i < self->size ; i++) {
        pfds_object* elt = self->elements[i];
        pfds_retain(elt);
        seed = op(ud, seed, elt);
    }
    pfds_release(self);
    return seed;
}

static pfds_catenablevtable ArrayList_catenablevtable = {
    .mempty = (pfds_object* (*)(void)) pfds_ArrayList_empty,
    .mappend = (pfds_object* (*)(pfds_object*, pfds_object*)) pfds_ArrayList_mappend,
    .concat = (pfds_object* (*)(size_t, pfds_object**)) pfds_ArrayList_concat,
};


static pfds_sequencevtable ArrayList_sequencevtable = {
    .catenable = &ArrayList_catenablevtable,
    .fromArray = (pfds_sequence* (*)(size_t, pfds_object**)) pfds_ArrayList_fromArray,
    .singleton = (pfds_sequence* (*)(pfds_object*))
        pfds_ArrayList_singleton,
    .isEmpty = pfds_ArrayList_isEmpty,
    .size = (size_t (*)(pfds_sequence*)) pfds_ArrayList_size,

    .insertBefore = (pfds_sequence* (*)(pfds_sequence*,size_t,pfds_object*))
        ArrayList_insertBefore,
    .insertAfter = pfds_sequence_defaultInsertAfter,
    .updateAt = (pfds_sequence* (*)(pfds_sequence*,size_t,pfds_object*))
        ArrayList_updateAt,
    .deleteAt = (pfds_sequence* (*)(pfds_sequence*,size_t))
        ArrayList_deleteAt,

    .popFront = pfds_sequence_defaultPopFront,
    .popBack = pfds_sequence_defaultPopBack,
    .split = (bool (*)(pfds_sequence**, pfds_object**, pfds_sequence**, pfds_sequence*, size_t)) pfds_ArrayList_split,

    .pushFront = (pfds_sequence* (*)(pfds_object*, pfds_sequence*)) pfds_ArrayList_pushFront,
    .pushBack = (pfds_sequence* (*)(pfds_sequence*, pfds_object*)) pfds_ArrayList_pushBack,

    .get = pfds_ArrayList_get,
    .front = pfds_sequence_defaultFront,
    .back = pfds_sequence_defaultBack,

    .reverse = (pfds_sequence* (*)(pfds_sequence*)) ArrayList_reverse,

    .reduceRight = (pfds_object* (*)(binop, void*, pfds_sequence*, pfds_object*))pfds_ArrayList_reduceRight,
    .reduceLeft = (pfds_object* (*)(binop, void*, pfds_object*, pfds_sequence*)) pfds_ArrayList_reduceLeft,
};

const pfds_objectvtable pfds_ArrayList_vtable = {
    .typename = "ArrayList",
    .destroy = ArrayList_destroy,
    .debugfputs = ArrayList_debugfputs,
    .cmp = (pfds_ordering (*)(pfds_object*, pfds_object*))
        pfds_ArrayList_cmp,

    .catenable = &ArrayList_catenablevtable,
    .sequence = &ArrayList_sequencevtable,
};

pfds_ArrayList* pfds_ArrayList_new(size_t size, pfds_object* elements[], bool owner) {
    pfds_ArrayList* self = (pfds_ArrayList*) pfds_object_new(sizeof(pfds_ArrayList), &pfds_ArrayList_vtable);
    self->size = size;
    if (size > 0) {
        if (owner) {
            self->elements = elements;
        } else {
            self->elements = (pfds_object**) calloc(sizeof(pfds_object*), size);
            memcpy(self->elements, elements, sizeof(pfds_object*) * size);
        }
    } else {
        if (owner && elements != NULL) {
            free(elements);
            // caller gave us an allocated empty ptr for some reason.  free it?
            // panic("empty elements");
        }
        self->elements = NULL;
    }
    return self;
}

extern pfds_ArrayList* pfds_ArrayList_empty() {
    return pfds_ArrayList_new(0, NULL, false);
}

extern pfds_ArrayList* pfds_ArrayList_singleton(pfds_object* elem) {
    pfds_object** elems = (pfds_object**) calloc(sizeof(pfds_object*), 1);
    elems[0] = elem;
    return pfds_ArrayList_new(1, elems, true);
}

/** add an element x in position n.
 *
 * \param self
 * \param n
 * \param x
 * \returns a copy of the sequence with the element added at the requested location.
 * \invariant give(return) take(self) (take x)
 *
 */
pfds_ArrayList* ArrayList_insertBefore(pfds_ArrayList* self, size_t n, pfds_object* x) {
    if (n > self->size) {
        panic("ArrayList index out of bounds");
    }

    size_t size = self->size + 1;

    pfds_object** elems = (pfds_object**) calloc(sizeof(pfds_object*), size);
    int i;
    for (i = 0; i < n; i++) {
        elems[i] = self->elements[i];
        pfds_retain(elems[i]);
    }
    elems[n] = x;
    i++;
    for (; i < size; i++) {
        elems[i] = self->elements[i - 1];
        pfds_retain(elems[i]);
    }

    pfds_release(self);

    return pfds_ArrayList_new(size, elems, true);
}

/** replace an element x in position n.
 *
 * \param self
 * \param n
 * \param x
 * \returns a copy of the sequence with the element added at the requested location.
 * \invariant give(return) take(self) (take x)
 *
 */
pfds_ArrayList* ArrayList_updateAt(pfds_ArrayList* self, size_t n, pfds_object* x) {
    if (n >= self->size) {
        panic("ArrayList index out of bounds");
    }

    size_t size = self->size;

    pfds_object** elems = (pfds_object**) calloc(sizeof(pfds_object*), size);
    int i;
    for (i = 0; i < n; i++) {
        elems[i] = self->elements[i];
        pfds_retain(elems[i]);
    }
    elems[n] = x;
    i++;
    for (; i < size; i++) {
        elems[i] = self->elements[i];
        pfds_retain(elems[i]);
    }

    pfds_release(self);

    return pfds_ArrayList_new(size, elems, true);
}
/** delete an element x in position n.
 *
 * \param self
 * \param n
 * \returns a copy of the sequence with the element added at the requested location.
 * \invariant give(return) take(self) (take x)
 *
 */
pfds_ArrayList* ArrayList_deleteAt(pfds_ArrayList* self, size_t n) {
    if (n >= self->size) {
        panic("ArrayList index out of bounds");
    }

    size_t size = self->size - 1;

    pfds_object** elems = (pfds_object**) calloc(sizeof(pfds_object*), size);
    int i;
    for (i = 0; i < n; i++) {
        elems[i] = self->elements[i];
        pfds_retain(elems[i]);
    }
    for (; i < size; i++) {
        elems[i] = self->elements[i + 1];
        pfds_retain(elems[i]);
    }

    pfds_release(self);

    return pfds_ArrayList_new(size, elems, true);
}

extern pfds_ArrayList* pfds_ArrayList_mappend(pfds_ArrayList* l, pfds_ArrayList* r) {
    size_t size = l->size + r->size;
    if (l->size == 0) {
        pfds_release(l);
        return r;
    } else if (r->size == 0) {
        pfds_release(r);
        return l;
    } else {
        pfds_object** elements = (pfds_object**) calloc(sizeof(pfds_object*), size);

        for(size_t i = 0; i < l->size; ++i) {
            elements[i] = l->elements[i];
            pfds_retain(elements[i]);
        }
        for(size_t i = 0; i < r->size; ++i) {
            elements[i + l->size] = r->elements[i];
            pfds_retain(elements[i + l->size]);
        }
        pfds_release(l);
        pfds_release(r);
        return pfds_ArrayList_new(size, elements, true);
    }
}

