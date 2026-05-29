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


#include "pfds/pfds-sequence.h"
#include "pfds/pfds-mapping.h"
#include "pfds/pfds-string.h"
#include "pfds/pfds-catenable.h"
#include "pfds/pfds-object-intl.h"
#include "misc.h"

#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#if defined (PFDS_GC_DEBUGREFCOUNT) || defined (PFDS_GC_REFCOUNT)
static size_t global_births = 0;
static size_t global_deaths = 0;
static size_t global_retaincount = 0;
static size_t global_releasecount = 0;

struct pfds_gcinfo pfds_getgcinfo(void) {
    struct pfds_gcinfo gcinfo = {
        .births = global_births,
        .deaths = global_deaths,
        .retaincount = global_retaincount,
        .releasecount = global_releasecount };
    return gcinfo;
}

#endif

extern pfds_object* pfds_object_new(size_t size, const pfds_objectvtable* vtable) {
    pfds_object* self = (pfds_object*) malloc(size);
    // memset(self, '\xFF', size);
    self->vtable = vtable;
#if defined (PFDS_GC_DEBUGREFCOUNT) || defined (PFDS_GC_REFCOUNT)
    global_births++; global_retaincount++;
#endif
#if defined (PFDS_GC_DEBUGREFCOUNT)
    self->retaincount = self->releasecount = 0;
    self->finalized = false;
    int refcount = 1 + self->retaincount - self->releasecount;
    fprintf(stderr, "GC: <%s:%p> NEW%s refcount=%d\n",
            self->vtable->typename, self,
            self->finalized ? " DEAD" : "", refcount);
#elif defined (PFDS_GC_REFCOUNT)
    self->refcount = 1;
#elif defined (PFDS_GC_NONE)
#endif
    return self;
}

#if defined (PFDS_GC_DEBUGREFCOUNT)
extern void pfds_object_retain(pfds_object* self, char* fn, int ln) {
    global_retaincount++;
    if(self != NULL) {
        if (self->retaincount == INT_MAX) panic("pfds_object_retain overflow");
        self->retaincount++;
        int refcount = 1 + self->retaincount - self->releasecount;
        fprintf(stderr, "GC: <%s:%p>%s ++refcount=%d %s:%d\n",
                self->vtable->typename, self,
                self->finalized ? " USE AFTER FREE/DEAD" : "", refcount,
                fn, ln);
    }
}
#elif defined (PFDS_GC_REFCOUNT)
extern void pfds_object_retain(pfds_object* self) {
    global_retaincount++;
    if(self != NULL) {
        if (self->refcount == INT_MAX) panic("pfds_object_retain overflow");
        self->refcount++;
    }
}
#elif defined (PFDS_GC_NONE)
#endif

#if defined (PFDS_GC_DEBUGREFCOUNT)
extern void pfds_object_release(pfds_object* self, char* fn, int ln) {
    if(self != NULL) {
        global_releasecount++;
        // if (self->releasecount == INT_MAX) panic("pfds_object_release overflow");
        self->releasecount++;
        int refcount = 1 + (ptrdiff_t) self->retaincount - self->releasecount;
        if (self->releasecount > self->retaincount + 1) {
            fprintf(stderr, "GC: <%s:%p>%s --refcount=%ld [r:%ld/o:%ld] %s:%d\n",
                    self->vtable->typename, self,
                    self->finalized ? " DEAD" : "", (long) refcount,
                    global_retaincount - global_releasecount,
                    global_births - global_deaths,
                    fn, ln
                    );
            panic("too many releases");
        }
        if (self->releasecount > self->retaincount) {
            if (self->vtable->destroy) {
                self->vtable->destroy(self);
            }
            self->finalized = true;
            global_deaths++;
        }
        fprintf(stderr, "GC: <%s:%p>%s --refcount=%ld [r:%ld/o:%ld] %s:%d\n",
                self->vtable->typename, self,
                self->finalized ? " DEAD" : "", (long) refcount,
                global_retaincount - global_releasecount,
                global_births - global_deaths,
                fn, ln
                );
    }
}
#elif defined (PFDS_GC_REFCOUNT)
extern void pfds_object_release(pfds_object* self) {
    if(self != NULL) {
        global_releasecount++;
        if (self->refcount == 0) {
            fprintf(stderr, "DOUBLE RELEASE: %s:%p\n", self->vtable->typename, self);
            panic("double release");
        }
        if (--(self->refcount) <= 0) {
            global_deaths++;
            if (self->vtable->destroy) {
                self->vtable->destroy(self);
            }
            free(self);
        }
    }
}
#elif defined (PFDS_GC_NONE)
#endif

