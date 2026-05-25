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

/* LinkedList is implemented in a slightly odd way, the empty list and
 * singleton list are both allocated objects, but with their tail pointers set
 * to null.  a more conventional approach where the empty list is just a null
 * pointer is less helpful in the way that this library works, where we need
 * something to hang a vtable off of.
 *
 * */
#include <assert.h>

#define PFDS_INTERNAL
#include "pfds.h"
#include "pfds/pfds-object-intl.h"
#include "pfds/pfds-linkedlist.h"

struct pfds_LinkedList
{
    pfds_object object;
    size_t size;
    pfds_object* head;
    pfds_LinkedList* tail;
};


pfds_LinkedList* LinkedList_mappend(pfds_LinkedList* self, pfds_LinkedList* other) {
    if (self->head == NULL) {
        pfds_release(self);
        return other;
    } else if (other->head == NULL) {
        pfds_release(other);
        return self;
    } else {
        size_t sz = self->size + other->size;
        pfds_LinkedList* cursor = self;
        pfds_LinkedList* acc = pfds_LinkedList_empty();
        pfds_LinkedList* result = acc;

        for(;;) {
            acc->size = sz;
            acc->head = cursor->head;
            pfds_retain(acc->head);
            sz--;
            cursor = cursor->tail;
            if (cursor->head == NULL ) {
                break;
            }
            acc->tail = pfds_LinkedList_empty();
            acc = acc->tail;
        }
        assert(sz == other->size);
        acc->tail = other;

        pfds_release(self);
        return result;
    }
}

pfds_LinkedList* LinkedList_concat(size_t n, pfds_LinkedList* xs[]) {
    if (n == 0) {
        return pfds_LinkedList_empty();
    } 
    pfds_LinkedList* self = xs[n-1];

    for (size_t i = 0; i < n - 1; i++) {
        self = LinkedList_mappend(xs[n - i - 2], self);
    }
    return self;
}

static pfds_catenablevtable LinkedList_catenable = {
    .mempty = (pfds_object * (*)()) pfds_LinkedList_empty,
    .mappend = (pfds_object* (*)(pfds_object*, pfds_object*)) LinkedList_mappend,
    .concat = (pfds_object* (*)(size_t, pfds_object*[])) LinkedList_concat,
};

pfds_ordering pfds_LinkedList_cmp(pfds_LinkedList* l, pfds_LinkedList* r) {
    while (l->head != NULL && r->head != NULL) {
        pfds_ordering c = pfds_cmp(l->head, r->head);
        if (c != PFDS_EQ) {
            return c;
        }
        l = l->tail;
        r = r->tail;
    }
    if (l->head == NULL) {
        if (r->head == NULL) {
            return PFDS_EQ;
        } else {
            return PFDS_LT;
        }
    } else {
        return PFDS_GT;
    }
}

bool LinkedList_isEmpty(pfds_sequence* self_obj) {
    pfds_LinkedList* self = (pfds_LinkedList*) self_obj;
    return self->head == NULL;
}

extern pfds_LinkedList* pfds_LinkedList_pushFront(pfds_object* head, pfds_LinkedList* tail) {
    pfds_LinkedList* self = pfds_LinkedList_empty();
    self->size = tail->size + 1;
    self->head = head;
    self->tail = tail;
    return self;
}

bool LinkedList_split(pfds_LinkedList** init, pfds_object ** link, pfds_LinkedList** tail, pfds_LinkedList * self, size_t pos) {
    if (self->size <= pos) {
        if (init) *init = NULL;
        if (link) *link = NULL;
        if (tail) *tail = NULL;
        pfds_release(self);
        return false;
    }
    pfds_LinkedList * cursor = self;
    if (init != NULL) {
        pfds_LinkedList* node = pfds_LinkedList_empty();
        *init = node;
        for(size_t i = 0; i < pos; i++) {
            node->size = pos - i;
            node->head = cursor->head;
            pfds_retain(node->head);
            node->tail = pfds_LinkedList_empty();

            node = node->tail;
            cursor = cursor->tail;
        }
    } else {
        for(size_t i = 0; i < pos ; i++) {
            cursor = cursor->tail;
        }
    }
    assert(cursor->size == self->size - pos);
    if (link != NULL) {
        *link = cursor->head;
        pfds_retain(*link);
    }
    cursor = cursor->tail;

    if (tail != NULL) {
        *tail = cursor;
        pfds_retain(*tail);
    }

    pfds_release(self);
    return true;
}

