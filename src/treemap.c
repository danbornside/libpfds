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

#include "pfds/pfds-treemap.h"
#include "pfds/pfds-mapping.h"
#include "pfds/pfds-catenable.h"
#include "pfds/pfds-fingertree.h"
#include "pfds/pfds-object-intl.h"
#include "misc.h"

struct pfds_TreeMap {
    pfds_object object;
    pfds_FingerTree* fingerTree;
};

struct TreeMapMeasure {
    pfds_object object;
    size_t size;
    pfds_object* minKey;
    pfds_object* maxKey;
};

struct TreeMapItem {
    pfds_object object;
    union {
        pfds_object_pair item;
        struct {
            pfds_object* key;
            pfds_object* value;
        };
    };
};


struct TreeMapMeasure* TreeMap_measure(struct TreeMapItem* item);
void TreeMap_destroy(pfds_TreeMap* self);
pfds_TreeMap* TreeMap_new(pfds_FingerTree* fingerTree);

const pfds_mappingvtable TreeMap_mapping = {
    .empty = (pfds_mapping* (*)())
        pfds_TreeMap_empty,
    .singleton = (pfds_mapping* (*)(pfds_object *key, pfds_object* value))
        pfds_TreeMap_singleton,
    .fromArray = (pfds_mapping* (*)(size_t, pfds_object_pair*))
        pfds_TreeMap_fromArray,
    .isEmpty = (bool (*)(pfds_mapping*))
        pfds_TreeMap_isEmpty,
    .size = (size_t (*)(pfds_mapping*))
        pfds_TreeMap_size,
    .lookup = (pfds_object* (*)(pfds_mapping*, pfds_object*))
        pfds_TreeMap_lookup,
    .popMin = (bool (*)(pfds_object_pair*, pfds_mapping**, pfds_mapping*))
        pfds_TreeMap_popMin,
    .insert = (pfds_mapping* (*)(pfds_mapping* self, pfds_object* key, pfds_object* value))
        pfds_TreeMap_insert,
    .erase = (pfds_mapping* (*)(pfds_mapping* self, pfds_object* key))
        pfds_TreeMap_erase,

};

const pfds_objectvtable pfds_TreeMap_vtable = {
    .typename = "TreeMap",
    .destroy = (void (*)(pfds_object*))
        TreeMap_destroy,
    .cmp = (pfds_ordering (*)(pfds_object*, pfds_object*))
        pfds_mapping_defaultCmp,
    .debugfputs = (int (*)(FILE*, pfds_object*))
        pfds_mapping_defaultDebugfputs,
    .mapping = &TreeMap_mapping,
};


bool pred_TreeMap_findKey(pfds_object* k, struct TreeMapMeasure* measure);

struct TreeMapItem* TreeMapItem_new(pfds_object* k, pfds_object* v);
void TreeMapItem_destroy(struct TreeMapItem* self);

const pfds_objectvtable TreeMapItem_vtable = {
    .typename = "TreeMapItem",
    .destroy = (void (*)(pfds_object*))
            TreeMapItem_destroy,
};

struct TreeMapMeasure* TreeMapMeasure_mempty(void);
struct TreeMapMeasure* TreeMapMeasure_mappend(struct TreeMapMeasure*, struct TreeMapMeasure*);
struct TreeMapMeasure* TreeMapMeasure_concat(size_t, struct TreeMapMeasure**);

const pfds_catenablevtable TreeMapMeasure_catenablevtable = {
    .mempty = (pfds_object* (*)(void))
        TreeMapMeasure_mempty,
    .mappend = (pfds_object* (*)(pfds_object*, pfds_object*))
        TreeMapMeasure_mappend,
    .concat = (pfds_object* (*)(size_t, pfds_object**))
        TreeMapMeasure_concat,
};

void TreeMapMeasure_destroy(struct TreeMapMeasure*);

const pfds_objectvtable TreeMapMeasure_vtable = {
    .typename = "TreeMapMeasure",
    .destroy = (void (*)(pfds_object*))
            TreeMapMeasure_destroy,
    .catenable = &TreeMapMeasure_catenablevtable,
};

struct TreeMapMeasure* TreeMap_measure(struct TreeMapItem* item) {
    struct TreeMapMeasure * measure = (struct TreeMapMeasure*) pfds_object_new(sizeof(struct TreeMapMeasure), &TreeMapMeasure_vtable);
    measure->size = 1;
    pfds_retain(item->key);
    pfds_retain(item->key);
    measure->minKey = measure->maxKey = item->key;
    return measure;
}


