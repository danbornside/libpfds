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

#include "pfds/pfds-object-intl.h"
#include "pfds/pfds-sequence.h"
#include "pfds/pfds-catenable.h"
#include "pfds/pfds-num.h"
#include "pfds/pfds-treelist.h"
#include "pfds/pfds-fingertree.h"
#include "misc.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>



struct pfds_TreeList {
    pfds_object object;
    pfds_FingerTree* fingerTree;
};


pfds_TreeList* TreeList_new(pfds_FingerTree* fingerTree) {
    pfds_TreeList* self = (pfds_TreeList*) pfds_object_new(sizeof(pfds_TreeList), &pfds_TreeList_vtable);
    self->fingerTree = fingerTree;
    return self;
}

pfds_object* measure_TreeList_size(pfds_object*) {
    return (pfds_object*) pfds_UInt64_new(1);
}

/** combine two elements of the group
 *
 * \param l
 * \param r
 * \invariant give(return) take(l) take (r)
 */
extern pfds_TreeList* pfds_TreeList_mappend(pfds_TreeList* l, pfds_TreeList* r) {
    pfds_FingerTree *ll = l->fingerTree;
    pfds_retain(ll);
    pfds_release(l);
    pfds_FingerTree *rr = r->fingerTree;
    pfds_retain(rr);
    pfds_release(r);
    return TreeList_new(pfds_FingerTree_app3(
            &pfds_catenable_sum, measure_TreeList_size,
            ll, 0, NULL, rr));
}

/** the empty TreeList
 *
 * \invariant give(return)
 */
extern pfds_TreeList* pfds_TreeList_mempty(void) { 
    return TreeList_new(pfds_FingerTree_empty());
}

/** construct a member of the group concatenating an array of several
 * elements of a suitable type.
 *
 * \param n number of elements
 * \param xs
 * \invariant give(return) take(xs[0..n])
 *
 */

extern pfds_TreeList* pfds_TreeList_concat(size_t n, pfds_TreeList** xs) {
    if (n == 0) {
        return pfds_TreeList_mempty();
    }
    pfds_TreeList* self = xs[0];
    for (size_t i = 1; i < n; i++) {
        self = pfds_TreeList_mappend(self, xs[i]);
    }
    return self;
}

const pfds_catenablevtable TreeList_catenable = {
    .mappend = (pfds_object* (*)(pfds_object*, pfds_object*))
        pfds_TreeList_mappend,
    .concat = (pfds_object* (*)(size_t, pfds_object**))
        pfds_TreeList_concat,
    .mempty = (pfds_object* (*)(void))
        pfds_TreeList_mempty,
};



extern pfds_TreeList* pfds_TreeList_singleton(pfds_object* x) {
    return TreeList_new(pfds_FingerTree_singleton(x));
}

/** construct a new sequence of the selected type from an array of the given objects.
 * the pfds_sequence_fromArray() global function returns the default implementation.
 *
 * \param n
 * \param xs
 * \invariant give(return) take(xs[0..n])
 */
extern pfds_TreeList* pfds_TreeList_fromArray(size_t n, pfds_object** xs) {
    return TreeList_new(pfds_FingerTree_pushBackArray(
            &pfds_catenable_sum, measure_TreeList_size,
            pfds_FingerTree_empty(),
            n, xs));
}

/** test if a TreeList is empty
 *
 * \param self
 * \return true if self is the empty sequence, false otherwise.
 * \invariant borrow(self)
 *
 */
extern bool pfds_TreeList_isEmpty(pfds_TreeList* self) {
    return pfds_FingerTree_isEmpty(self->fingerTree);
}

/** add an element to the front of a TreeList
 *
 * \param head
 * \param tail
 * \invariant give(return) take(head) take(tail)
 */
extern pfds_TreeList* pfds_TreeList_pushFront (pfds_object* head, pfds_TreeList* tail) {
    pfds_FingerTree* fingerTree = tail->fingerTree;
    pfds_retain(fingerTree);
    pfds_release(tail);
    return TreeList_new(
            pfds_FingerTree_pushFront(
                &pfds_catenable_sum, measure_TreeList_size,
                head, fingerTree));
}


/** add an element to the back of a sequence.
 *
 * \param last
 * \param init
 * \invariant give(return) take(init) take(last)
 */
extern pfds_TreeList* pfds_TreeList_pushBack (pfds_TreeList* init, pfds_object* last) {
    pfds_FingerTree* fingerTree = init->fingerTree;
    pfds_retain(fingerTree);
    pfds_release(init);
    return TreeList_new(
            pfds_FingerTree_pushBack(
                &pfds_catenable_sum, measure_TreeList_size,
                fingerTree, last));
}

bool pred_TreeList_size(unsigned long n, pfds_UInt64* m) {
    return n < pfds_UInt64_get(m);
}


/** split sequence into intial sequence, selected element, and remaining sequence.  outputs NULL and returns false if sequence does not contain selected element
 *
 * \param init[out] first n-1 elements
 * \param link[out] nth element
 * \param last[out] n+1 thru last elements
 * \param self[in] sequence to be examined.
 * \param size[in] sequence to be examined.
 * \return true if sequence is large enough to contain nth element.
 * \invariant when(return == true) give(init) give(link) give(tail) take(self)
 *      | when(return == false) take(self)
 */