pfds_object* pfds_LinkedList_front(pfds_sequence* self_obj) {
    pfds_LinkedList* self = (pfds_LinkedList*) self_obj;
    if (self->head == NULL) {
        return NULL;
    } else {
        pfds_retain(self->head);
        return self->head;
    }
}

pfds_LinkedList* LinkedList_fromArray(size_t n, pfds_object** elements) {
    pfds_LinkedList* self = pfds_LinkedList_empty();
    for(; n > 0; n--) {
        self = pfds_LinkedList_pushFront(elements[n-1], self);
    }
    return self;
}

pfds_LinkedList* pfds_LinkedList_singleton(pfds_object* x) {
    return pfds_LinkedList_pushFront(x, pfds_LinkedList_empty());
}
pfds_LinkedList* LinkedList_insertBefore(pfds_LinkedList* self, size_t pos, pfds_object* x) {
    if (pos > self->size) {
        panic("LinkedList index out of bounds");
    }

    size_t size = self->size + 1;
    pfds_LinkedList* result = pfds_LinkedList_empty();
    pfds_LinkedList* current = result;

    while (true) {
        current->size = size;
        if (pos == 0) {
            current->head = x;
            current->tail = self;
            return result;
        } else {
            pfds_retain(self->head);
            pfds_retain(self->tail);
            current->head = self->head;
            current->tail = pfds_LinkedList_empty();

            current = current->tail;
            size--;
            pos--;
            pfds_LinkedList* tail = self->tail;
            pfds_release(self);
            self = tail;
        }
    }
}

pfds_LinkedList* LinkedList_updateAt(pfds_LinkedList* self, size_t pos, pfds_object* x) {
    if (pos >= self->size) {
        panic("LinkedList index out of bounds");
    }

    size_t size = self->size;
    pfds_LinkedList* result = pfds_LinkedList_empty();
    pfds_LinkedList* current = result;

    while (true) {
        current->size = size;
        if (pos == 0) {
            current->head = x;
            assert(size >= 1);
            current->tail = self->tail;
            if (self->tail != NULL) {
                pfds_retain(current->tail);
            }
            pfds_release(self);
            return result;
        } else {
            pfds_retain(self->head);
            pfds_retain(self->tail);
            current->head = self->head;
            current->tail = pfds_LinkedList_empty();

            current = current->tail;
            size--;
            pos--;
            pfds_LinkedList* tail = self->tail;
            pfds_release(self);
            self = tail;
        }
    }
}
pfds_LinkedList* LinkedList_deleteAt(pfds_LinkedList* self, size_t pos) {

    size_t size = self->size - 1;

    if (pos >= self->size) {
        panic("LinkedList index out of bounds");
    }

    if (pos == 0) {
        pfds_LinkedList* result = self->tail;
        pfds_retain(result);
        pfds_release(self);
        return result;
    }

    pfds_LinkedList* result = pfds_LinkedList_empty();
    pfds_LinkedList* current = result;

    while (true) {
        current->head = self->head;
        pfds_retain(current->head);
        current->size = size;

        if (pos == 1) {
            current->tail = self->tail->tail;
            pfds_retain(current->tail);
            pfds_release(self);
            return result;
        } else {
            size--;
            pos--;
            current->tail = pfds_LinkedList_empty();
            pfds_LinkedList* tail = self->tail;
            pfds_retain(tail);
            pfds_release(self);
            self = tail;
            current = current->tail;
        }
    }
}

pfds_object* LinkedList_reduceRight(binop op, void* ud, pfds_LinkedList* self, pfds_object* seed) {
    pfds_LinkedList* current = (pfds_LinkedList*) pfds_sequence_reverse((pfds_sequence*) self);
    self = current;
    while(current->head != NULL) {
        pfds_object* elt = current->head;
        pfds_retain(elt);
        seed = op(ud, elt, seed);
        current = current->tail;
    }
    pfds_release(self);
    return seed;
}