struct TreeMapItem* TreeMapItem_new(pfds_object* k, pfds_object* v) {
    struct TreeMapItem* self = (struct TreeMapItem*) pfds_object_new(sizeof(struct TreeMapItem), &TreeMapItem_vtable);
    self->key = k;
    self->value = v;
    return self;
}

void TreeMapItem_destroy(struct TreeMapItem* self) {
    pfds_release(self->key);
    pfds_release(self->value);
}


void TreeMapMeasure_destroy(struct TreeMapMeasure* self) {
    pfds_release(self->minKey);
    pfds_release(self->maxKey);
}

struct TreeMapMeasure* TreeMapMeasure_mempty(void) {
    struct TreeMapMeasure * measure = (struct TreeMapMeasure*) pfds_object_new(sizeof(struct TreeMapMeasure), &TreeMapMeasure_vtable);
    measure->size = 0;
    measure->minKey = measure->maxKey = NULL;
    return measure;
}

struct TreeMapMeasure* TreeMapMeasure_mappend(struct TreeMapMeasure* l, struct TreeMapMeasure* r) {

    struct TreeMapMeasure * measure = (struct TreeMapMeasure*) pfds_object_new(sizeof(struct TreeMapMeasure), &TreeMapMeasure_vtable);
    measure->size = l->size + r->size;
    if (l->minKey == NULL) {
        measure->minKey = r->minKey;
    } else if (r->minKey == NULL) {
        measure->minKey = l->minKey;
    } else {
        switch (pfds_cmp(l->minKey, r->minKey)) {
            case PFDS_LT:
                measure->minKey = l->minKey;
            default:
                measure->minKey = r->minKey;
        }
    }
    pfds_retain(measure->minKey);

    if (l->maxKey == NULL) {
        measure->maxKey = r->maxKey;
    } else if (r->maxKey == NULL) {
        measure->maxKey = l->maxKey;
    } else {
        switch (pfds_cmp(l->maxKey, r->maxKey)) {
            case PFDS_GT:
                measure->maxKey = l->maxKey;
            default:
                measure->maxKey = r->maxKey;
        }
    }
    pfds_retain(measure->maxKey);
    pfds_release(l);
    pfds_release(r);

    return measure;
}

struct TreeMapMeasure* TreeMapMeasure_concat(size_t, struct TreeMapMeasure**) {
    panic("TODO");
}


pfds_TreeMap* TreeMap_new(pfds_FingerTree* fingerTree) {
    pfds_TreeMap * self = (pfds_TreeMap*) pfds_object_new(sizeof(pfds_TreeMap), &pfds_TreeMap_vtable);
    self->fingerTree = fingerTree;
    return self;
}

void TreeMap_destroy(pfds_TreeMap* self) {
    pfds_release(self->fingerTree);
}

pfds_TreeMap* pfds_TreeMap_fromArray(size_t n, pfds_object_pair items[]) {
    pfds_TreeMap* self = pfds_TreeMap_empty();
    for (size_t i = 0 ; i < n; i ++) {
        self = pfds_TreeMap_insert(self, items[i].key, items[i].value);
    }
    return self;
}

extern pfds_TreeMap* pfds_TreeMap_singleton(pfds_object* key, pfds_object* value) {
    return TreeMap_new(pfds_FingerTree_singleton((pfds_object*) TreeMapItem_new(key, value)));
}

extern pfds_TreeMap* pfds_TreeMap_empty(void) {
    return TreeMap_new(pfds_FingerTree_empty());
}

bool pfds_TreeMap_isEmpty(pfds_TreeMap* self) {
    return pfds_FingerTree_isEmpty(self->fingerTree);
}

size_t pfds_TreeMap_size(pfds_TreeMap* self) {
    struct TreeMapMeasure* mm = (struct TreeMapMeasure*) pfds_FingerTree_measure(
            &TreeMapMeasure_catenablevtable,
            (pfds_measure) TreeMap_measure,
            self->fingerTree);
    size_t result = mm->size;
    pfds_release(mm);
    return result;
}

pfds_object* pfds_TreeMap_lookup(pfds_TreeMap* self, pfds_object* key) {
    // self,key
    pfds_retain(self->fingerTree);
    // self,key,self->fingerTree

    struct TreeMapItem* pivot;
    bool result = pfds_FingerTree_split( NULL, (pfds_object**) &pivot, NULL,
            &TreeMapMeasure_catenablevtable, (pfds_measure) TreeMap_measure,
            (bool (*)(void*, pfds_object*)) pred_TreeMap_findKey, key,
            self->fingerTree);
    // pivot,self,key
    if (!result) {
        // self,key
        return NULL;
    } else if (pfds_cmp(pivot->key, key) == PFDS_EQ) {
        // pivot,self,key
        pfds_object* val = pivot->value;
        pfds_release(pivot);
        // self[val],key
        return val;
    } else {
        pfds_release(pivot);
        return NULL;
    }

}

