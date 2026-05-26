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

#include <assert.h>

#include "test_pfds.h"
#include "pfds.h"
#include "pfds/pfds-object-intl.h"
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


const CCHECK_Gen genBoxDouble;
const CCHECK_Gen genBoxUInt64;
CCHECK_Gen * genArrayList(const CCHECK_Gen* elements);

bool prop_sequence_empty_isEmpty(const pfds_objectvtable *vtable);

bool prop_sequence_pushPopFront(const pfds_objectvtable *vtable, pfds_object* x, pfds_ArrayList * xsArray);
bool prop_sequence_pushPopBack(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_object* x);

bool prop_sequence_pushFront(const pfds_objectvtable *vtable, pfds_object* x, pfds_ArrayList * xsArray);
bool prop_sequence_pushBack(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_object* x);

bool prop_sequence_get(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, size_t i);
bool prop_sequence_getGet(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, size_t i);
bool prop_sequence_getSplit(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, size_t i);
bool prop_sequence_insertBeforeGet(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_object* x, size_t i);
bool prop_sequence_insertAfterGet(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_object* x, size_t i);
bool prop_sequence_updateGet(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_object* x, size_t i);
bool prop_sequence_delete(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, size_t i);

bool prop_sequence_toString(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray);

bool prop_sequence_reduceLeft(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray);
bool prop_sequence_reduceRight(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray);
bool prop_sequence_reverse(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray);