pfds_object* LinkedList_back(pfds_LinkedList* self) {
    if (self->head == NULL) {
        return NULL;
    }
    while (self->tail != NULL && self->tail->tail != NULL) {
        self = self->tail;
    }
    pfds_retain(self->head);
    return self->head;
}

pfds_LinkedList* LinkedList_pushBack(pfds_LinkedList* self, pfds_object* back) {
    pfds_LinkedList* current = self;
    pfds_LinkedList* result = pfds_LinkedList_empty();
    pfds_LinkedList* accum = result;
    while (current->head != NULL) {
        accum->size = current->size + 1;
        accum->head = current->head;
        pfds_retain(accum->head);
        accum->tail = pfds_LinkedList_empty();
        accum = accum->tail;
        current = current->tail;
    }
    accum->size = 1;
    accum->head = back; // take back
    accum->tail = pfds_LinkedList_empty();
    pfds_release(self);
    return result;
}

pfds_object* LinkedList_get(pfds_LinkedList* self, size_t n) {
    if (n >= self->size) {
        return NULL;
    }
    for(;;) {
        if (n == 0) {
            pfds_retain(self->head);
            return self->head;
        }
        assert(self->tail != NULL);
        self = self->tail;
        n--;
    }
}

size_t LinkedList_size(pfds_LinkedList * self) {
    return self->size;
}

static pfds_sequencevtable LinkedList_sequence = {
    .catenable = &LinkedList_catenable,
    .isEmpty = LinkedList_isEmpty,
    .pushFront = (pfds_sequence* (*)(pfds_object*,pfds_sequence*)) pfds_LinkedList_pushFront,
    .popFront = pfds_sequence_defaultPopFront, // LinkedList_popFront,
    .front = pfds_LinkedList_front,
    .reverse = pfds_sequence_defaultReverse,
    .fromArray = (pfds_sequence* (*)(size_t, pfds_object**)) LinkedList_fromArray,

    .reduceRight = (pfds_object* (*)(binop, void*, pfds_sequence*, pfds_object*)) LinkedList_reduceRight,
    .popBack = pfds_sequence_defaultPopBack,
    .reduceLeft = pfds_sequence_defaultReduceLeft,
    .back = (pfds_object* (*)(pfds_sequence*)) LinkedList_back,
    .pushBack = (pfds_sequence* (*)(pfds_sequence*, pfds_object*)) LinkedList_pushBack,
    .split = (bool (*)(pfds_sequence**, pfds_object**, pfds_sequence**, pfds_sequence*, size_t))
        LinkedList_split,
    .get = (pfds_object* (*)(pfds_sequence*, size_t)) LinkedList_get,
    .size = (size_t (*)(pfds_sequence*)) LinkedList_size,
    .singleton = (pfds_sequence* (*)(pfds_object*))
        pfds_LinkedList_singleton,
    .insertBefore = (pfds_sequence* (*)(pfds_sequence*, size_t, pfds_object*))
        LinkedList_insertBefore,
    .insertAfter = pfds_sequence_defaultInsertAfter,
    .updateAt = (pfds_sequence* (*)(pfds_sequence*, size_t, pfds_object*))
        LinkedList_updateAt,
    .deleteAt = (pfds_sequence* (*)(pfds_sequence*, size_t))
        LinkedList_deleteAt,
};

void LinkedList_destroy(pfds_LinkedList* self) {
    if (self->head != NULL) pfds_release(self->head);
    if (self->tail != NULL) pfds_release(self->tail);
}

const pfds_objectvtable pfds_LinkedList_vtable = {
    .typename = "LinkedList",
    .destroy = (void (*)(pfds_object*)) LinkedList_destroy,
    .debugfputs = (int (*)(FILE*, pfds_object*))
        pfds_sequence_defaultDebugfputs,
        // LinkedList_debugfputs,
    .cmp = (pfds_ordering (*)(pfds_object*, pfds_object*))
        pfds_LinkedList_cmp,
    .catenable = &LinkedList_catenable,

    .sequence = &LinkedList_sequence,
};

extern pfds_LinkedList* pfds_LinkedList_empty() {
    pfds_LinkedList* self = (pfds_LinkedList*) pfds_object_new(sizeof(pfds_LinkedList), &pfds_LinkedList_vtable);
    self->size = 0;
    self->head = NULL;
    self->tail = NULL;
    return self;
}
