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


// test infra for working generically with types that implement the sequence interface.

#include "test_pfds.h"
#include "pfds.h"
#include "pfds/pfds-arraylist.h"
#include "pfds/pfds-linkedlist.h"

void test_gc_sequence_fromArray0(const pfds_objectvtable* dict);
void test_gc_sequence_fromArray(const pfds_objectvtable* dict);
void test_gc_sequence_fromArray2(const pfds_objectvtable* dict);
void test_gc_sequence_fromArrayLoop(const pfds_objectvtable* dict);
void test_gc_sequence_singleton(const pfds_objectvtable* dict);
void test_gc_sequence_pushFront(const pfds_objectvtable* dict);
void test_gc_sequence_pushBack(const pfds_objectvtable* dict);
void test_gc_sequence_popFront(const pfds_objectvtable* dict);
void test_gc_sequence_insert(const pfds_objectvtable* dict);
void test_gc_sequence_insertAfter(const pfds_objectvtable* dict);
void test_gc_sequence_update(const pfds_objectvtable* dict);
void test_gc_sequence_delete(const pfds_objectvtable* dict);
void test_gc_sequence_get(const pfds_objectvtable* dict);
void test_gc_sequence_getPastEnd(const pfds_objectvtable* dict);
void test_gc_sequence_front(const pfds_objectvtable* dict);
void test_gc_sequence_back(const pfds_objectvtable* dict);
void test_gc_sequence_popBack(const pfds_objectvtable* dict);
void test_gc_sequence_popFront_empty(const pfds_objectvtable* dict);
void test_gc_sequence_front_empty(const pfds_objectvtable* dict);
void test_gc_sequence_back_empty(const pfds_objectvtable* dict);
void test_gc_sequence_popBack_empty(const pfds_objectvtable* dict);
void test_gc_sequence_split(const pfds_objectvtable* dict);
void test_gc_sequence_split1(const pfds_objectvtable* dict);
void test_gc_sequence_split2(const pfds_objectvtable* dict);
void test_gc_sequence_split3(const pfds_objectvtable* dict);
void test_gc_sequence_mappend_1(const pfds_objectvtable* dict);
void test_gc_sequence_mappend_2(const pfds_objectvtable* dict);
void test_gc_sequence_mappend_4(const pfds_objectvtable* dict);
void test_gc_sequence_mappend_3(const pfds_objectvtable* dict);
void test_gc_sequence_mpow(const pfds_objectvtable* dict);
void test_gc_sequence_isEmpty(const pfds_objectvtable* dict);
void test_gc_sequence_concatEmpty(const pfds_objectvtable* dict);
void test_gc_sequence_concat(const pfds_objectvtable* dict);
void test_gc_sequence_reduceRight(const pfds_objectvtable* dict);
void test_gc_sequence_reduceLeft(const pfds_objectvtable* dict);

void bench_lookup(struct benchState *bs, const pfds_objectvtable *vtable);
void bench_fillFromArray(struct benchState *bs, const pfds_objectvtable *vtable);
void bench_fillPushFront(struct benchState *bs, const pfds_objectvtable *vtable);
void bench_queueLeft(struct benchState *bs, const pfds_objectvtable *vtable);
void bench_fillPushBack(struct benchState *bs, const pfds_objectvtable *vtable);
void bench_shuffle(struct benchState *bs, const pfds_objectvtable *vtable);



int prop_sequence_empty_isEmpty(const pfds_objectvtable *vtable);

struct testModule getTestSequenceModule () {
    struct testModule testSequenceModule = {
        .testModule = "sequence",
    };