bool prop_sequence_cmpRefl(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray);
bool prop_sequence_cmpAntisym(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_ArrayList * ysArray);
bool prop_sequence_cmpTrans(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_ArrayList * ysArray, pfds_ArrayList* zsArray);
bool prop_sequence_cmpLexicalShortest(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_ArrayList * ysArray);
bool prop_sequence_cmpLexicalPrefix(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_ArrayList * ysArray, pfds_ArrayList * zsArray);


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

    // bit of reverse hungarian notation:

    CCHECK_Gen * genArrayDbls = genArrayList(&genBoxDouble);
    CCHECK_Gen * genNEArrayDbls = genSize(
            genArrayList(&genBoxDouble),
            clampBelow,
            (void*) 1);

    static CCHECK_Gen * gen_Dbls[3];
    gen_Dbls[0] = genArrayDbls;
    gen_Dbls[1] = 0;

    static CCHECK_Gen * gen_Dbl_Dbls[3];
    gen_Dbl_Dbls[0] = (CCHECK_Gen *) &genBoxDouble;
    gen_Dbl_Dbls[1] = genArrayDbls;
    gen_Dbl_Dbls[2] = 0;

    static CCHECK_Gen * gen_Dbls_Dbl[3];
    gen_Dbls_Dbl[0] = genArrayDbls;
    gen_Dbls_Dbl[1] = (CCHECK_Gen *) &genBoxDouble;
    gen_Dbls_Dbl[2] = 0;

    static CCHECK_Gen * gen_DblsNE_Int[3];
    gen_DblsNE_Int[0] = genNEArrayDbls;
    gen_DblsNE_Int[1] = (CCHECK_Gen *) &genUInt64;
    gen_DblsNE_Int[2] = 0;

    static CCHECK_Gen * gen_Dbls_Dbl_Int[4];
    gen_Dbls_Dbl_Int[0] = genArrayDbls;
    gen_Dbls_Dbl_Int[1] = (CCHECK_Gen *) &genBoxDouble;
    gen_Dbls_Dbl_Int[2] = (CCHECK_Gen *) &genUInt64;
    gen_Dbls_Dbl_Int[3] = 0;

    static CCHECK_Gen * gen_DblsNE_Dbl_Int[4];
    gen_DblsNE_Dbl_Int[0] = genNEArrayDbls;
    gen_DblsNE_Dbl_Int[1] = (CCHECK_Gen *) &genBoxDouble;
    gen_DblsNE_Dbl_Int[2] = (CCHECK_Gen *) &genUInt64;
    gen_DblsNE_Dbl_Int[3] = 0;

    static CCHECK_Gen * gen_Dbls_Dbls[3];
    gen_Dbls_Dbls[0] = genArrayDbls;
    gen_Dbls_Dbls[1] = genArrayDbls;
    gen_Dbls_Dbls[2] = 0;

    static CCHECK_Gen * gen_Dbls_DblsNE[3];
    gen_Dbls_DblsNE[0] = genArrayDbls;
    gen_Dbls_DblsNE[1] = genNEArrayDbls;
    gen_Dbls_DblsNE[2] = 0;

    static CCHECK_Gen * gen_Dbls_Dbls_Dbls[4];
    gen_Dbls_Dbls_Dbls[0] = genArrayDbls;
    gen_Dbls_Dbls_Dbls[1] = genArrayDbls;
    gen_Dbls_Dbls_Dbls[2] = genArrayDbls;
    gen_Dbls_Dbls_Dbls[3] = 0;


    typedef bool (*propSequenceFn)(const pfds_objectvtable*, ...);
    struct propSequenceMethods {
        // void (*propFn)(const pfds_objectvtable*, ...);
        propSequenceFn propFn;
        const char* desc;
        const CCHECK_Gen ** gens;
    } propSequenceMethods[] = {
        { .propFn = (propSequenceFn) prop_sequence_empty_isEmpty, .desc = "isEmpty(empty)", .gens = 0 },
        { .propFn = (propSequenceFn) prop_sequence_pushPopFront, .desc = "push-pop",
            .gens = (const CCHECK_Gen**) gen_Dbl_Dbls },

        { .propFn = (propSequenceFn) prop_sequence_pushPopBack, .desc = "prop_sequence_pushPopBack",
            .gens = (const CCHECK_Gen**) gen_Dbls_Dbl },
        { .propFn = (propSequenceFn) prop_sequence_pushFront, .desc = "prop_sequence_pushFront",
            .gens = (const CCHECK_Gen**) gen_Dbl_Dbls  },
        { .propFn = (propSequenceFn) prop_sequence_pushBack, .desc = "prop_sequence_pushBack",
            .gens = (const CCHECK_Gen**) gen_Dbls_Dbl },
        { .propFn = (propSequenceFn) prop_sequence_get, .desc = "prop_sequence_get",
            .gens = (const CCHECK_Gen**) gen_DblsNE_Int },
        { .propFn = (propSequenceFn) prop_sequence_getGet, .desc = "prop_sequence_getGet",
            .gens = (const CCHECK_Gen**) gen_DblsNE_Int },
        { .propFn = (propSequenceFn) prop_sequence_getSplit, .desc = "prop_sequence_getSplit",
            .gens = (const CCHECK_Gen**) gen_DblsNE_Int },
        { .propFn = (propSequenceFn) prop_sequence_insertBeforeGet, .desc = "prop_sequence_insertBeforeGet",
            .gens = (const CCHECK_Gen**) gen_Dbls_Dbl_Int },
        { .propFn = (propSequenceFn) prop_sequence_insertAfterGet, .desc = "prop_sequence_insertAfterGet",
            .gens = (const CCHECK_Gen**) gen_DblsNE_Dbl_Int },
        { .propFn = (propSequenceFn) prop_sequence_updateGet, .desc = "prop_sequence_updateGet",
            .gens = (const CCHECK_Gen**) gen_DblsNE_Dbl_Int },
        { .propFn = (propSequenceFn) prop_sequence_delete, .desc = "prop_sequence_delete",
            .gens = (const CCHECK_Gen**) gen_DblsNE_Int },
        { .propFn = (propSequenceFn) prop_sequence_toString, .desc = "prop_sequence_toString",
            .gens = (const CCHECK_Gen**) gen_Dbls },
        { .propFn = (propSequenceFn) prop_sequence_reduceLeft, .desc = "prop_sequence_reduceLeft",
            .gens = (const CCHECK_Gen**) gen_Dbls },
        { .propFn = (propSequenceFn) prop_sequence_reduceRight, .desc = "prop_sequence_reduceRight",
            .gens = (const CCHECK_Gen**) gen_Dbls },
        { .propFn = (propSequenceFn) prop_sequence_reverse, .desc = "prop_sequence_reverse",
            .gens = (const CCHECK_Gen**) gen_Dbls },
        { .propFn = (propSequenceFn) prop_sequence_cmpRefl, .desc = "prop_sequence_cmpRefl",
            .gens = (const CCHECK_Gen**) gen_Dbls },
        { .propFn = (propSequenceFn) prop_sequence_cmpAntisym, .desc = "prop_sequence_cmpAntisym",
            .gens = (const CCHECK_Gen**) gen_Dbls_Dbls },
        { .propFn = (propSequenceFn) prop_sequence_cmpTrans, .desc = "prop_sequence_cmpTrans",
            .gens = (const CCHECK_Gen**) gen_Dbls_Dbls_Dbls },
        { .propFn = (propSequenceFn) prop_sequence_cmpLexicalShortest, .desc = "prop_sequence_cmpLexicalShortest",
            .gens = (const CCHECK_Gen**) gen_Dbls_DblsNE },
        { .propFn = (propSequenceFn) prop_sequence_cmpLexicalPrefix, .desc = "prop_sequence_cmpLexicalPrefix",
            .gens = (const CCHECK_Gen**) gen_Dbls_Dbls_Dbls },

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
                    100,
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


bool prop_sequence_empty_isEmpty(const pfds_objectvtable *vtable) {
    pfds_sequence* xs = (pfds_sequence*) vtable->catenable->mempty();
    bool res = pfds_sequence_isEmpty(xs);
    pfds_release(xs);
    return res;
}

bool prop_sequence_pushPopFront(const pfds_objectvtable *vtable, pfds_object* x, pfds_ArrayList * xsArray) {
    // x, xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // x, xs, xsArray

    pfds_retain(x); pfds_retain(xs);
    // x, xs, x, xs, xsArray

    pfds_sequence* ys = pfds_sequence_pushFront(x, xs);
    // ys, x, xs, xsArray

    // ys, x, xs, xsArray
    pfds_object* y;
    bool result = pfds_sequence_popFront(&y, &ys, ys)
        && pfds_cmp(x, y) == PFDS_EQ
        && pfds_cmp(xs, ys) == PFDS_EQ;
    // y, ys, x, xs, xsArray

    pfds_release(xs);
    pfds_release(y); pfds_release(ys);
    // x, xsArray

    return result;
}

bool prop_sequence_pushFront(const pfds_objectvtable *vtable, pfds_object* x, pfds_ArrayList * xsArray) {
    // x, xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // x, xs, xsArray

    pfds_retain(x);
    // x, x, xs, xsArray

    pfds_sequence* ys = pfds_sequence_pushFront(x, xs);
    // x, ys, xsArray

    // x ys, xsArray
    pfds_object* y = pfds_sequence_front(ys);
    // x, ys, xsArray (borrow y ys)


    bool result = y != NULL && pfds_cmp(x, y) == PFDS_EQ;

    // x, ys, xsArray (borrow y ys)
    pfds_release(ys);
    // x, xsArray

    return result;
}


void generateBoxDouble(pfds_Double** result, void* userData, int size, SplitMix64 *randGen) {
    *result = pfds_Double_new(SplitMix64_nextDouble(randGen));
}

const CCHECK_Gen genBoxDouble = {
    .genType = &ffi_type_pointer,
    .generate = (void (*)(void*, void*, int, SplitMix64 *))
        generateBoxDouble,
    .show = pfds_defaultGenShow,
    .dispose = pfds_defaultGenDispose,
};

void generateBoxUInt64(pfds_UInt64** result, void* userData, int size, SplitMix64 *randGen) {
    *result = pfds_UInt64_new(SplitMix64_nextInt64(randGen));
}

const CCHECK_Gen genBoxUInt64 = {
    .genType = &ffi_type_pointer,
    .generate = (void (*)(void*, void*, int, SplitMix64 *))
        generateBoxUInt64,
    .show = pfds_defaultGenShow,
    .dispose = pfds_defaultGenDispose,
};

void generateArrayList(pfds_ArrayList** result, CCHECK_Gen* elementGen, int size, SplitMix64 *randGen) {
    if (size <= 0) {
        *result = pfds_ArrayList_empty();
    } else {
        pfds_object** elements = (pfds_object**) calloc(sizeof(pfds_object*), size);
        for (size_t i = 0 ; i < size ; i++) {
            elementGen->generate(&elements[i], elementGen->userData, size, randGen);
        }
        *result = pfds_ArrayList_fromArrayEx(size, elements, true);
    }
}

CCHECK_Gen * genArrayList(const CCHECK_Gen* elementGen) {
    assert(elementGen->dispose == pfds_defaultGenDispose);

    CCHECK_Gen* gen = (CCHECK_Gen*) malloc(sizeof(CCHECK_Gen));
    gen->genType = &ffi_type_pointer;
    gen->userData = (void*) elementGen;
    gen->generate = (void (*)(void* , void*, int, SplitMix64 *))
            generateArrayList;
    gen->show = pfds_defaultGenShow;
    gen->dispose = pfds_defaultGenDispose;

    return gen;
}


bool prop_sequence_pushPopBack(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_object* x) {
    // xsArray, x
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // xs, xsArray, x

    pfds_retain(x); pfds_retain(xs);
    // x, xs, xs, xsArray, x

    pfds_sequence* ys = pfds_sequence_pushBack(xs, x);
    // ys, xs, xsArray, x

    pfds_object* y;
    bool result = pfds_sequence_popBack(&ys, &y, ys)
        && pfds_cmp(x, y) == PFDS_EQ
        && pfds_cmp(xs, ys) == PFDS_EQ;
    // y, ys, xs, xsArray, x

    pfds_release(xs);
    pfds_release(y); pfds_release(ys);
    // xsArray, x

    return result;
}

bool prop_sequence_pushBack(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_object* x) {
    // x, xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // x, xs, xsArray

    pfds_retain(x);
    // x, x, xs, xsArray

    pfds_sequence* ys = pfds_sequence_pushBack(xs, x);
    // x, ys, xsArray

    pfds_object* y = pfds_sequence_back(ys);
    // x, ys, xsArray (borrow y ys)


    bool result = y != NULL && pfds_cmp(x, y) == PFDS_EQ;

    // x, ys, xsArray (borrow y ys)
    pfds_release(ys);
    // x, xsArray

    return result;
}


bool prop_sequence_get(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, size_t i) {
    size_t sz = pfds_sequence_size((pfds_sequence*) xsArray);
    assert(sz >= 0);
    i = i % sz;

    // xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // xs, xsArray

    pfds_object * x = pfds_sequence_get(xs, i);
    pfds_object * xA = pfds_sequence_get((pfds_sequence*) xsArray, i);
    // xs[x], xsArray[xA]

    bool result = x != NULL
        && xA != NULL
        && pfds_cmp(x, xA) == PFDS_EQ;

    pfds_release(xs);
    // xsArray[xA]
    return result;
}

bool prop_sequence_getGet(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, size_t i) {
    size_t sz = pfds_sequence_size((pfds_sequence*) xsArray);

    i = i % sz;

    // xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // xs, xsArray

    pfds_object * x = pfds_sequence_get(xs, i);
    pfds_object * y = pfds_sequence_get(xs, i);
    // xs[x,y], xsArray

    bool result = x != NULL
        && y != NULL
        && pfds_cmp(x, y) == PFDS_EQ;

    pfds_release(xs);
    // xsArray

    return result;
}

bool prop_sequence_getSplit(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, size_t i) {
    size_t sz = pfds_sequence_size((pfds_sequence*) xsArray);

    i = i % sz;

    // xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // xs, xsArray

    pfds_object* x = pfds_sequence_get(xs, i);
    pfds_retain(x);
    // xs, x, xsArray

    pfds_sequence *ys, *zs;
    pfds_object* y;

    bool result = pfds_sequence_split(&ys, &y, &zs, xs, i) // ys, y, zs, x, xsArray
        && ys != NULL && y != NULL && zs != NULL
        && pfds_cmp(x, y) == PFDS_EQ
        && pfds_sequence_size(ys) == i;

    pfds_release(ys); pfds_release(y);
    pfds_release(zs); pfds_release(x);
    // xsArray

    return result;
}

bool prop_sequence_insertBeforeGet(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_object* x, size_t i) {
    size_t sz = pfds_sequence_size((pfds_sequence*) xsArray);

    i = i % (sz + 1);

    // x, xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // xs, x, xsArray

    pfds_retain(x);
    xs = pfds_sequence_insertBefore(xs, i, x);
    // xs, x, xsArray

    pfds_object* y = pfds_sequence_get(xs, i);
    // xs[y], x, xsArray

    bool result = pfds_cmp(x, y) == PFDS_EQ;
    pfds_release(xs);

    return result;
}

bool prop_sequence_insertAfterGet(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_object* x, size_t i) {
    size_t sz = pfds_sequence_size((pfds_sequence*) xsArray);

    i = i % sz;

    // x, xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // xs, x, xsArray

    pfds_retain(x);
    xs = pfds_sequence_insertAfter(xs, i, x);
    // xs, x, xsArray

    pfds_object* y = pfds_sequence_get(xs, i + 1);
    // xs[y], x, xsArray

    bool result = pfds_cmp(x, y) == PFDS_EQ;
    pfds_release(xs);

    return result;
}

bool prop_sequence_updateGet(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_object* x, size_t i) {
    size_t sz = pfds_sequence_size((pfds_sequence*) xsArray);

    i = i % sz;

    // x, xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // xs, x, xsArray

    pfds_retain(x);
    xs = pfds_sequence_updateAt(xs, i, x);
    // xs, x, xsArray

    pfds_object* y = pfds_sequence_get(xs, i);
    // xs[y], x, xsArray

    bool result = pfds_cmp(x, y) == PFDS_EQ;
    pfds_release(xs);

    return result;
}

bool prop_sequence_delete(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, size_t i) {
    size_t sz = pfds_sequence_size((pfds_sequence*) xsArray);

    i = i % sz;

    // xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // xs, xsArray

    xs = pfds_sequence_deleteAt(xs, i);
    // xs, xsArray

    size_t pos = 0;
    pfds_object* x;
    while(pfds_sequence_popFront(&x, &xs, xs)) {
        size_t pos2 = pos + (pos >= i ? 1 : 0);
        // x, xs, xArray
        if (pos2 >= sz) {
            pfds_release(x); pfds_release(xs);
            return false;
        }
        pfds_object* y = pfds_sequence_get((pfds_sequence*) xsArray, pos2);
        // x, xs, xArray[y]
        if (pfds_cmp(x, y) != PFDS_EQ) {
            pfds_release(x); pfds_release(xs);
            return false;
        }
        pfds_release(x);
        pos++;
        // xs, xArray[y]
    }
    // xsArray

    return (pos + 1) == sz;
}


bool prop_sequence_toString(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray) {
    // xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // xs, xsArray

    pfds_String* expected = pfds_toString(xsArray);
    pfds_String* actual = pfds_toString(xs);
    // expected, actual, xs, xsArray

    bool result = pfds_cmp(expected, actual) == PFDS_EQ;
    pfds_release(expected);
    pfds_release(actual);
    pfds_release(xs);

    return result;
}


bool prop_sequence_reduceLeft(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray) {
    // xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // xs, xsArray

    pfds_object* leftAssoc = pfds_sequence_reduceLeft(
                (binop) test_binop, 0,
                (pfds_object*) pfds_String_fromConstCstring("ROOT"),
                xs);
    // leftAssoc, xsArray

    pfds_retain(xsArray);
    pfds_object* leftAssocRef = pfds_sequence_reduceLeft(
                (binop) test_binop, 0,
                (pfds_object*) pfds_String_fromConstCstring("ROOT"),
                (pfds_sequence*) xsArray);
    // leftAssocRef, leftAssoc, xsArray
    bool result = pfds_cmp(leftAssoc, leftAssocRef) == PFDS_EQ;
    pfds_release(leftAssoc); pfds_release(leftAssocRef);
    // xsArray

    return result;
}

bool prop_sequence_reduceRight(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray) {
    // xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // xs, xsArray

    pfds_object* rightAssoc = pfds_sequence_reduceRight(
                (binop) test_binop, 0,
                xs,
                (pfds_object*) pfds_String_fromConstCstring("ROOT"));
    // rightAssoc, xsArray

    pfds_retain(xsArray);
    pfds_object* rightAssocRef = pfds_sequence_reduceRight(
                (binop) test_binop, 0,
                (pfds_sequence*) xsArray,
                (pfds_object*) pfds_String_fromConstCstring("ROOT"));
    // rightAssocRef, rightAssoc, xsArray
    bool result = pfds_cmp(rightAssoc, rightAssocRef) == PFDS_EQ;
    pfds_release(rightAssoc); pfds_release(rightAssocRef);
    // xsArray

    return result;
}

bool prop_sequence_reverse(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray) {
    // xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // xs, xsArray
    xs = pfds_sequence_reverse(xs);

    size_t i = 0;
    size_t size = pfds_sequence_size((pfds_sequence*) xsArray);

    pfds_object* x;
    // xs, xsArray
    while(pfds_sequence_popFront(&x, &xs, xs)) {
        // x, xs, xArray
        if (i >= size) {
            pfds_release(x); pfds_release(xs);
            return false;
        }
        pfds_object* y = pfds_sequence_get((pfds_sequence*) xsArray, size - i - 1);
        // x, xs, xArray[y]
        if (pfds_cmp(x, y) != PFDS_EQ) {
            pfds_release(x); pfds_release(xs);
            return false;
        }
        pfds_release(x);
        i++;
        // xs, xArray[y]
    }
    // xArray

    return i == size;
}


bool prop_sequence_cmpRefl(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray) {
    // xsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    // xs, xsArray

    bool result = pfds_cmp(xs, xs) == PFDS_EQ;
    pfds_release(xs);
    // xsArray

    return result;
}

bool prop_sequence_cmpAntisym(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_ArrayList * ysArray) {
    // xsArray ysArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    pfds_retain(ysArray);
    pfds_sequence* ys = pfds_sequence_fromArrayList(vtable->sequence, ysArray);
    // xs, ys, xsArray, ysArray

    pfds_ordering xRy = pfds_cmp(xs, ys);
    pfds_ordering yRx = pfds_cmp(ys, xs);
    pfds_release(xs); pfds_release(ys);

    // xsArray ysArray
    switch(xRy) {
        case PFDS_LT: return yRx == PFDS_GT;
        case PFDS_EQ: return yRx == PFDS_EQ;
        case PFDS_GT: return yRx == PFDS_LT;
        default: return false;
    }
}

bool prop_sequence_cmpTrans(const pfds_objectvtable *vtable,
        pfds_ArrayList * xsArray,
        pfds_ArrayList * ysArray,
        pfds_ArrayList * zsArray) {

    // xsArray ysArray zsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    pfds_retain(ysArray);
    pfds_sequence* ys = pfds_sequence_fromArrayList(vtable->sequence, ysArray);
    pfds_retain(zsArray);
    pfds_sequence* zs = pfds_sequence_fromArrayList(vtable->sequence, zsArray);
    // xs, ys, zs, xsArray, ysArray, zsArray

    pfds_ordering xRy = pfds_cmp(xs, ys);
    pfds_ordering yRz = pfds_cmp(ys, zs);
    pfds_ordering xRz = pfds_cmp(xs, zs);
    pfds_release(xs);
    pfds_release(ys);
    pfds_release(zs);
    // xsArray, ysArray, zsArray

    switch (xRy) {
        case PFDS_LT:
            switch (yRz) {
                case PFDS_LT: return xRz == PFDS_LT;
                case PFDS_EQ: return xRz == PFDS_LT;
                case PFDS_GT: return true;
            };
            break;
        case PFDS_EQ: return yRz == xRz;
        case PFDS_GT:
            switch (yRz) {
                case PFDS_LT: return true;
                case PFDS_EQ: return xRz == PFDS_GT;
                case PFDS_GT: return xRz == PFDS_GT;
            };
            break;
    }
    fprintf(stderr, "cmp returned invalid ordering");
    return false;

}

bool prop_sequence_cmpLexicalShortest(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_ArrayList * ysArray) {
    assert(pfds_sequence_size((pfds_sequence*) ysArray) > 0);

    // xsArray ysArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    pfds_retain(ysArray);
    pfds_sequence* ys = pfds_sequence_fromArrayList(vtable->sequence, ysArray);
    // xs, ys, xsArray, ysArray

    pfds_retain(xs);
    ys = pfds_sequence_mappend(xs, ys);
    // xs, ys, xsArray, ysArray

    pfds_ordering xRy = pfds_cmp(xs, ys);

    pfds_release(xs);
    pfds_release(ys);
    // xsArray, ysArray

    return xRy == PFDS_LT;
}

bool prop_sequence_cmpLexicalPrefix(const pfds_objectvtable *vtable, pfds_ArrayList * xsArray, pfds_ArrayList * ysArray, pfds_ArrayList * zsArray) {
    // xsArray ysArray zsArray
    pfds_retain(xsArray);
    pfds_sequence* xs = pfds_sequence_fromArrayList(vtable->sequence, xsArray);
    pfds_retain(ysArray);
    pfds_sequence* ys = pfds_sequence_fromArrayList(vtable->sequence, ysArray);
    pfds_retain(zsArray);
    pfds_sequence* zs = pfds_sequence_fromArrayList(vtable->sequence, zsArray);
    // xs, ys, zs, xsArray, ysArray, zsArray

    pfds_retain(xs); pfds_retain(ys);
    pfds_sequence* xys = pfds_sequence_mappend(xs, ys);
    pfds_retain(xs); pfds_retain(zs);
    pfds_sequence* xzs = pfds_sequence_mappend(xs, zs);
    // xys, xzs, xs, ys, zs, xsArray, ysArray, zsArray

    pfds_ordering yRz = pfds_cmp(ys, zs);
    pfds_ordering xyRxz = pfds_cmp(xys, xzs);

    pfds_release(xys); pfds_release(xzs); pfds_release(xs); pfds_release(ys); pfds_release(zs);
    // xsArray, ysArray, zsArray

    return yRz == xyRxz;
}