extern bool pfds_TreeList_split (pfds_TreeList** init, pfds_object** link, pfds_TreeList** tail, pfds_TreeList* self, size_t n) {
    // [self]
    pfds_FingerTree* fingerTree = self->fingerTree;
    pfds_retain(fingerTree);
    pfds_release(self);
    // [fingerTree]
    pfds_FingerTree* myInit;
    pfds_object* myPivot;
    pfds_FingerTree* myTail;
    if(pfds_FingerTree_split(
            &myInit, &myPivot, &myTail,
            &pfds_catenable_sum, measure_TreeList_size,
            (bool (*)(void*, pfds_object*)) pred_TreeList_size, (void*) n,
            fingerTree)) {
        // [myInit, myPivot, myTail]
        OPTIONAL_OUTPARAM2(init, TreeList_new(myInit), pfds_release(myInit));
        OPTIONAL_OUTPARAM2(link, myPivot,              pfds_release(myPivot));
        OPTIONAL_OUTPARAM2(tail, TreeList_new(myTail), pfds_release(myTail));
        return true;
    } else {
        OPTIONAL_OUTPARAM(init, NULL);
        OPTIONAL_OUTPARAM(link, NULL);
        OPTIONAL_OUTPARAM(tail, NULL);
        pfds_release(myInit);
        return false;
    }
}

/** return the number of elementst in a sequence
 *
 * \param self
 * \return number of elements in self
 * \invariant borrow(self)
 */
extern size_t pfds_TreeList_size (pfds_TreeList* self) {
    pfds_UInt64* boxSize = (pfds_UInt64*) pfds_FingerTree_measure(
            &pfds_catenable_sum, measure_TreeList_size,
            self->fingerTree);
    size_t size = pfds_UInt64_get(boxSize);
    pfds_release(boxSize);
    return size;
}


/** return the nth element or NULL if the container does not contain enough elements
 *
 * \param self
 * \param n
 * \returns the nth element of self if it exists, NULL otherwise
 * \invariant when(return != NULL) lend(return, self)
 *
 */
extern pfds_object* pfds_TreeList_get (pfds_TreeList* self, size_t n) {
    if (pfds_TreeList_size(self) > n) {
        pfds_object* res;
        pfds_retain(self->fingerTree);
        pfds_FingerTree_split(NULL, &res, NULL,
                &pfds_catenable_sum, measure_TreeList_size,
                (bool (*)(void*, pfds_object*)) pred_TreeList_size, (void*) n,
                self->fingerTree);
        return res;
    } else {
        return NULL;
    }
}

const pfds_sequencevtable TreeList_sequence = {
    .catenable = &TreeList_catenable,
    .fromArray = (pfds_sequence* (*)(size_t, pfds_object**))
        pfds_TreeList_fromArray,
    .singleton = (pfds_sequence* (*)(pfds_object*))
        pfds_TreeList_singleton,

    .popFront = pfds_sequence_defaultPopFront,
    .popBack = pfds_sequence_defaultPopBack,
    .front = pfds_sequence_defaultFront,
    .back = pfds_sequence_defaultBack,
    .reduceLeft = (pfds_object* (*)(binop, void*, pfds_object*, pfds_sequence*))
        pfds_sequence_defaultReduceLeft,
    .reduceRight = (pfds_object* (*)(binop fn, void* ud, pfds_sequence* self, pfds_object* initial))
        pfds_sequence_defaultReduceRight,
    .reverse = (pfds_sequence* (*)(pfds_sequence*))
        pfds_sequence_defaultReverse,

    .insertBefore = pfds_sequence_defaultInsertBefore,
    .insertAfter = pfds_sequence_defaultInsertAfter,
    .updateAt = pfds_sequence_defaultUpdateAt,
    .deleteAt = pfds_sequence_defaultDeleteAt,

    .isEmpty = (bool (*)(pfds_sequence*))
        pfds_TreeList_isEmpty,
    .pushFront = (pfds_sequence* (*)(pfds_object* head, pfds_sequence* tail))
        pfds_TreeList_pushFront,
    .pushBack = (pfds_sequence* (*)(pfds_sequence* init, pfds_object* last))
        pfds_TreeList_pushBack,
    .split = (bool (*)(pfds_sequence**, pfds_object**, pfds_sequence**, pfds_sequence*, size_t))
        pfds_TreeList_split,
    .get = (pfds_object* (*)(pfds_sequence* self, size_t n))
        pfds_TreeList_get,
    .size = (size_t (*)(pfds_sequence*))
        pfds_TreeList_size,

};


/** finalizer for object.
 * \private \memberof pfds_TreeList
 *
 * if not null, called by the garbage collector just before an object is freed
 * use this method to free resources and decrease refcounts of owned members
 *
 * \param self
 * \invariant invalid(self)
 *
 */
void TreeList_destroy(pfds_TreeList* self) {
    pfds_release(self->fingerTree);
}


extern int pfds_TreeList_debugfputs(FILE* stream, pfds_TreeList* self) {
    int n = fprintf(stream, "TREELIST:{");
    n += pfds_FingerTree_debugfputs(stream, self->fingerTree, 0);
    n += fprintf(stream, "}");
    return n;
}

const pfds_objectvtable pfds_TreeList_vtable = {
    .typename = "TreeList",
    .destroy = (void (*)(pfds_object*))
        TreeList_destroy,
    .debugfputs = (int (*)(FILE*, pfds_object*))
        pfds_sequence_defaultDebugfputs,
        // pfds_TreeList_debugfputs,
    .cmp = (pfds_ordering (*)(pfds_object*, pfds_object*))
        pfds_sequence_defaultCmp,
    .catenable = &TreeList_catenable,
    .sequence = &TreeList_sequence,
};