    classyBenchmark(&testSequenceModule, "lookup",        &pfds_ArrayList_vtable,   10, 10000, bench_lookup);
    classyBenchmark(&testSequenceModule, "lookup",        &pfds_TreeList_vtable,   10, 10000, bench_lookup);
    classyBenchmark(&testSequenceModule, "queueLeft",     &pfds_TreeList_vtable,   10, 10000, bench_queueLeft);
    classyBenchmark(&testSequenceModule, "queueLeft",     &pfds_ArrayList_vtable,  10, 2000, bench_queueLeft);
    classyBenchmark(&testSequenceModule, "queueLeft",     &pfds_LinkedList_vtable,   10, 1000, bench_queueLeft);
    classyBenchmark(&testSequenceModule, "fillFromArray", &pfds_TreeList_vtable,   10, 10000, bench_fillFromArray);
    classyBenchmark(&testSequenceModule, "fillFromArray", &pfds_LinkedList_vtable, 10, 10000, bench_fillFromArray);
    classyBenchmark(&testSequenceModule, "fillFromArray", &pfds_ArrayList_vtable,  100, 100000, bench_fillFromArray);
    classyBenchmark(&testSequenceModule, "fillPushFront", &pfds_TreeList_vtable, 10, 10000, bench_fillPushFront);
    classyBenchmark(&testSequenceModule, "fillPushFront", &pfds_LinkedList_vtable, 10, 10000, bench_fillPushFront);
    classyBenchmark(&testSequenceModule, "fillPushFront", &pfds_ArrayList_vtable, 10, 1000, bench_fillPushFront);
    classyBenchmark(&testSequenceModule, "fillPushBack",  &pfds_TreeList_vtable, 10, 10000, bench_fillPushBack);
    classyBenchmark(&testSequenceModule, "fillPushBack",  &pfds_LinkedList_vtable, 10, 500, bench_fillPushBack);
    classyBenchmark(&testSequenceModule, "fillPushBack",  &pfds_ArrayList_vtable, 10, 1000, bench_fillPushBack);
    classyBenchmark(&testSequenceModule, "shuffle",       &pfds_ArrayList_vtable, 10, 1000, bench_shuffle);
    classyBenchmark(&testSequenceModule, "shuffle",       &pfds_LinkedList_vtable, 10, 250, bench_shuffle);
    classyBenchmark(&testSequenceModule, "shuffle",       &pfds_TreeList_vtable, 10, 10000, bench_shuffle);

    struct gcSequenceMethods {
        void (*testFn)(const pfds_objectvtable*);
        const char* desc;
    } gcSequenceMethods[] = {
        { .testFn = test_gc_sequence_fromArray0, .desc = "fromArray/empty", },
        { .testFn = test_gc_sequence_fromArray, .desc = "fromArray/nonEmpty", },
        { .testFn = test_gc_sequence_fromArray2, .desc = "fromArray/dup", },
        { .testFn = test_gc_sequence_fromArrayLoop, .desc = "fromArray/loop", },
        { .testFn = test_gc_sequence_singleton, .desc = "singleton", },

        { .testFn = test_gc_sequence_popFront_empty, .desc = "popFront/empty", },
        { .testFn = test_gc_sequence_popFront, .desc = "popFront/x1", },
        { .testFn = test_gc_sequence_front_empty, .desc = "front/empty", },
        { .testFn = test_gc_sequence_front, .desc = "front/x1", },
        { .testFn = test_gc_sequence_pushFront, .desc = "pushFront/empty", },

        { .testFn = test_gc_sequence_popBack_empty, .desc = "popBack/empty", },
        { .testFn = test_gc_sequence_popBack, .desc = "popBack/x1", },
        { .testFn = test_gc_sequence_back_empty, .desc = "back/empty", },
        { .testFn = test_gc_sequence_back, .desc = "back/x1", },
        { .testFn = test_gc_sequence_pushBack, .desc = "pushBack/empty", },

        { .testFn = test_gc_sequence_get, .desc = "get", },
        { .testFn = test_gc_sequence_getPastEnd, .desc = "get past end", },

        { .testFn = test_gc_sequence_insert, .desc = "insertBefore", },
        { .testFn = test_gc_sequence_insertAfter, .desc = "insertAfter", },
        { .testFn = test_gc_sequence_update, .desc = "update", },
        { .testFn = test_gc_sequence_delete, .desc = "delete", },

        { .testFn = test_gc_sequence_reduceLeft, .desc = "reduceLeft", },
        { .testFn = test_gc_sequence_reduceRight, .desc = "reduceRight", },

        { .testFn = test_gc_sequence_split, .desc = "split/all", },
        { .testFn = test_gc_sequence_split1, .desc = "split/nullInit", },
        { .testFn = test_gc_sequence_split2, .desc = "split/nullLink", },
        { .testFn = test_gc_sequence_split3, .desc = "split/nullTail", },
        { .testFn = test_gc_sequence_mpow, .desc = "mappend/dup", },
        { .testFn = test_gc_sequence_mappend_1, .desc = "mappend/e.1", },
        { .testFn = test_gc_sequence_mappend_2, .desc = "mappend/4.4", },
        { .testFn = test_gc_sequence_mappend_3, .desc = "mappend/3.3", },
        { .testFn = test_gc_sequence_mappend_4, .desc = "mappend/2.2", },
        { .testFn = test_gc_sequence_concat, .desc = "concat", },
        { .testFn = test_gc_sequence_concatEmpty, .desc = "concat/empty", },
        { .testFn = test_gc_sequence_isEmpty, .desc = "isEmpty", },
        { 0 }
    };