extern int pfds_object_debugfputs(FILE* stream, pfds_object* self) {
    if (self == NULL) {
        return fprintf(stream, "%s", "<NULL>");
    } else {
        if (self->vtable->debugfputs == NULL) {
            return fprintf(stream, "<OBJECT@%p>", self);
        } else {
            return self->vtable->debugfputs(stream, self);
        }
    }
}

extern pfds_String* pfds_object_toString(pfds_object* self) {
    char* buf;
    size_t size = 0;
    FILE* stream = open_memstream(&buf, &size);
    pfds_object_debugfputs(stream, self);
    fclose(stream);
    return pfds_String_new(buf, size, true);
}

extern pfds_object* pfds_object_mappend(pfds_object* l, pfds_object* r) {
    if (l == NULL || r == NULL) {
        panic("mappend on NULL");
    } else if ( l->vtable == NULL
            || l->vtable->catenable == NULL
            || l->vtable->catenable->mappend == NULL) {
        panic("mappend not a catenable");
    } else if (l->vtable != r->vtable) {
        panic("mappend on dissimilar types");
    } else {
        return l->vtable->catenable->mappend(l, r);
    }
}

extern pfds_ordering pfds_object_cmp(pfds_object* l, pfds_object* r) {
    const pfds_objectvtable *vtable = (l && l->vtable) ? l->vtable : NULL;
    if (vtable == NULL
            || vtable->cmp == NULL
            || r->vtable != vtable
       ) {
        panic("cmp on incompatible types");
    }
    return vtable->cmp(l, r);
}