bool pfds_TreeMap_popMin(pfds_object_pair* head, pfds_TreeMap** tail, pfds_TreeMap* self) {
    struct TreeMapItem* myHead;
    pfds_FingerTree* myTail;
    pfds_retain(self->fingerTree);
    bool result = pfds_FingerTree_popFront(
            head == NULL ? NULL : (pfds_object**) &myHead,
            tail == NULL ? NULL : &myTail,
            &TreeMapMeasure_catenablevtable, (pfds_measure) TreeMap_measure,
            self->fingerTree);
    if (result && head != NULL) {
        *head = myHead->item;
        pfds_retain(head->key);
        pfds_retain(head->value);
        pfds_release(myHead);
    }
    if (result && tail != NULL) {
        *tail = TreeMap_new(myTail);
    }
    pfds_release(self);
    return result;
}

pfds_TreeMap* pfds_TreeMap_erase(pfds_TreeMap* self, pfds_object* key) {
    // if (pfds_FingerTree_isEmpty(fingerTree)) {
    //     return self;
    // }

    pfds_FingerTree* fingerTree = self->fingerTree;
    pfds_retain(fingerTree);

    pfds_FingerTree *init;
    struct TreeMapItem *oldPivot;
    pfds_FingerTree *tail;

    bool foundPivot = pfds_FingerTree_split(&init, (pfds_object**) &oldPivot, &tail,
            &TreeMapMeasure_catenablevtable, (pfds_measure) TreeMap_measure,
            (bool (*)(void*, pfds_object*)) pred_TreeMap_findKey, key,
            fingerTree);
    if (foundPivot) { // init <> ... <> tail
        pfds_release(self);
        size_t nPivots;
        struct TreeMapItem* newPivots[1];
        if(pfds_cmp(oldPivot->key, key) != PFDS_EQ) { // init <> oldPivot <> tail
            nPivots = 1;
            newPivots[0] = oldPivot;
        } else { // init <> tail
            nPivots = 0;
            pfds_release(oldPivot);
        }
        return TreeMap_new(pfds_FingerTree_app3(
                    &TreeMapMeasure_catenablevtable, (pfds_measure) TreeMap_measure,
                    init, nPivots, (pfds_object**) newPivots, tail));
    } else { // init
        return self;
    }

}

bool pred_TreeMap_findKey(pfds_object* k, struct TreeMapMeasure* measure) {
    return measure->maxKey != NULL && pfds_cmp(k, measure->maxKey) <= PFDS_EQ;
}

pfds_TreeMap* pfds_TreeMap_insert(pfds_TreeMap* self, pfds_object* key, pfds_object* value) {
    pfds_FingerTree* fingerTree = self->fingerTree;
    pfds_retain(fingerTree);
    pfds_release(self);

    // if (pfds_FingerTree_isEmpty(fingerTree)) {
    //     pfds_release(fingerTree);
    //     return pfds_TreeMap_singleton(key, value);
    // }

    pfds_FingerTree *init;
    struct TreeMapItem *oldPivot;
    pfds_FingerTree *tail;

    bool foundPivot = pfds_FingerTree_split(&init, (pfds_object**) &oldPivot, &tail,
            &TreeMapMeasure_catenablevtable, (pfds_measure) TreeMap_measure,
            (bool (*)(void*, pfds_object*)) pred_TreeMap_findKey, key,
            fingerTree);
    if (foundPivot) { // init <> ... <> tail
        size_t nPivots;
        struct TreeMapItem* newPivots[2];
        newPivots[0] = TreeMapItem_new(key, value);
        if(pfds_cmp(oldPivot->key, key) != PFDS_EQ) { // init <> key.value <> oldPivot <> tail
            nPivots = 2;
            newPivots[1] = oldPivot;
        } else { // init <> key.value <> tail
            nPivots = 1;
            pfds_release(oldPivot);
        }
        return TreeMap_new(pfds_FingerTree_app3(
                    &TreeMapMeasure_catenablevtable, (pfds_measure) TreeMap_measure,
                    init, nPivots, (pfds_object**) newPivots, tail));
    } else { // init <> key.value
        return TreeMap_new(pfds_FingerTree_pushBack(
                    &TreeMapMeasure_catenablevtable, (pfds_measure) TreeMap_measure,
                    init,
                    (pfds_object*) TreeMapItem_new(key, value)));
    }

}