    // const CCHECK_Gen * gensIntInt[] = {
    //     &genInt,
    //     &genInt,
    //     0,
    // };

    typedef int (*propSequenceFn)(const pfds_objectvtable*, ...);
    struct propSequenceMethods {
        // void (*propFn)(const pfds_objectvtable*, ...);
        propSequenceFn propFn;
        const char* desc;
        const CCHECK_Gen ** gens;
    } propSequenceMethods[] = {
        { .propFn = (propSequenceFn) prop_sequence_empty_isEmpty, .desc = "isEmpty(empty)", .gens = 0 },
        { 0 },
    };

    struct sequenceInstance {
        const pfds_objectvtable* vtable;
    } sequenceInstance[] = {
        { &pfds_ArrayList_vtable },
        { &pfds_LinkedList_vtable },
        { &pfds_TreeList_vtable },
        { 0 }
    };

    for (size_t j = 0; sequenceInstance[j].vtable ; j++) {
        for(size_t i = 0 ; propSequenceMethods[i].propFn ; i++) {
            classyProp(&testSequenceModule,
                    propSequenceMethods[i].desc,
                    sequenceInstance[j].vtable,
                    1,
                    propSequenceMethods[i].propFn,
                    propSequenceMethods[i].gens);
        }
    }
    for (size_t j = 0; sequenceInstance[j].vtable ; j++) {
        for(size_t i = 0 ; gcSequenceMethods[i].testFn ; i++) {
            classyGCTest(
                    &testSequenceModule,
                    gcSequenceMethods[i].desc,
                    gcSequenceMethods[i].testFn,
                    (void*) sequenceInstance[j].vtable);
        }
    }


    return testSequenceModule;
}

/** exercise a list of size n by looking up 100 elements  */
void bench_lookup(struct benchState *bs, const pfds_objectvtable *vtable) {
    pfds_object** fixture = (pfds_object**) calloc(sizeof(pfds_object*), bs->n);
    for (size_t i = 0 ; i < bs->n ; i++) {
        fixture[i] = (pfds_object*) pfds_UInt64_new(i);
    }
    pfds_sequence* xs0 = vtable->sequence->fromArray(bs->n, fixture);

    for (int iter = 0; iter < bs->iterations; iter++) {

        pfds_sequence* xs = xs0;
        pfds_retain(xs);

        bench_begin(bs, iter);
        for (int i = 0; i < 100; i++) {
            int idx = SplitMix64_nextInt64Range(&bs->gen, 0, bs->n);
            // x is lent for the lifetime of xs.
            pfds_object *x = pfds_sequence_get(xs, idx);
            if ( x == NULL
                    || ( x != NULL
                        && x != fixture[idx]
                        && pfds_cmp(x, fixture[idx]) != PFDS_EQ)
                    ) {
                CU_FAIL_FATAL("get returned incorrect element");
            }
        }
        bench_end(bs, iter);

        pfds_release(xs);
    }
    pfds_release(xs0);
    free(fixture);
}

/** exercise a fifo queue of size n by shifting 1000 elements from the back to front */
void bench_queueLeft(struct benchState *bs, const pfds_objectvtable *vtable) {
    pfds_object** fixture = (pfds_object**) calloc(sizeof(pfds_object*), bs->n);
    for (size_t i = 0 ; i < bs->n ; i++) {
        fixture[i] = (pfds_object*) pfds_Double_new(SplitMix64_nextDouble(&bs->gen));
    }
    pfds_sequence* xs0 = vtable->sequence->fromArray(bs->n, fixture);
    free(fixture);


    for (int iter = 0; iter < bs->iterations; iter++) {
        pfds_sequence* xs = xs0;
        pfds_retain(xs);

        bench_begin(bs, iter);
        for (int i = 0; i < 1000; i++) {
            xs = pfds_sequence_pushBack(
                    xs,
                    (pfds_object*) pfds_Double_new(SplitMix64_nextDouble(&bs->gen)));
            if (!pfds_sequence_popFront(NULL, &xs, xs)) {
                CU_FAIL_FATAL("empty queue");
            }
        }

        bench_end(bs, iter);

        pfds_release(xs);
    }


    bench_teardown(bs);
    pfds_release(xs0);
}