#define GUARD_SEQUENCE_METHOD(self, method) \
    {   if (self == NULL) panic ("Null Reference::" #method); \
        if (self->object.vtable == NULL || self->object.vtable->sequence == NULL) panic ("not a sequence: " #method); \
        if (self->object.vtable->sequence->method == NULL) { \
            fprintf(stderr, "%s  ", self->object.vtable->typename); \
            panic ( "sequence->" #method " not implemented"); \
        } \
    }

extern bool pfds_sequence_isEmpty (pfds_sequence* self) {
    GUARD_SEQUENCE_METHOD(self, isEmpty);
    return self->object.vtable->sequence->isEmpty(self);
}
extern pfds_sequence* pfds_sequence_pushFront (pfds_object* elem,pfds_sequence* self) {
    GUARD_SEQUENCE_METHOD(self, pushFront);
    return self->object.vtable->sequence->pushFront(elem, self);

}
extern bool pfds_sequence_popFront (pfds_object** head, pfds_sequence** tail, pfds_sequence* self) {
    GUARD_SEQUENCE_METHOD(self, popFront);
    return self->object.vtable->sequence->popFront(head, tail, self);
}

extern pfds_object* pfds_sequence_front(pfds_sequence* self) {
    GUARD_SEQUENCE_METHOD(self, front);
    return self->object.vtable->sequence->front(self);
}

extern pfds_object* pfds_sequence_back(pfds_sequence* self) {
    GUARD_SEQUENCE_METHOD(self, back);
    return self->object.vtable->sequence->back(self);
}

extern pfds_object* pfds_sequence_defaultFront(pfds_sequence* self) {
    return pfds_sequence_get(self, 0);
}

extern pfds_object* pfds_sequence_defaultBack(pfds_sequence* self) {
    return pfds_sequence_get(self, pfds_sequence_size(self) - 1);
}

extern size_t pfds_sequence_size(pfds_sequence* self) {
    GUARD_SEQUENCE_METHOD(self, size);
    return self->object.vtable->sequence->size(self);
}

extern pfds_object* pfds_sequence_get(pfds_sequence* self, size_t idx) {
    GUARD_SEQUENCE_METHOD(self, get);
    return self->object.vtable->sequence->get(self, idx);
}

extern pfds_object* pfds_sequence_reduceLeft(binop xy2x, void* ud, pfds_object* x, pfds_sequence* self) {
    GUARD_SEQUENCE_METHOD(self, reduceLeft);
    return self->object.vtable->sequence->reduceLeft(xy2x, ud, x, self);
}

extern pfds_object* pfds_sequence_defaultReduceLeft(binop xy2x, void* ud, pfds_object* x, pfds_sequence* self) {
    pfds_object* elem;
    while(pfds_sequence_popFront(&elem, &self, self)) {
        x = xy2x(ud, x, elem);
    }
    return x;
}

extern pfds_object* pfds_sequence_defaultReduceRight(binop xy2x, void* ud, pfds_sequence* self, pfds_object* y) {
    pfds_object* elem;
    while(pfds_sequence_popBack(&self, &elem, self)) {
        y = xy2x(ud, elem, y);
    }
    return y;
}

extern pfds_object* pfds_sequence_reduceRight(binop xy2y, void* ud, pfds_sequence* self, pfds_object* y) {
    GUARD_SEQUENCE_METHOD(self, reduceRight);
    return self->object.vtable->sequence->reduceRight(xy2y, ud, self, y);
}

extern pfds_sequence* pfds_sequence_reverse(pfds_sequence* self) {
    GUARD_SEQUENCE_METHOD(self, reverse);
    return self->object.vtable->sequence->reverse(self);
}

extern pfds_sequence* pfds_sequence_pushBack(pfds_sequence* self, pfds_object* elem) {
    GUARD_SEQUENCE_METHOD(self, pushBack);
    return self->object.vtable->sequence->pushBack(self, elem);
}

extern bool pfds_sequence_split(pfds_sequence** init, pfds_object** link, pfds_sequence** tail, pfds_sequence* self, size_t pos) {
    GUARD_SEQUENCE_METHOD(self, split);
    return self->object.vtable->sequence->split(init, link, tail, self, pos);
}

extern bool pfds_sequence_popBack(pfds_sequence** init, pfds_object** last, pfds_sequence* self) {
    GUARD_SEQUENCE_METHOD(self, popBack);
    return self->object.vtable->sequence->popBack(init, last, self);
}

extern pfds_sequence* pfds_sequence_deleteAt(pfds_sequence* self, size_t n) {
    GUARD_SEQUENCE_METHOD(self, deleteAt);
    return self->object.vtable->sequence->deleteAt(self, n);
}
extern pfds_sequence* pfds_sequence_insertBefore(pfds_sequence* self, size_t n, pfds_object* x) {
    GUARD_SEQUENCE_METHOD(self, insertBefore);
    return self->object.vtable->sequence->insertBefore(self, n, x);
}
extern pfds_sequence* pfds_sequence_insertAfter(pfds_sequence* self, size_t n, pfds_object* x) {
    GUARD_SEQUENCE_METHOD(self, insertAfter);
    return self->object.vtable->sequence->insertAfter(self, n, x);
}
extern pfds_sequence* pfds_sequence_updateAt(pfds_sequence* self, size_t n, pfds_object* x) {
    GUARD_SEQUENCE_METHOD(self, updateAt);
    return self->object.vtable->sequence->updateAt(self, n, x);
}

extern bool pfds_sequence_defaultPopFront(pfds_object** head, pfds_sequence** tail, pfds_sequence* self) {
    return pfds_sequence_split(NULL, head, tail, self, 0);
}

extern bool pfds_sequence_defaultPopBack(pfds_sequence** init, pfds_object** last, pfds_sequence* self) {
    return pfds_sequence_split(init, last, NULL, self, pfds_sequence_size(self) - 1);
}

extern pfds_sequence* pfds_sequence_defaultInsertBefore(pfds_sequence* self, size_t n, pfds_object* x) {
    if (n == 0) {
        return pfds_sequence_pushFront(x, self);
    } else {
        size_t mySize = pfds_sequence_size(self);
        if (n == mySize) {
            return pfds_sequence_pushBack(self, x);
        } else if (n > mySize) {
            panic("list index out of bounds");
        } else {
            const pfds_sequencevtable* dict = self->object.vtable->sequence;

            pfds_sequence* init;
            pfds_object* link[2];
            link[0] = x;
            pfds_sequence* tail;
            assert(pfds_sequence_split(&init, &link[1], &tail, self, n));
            pfds_sequence* links = dict->fromArray(2, link);

            pfds_sequence* selves[3] = { init, links, tail };

            return (pfds_sequence*) dict->catenable->concat(3, (pfds_object**) selves);
        }
    }
}
extern pfds_sequence* pfds_sequence_defaultUpdateAt(pfds_sequence* self, size_t n, pfds_object* x) {
    size_t mySize = pfds_sequence_size(self);
    if (n > mySize) {
        panic("list index out of bounds");
    } else {
        const pfds_sequencevtable* dict = self->object.vtable->sequence;

        pfds_sequence* init;
        pfds_sequence* tail;
        assert(pfds_sequence_split(&init, NULL, &tail, self, n));

        // pfds_sequence* links = ;
        pfds_sequence* selves[3] = { init, dict->singleton(x), tail };

        return (pfds_sequence*) dict->catenable->concat(3, (pfds_object**) selves);
    }
}

extern int pfds_sequence_defaultDebugfputs (FILE* stream, pfds_sequence* self) {
    bool printedBrace = false;
    int n = 0;
    pfds_retain(self);
    pfds_object* head = NULL;
    pfds_sequence* tail = self;
    while(pfds_sequence_popFront(&head, &tail, tail)) {
        n += fprintf(stream, "%s", printedBrace ? ", " : "[");
        printedBrace = true;
        n += pfds_object_debugfputs(stream, head);
        pfds_release(head);
    }
    n += fprintf(stream, "%s", printedBrace ? "]": "[]");
    return n;
}

extern pfds_sequence* pfds_sequence_defaultReverse(pfds_sequence* self) {
    pfds_sequence* result = (pfds_sequence*) self->object.vtable->catenable->mempty();
    pfds_object* head;
    while(pfds_sequence_popFront(&head, &self, self)) {
        result = pfds_sequence_pushFront(head, result);
    }
    return result;
}

extern pfds_sequence* pfds_sequence_mappend(pfds_sequence* l, pfds_sequence* r) {
    return (pfds_sequence*) pfds_object_mappend((pfds_object*) l, (pfds_object*) r);
}


extern pfds_sequence* pfds_sequence_defaultDeleteAt(pfds_sequence* self, size_t n) {
    pfds_sequence* init;
    pfds_object* link;
    pfds_sequence* tail;
    bool res = pfds_sequence_split(&init, &link, &tail, self, n);
    if (res) {
        pfds_release(link);
        return pfds_sequence_mappend(init, tail);
    } else {
        return NULL;
    }
}

extern pfds_sequence* pfds_sequence_defaultInsertAfter(pfds_sequence* self, size_t n, pfds_object* x) {
    return pfds_sequence_insertBefore(self, n+1, x);
}


#define GUARD_MAPPING_METHOD(self, method) \
    {   if (self == NULL) panic ("Null Reference::" #method); \
        if (self->object.vtable == NULL || self->object.vtable->mapping == NULL) panic ("not a mapping: " #method); \
        if (self->object.vtable->mapping->method == NULL) { \
            fprintf(stderr, "%s  ", self->object.vtable->typename); \
            panic ( "mapping->" #method " not implemented"); \
        } \
    }


extern bool pfds_mapping_isEmpty (pfds_mapping* self) {
    GUARD_MAPPING_METHOD(self, isEmpty);
    return self->object.vtable->mapping->isEmpty(self);
}

extern size_t pfds_mapping_size (pfds_mapping* self) {
    GUARD_MAPPING_METHOD(self, size);
    return self->object.vtable->mapping->size(self);
}

pfds_object* pfds_mapping_lookup(pfds_mapping* self, pfds_object* key) {
    GUARD_MAPPING_METHOD(self, lookup);
    return self->object.vtable->mapping->lookup(self, key);
}

bool pfds_mapping_popMin(pfds_object_pair* item, pfds_mapping** rest, pfds_mapping* self) {
    GUARD_MAPPING_METHOD(self, popMin);
    return self->object.vtable->mapping->popMin(item, rest, self);
}

pfds_mapping* pfds_mapping_insert(pfds_mapping* self, pfds_object* key, pfds_object* value) {
    GUARD_MAPPING_METHOD(self, insert);
    return self->object.vtable->mapping->insert(self, key, value);
}

pfds_mapping* pfds_mapping_erase(pfds_mapping* self, pfds_object* key) {
    GUARD_MAPPING_METHOD(self, erase);
    return self->object.vtable->mapping->erase(self, key);
}

int pfds_mapping_defaultDebugfputs(FILE* stream, pfds_mapping* self) {
    pfds_retain(self);
    pfds_object_pair item;
    int n = fprintf(stream, "{");
    while(pfds_mapping_popMin(&item, &self, self)) {
        n += fprintf(stream, "%s", n == 1 ? "" : ", ");
        n += pfds_object_debugfputs(stream, item.key);
        n += fprintf(stream, ":");
        n += pfds_object_debugfputs(stream, item.value);
        pfds_release(item.key);
        pfds_release(item.value);
    }
    n += fprintf(stream, "}");
    return n;
}

pfds_ordering pfds_mapping_defaultCmp(pfds_mapping* l, pfds_mapping* r) {
    size_t lSize = pfds_mapping_size(l);
    size_t rSize = pfds_mapping_size(r);
    if (lSize < rSize) {
        return PFDS_LT;
    } else if (lSize > rSize) {
        return PFDS_GT;
    } else if (l == 0) {
        return PFDS_EQ;
    } else {
        pfds_retain(l);
        pfds_retain(r);
        // l0, r0, l, r
        pfds_object_pair lItem, rItem;
        pfds_ordering result;
        do {
            bool lSplit = pfds_mapping_popMin(&lItem, &l, l);
            bool rSplit = pfds_mapping_popMin(&rItem, &r, r);
            // lItem, l, rItem, r, l0, r0
            if (lSplit != rSplit) {
                panic("invalid popMin in pfds_mapping_defaultCmp");
            }
            if (!lSplit) {
                //  l0, r0
                return PFDS_EQ;
            }
            result = pfds_cmp(lItem.key, rItem.key);
            if (result != PFDS_EQ) {
                break;
            }
            result = pfds_cmp(lItem.value, rItem.value);
            if (result != PFDS_EQ) {
                break;
            }
        } while (true);
        // lItem, l, rItem, r, l0, r0
        pfds_release(lItem.key); pfds_release(lItem.value); pfds_release(l);
        pfds_release(rItem.key); pfds_release(rItem.value); pfds_release(r);
        return result;
    }
}



void* bsearch_alt(const void *userData, const void *base, size_t n, size_t size, bool (*pred)(const void*, const void*)) {
    assert(size > 0);
    if (n == 0) {
        return (void*) base;
    }
    if (pred(userData, base)) {
        return (void *) base;
    }
    if (!pred(userData, base + (n - 1) * size)) {
        return (void *) (base + n * size);
    }
    size_t lb = 0;
    size_t ub = (n-1);
    while (lb + 1 < ub) {
        size_t mid = (lb + ub) >> 1;
        if (pred(userData, base + mid * size)) {
            ub = mid;
        } else {
            lb = mid;
        }
    }
    return (void *) (base + ub * size);

}