/** construct container of size n directly from an array of size n */
void bench_fillFromArray(struct benchState *bs, const pfds_objectvtable *vtable) {
    pfds_object** fixture = (pfds_object**) calloc(sizeof(pfds_object*), bs->n);
    for (size_t i = 0 ; i < bs->n ; i++) {
        fixture[i] = (pfds_object*) pfds_Double_new(SplitMix64_nextDouble(&bs->gen));
    }

    for (int iter = 0; iter < bs->iterations; iter++) {
        bench_begin(bs, iter);
        for (size_t i = 0 ; i < bs->n ; i++) {
            pfds_retain(fixture[i]);
        }
        pfds_sequence* xs = vtable->sequence->fromArray(bs->n, fixture);
        bench_end(bs, iter);
        pfds_release(xs);
    }


    bench_teardown(bs);
    for (size_t i = 0 ; i < bs->n ; i++) {
        pfds_release(fixture[i]);
    }
    free(fixture);

}


/** construct container of size n by pushing elements one at a time on the front */
void bench_fillPushFront(struct benchState *bs, const pfds_objectvtable *vtable) {
    pfds_object** fixture = (pfds_object**) calloc(sizeof(pfds_object*), bs->n);
    for (size_t i = 0 ; i < bs->n ; i++) {
        fixture[i] = (pfds_object*) pfds_Double_new(SplitMix64_nextDouble(&bs->gen));
    }

    for (int iter = 0; iter < bs->iterations; iter++) {
        bench_begin(bs, iter);
        pfds_sequence* xs = (pfds_sequence*) vtable->catenable->mempty();
        for (size_t i = 0 ; i < bs->n ; i++) {
            pfds_retain(fixture[i]);
            xs = pfds_sequence_pushFront(fixture[i], xs);
        }
        bench_end(bs, iter);
        pfds_release(xs);
    }


    bench_teardown(bs);
    for (size_t i = 0 ; i < bs->n ; i++) {
        pfds_release(fixture[i]);
    }
    free(fixture);

}

void bench_shuffle(struct benchState *bs, const pfds_objectvtable *vtable) {
    pfds_object** fixture = (pfds_object**) calloc(sizeof(pfds_object*), bs->n);
    for (size_t i = 0 ; i < bs->n ; i++) {
        fixture[i] = (pfds_object*) pfds_Double_new(SplitMix64_nextDouble(&bs->gen));
    }
    pfds_sequence* xs0 = vtable->sequence->fromArray(bs->n, fixture);
    free(fixture);

    for (int iter = 0; iter < bs->iterations; iter++) {
        pfds_sequence* xs = xs0; pfds_retain(xs);
        bench_begin(bs, iter);

        pfds_sequence* ys;
        pfds_object* z;
        pfds_sequence* ws;

        for (int i = 0; i < 100 ; i++) {
            size_t pos = SplitMix64_nextInt64Range(&bs->gen, 0, bs->n);
            CU_ASSERT(pfds_sequence_split(&ys, &z, &ws, xs, pos));
            xs = pfds_sequence_mappend(ws, pfds_sequence_pushFront(z, ys));
        }

        bench_end(bs, iter);
        pfds_release(xs);
    }

    bench_teardown(bs);
    pfds_release(xs0);

}


/** construct container of size n by pushing elements one at a time on the back */
void bench_fillPushBack(struct benchState *bs, const pfds_objectvtable *vtable) {
    pfds_object** fixture = (pfds_object**) calloc(sizeof(pfds_object*), bs->n);
    for (size_t i = 0 ; i < bs->n ; i++) {
        fixture[i] = (pfds_object*) pfds_Double_new(SplitMix64_nextDouble(&bs->gen));
    }

    for (int iter = 0; iter < bs->iterations; iter++) {
        bench_begin(bs, iter);
        pfds_sequence* xs = (pfds_sequence*) vtable->catenable->mempty();
        for (size_t i = 0 ; i < bs->n ; i++) {
            pfds_retain(fixture[i]);
            xs = pfds_sequence_pushBack(xs, fixture[i]);
        }
        bench_end(bs, iter);
        pfds_release(xs);
    }


    bench_teardown(bs);
    for (size_t i = 0 ; i < bs->n ; i++) {
        pfds_release(fixture[i]);
    }
    free(fixture);

}


void test_gc_sequence_fromArray0(const pfds_objectvtable* dict) {

    pfds_object* elts[0] = { };
    pfds_sequence* theList = dict->sequence->fromArray(0, elts);
    ASSERT_ONE_REF(theList);
    CU_ASSERT_EQUAL(pfds_sequence_size(theList), 0);
    pfds_release(theList);

}

void test_gc_sequence_fromArray(const pfds_objectvtable* dict) {

    pfds_Double* theDouble = pfds_Double_new(5.0);
    pfds_object* elts[1] = {(pfds_object*) theDouble};
    pfds_sequence* theList = dict->sequence->fromArray(1, elts);
    ASSERT_ONE_REF(theList);
    CU_ASSERT_EQUAL(pfds_sequence_size(theList), 1);
    pfds_release(theList);

}

void test_gc_sequence_fromArray2(const pfds_objectvtable* dict) {

    pfds_Double* theDouble = pfds_Double_new(5.0);
    pfds_retain(theDouble);
    pfds_object* elts[1] = {(pfds_object*) theDouble};
    pfds_sequence* theList = dict->sequence->fromArray(1, elts);
    ASSERT_ONE_REF(theList);
    pfds_release(theList);
    ASSERT_ONE_REF(theDouble);
    pfds_release(theDouble);
}

void test_gc_sequence_fromArrayLoop(const pfds_objectvtable* dict) {
    size_t maxElts = 11;
    pfds_object** elts = alloca(sizeof(pfds_object*) * maxElts);

    for(int i = 0; i < maxElts; i++) {
        PREPARE_GC_COUNTS(gcCounts);
        for(int j = 0; j < i ; j++) {
            elts[j] = (pfds_object*) pfds_Double_new(5.0);
        }
        pfds_sequence* theList = dict->sequence->fromArray(i, elts);
        ASSERT_ONE_REF(theList);
        pfds_release(theList);
        ASSERT_GC_COUNTS(gcCounts, 0, 0);
    }

}

void test_gc_sequence_singleton(const pfds_objectvtable* dict) {

    pfds_object* theDouble = (pfds_object*) mkTestValue_Double(NULL, 123);

    pfds_sequence * xs = dict->sequence->singleton(theDouble);

    ASSERT_ONE_REF(xs);

    pfds_release(xs);
}

void test_gc_sequence_pushFront(const pfds_objectvtable* dict) {

    pfds_sequence* list1 = (pfds_sequence*) dict->sequence->catenable->mempty();
    pfds_Double* theDouble = pfds_Double_new(5.0);
    pfds_sequence* list2 = pfds_sequence_pushFront((pfds_object*) theDouble, list1);
    ASSERT_ONE_REF(list2);
    pfds_release(list2);

}

void test_gc_sequence_pushBack(const pfds_objectvtable* dict) {


    pfds_sequence* list1 = (pfds_sequence*) dict->sequence->catenable->mempty();
    pfds_Double* theDouble = pfds_Double_new(5.0);
    pfds_sequence* list2 = pfds_sequence_pushBack(list1, (pfds_object*) theDouble);
    ASSERT_ONE_REF(list2);
    pfds_release(list2);
}

void test_gc_sequence_popFront(const pfds_objectvtable* dict) {


    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 2);
    ASSERT_ONE_REF(xs);
    CU_ASSERT_FALSE(pfds_sequence_isEmpty(xs));
    pfds_object* head = (pfds_object*) -1;
    pfds_sequence* tail = (pfds_sequence*) -1;
    CU_ASSERT(pfds_sequence_popFront(&head, &tail, xs));

    ASSERT_ONE_REF(tail);
    pfds_release(tail);

    ASSERT_ONE_REF(head);
    pfds_release(head);
}

void test_gc_sequence_insert(const pfds_objectvtable* dict) {


    pfds_sequence* xs = pfds_sequence_insertBefore(
            mkTestValue_sequence(dict->sequence, 3),
            2,
            (pfds_object*) mkTestValue_Double(NULL, 123));
    CU_ASSERT_NOT_EQUAL(xs, NULL);
    pfds_release(xs);
}

void test_gc_sequence_insertAfter(const pfds_objectvtable* dict) {


    pfds_sequence* xs = pfds_sequence_insertAfter(
            mkTestValue_sequence(dict->sequence, 3),
            2,
            (pfds_object*) mkTestValue_Double(NULL, 123));
    CU_ASSERT_NOT_EQUAL(xs, NULL);
    pfds_release(xs);
}
void test_gc_sequence_update(const pfds_objectvtable* dict) {

    pfds_sequence* xs = pfds_sequence_updateAt(
            mkTestValue_sequence(dict->sequence, 3),
            2,
            (pfds_object*) mkTestValue_Double(NULL, 123));
    CU_ASSERT_NOT_EQUAL(xs, NULL);
    pfds_release(xs);
}

void test_gc_sequence_delete(const pfds_objectvtable* dict) {

    pfds_sequence* xs = pfds_sequence_deleteAt(mkTestValue_sequence(dict->sequence, 3), 2);
    CU_ASSERT_NOT_EQUAL(xs, NULL);
    pfds_release(xs);
}

void test_gc_sequence_get(const pfds_objectvtable* dict) {

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 2);
    ASSERT_ONE_REF(xs);
    CU_ASSERT_EQUAL(pfds_sequence_size(xs), 2);
    pfds_object* x = pfds_sequence_get(xs, 1);

    ASSERT_ONE_REF(xs);
    pfds_release(xs);

    ASSERT_ONE_REF(x);
    pfds_release(x);
}

void test_gc_sequence_getPastEnd(const pfds_objectvtable* dict) {

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 2);
    ASSERT_ONE_REF(xs);
    CU_ASSERT_EQUAL(pfds_sequence_size(xs), 2);
    pfds_object* x = pfds_sequence_get(xs, 4);

    CU_ASSERT_EQUAL(x, NULL);
    ASSERT_ONE_REF(xs);
    pfds_release(xs);
}


void test_gc_sequence_front(const pfds_objectvtable* dict) {

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 2);
    ASSERT_ONE_REF(xs);
    CU_ASSERT_FALSE(pfds_sequence_isEmpty(xs));
    pfds_object* head = pfds_sequence_front(xs);

    ASSERT_ONE_REF(xs);
    pfds_release(xs);

    ASSERT_ONE_REF(head);
    pfds_release(head);
}

void test_gc_sequence_back(const pfds_objectvtable* dict) {

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 2);
    ASSERT_ONE_REF(xs);
    CU_ASSERT_FALSE(pfds_sequence_isEmpty(xs));
    pfds_object* last = pfds_sequence_back(xs);

    ASSERT_ONE_REF(xs);
    pfds_release(xs);

    ASSERT_ONE_REF(last);
    pfds_release(last);
}

void test_gc_sequence_popBack(const pfds_objectvtable* dict) {

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 2);
    ASSERT_ONE_REF(xs);
    CU_ASSERT_FALSE(pfds_sequence_isEmpty(xs));

    pfds_object* last = (pfds_object*) -1;
    pfds_sequence* init = (pfds_sequence*) -1;
    CU_ASSERT(pfds_sequence_popBack(&init, &last, xs));

    ASSERT_ONE_REF(init);
    pfds_release(init);

    ASSERT_ONE_REF(last);
    pfds_release(last);
}

void test_gc_sequence_popFront_empty(const pfds_objectvtable* dict) {


    pfds_sequence* e = (pfds_sequence*) dict->sequence->catenable->mempty();
    ASSERT_ONE_REF(e);
    pfds_object* head = (pfds_object*) -1;
    pfds_sequence* tail = (pfds_sequence*) -1;
    CU_ASSERT_FALSE(pfds_sequence_popFront(&head, &tail, e));
    CU_ASSERT_EQUAL(head, NULL);
    CU_ASSERT_EQUAL(tail, NULL);
}

void test_gc_sequence_front_empty(const pfds_objectvtable* dict) {


    pfds_sequence* e = (pfds_sequence*) dict->sequence->catenable->mempty();
    ASSERT_ONE_REF(e);
    pfds_object* head = pfds_sequence_front(e);
    CU_ASSERT_EQUAL(head, NULL);

    ASSERT_ONE_REF(e);
    pfds_release(e);
}

void test_gc_sequence_back_empty(const pfds_objectvtable* dict) {


    pfds_sequence* e = (pfds_sequence*) dict->sequence->catenable->mempty();
    ASSERT_ONE_REF(e);
    pfds_object* last = pfds_sequence_back(e);
    CU_ASSERT_EQUAL(last, NULL);

    ASSERT_ONE_REF(e);
    pfds_release(e);
}

void test_gc_sequence_popBack_empty(const pfds_objectvtable* dict) {


    pfds_sequence* e = (pfds_sequence*) dict->sequence->catenable->mempty();
    ASSERT_ONE_REF(e);
    pfds_object* last = (pfds_object*) -1;
    pfds_sequence* init = (pfds_sequence*) -1;
    CU_ASSERT_FALSE(pfds_sequence_popBack(&init, &last, e));
    CU_ASSERT_EQUAL(init, NULL);
    CU_ASSERT_EQUAL(last, NULL);
}

void test_gc_sequence_split(const pfds_objectvtable* dict) {

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 3);
    CU_ASSERT_EQUAL(pfds_sequence_size(xs), 3);

    pfds_sequence* init = (pfds_sequence*) -1;
    pfds_object* link = (pfds_object*) -1;
    pfds_sequence* tail = (pfds_sequence*) -1;

    CU_ASSERT(pfds_sequence_split(&init, &link, &tail, xs, 1));
    CU_ASSERT_NOT_EQUAL(init, (void*) -1);
    CU_ASSERT_NOT_EQUAL(link, (void*) -1);
    CU_ASSERT_NOT_EQUAL(tail, (void*) -1);

    ASSERT_ONE_REF(init);
    ASSERT_ONE_REF(link);
    ASSERT_ONE_REF(tail);

    pfds_release(init);
    pfds_release(link);
    pfds_release(tail);
}

void test_gc_sequence_split1(const pfds_objectvtable* dict) {

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 3);
    CU_ASSERT_EQUAL(pfds_sequence_size(xs), 3);

    pfds_object* link = (pfds_object*) -1;
    pfds_sequence* tail = (pfds_sequence*) -1;

    CU_ASSERT(pfds_sequence_split(NULL, &link, &tail, xs, 1));
    CU_ASSERT_NOT_EQUAL(link, (void*) -1);
    CU_ASSERT_NOT_EQUAL(tail, (void*) -1);

    ASSERT_ONE_REF(link);
    ASSERT_ONE_REF(tail);

    pfds_release(link);
    pfds_release(tail);
}

void test_gc_sequence_split2(const pfds_objectvtable* dict) {

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 3);
    CU_ASSERT_EQUAL(pfds_sequence_size(xs), 3);

    pfds_sequence* init = (pfds_sequence*) -1;
    pfds_sequence* tail = (pfds_sequence*) -1;

    CU_ASSERT(pfds_sequence_split(&init, NULL, &tail, xs, 1));
    CU_ASSERT_NOT_EQUAL(init, (void*) -1);
    CU_ASSERT_NOT_EQUAL(tail, (void*) -1);

    ASSERT_ONE_REF(init);
    ASSERT_ONE_REF(tail);

    pfds_release(init);
    pfds_release(tail);
}

void test_gc_sequence_split3(const pfds_objectvtable* dict) {

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 3);
    CU_ASSERT_EQUAL(pfds_sequence_size(xs), 3);

    pfds_sequence* init = (pfds_sequence*) -1;
    pfds_object* link = (pfds_object*) -1;

    CU_ASSERT(pfds_sequence_split(&init, &link, NULL, xs, 1));
    CU_ASSERT_NOT_EQUAL(init, (void*) -1);
    CU_ASSERT_NOT_EQUAL(link, (void*) -1);

    ASSERT_ONE_REF(init);
    ASSERT_ONE_REF(link);

    pfds_release(init);
    pfds_release(link);
}

void test_gc_sequence_mappend_1(const pfds_objectvtable* dict) {
    // this test exists to exercise a specific corner case in FingerTree_app3

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 3);
    pfds_sequence* ys = mkTestValue_sequence(dict->sequence, 1);
    CU_ASSERT_EQUAL(pfds_sequence_size(xs), 3);
    CU_ASSERT_EQUAL(pfds_sequence_size(ys), 1);

    xs = pfds_sequence_mappend(xs, ys);
    pfds_release(xs);
}
void test_gc_sequence_mappend_2(const pfds_objectvtable* dict) {
    // this test exists to exercise a specific corner case in FingerTree_app3

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 2);
    xs = pfds_sequence_pushBack(xs, (pfds_object*) mkTestValue_Double(NULL, 10));
    xs = pfds_sequence_pushBack(xs, (pfds_object*) mkTestValue_Double(NULL, 11));
    xs = pfds_sequence_pushBack(xs, (pfds_object*) mkTestValue_Double(NULL, 12));

    pfds_sequence* ys = mkTestValue_sequence(dict->sequence, 2);
    ys = pfds_sequence_pushFront((pfds_object*) mkTestValue_Double(NULL, 10), ys);
    ys = pfds_sequence_pushFront((pfds_object*) mkTestValue_Double(NULL, 11), ys);
    ys = pfds_sequence_pushFront((pfds_object*) mkTestValue_Double(NULL, 12), ys);

    xs = pfds_sequence_mappend(xs, ys);
    pfds_release(xs);
}
void test_gc_sequence_mappend_4(const pfds_objectvtable* dict) {
    // this test exists to exercise a specific corner case in FingerTree_app3

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 2);
    xs = pfds_sequence_pushBack(xs, (pfds_object*) mkTestValue_Double(NULL, 10));

    pfds_sequence* ys = mkTestValue_sequence(dict->sequence, 2);
    ys = pfds_sequence_pushFront((pfds_object*) mkTestValue_Double(NULL, 10), ys);

    xs = pfds_sequence_mappend(xs, ys);
    pfds_release(xs);
}
void test_gc_sequence_mappend_3(const pfds_objectvtable* dict) {
    // this test exists to exercise a specific corner case in FingerTree_app3

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 2);
    xs = pfds_sequence_pushBack(xs, (pfds_object*) mkTestValue_Double(NULL, 10));
    xs = pfds_sequence_pushBack(xs, (pfds_object*) mkTestValue_Double(NULL, 11));

    pfds_sequence* ys = mkTestValue_sequence(dict->sequence, 2);
    ys = pfds_sequence_pushFront((pfds_object*) mkTestValue_Double(NULL, 10), ys);
    ys = pfds_sequence_pushFront((pfds_object*) mkTestValue_Double(NULL, 11), ys);

    xs = pfds_sequence_mappend(xs, ys);
    pfds_release(xs);
}
void test_gc_sequence_mpow(const pfds_objectvtable* dict) {

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 3);
    CU_ASSERT_EQUAL(pfds_sequence_size(xs), 3);

    pfds_retain(xs);
    xs = pfds_sequence_mappend(xs, xs);
    ASSERT_ONE_REF(xs);
    pfds_release(xs);
}

void test_gc_sequence_isEmpty(const pfds_objectvtable* dict) {

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 3);
    CU_ASSERT_FALSE(pfds_sequence_isEmpty(xs));
    ASSERT_ONE_REF(xs);
    pfds_release(xs);
}

void test_gc_sequence_concatEmpty(const pfds_objectvtable* dict) {

    pfds_sequence* elts[0] = {
    };
    pfds_object* ys = dict->sequence->catenable->concat(0, (pfds_object**) elts);
    ASSERT_ONE_REF(ys);
    pfds_release(ys);
}

void test_gc_sequence_concat(const pfds_objectvtable* dict) {

    pfds_sequence* elts[3] = {
        mkTestValue_sequence(dict->sequence, 1),
        mkTestValue_sequence(dict->sequence, 2),
        mkTestValue_sequence(dict->sequence, 3),
    };
    pfds_object* ys = dict->catenable->concat(3, (pfds_object**) elts);
    ASSERT_ONE_REF(ys);
    pfds_release(ys);
}

void test_gc_sequence_reduceRight(const pfds_objectvtable* dict) {

    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 3);
    pfds_object* y = pfds_sequence_reduceRight((binop) test_binop, NULL,
            xs,
            (pfds_object*) pfds_String_fromCstring("ROOT"));
    ASSERT_ONE_REF(y);

    pfds_release(y);
}
void test_gc_sequence_reduceLeft(const pfds_objectvtable* dict) {


    pfds_sequence* xs = mkTestValue_sequence(dict->sequence, 3);
    pfds_object* y = pfds_sequence_reduceLeft((binop) test_binop, NULL,
            (pfds_object*) pfds_String_fromCstring("ROOT"),
            xs);
    ASSERT_ONE_REF(y);

    pfds_release(y);
}

pfds_sequence* mkTestValue_sequence(const pfds_sequencevtable* vtable, int seed) {
    switch(seed) {
        case 1:
            pfds_Double* array1[1] = { pfds_Double_new(1)};
            return vtable->fromArray(1, (pfds_object**) array1);
        case 2:
            pfds_Double* array2[2] = { pfds_Double_new(2), pfds_Double_new(3)};
            return vtable->fromArray(2, (pfds_object**) array2);
        case 3:
            pfds_Double* array3[3] = { pfds_Double_new(4), pfds_Double_new(5), pfds_Double_new(6)};
            return vtable->fromArray(3, (pfds_object**) array3);

        case 4:
            pfds_Double* array4[3] = { pfds_Double_new(4), pfds_Double_new(6), pfds_Double_new(7)};
            return vtable->fromArray(3, (pfds_object**) array4);

        case 5:
            pfds_Double* array5[3] = { pfds_Double_new(2), pfds_Double_new(3), pfds_Double_new(5)};
            return vtable->fromArray(3, (pfds_object**) array5);
        default:
            return (pfds_sequence*) vtable->catenable->mempty();
    }
}


int prop_sequence_empty_isEmpty(const pfds_objectvtable *vtable) {
    pfds_sequence* xs = (pfds_sequence*) vtable->catenable->mempty();
    int res = pfds_sequence_isEmpty(xs);
    pfds_release(xs);
    return res;
}
