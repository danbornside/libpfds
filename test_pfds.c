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

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include <assert.h>
#include <malloc.h>
#include <regex.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <argp.h>
#include <ffi.h>

#include "test_pfds.h"
#include "pfds.h"
#include "pfds/pfds-linkedlist.h"
#include "pfds/pfds-arraylist.h"
#include "pfds/pfds-arraymap.h"
#include "ccheck.h"

void display_mallinfodelta(FILE* stream, struct mallinfo2 *start, struct mallinfo2 *end) {
    fprintf(stream, "\n");
    fprintf(stream, "                                       start\tend\tdiff\n");
    // fprintf(stream, "Total non-mmapped bytes (arena):       %zu\t%zu\t%zd\t\n", start->arena   , end->arena   , end->arena   - start->arena   );
    // fprintf(stream, "# of free chunks (ordblks):            %zu\t%zu\t%zd\t\n", start->ordblks , end->ordblks , end->ordblks - start->ordblks );
    // fprintf(stream, "# of free fastbin blocks (smblks):     %zu\t%zu\t%zd\t\n", start->smblks  , end->smblks  , end->smblks  - start->smblks  );
    // fprintf(stream, "# of mapped regions (hblks):           %zu\t%zu\t%zd\t\n", start->hblks   , end->hblks   , end->hblks   - start->hblks   );
    // fprintf(stream, "Bytes in mapped regions (hblkhd):      %zu\t%zu\t%zd\t\n", start->hblkhd  , end->hblkhd  , end->hblkhd  - start->hblkhd  );
    // fprintf(stream, "Max. total allocated space (usmblks):  %zu\t%zu\t%zd\t\n", start->usmblks , end->usmblks , end->usmblks - start->usmblks );
    // fprintf(stream, "Free bytes held in fastbins (fsmblks): %zu\t%zu\t%zd\t\n", start->fsmblks , end->fsmblks , end->fsmblks - start->fsmblks );
    fprintf(stream, "Total allocated space (uordblks):      %zu\t%zu\t%zd\t\n", start->uordblks, end->uordblks, end->uordblks- start->uordblks);
    // fprintf(stream, "Total free space (fordblks):           %zu\t%zu\t%zd\t\n", start->fordblks, end->fordblks, end->fordblks- start->fordblks);
    // fprintf(stream, "Topmost releasable block (keepcost):   %zu\t%zu\t%zd\t\n", start->keepcost, end->keepcost, end->keepcost- start->keepcost);
}



pfds_sequence* mkTestValue_sequence(const pfds_sequencevtable* vtable, int seed);
void test_catenable_vtable(const pfds_catenablevtable* vtable);

void test_String(void) {
    PREPARE_GC_COUNTS(gcCounts);
    pfds_String* x = pfds_String_fromConstCstring("x");
    ASSERT_PFDS_STRING_EQUALS(x, "x");

    pfds_String* y = pfds_String_fromCstring("y");


    CU_ASSERT_EQUAL(pfds_cmp(x, y), PFDS_LT);
    CU_ASSERT_EQUAL(pfds_cmp(x, x), PFDS_EQ);
    CU_ASSERT_EQUAL(pfds_cmp(y, x), PFDS_GT);

    y = pfds_mappend(y, x);
    ASSERT_PFDS_STRING_EQUALS(y, "yx");

    pfds_release(y);
    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_Double(void) {
    PREPARE_GC_COUNTS(gcCounts);
    pfds_Double* x = pfds_Double_new(123.456);
    ASSERT_PFDS_STRING_EQUALS(x, "123.456000");
    pfds_Double* y = pfds_Double_new(654.321);

    ASSERT_PFDS_EQUALS(x, x);

    CU_ASSERT_EQUAL(pfds_cmp(x, y), PFDS_LT);
    CU_ASSERT_EQUAL(pfds_cmp(x, x), PFDS_EQ);
    CU_ASSERT_EQUAL(pfds_cmp(y, x), PFDS_GT);

    pfds_release(x);
    pfds_release(y);
    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_ArrayList(void) {
    PREPARE_GC_COUNTS(gcCounts);

    pfds_ArrayList* xs = pfds_ArrayList_empty();
    // xs
    ASSERT_PFDS_STRING_EQUALS(xs, "[]");

    pfds_ArrayList* ys = pfds_ArrayList_singleton(
        (pfds_object*) pfds_Double_new(1));
    // ys, xs
    ASSERT_PFDS_STRING_EQUALS(ys, "[1.000000]");

    ys = pfds_ArrayList_pushBack(ys, (pfds_object*) pfds_Double_new(2));
    // ys, xs
    ASSERT_PFDS_STRING_EQUALS(ys, "[1.000000, 2.000000]");

    xs = pfds_ArrayList_pushBack(xs, (pfds_object*) pfds_Double_new(3));
    // ys, xs
    ASSERT_PFDS_STRING_EQUALS(xs, "[3.000000]");

    CU_ASSERT_EQUAL(pfds_cmp(xs, ys), PFDS_GT);
    CU_ASSERT_EQUAL(pfds_cmp(xs, xs), PFDS_EQ);
    CU_ASSERT_EQUAL(pfds_cmp(ys, xs), PFDS_LT);

    // xs = pfds_ArrayList_mappend(xs, xs);
    pfds_retain(xs);
    // xs, ys, xs
    xs = pfds_mappend(xs, xs);
    // ys, xs
    ASSERT_PFDS_STRING_EQUALS(xs, "[3.000000, 3.000000]");

    xs = pfds_ArrayList_mappend(xs, ys);
    // xs
    ASSERT_PFDS_STRING_EQUALS(xs, "[3.000000, 3.000000, 1.000000, 2.000000]");

    pfds_release(xs);
    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_treelist_cmp_1() {

    PREPARE_GC_COUNTS(gcCounts);

    const void* ud = pfds_TreeList_vtable.sequence;
    pfds_object* x1;
    pfds_object* x2;

    x1 = (pfds_object*) mkTestValue_sequence(ud, 1);
    x2 = (pfds_object*) mkTestValue_sequence(ud, 1);
    pfds_cmp(x2, x1);
    pfds_release(x1);
    pfds_release(x2);
    ASSERT_GC_COUNTS(gcCounts, 0, 0);

}

void test_catenable_ident(struct catenableInstance * inst) {

// void test_catenable(const pfds_catenablevtable *dict, pfds_object*(*mkValue)(void*, int), void* ud) {
    test_catenable_vtable(inst->dict);

    PREPARE_GC_COUNTS(gcCounts);
    pfds_object* x1 = inst->mkValue(inst->ud, 1);

    pfds_object* e = inst->dict->mempty();

    pfds_retain(e); pfds_retain(x1);
    pfds_object* actual = inst->dict->mappend(e, x1);
    ASSERT_PFDS_EQUALS(actual, x1); // left identity
    pfds_release(actual);

    pfds_retain(x1);
    actual = inst->dict->mappend(x1, e);
    ASSERT_PFDS_EQUALS(actual, x1); // right identity
    pfds_release(actual);
    pfds_release(x1);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_catenable_assoc(struct catenableInstance * inst) {
    PREPARE_GC_COUNTS(gcCounts);
    pfds_object* x1 = inst->mkValue(inst->ud, 1);
    pfds_object* x2 = inst->mkValue(inst->ud, 2);
    pfds_object* x3 = inst->mkValue(inst->ud, 3);

    // [x1, x2, x3]

    pfds_retain(x1); pfds_retain(x2); pfds_retain(x3);
    // [x1, x2, x3, x1, x2, x3]
    pfds_object* left = inst->dict->mappend(inst->dict->mappend(x1, x2), x3);
    // [left, x1, x2, x3]
    pfds_object* right = inst->dict->mappend(x1, inst->dict->mappend(x2, x3));
    // [left, right]
    ASSERT_PFDS_EQUALS(left, right) // associativity.
    pfds_release(left);
    pfds_release(right);

    // []
    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_catenable_concat(struct catenableInstance * inst) {
    PREPARE_GC_COUNTS(gcCounts);

    pfds_object* x1 = inst->mkValue(inst->ud, 1);
    pfds_object* x2 = inst->mkValue(inst->ud, 2);
    pfds_object* x3 = inst->mkValue(inst->ud, 3);
    // [x1, x2, x3]

    // ASSERT_GC_COUNTS(gcCounts, 9, 9);

    pfds_retain(x1);
    pfds_retain(x2);
    pfds_retain(x3);
    // [x1, x2, x3, x1, x2, x3]

    // ASSERT_GC_COUNTS(gcCounts, 12, 9);

    pfds_object* x123Array[3] = {x1, x2, x3};
    pfds_object* x123Concat = inst->dict->concat(3, x123Array);
    // [x123Concat, x1, x2, x3]

    // ASSERT_GC_COUNTS(gcCounts, 10, 10);

    pfds_object* x12_3 = inst->dict->mappend(inst->dict->mappend(x1, x2), x3);
    // [x123Concat, x12_3]

    // ASSERT_GC_COUNTS(gcCounts, 8, 8);

    ASSERT_PFDS_EQUALS(x123Concat, x12_3);
    CU_ASSERT((bool) x123Concat);

    pfds_release(x123Concat);
    // ASSERT_GC_COUNTS(gcCounts, 7, 7);
    pfds_release(x12_3);
    ASSERT_GC_COUNTS(gcCounts, 0, 0);

}


pfds_String* mkTestValue_String(void* ud, int seed) {
    int len = snprintf(0, 0, "<%d>", seed);
    char* buf = (char*) malloc(len+1);
    snprintf(buf, len+1, "<%d>", seed);
    return pfds_String_fromCstring(buf);
}

pfds_Double* mkTestValue_Double(void* ud, int seed) {
    return pfds_Double_new(seed);
}

pfds_UInt64* mkTestValue_UInt64(void* ud, int seed) {
    return pfds_UInt64_new(seed);
}

pfds_String * test_binop(void* ud, pfds_object* l, pfds_object * r) {
    char* buf;
    size_t size = 0;
    FILE* stream = open_memstream(&buf, &size);

    fputs("(", stream);
    l->vtable->debugfputs(stream, l);
    fputs("<+>", stream);
    r->vtable->debugfputs(stream, r);
    fputs(")", stream);
    fflush(stream);

    pfds_release(l);
    pfds_release(r);

    return pfds_String_fromCstring(buf);
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

void test_sequence(pfds_sequencevtable *vtable) {
    PREPARE_GC_COUNTS(gcCounts);
    pfds_sequence* xs = (pfds_sequence*) vtable->catenable->mempty(); // []

    // [xs]

    CU_ASSERT(pfds_sequence_isEmpty(xs));

    xs = pfds_sequence_pushFront((pfds_object*) pfds_Double_new(1), xs); // 1 -> []
    xs = pfds_sequence_pushFront((pfds_object*) pfds_Double_new(2), xs); // 2 -> [1]
    xs = pfds_sequence_pushFront((pfds_object*) pfds_Double_new(3), xs); // 3 -> [2, 1]

    // [xs]

    pfds_Double* x = (pfds_Double*) pfds_sequence_front(xs);
    CU_ASSERT_EQUAL(pfds_Double_get(x), 3);

    // [x, xs]
    pfds_release(x);

    x = (pfds_Double*) pfds_sequence_front(xs);
    CU_ASSERT_EQUAL(pfds_Double_get(x), 3);
    // [x, xs]
    pfds_release(x);
    // [xs]

    CU_ASSERT(pfds_sequence_popFront(NULL, &xs, xs));   // _ <- [2, 1]

    // [xs]

    x = (pfds_Double*) pfds_sequence_front(xs);
    // [x, xs]
    CU_ASSERT_EQUAL(pfds_Double_get(x), 2);

    CU_ASSERT(pfds_sequence_popFront(NULL, &xs, xs)); // _ <- [1]
    CU_ASSERT((bool) xs);
    // [x, xs]

    ASSERT_PFDS_STRING_EQUALS(xs, "[1.000000]");
    CU_ASSERT(pfds_sequence_popFront(NULL, &xs, xs)); // _ <- []
    // [x, xs]
    pfds_release(x);

    ASSERT_PFDS_STRING_EQUALS(xs, "[]");

    CU_ASSERT_FALSE(pfds_sequence_popFront(NULL, &xs, xs)); // _ <-!! []
    // []

    CU_ASSERT_EQUAL(xs, NULL);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);

    pfds_Double* arrayOfStuff[3] = { pfds_Double_new(4), pfds_Double_new(5), pfds_Double_new(6) };

    xs = vtable->fromArray(3, (pfds_object**) arrayOfStuff);
    // [xs]
    ASSERT_PFDS_STRING_EQUALS(xs, "[4.000000, 5.000000, 6.000000]");

    CU_ASSERT_EQUAL(pfds_sequence_size(xs), 3);

    pfds_retain(xs);
    // [xs,xs]
    pfds_object* leftAssoc = pfds_sequence_reduceLeft(
                (binop) test_binop, 0,
                (pfds_object*) pfds_String_fromCstring("ROOT"),
                xs);
    // [leftAssoc,xs]
    ASSERT_PFDS_STRING_EQUALS(leftAssoc, "(((ROOT<+>4.000000)<+>5.000000)<+>6.000000)");
    pfds_release(leftAssoc);
    // [xs]

    pfds_retain(xs);
    // [xs,xs]
    pfds_object* rightAssoc = pfds_sequence_reduceRight(
                (binop) test_binop, 0,
                xs,
                (pfds_object*) pfds_String_fromConstCstring("ROOT"));
    // [rightAssoc,xs]
    ASSERT_PFDS_STRING_EQUALS(rightAssoc, "(4.000000<+>(5.000000<+>(6.000000<+>ROOT)))");
    pfds_release(rightAssoc);
    // [xs]

    xs = pfds_sequence_reverse(xs);
    // [xs]
    ASSERT_PFDS_STRING_EQUALS(xs, "[6.000000, 5.000000, 4.000000]");

    xs = pfds_sequence_pushBack(xs, (pfds_object*) pfds_Double_new(7)); // [6,5,4] <- 7
    // [xs]

    ASSERT_PFDS_STRING_EQUALS(xs, "[6.000000, 5.000000, 4.000000, 7.000000]");

    x = (pfds_Double*) pfds_sequence_back(xs);
    ASSERT_PFDS_STRING_EQUALS(x, "7.000000");
    pfds_release(x);
    x = (pfds_Double*) pfds_sequence_get(xs, 1);
    ASSERT_PFDS_STRING_EQUALS(x, "5.000000");
    pfds_release(x);

    pfds_sequence *ys, *zs;

    CU_ASSERT(pfds_sequence_split(&ys, (pfds_object**) &x, &zs, xs, 2)); // [6,5] 4 [7] <- [6,5,4,7]
    // [ys,x,zs]
    ASSERT_ONE_REF(x);
    ASSERT_ONE_REF(ys);
    ASSERT_ONE_REF(zs);
    ASSERT_PFDS_STRING_EQUALS(ys, "[6.000000, 5.000000]");
    ASSERT_PFDS_STRING_EQUALS(x, "4.000000");
    ASSERT_PFDS_STRING_EQUALS(zs, "[7.000000]");

    pfds_release(zs); // last ref to zs
    pfds_release(x); // we're about to lose our ref to x
    // [ys]

    CU_ASSERT(pfds_sequence_popBack(&ys, (pfds_object**) &x, ys)); // [6] -> 5
    // [x,ys]
    ASSERT_PFDS_STRING_EQUALS(x, "5.000000");
    pfds_release(x); // we're about to lose our ref to x
    // [ys]
    CU_ASSERT(pfds_sequence_popBack(&ys, (pfds_object**) &x, ys));
    // [x,ys]
    pfds_release(x); // we're about to lose our ref to x again
    // [ys]
    CU_ASSERT_FALSE(pfds_sequence_popBack(&ys, (pfds_object**) &x, ys));
    // []

    ASSERT_GC_COUNTS(gcCounts, 0, 0);

    xs = mkTestValue_sequence(vtable, 3);
    ASSERT_PFDS_STRING_EQUALS(xs, "[4.000000, 5.000000, 6.000000]");

    xs = pfds_sequence_insertBefore(xs, 1, (pfds_object*) mkTestValue_Double(NULL, 7));
    ASSERT_PFDS_STRING_EQUALS(xs, "[4.000000, 7.000000, 5.000000, 6.000000]");

    xs = pfds_sequence_insertAfter(xs, 1, (pfds_object*) mkTestValue_Double(NULL, 8));
    ASSERT_PFDS_STRING_EQUALS(xs, "[4.000000, 7.000000, 8.000000, 5.000000, 6.000000]");

    xs = pfds_sequence_insertBefore(xs, 0, (pfds_object*) mkTestValue_Double(NULL, 9));
    ASSERT_PFDS_STRING_EQUALS(xs, "[9.000000, 4.000000, 7.000000, 8.000000, 5.000000, 6.000000]");

    xs = pfds_sequence_insertBefore(xs, 6, (pfds_object*) mkTestValue_Double(NULL, 10));
    ASSERT_PFDS_STRING_EQUALS(xs,
            "[9.000000, 4.000000, 7.000000, 8.000000, 5.000000, 6.000000, 10.000000]");

    xs = pfds_sequence_updateAt(xs, 2, (pfds_object*) mkTestValue_Double(NULL, 11));
    ASSERT_PFDS_STRING_EQUALS(xs,
            "[9.000000, 4.000000, 11.000000, 8.000000, 5.000000, 6.000000, 10.000000]");

    xs = pfds_sequence_deleteAt(xs, 2);
    ASSERT_PFDS_STRING_EQUALS(xs,
            "[9.000000, 4.000000, 8.000000, 5.000000, 6.000000, 10.000000]");
    xs = pfds_sequence_deleteAt(xs, 0);
    ASSERT_PFDS_STRING_EQUALS(xs, "[4.000000, 8.000000, 5.000000, 6.000000, 10.000000]");

    xs = pfds_sequence_deleteAt(xs, 4);
    ASSERT_PFDS_STRING_EQUALS(xs, "[4.000000, 8.000000, 5.000000, 6.000000]");

    pfds_release(xs);
    ASSERT_GC_COUNTS(gcCounts, 0, 0);

    xs = mkTestValue_sequence(vtable, 1); // [1]
    ys = mkTestValue_sequence(vtable, 2); // [2,3]

    CU_ASSERT_EQUAL(pfds_cmp(xs, ys), PFDS_LT);
    CU_ASSERT_EQUAL(pfds_cmp(ys, xs), PFDS_GT);

    pfds_release(xs);
    xs = mkTestValue_sequence(vtable, 5); // [2,3,5]
    CU_ASSERT_EQUAL(pfds_cmp(xs, ys), PFDS_GT);
    CU_ASSERT_EQUAL(pfds_cmp(ys, xs), PFDS_LT);
    pfds_release(xs);

    xs = mkTestValue_sequence(vtable, 2); // [2,3]
    CU_ASSERT_EQUAL(pfds_cmp(xs, ys), PFDS_EQ);

    pfds_release(xs);
    pfds_release(ys);
    ASSERT_GC_COUNTS(gcCounts, 0, 0);

    xs = mkTestValue_sequence(vtable, 3); // [4,5,6]
    ys = mkTestValue_sequence(vtable, 4); // [4,6,7]
    CU_ASSERT_EQUAL(pfds_cmp(xs, ys), PFDS_LT);

    pfds_release(xs);
    pfds_release(ys);
    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

CU_BOOL test_assoc_int(int a, int b, int c) {
    return (a + b) + c == a + (b + c);
}

CU_BOOL test_lessthan_5(int n) {
    return n < 5;
}

void imperative_prop(CCHECK_Context *ctx, void* userData) {
    int a = *(int*) CCHECK_generate(ctx, &genInt, "a");
    int b = *(int*) CCHECK_generate(ctx, &genInt, "b");

    CCHECK_assert_prop(ctx, a+b == b+a /* commutative */ );

    int c = *(int*) CCHECK_generate(ctx, &genInt, "c");
    CCHECK_assert_prop(ctx, ((a+b)+c) == (a+(b+c)) /* associative */ );
}

void test_props_imperatively(void) {
    CCHECK_test_prop_impl(NULL, 10, imperative_prop, NULL);
}

void test_catenable_vtable(const pfds_catenablevtable* vtable) {
    if (vtable) {
        CU_ASSERT(vtable->mempty != NULL);
        CU_ASSERT(vtable->mappend != NULL);
        CU_ASSERT(vtable->concat != NULL);
    }
}

void test_vtable(const pfds_objectvtable* vtable) {
    CU_ASSERT(vtable->typename != NULL);
    CU_ASSERT(vtable->cmp != NULL);
    CU_ASSERT(vtable->debugfputs != NULL);

    if (vtable->catenable) {
        test_catenable_vtable(vtable->catenable);
    }


    if (vtable->sequence) {

        // sequence is a monoid
        CU_ASSERT(vtable->sequence->catenable != NULL);
        CU_ASSERT(vtable->sequence->catenable == vtable->catenable);
        CU_ASSERT(vtable->catenable->mempty != NULL);

        // sequence can be built from elements
        CU_ASSERT(vtable->sequence->fromArray != NULL);
        CU_ASSERT(vtable->sequence->singleton != NULL);

        CU_ASSERT(vtable->sequence->isEmpty != NULL);

        CU_ASSERT(vtable->sequence->insertBefore != NULL);
        CU_ASSERT(vtable->sequence->insertAfter != NULL);
        CU_ASSERT(vtable->sequence->updateAt != NULL);
        CU_ASSERT(vtable->sequence->deleteAt != NULL);


        // all or none.
        CU_ASSERT(vtable->sequence->popFront != NULL);
        CU_ASSERT(vtable->sequence->reduceRight != NULL);
        CU_ASSERT(vtable->sequence->front != NULL);

        // all or none.
        CU_ASSERT(vtable->sequence->popBack != NULL);
        CU_ASSERT(vtable->sequence->reduceLeft != NULL);
        CU_ASSERT(vtable->sequence->back != NULL);

        // at least one.
        CU_ASSERT(vtable->sequence->pushFront != NULL);
        CU_ASSERT(vtable->sequence->pushBack != NULL);

        CU_ASSERT(vtable->sequence->reverse != NULL);

        // all or none
        CU_ASSERT(vtable->sequence->split != NULL);
        CU_ASSERT(vtable->sequence->get != NULL);
        CU_ASSERT(vtable->sequence->size != NULL);

    }

    if (vtable->mapping) {
        CU_ASSERT(vtable->mapping->fromArray != NULL);
        CU_ASSERT(vtable->mapping->singleton != NULL);
        CU_ASSERT(vtable->mapping->empty != NULL);
        CU_ASSERT(vtable->mapping->isEmpty != NULL);
        CU_ASSERT(vtable->mapping->size != NULL);
        CU_ASSERT(vtable->mapping->lookup != NULL);
        CU_ASSERT(vtable->mapping->insert != NULL);
        CU_ASSERT(vtable->mapping->erase != NULL);

        // CU_ASSERT(vtable->mapping->unionFirst != NULL);
        // CU_ASSERT(vtable->mapping->intersectionFirst != NULL);
        // CU_ASSERT(vtable->mapping->popMin != NULL);
        // CU_ASSERT(vtable->mapping->popMax != NULL);
        // CU_ASSERT(vtable->mapping->minKey != NULL);
        // CU_ASSERT(vtable->mapping->maxKey != NULL);
    }
}




const char *argp_program_version = "test_pfds 0.1";
const char *argp_program_bug_address = "<dan.bornside@gmail.com>";
static char doc[] = "I have no idea what this is meant to be...";
static char args_doc[] = "";

static struct argp_option options[] = {
    {"pattern", 'p', "REGEXP", 0, "run tests that match regex(7) pattern"},
    {"benchmarks", 'b', "FILE", 0, "save benchmark results to file"},
    {"benchmark-size", 'n', "NUMBER", 0, "multiplier for the maximum size of benchmarks [default 1.0]"},
    {"benchmark-iterations", 'i', "NUMBER", 0, "number of iterations at each benchmark size to test [default 1]"},
    {"benchmark-steps", 's', "NUMBER", 0, "number of size steps between min and max to benchmark [default 5]"},
    {"list", 't', 0, 0, "only list tests and exit."},
    {"verbose", 'v', 0, 0, "increase test output"},
    { 0 }
};

enum test_mode {
    TEST_MODE_NONE = 0, // no optiom specified, do basic.
    TEST_MODE_LIST,
};

struct arguments {
    int argc;
    regex_t *preg;
    enum test_mode mode;
    int verbosity;
    char* benchmarkResultsPath;

};

// these kinda have to be globals because the alternative is mallocing a big config structure or even more libffi nonsense.
static FILE* benchmarkResults;
int bench_iterations = 1;
int bench_steps = 5;
double bench_size_mult = 1.0;

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
        case 'p':
            if (arguments->preg) argp_usage(state);
            arguments->preg = (regex_t*) malloc(sizeof(regex_t));
            int err = regcomp(arguments->preg, arg, REG_EXTENDED | REG_NOSUB);
            if (err) {
                size_t errLen = regerror(err, arguments->preg, 0, 0);
                char* errstr = (char*) malloc(errLen+1);
                regerror(err, arguments->preg, errstr, errLen+1);
                fprintf(stderr, "Invalid pattern: %s\n\n", errstr);
                argp_usage(state);
            };
            break;
        case 'b':
            if (arguments->benchmarkResultsPath) argp_usage(state);
            arguments->benchmarkResultsPath = arg;
            break;
        case 'n':
            double lf;
            int n;
            if (sscanf(arg, "%lf%n", &lf, &n) != 1 || arg[n] != '\0' || lf <= 0.0) {
                fprintf(stderr, "benchmark-size must be a postive number\n");
                argp_usage(state);
            }
            bench_size_mult = lf;
            break;
        case 'i':
            unsigned int u;
            if (sscanf(arg, "%u%n", &u, &n) != 1 || arg[n] != '\0' || u <= 0) {
                fprintf(stderr, "benchmark-iterations must be a postive number\n");
                argp_usage(state);
            }
            bench_iterations = u;
            break;
        case 's':
            if (sscanf(arg, "%u%n", &u, &n) != 1 || arg[n] != '\0' || u <= 0) {
                fprintf(stderr, "benchmark-steps must be a postive number\n");
                argp_usage(state);
            }
            bench_steps = u;
            break;
        case 'v':
            arguments->verbosity++;
            break;
        case 't':
            arguments->mode = TEST_MODE_LIST;
            break;

        case ARGP_KEY_ARG: /* do nothing, successfully */
            break;

        case ARGP_KEY_END:
            arguments->argc = state->arg_num;
            /* arg_num is how many positional arguments we got */
            break;
        default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}


static struct argp argp = {options, parse_opt, args_doc, doc};



void test_gc_new_release_double(void) {
        pfds_Double* theDouble = pfds_Double_new(5.0);
        ASSERT_ONE_REF(theDouble);
        pfds_release(theDouble);
}

void test_gc_new_release_string(void) {
        pfds_String* theString = pfds_String_fromCstring("string");
        ASSERT_ONE_REF(theString);
        pfds_release(theString);

}

void test_gc_new_release_constString(void) {

    pfds_String* theString = pfds_String_fromConstCstring("string");
    ASSERT_ONE_REF(theString);
    pfds_release(theString);

}

void test_gc_new_release_arrayListEmpty(void) {

    pfds_ArrayList* theArrayList = pfds_ArrayList_empty();
    ASSERT_ONE_REF(theArrayList);
    pfds_release(theArrayList);

}

void test_gc_new_release_arrayListFromArray0(void) {

    pfds_object* elts[0] = {};
    pfds_ArrayList* theArrayList = pfds_ArrayList_fromArray(0, elts);
    ASSERT_ONE_REF(theArrayList);
    pfds_release(theArrayList);


}

void test_gc_new_release_arrayListFromArray(void) {

    pfds_Double* theDouble = pfds_Double_new(5.0);
    pfds_object* elts[1] = {(pfds_object*) theDouble};
    pfds_ArrayList* theArrayList = pfds_ArrayList_fromArray(1, elts);
    ASSERT_ONE_REF(theArrayList);
    pfds_release(theArrayList);


}

void test_gc_sequence_fromArray(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_Double* theDouble = pfds_Double_new(5.0);
    pfds_object* elts[1] = {(pfds_object*) theDouble};
    pfds_sequence* theList = dict->fromArray(1, elts);
    ASSERT_ONE_REF(theList);
    CU_ASSERT_EQUAL(pfds_sequence_size(theList), 1);
    pfds_release(theList);


    ASSERT_GC_COUNTS(gcCounts, 0, 0);

}

void test_gc_sequence_fromArray0(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_object* elts[0] = { };
    pfds_sequence* theList = dict->fromArray(0, elts);
    ASSERT_ONE_REF(theList);
    CU_ASSERT_EQUAL(pfds_sequence_size(theList), 0);
    pfds_release(theList);


    ASSERT_GC_COUNTS(gcCounts, 0, 0);

}

struct gcObjectInstances {
    const pfds_objectvtable* vtable;
    pfds_object* (*mkValue)(void*, int seed);
    void* mkValueUD;
};


void test_gc_newrelease(void* argsPtr) {
    struct gcObjectInstances* args = (struct gcObjectInstances*) argsPtr;

    PREPARE_GC_COUNTS(gcCounts);

    pfds_object* self = args->mkValue(args->mkValueUD, 1);
    ASSERT_ONE_REF(self);
    pfds_release(self);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_debugputstr(void* argsPtr) {
    struct gcObjectInstances* args = (struct gcObjectInstances*) argsPtr;

    PREPARE_GC_COUNTS(gcCounts);

    pfds_object* self = args->mkValue(args->mkValueUD, 1);

    FILE* devNull = fopen("/dev/null", "w");
    pfds_object_debugfputs(devNull, self);
    fclose(devNull);

    ASSERT_ONE_REF(self);
    pfds_release(self);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_cmp(void* argsPtr) {
    struct gcObjectInstances* args = (struct gcObjectInstances*) argsPtr;

    PREPARE_GC_COUNTS(gcCounts);

    pfds_object* l = args->mkValue(args->mkValueUD, 1);
    pfds_object* lPrime = args->mkValue(args->mkValueUD, 1);
    pfds_object* r = args->mkValue(args->mkValueUD, 2);
    pfds_cmp(l, r);
    CU_ASSERT_EQUAL(pfds_cmp(l, l), PFDS_EQ);

    pfds_release(l);
    pfds_release(lPrime);
    pfds_release(r);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_new_release_arrayListFromArray2(void) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_Double* theDouble = pfds_Double_new(5.0);
    pfds_retain(theDouble);
    pfds_object* elts[1] = {(pfds_object*) theDouble};
    pfds_ArrayList* theArrayList = pfds_ArrayList_fromArray(1, elts);
    ASSERT_ONE_REF(theArrayList);
    pfds_release(theArrayList);
    ASSERT_ONE_REF(theDouble);
    pfds_release(theDouble);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);

}

void test_gc_new_release_linkedListNil(void) {

    pfds_LinkedList* theList = pfds_LinkedList_empty();
    ASSERT_ONE_REF(theList);
    pfds_release(theList);


}

void test_gc_new_release_linkedListCons(void) {

    pfds_LinkedList* theList = pfds_LinkedList_empty();
    pfds_Double* theDouble = pfds_Double_new(5.0);
    theList = pfds_LinkedList_pushFront((pfds_object*) theDouble, theList);
    ASSERT_ONE_REF(theList);
    pfds_release(theList);


}

void test_gc_sequence_fromArray2(pfds_sequencevtable* dict) {
    PREPARE_GC_COUNTS(gcCounts);

    pfds_Double* theDouble = pfds_Double_new(5.0);
    pfds_retain(theDouble);
    pfds_object* elts[1] = {(pfds_object*) theDouble};
    pfds_sequence* theList = dict->fromArray(1, elts);
    ASSERT_ONE_REF(theList);
    pfds_release(theList);
    ASSERT_ONE_REF(theDouble);
    pfds_release(theDouble);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_fromArrayLoop(pfds_sequencevtable* dict) {
    size_t maxElts = 11;
    pfds_object** elts = alloca(sizeof(pfds_object*) * maxElts);

    for(int i = 0; i < maxElts; i++) {
        PREPARE_GC_COUNTS(gcCounts);
        for(int j = 0; j < i ; j++) {
            elts[j] = (pfds_object*) pfds_Double_new(5.0);
        }
        pfds_sequence* theList = dict->fromArray(i, elts);
        ASSERT_ONE_REF(theList);
        pfds_release(theList);
        ASSERT_GC_COUNTS(gcCounts, 0, 0);
    }

}

void test_gc_sequence_singleton(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_object* theDouble = (pfds_object*) mkTestValue_Double(NULL, 123);

    pfds_sequence * xs = dict->singleton(theDouble);

    ASSERT_ONE_REF(xs);

    pfds_release(xs);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_pushFront(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);


    pfds_sequence* list1 = (pfds_sequence*) dict->catenable->mempty();
    pfds_Double* theDouble = pfds_Double_new(5.0);
    pfds_sequence* list2 = pfds_sequence_pushFront((pfds_object*) theDouble, list1);
    ASSERT_ONE_REF(list2);
    pfds_release(list2);


    ASSERT_GC_COUNTS(gcCounts, 0, 0);

}

void test_gc_sequence_pushBack(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* list1 = (pfds_sequence*) dict->catenable->mempty();
    pfds_Double* theDouble = pfds_Double_new(5.0);
    pfds_sequence* list2 = pfds_sequence_pushBack(list1, (pfds_object*) theDouble);
    ASSERT_ONE_REF(list2);
    pfds_release(list2);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_popFront(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 2);
    ASSERT_ONE_REF(xs);
    CU_ASSERT_FALSE(pfds_sequence_isEmpty(xs));
    pfds_object* head = (pfds_object*) -1;
    pfds_sequence* tail = (pfds_sequence*) -1;
    CU_ASSERT(pfds_sequence_popFront(&head, &tail, xs));

    ASSERT_ONE_REF(tail);
    pfds_release(tail);

    ASSERT_ONE_REF(head);
    pfds_release(head);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_insert(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = pfds_sequence_insertBefore(
            mkTestValue_sequence(dict, 3),
            2,
            (pfds_object*) mkTestValue_Double(NULL, 123));
    CU_ASSERT_NOT_EQUAL(xs, NULL);
    pfds_release(xs);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_insertAfter(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = pfds_sequence_insertAfter(
            mkTestValue_sequence(dict, 3),
            2,
            (pfds_object*) mkTestValue_Double(NULL, 123));
    CU_ASSERT_NOT_EQUAL(xs, NULL);
    pfds_release(xs);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}
void test_gc_sequence_update(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = pfds_sequence_updateAt(
            mkTestValue_sequence(dict, 3),
            2,
            (pfds_object*) mkTestValue_Double(NULL, 123));
    CU_ASSERT_NOT_EQUAL(xs, NULL);
    pfds_release(xs);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_delete(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = pfds_sequence_deleteAt(mkTestValue_sequence(dict, 3), 2);
    CU_ASSERT_NOT_EQUAL(xs, NULL);
    pfds_release(xs);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_get(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 2);
    ASSERT_ONE_REF(xs);
    CU_ASSERT_EQUAL(pfds_sequence_size(xs), 2);
    pfds_object* x = pfds_sequence_get(xs, 1);

    ASSERT_ONE_REF(xs);
    pfds_release(xs);

    ASSERT_ONE_REF(x);
    pfds_release(x);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_getPastEnd(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 2);
    ASSERT_ONE_REF(xs);
    CU_ASSERT_EQUAL(pfds_sequence_size(xs), 2);
    pfds_object* x = pfds_sequence_get(xs, 4);

    CU_ASSERT_EQUAL(x, NULL);
    ASSERT_ONE_REF(xs);
    pfds_release(xs);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}


void test_gc_sequence_front(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 2);
    ASSERT_ONE_REF(xs);
    CU_ASSERT_FALSE(pfds_sequence_isEmpty(xs));
    pfds_object* head = pfds_sequence_front(xs);

    ASSERT_ONE_REF(xs);
    pfds_release(xs);

    ASSERT_ONE_REF(head);
    pfds_release(head);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_back(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 2);
    ASSERT_ONE_REF(xs);
    CU_ASSERT_FALSE(pfds_sequence_isEmpty(xs));
    pfds_object* last = pfds_sequence_back(xs);

    ASSERT_ONE_REF(xs);
    pfds_release(xs);

    ASSERT_ONE_REF(last);
    pfds_release(last);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_popBack(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 2);
    ASSERT_ONE_REF(xs);
    CU_ASSERT_FALSE(pfds_sequence_isEmpty(xs));

    pfds_object* last = (pfds_object*) -1;
    pfds_sequence* init = (pfds_sequence*) -1;
    CU_ASSERT(pfds_sequence_popBack(&init, &last, xs));

    ASSERT_ONE_REF(init);
    pfds_release(init);

    ASSERT_ONE_REF(last);
    pfds_release(last);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_popFront_empty(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);


    pfds_sequence* e = (pfds_sequence*) dict->catenable->mempty();
    ASSERT_ONE_REF(e);
    pfds_object* head = (pfds_object*) -1;
    pfds_sequence* tail = (pfds_sequence*) -1;
    CU_ASSERT_FALSE(pfds_sequence_popFront(&head, &tail, e));
    CU_ASSERT_EQUAL(head, NULL);
    CU_ASSERT_EQUAL(tail, NULL);


    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_front_empty(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);


    pfds_sequence* e = (pfds_sequence*) dict->catenable->mempty();
    ASSERT_ONE_REF(e);
    pfds_object* head = pfds_sequence_front(e);
    CU_ASSERT_EQUAL(head, NULL);

    ASSERT_ONE_REF(e);
    pfds_release(e);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_back_empty(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);


    pfds_sequence* e = (pfds_sequence*) dict->catenable->mempty();
    ASSERT_ONE_REF(e);
    pfds_object* last = pfds_sequence_back(e);
    CU_ASSERT_EQUAL(last, NULL);

    ASSERT_ONE_REF(e);
    pfds_release(e);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_popBack_empty(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);


    pfds_sequence* e = (pfds_sequence*) dict->catenable->mempty();
    ASSERT_ONE_REF(e);
    pfds_object* last = (pfds_object*) -1;
    pfds_sequence* init = (pfds_sequence*) -1;
    CU_ASSERT_FALSE(pfds_sequence_popBack(&init, &last, e));
    CU_ASSERT_EQUAL(init, NULL);
    CU_ASSERT_EQUAL(last, NULL);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_split(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 3);
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

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_split1(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 3);
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

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_split2(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 3);
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

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_split3(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 3);
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

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_mappend_1(pfds_sequencevtable* dict) {
    // this test exists to exercise a specific corner case in FingerTree_app3

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 3);
    pfds_sequence* ys = mkTestValue_sequence(dict, 1);
    CU_ASSERT_EQUAL(pfds_sequence_size(xs), 3);
    CU_ASSERT_EQUAL(pfds_sequence_size(ys), 1);

    xs = pfds_sequence_mappend(xs, ys);
    pfds_release(xs);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}
void test_gc_sequence_mappend_2(pfds_sequencevtable* dict) {
    // this test exists to exercise a specific corner case in FingerTree_app3

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 2);
    xs = pfds_sequence_pushBack(xs, (pfds_object*) mkTestValue_Double(NULL, 10));
    xs = pfds_sequence_pushBack(xs, (pfds_object*) mkTestValue_Double(NULL, 11));
    xs = pfds_sequence_pushBack(xs, (pfds_object*) mkTestValue_Double(NULL, 12));

    pfds_sequence* ys = mkTestValue_sequence(dict, 2);
    ys = pfds_sequence_pushFront((pfds_object*) mkTestValue_Double(NULL, 10), ys);
    ys = pfds_sequence_pushFront((pfds_object*) mkTestValue_Double(NULL, 11), ys);
    ys = pfds_sequence_pushFront((pfds_object*) mkTestValue_Double(NULL, 12), ys);

    xs = pfds_sequence_mappend(xs, ys);
    pfds_release(xs);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}
void test_gc_sequence_mappend_4(pfds_sequencevtable* dict) {
    // this test exists to exercise a specific corner case in FingerTree_app3

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 2);
    xs = pfds_sequence_pushBack(xs, (pfds_object*) mkTestValue_Double(NULL, 10));

    pfds_sequence* ys = mkTestValue_sequence(dict, 2);
    ys = pfds_sequence_pushFront((pfds_object*) mkTestValue_Double(NULL, 10), ys);

    xs = pfds_sequence_mappend(xs, ys);
    pfds_release(xs);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}
void test_gc_sequence_mappend_3(pfds_sequencevtable* dict) {
    // this test exists to exercise a specific corner case in FingerTree_app3

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 2);
    xs = pfds_sequence_pushBack(xs, (pfds_object*) mkTestValue_Double(NULL, 10));
    xs = pfds_sequence_pushBack(xs, (pfds_object*) mkTestValue_Double(NULL, 11));

    pfds_sequence* ys = mkTestValue_sequence(dict, 2);
    ys = pfds_sequence_pushFront((pfds_object*) mkTestValue_Double(NULL, 10), ys);
    ys = pfds_sequence_pushFront((pfds_object*) mkTestValue_Double(NULL, 11), ys);

    xs = pfds_sequence_mappend(xs, ys);
    pfds_release(xs);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}
void test_gc_sequence_mpow(pfds_sequencevtable* dict) {
    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 3);
    CU_ASSERT_EQUAL(pfds_sequence_size(xs), 3);

    pfds_retain(xs);
    xs = pfds_sequence_mappend(xs, xs);
    ASSERT_ONE_REF(xs);
    pfds_release(xs);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_isEmpty(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 3);
    CU_ASSERT_FALSE(pfds_sequence_isEmpty(xs));
    ASSERT_ONE_REF(xs);
    pfds_release(xs);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_concatEmpty(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* elts[0] = {
    };
    pfds_object* ys = dict->catenable->concat(0, (pfds_object**) elts);
    ASSERT_ONE_REF(ys);
    pfds_release(ys);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_concat(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* elts[3] = {
        mkTestValue_sequence(dict, 1),
        mkTestValue_sequence(dict, 2),
        mkTestValue_sequence(dict, 3),
    };
    pfds_object* ys = dict->catenable->concat(3, (pfds_object**) elts);
    ASSERT_ONE_REF(ys);
    pfds_release(ys);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

void test_gc_sequence_reduceRight(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 3);
    pfds_object* y = pfds_sequence_reduceRight((binop) test_binop, NULL,
            xs,
            (pfds_object*) pfds_String_fromCstring("ROOT"));
    ASSERT_ONE_REF(y);

    pfds_release(y);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}
void test_gc_sequence_reduceLeft(pfds_sequencevtable* dict) {

    PREPARE_GC_COUNTS(gcCounts);

    pfds_sequence* xs = mkTestValue_sequence(dict, 3);
    pfds_object* y = pfds_sequence_reduceLeft((binop) test_binop, NULL,
            (pfds_object*) pfds_String_fromCstring("ROOT"),
            xs);
    ASSERT_ONE_REF(y);

    pfds_release(y);

    ASSERT_GC_COUNTS(gcCounts, 0, 0);
}

struct benchEvent {
    struct timespec realtime;
    struct timespec monotonic;
    struct timespec cpu;
};

struct benchRun {
    struct benchEvent start;
    struct benchEvent end;
};

struct benchState {
    int iterations;
    size_t n;
    SplitMix64 gen;
    struct benchEvent setup;
    struct benchRun *runs;
    struct benchEvent teardown;
    struct benchEvent done;
};

int benchGetEventTime(struct benchEvent* evt) {
    int res = 0;
    res = clock_gettime(CLOCK_REALTIME, &evt->realtime);
    res = clock_gettime(CLOCK_MONOTONIC, &evt->monotonic) || res;
    res = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &evt->cpu) || res;
    return res;
};


void bench_setup(struct benchState *bs) {
    benchGetEventTime(&bs->setup);
}
void bench_begin(struct benchState *bs, int iter) {
    assert(0 <= iter && iter < bs->iterations);
    benchGetEventTime(&bs->runs[iter].start);
}
void bench_end(struct benchState *bs, int iter) {
    assert(0 <= iter && iter < bs->iterations);
    benchGetEventTime(&bs->runs[iter].end);
}
void bench_teardown(struct benchState *bs) {
    benchGetEventTime(&bs->teardown);
}
void bench_done(struct benchState *bs) {
    benchGetEventTime(&bs->done);
}

struct benchConfig {
    const char* benchmark;
    const char* type;
    int iterations;
    int n;
};

enum benchEventTag {
    BENCHEVENT_SETUP,
    BENCHEVENT_START,
    BENCHEVENT_END,
    BENCHEVENT_TEARDOWN,
    BENCHEVENT_DONE,
};

const char* tagStr(enum benchEventTag tag) {
    switch (tag) {
        case BENCHEVENT_SETUP: return "SETUP";
        case BENCHEVENT_START: return "START";
        case BENCHEVENT_END: return "END";
        case BENCHEVENT_TEARDOWN: return "TEARDOWN";
        case BENCHEVENT_DONE: return "DONE";
        default: return "";
    }
}

void writeBenchmarkResult(FILE* stream, struct benchConfig * conf, enum benchEventTag tag, int iter, struct benchEvent * event) {
    fprintf(stream, "\"%s\",\"%s\",%d,%d,\"%s\"", conf->benchmark, conf->type, conf->iterations, conf->n, tagStr(tag));
    if (iter == -1) {
        fputs(",\"\"", stream);
    } else {
        fprintf(stream, ",%d", iter);
    }
    fprintf(stream, ",%jd.%09ld", (intmax_t) event->realtime.tv_sec, event->realtime.tv_nsec);
    fprintf(stream, ",%jd.%09ld", (intmax_t) event->monotonic.tv_sec, event->monotonic.tv_nsec);
    fprintf(stream, ",%jd.%09ld\n", (intmax_t) event->cpu.tv_sec, event->cpu.tv_nsec);
}

void writeBenchmarkHeader(FILE* stream) {
    fprintf(stream, "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"", "benchmark", "type", "iterations", "n", "event");
    fprintf(stream, ",\"%s\"", "iteration");
    fprintf(stream, ",\"%s\"", "realtime");
    fprintf(stream, ",\"%s\"", "monotonic");
    fprintf(stream, ",\"%s\"\n", "cpu");
}

typedef void (*benchFn)(struct benchState *, void*);

void runBenchmark(struct benchConfig *conf, benchFn benchFn, void* ud, FILE* results) {
    struct benchState bs;
    bs.iterations = conf->iterations;
    // SplitMix64_initDefault(&bs.gen);
    SplitMix64_init(&bs.gen, 0);
    bs.runs = alloca(sizeof(struct benchRun) * conf->iterations);

    bs.n = conf->n;

    bench_setup(&bs);
    benchFn(&bs, ud);
    bench_done(&bs);

    if (results != NULL) {
        writeBenchmarkResult(results, conf, BENCHEVENT_SETUP, -1, &bs.setup);
        for (int i = 0 ; i < conf->iterations ; i++) {
            writeBenchmarkResult(results, conf, BENCHEVENT_START, i, &bs.runs[i].start);
            writeBenchmarkResult(results, conf, BENCHEVENT_END, i, &bs.runs[i].end);
        }
        writeBenchmarkResult(results, conf, BENCHEVENT_TEARDOWN, -1, &bs.teardown);
        writeBenchmarkResult(results, conf, BENCHEVENT_DONE, -1, &bs.done);
        fflush(results);
    }

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

void test_treelist_lookup_1 (void) {

    unsigned int n = 6;
    unsigned int m = 100;

    SplitMix64 gen;
    SplitMix64_initDefault(&gen);

    pfds_object** fixture = (pfds_object**) calloc(sizeof(pfds_object*), n);
    for (size_t i = 0 ; i < n ; i++) {
        fixture[i] = (pfds_object*) pfds_Double_new(SplitMix64_nextDouble(&gen));
    }
    pfds_sequence* xs0 = (pfds_sequence*) pfds_TreeList_fromArray(n, fixture);

    pfds_sequence* xs = xs0;
    pfds_retain(xs);

    for (int i = 0; i < m; i++) {
        int idx = SplitMix64_nextInt64Range(&gen, 0, n);
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

    pfds_release(xs);
    pfds_release(xs0);

    free(fixture);

}
void test_treelist_debugfputs (void) {
    PREPARE_GC_COUNTS(gcCounts);
    pfds_object* elements[50];
    for (int i = 0 ; i < 50; i++) {
        elements[i] = (pfds_object*) pfds_UInt64_new(i);
    }

    char* buf;
    size_t size;
    FILE* stream = open_memstream(&buf, &size);

    pfds_TreeList* lst = pfds_TreeList_mempty();

    pfds_TreeList_debugfputs(stream, lst);
    pfds_release(lst);
    fputs("\n", stream);

    pfds_retain_array(1, elements);
    lst = pfds_TreeList_singleton(elements[0]);
    pfds_TreeList_debugfputs(stream, lst);
    pfds_release(lst);
    fputs("\n", stream);

    pfds_retain_array(2, elements);
    lst = pfds_TreeList_fromArray(2, elements);
    pfds_TreeList_debugfputs(stream, lst);
    pfds_release(lst);
    fputs("\n", stream);

    pfds_retain_array(6, elements);
    lst = pfds_TreeList_fromArray(6, elements);
    pfds_TreeList_debugfputs(stream, lst);
    pfds_release(lst);
    fputs("\n", stream);

    lst = pfds_TreeList_fromArray(50, elements);
    pfds_TreeList_debugfputs(stream, lst);
    pfds_release(lst);
    fputs("\n", stream);

    fflush(stream);

    CU_ASSERT_STRING_EQUAL( buf,
        "TREELIST:{EMPTY}\n"
        "TREELIST:{SINGLE(0)}\n"
        "TREELIST:{DEEP(<m:2>, D1[0], EMPTY, D1[1])}\n"
        "TREELIST:{DEEP(<m:6>, D1[0], SINGLE(N3[<m:3>, 1, 2, 3]), D2[4, 5])}\n"
        "TREELIST:{DEEP(<m:50>, D1[0], DEEP(<m:45>, D1[N3[<m:3>, 1, 2, 3]], DEEP(<m:36>,"
        " D1[N3[<m:9>, N3[<m:3>, 4, 5, 6], N3[<m:3>, 7, 8, 9], N3[<m:3>, 10, 11, 12]]],"
        " EMPTY, D3[N3[<m:9>, N3[<m:3>, 13, 14, 15], N3[<m:3>, 16, 17, 18], N3[<m:3>, 19,"
        " 20, 21]], N3[<m:9>, N3[<m:3>, 22, 23, 24], N3[<m:3>, 25, 26, 27], N3[<m:3>, 28,"
        " 29, 30]], N3[<m:9>, N3[<m:3>, 31, 32, 33], N3[<m:3>, 34, 35, 36], N3[<m:3>, 37,"
        " 38, 39]]]), D2[N3[<m:3>, 40, 41, 42], N3[<m:3>, 43, 44, 45]]), D4[46, 47, 48,"
        " 49])}\n");


    free(buf);
    ASSERT_GC_COUNTS(gcCounts, 0, 0);

}

void test_treelist_lookup_2 (void) {
    const pfds_objectvtable *vtable = &pfds_TreeList_vtable;
    struct benchState myBs = {
        .n = 48,
        .iterations = 1,
    };
    // SplitMix64_init(&myBs.gen, 1000);
    struct benchState *bs = &myBs;

    pfds_object** fixture = (pfds_object**) calloc(sizeof(pfds_object*), bs->n);
    for (size_t i = 0 ; i < bs->n ; i++) {
        fixture[i] = (pfds_object*) pfds_UInt64_new(i);
    }
    pfds_sequence* xs0 = vtable->sequence->fromArray(bs->n, fixture);

    for (int iter = 0; iter < bs->iterations; iter++) {

        pfds_sequence* xs = xs0;
        pfds_retain(xs);

        int indices[3] = {32, 13, 0};
        for (int i = 0; i < 3; i++) {
            int idx = indices[i];
            pfds_object *x = pfds_sequence_get(xs, idx);
            if ( x == NULL
                    || ( x != NULL
                        && x != fixture[idx]
                        && pfds_cmp(x, fixture[idx]) != PFDS_EQ)
                    ) {
                CU_FAIL_FATAL("get returned incorrect element");
            }
        }
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
void bench_fillFromArray(struct benchState *bs, pfds_objectvtable *vtable) {
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
void bench_fillPushFront(struct benchState *bs, pfds_objectvtable *vtable) {
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

void bench_shuffle(struct benchState *bs, pfds_objectvtable *vtable) {
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
void bench_fillPushBack(struct benchState *bs, pfds_objectvtable *vtable) {
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


struct benchmarkParams {
    unsigned int minSize;
    unsigned int maxSize;
    const char* benchName;
    const char* benchDescrption;

};

#define CLASSY_BENCHMARK(nm, dict, min, max, fn) { .name = nm, .typename = dict.typename, .minSize = min, .maxSize = max, .benchFn = (benchFn) fn, .benchData = (void*) &dict, }

struct testBenchmark {
    const char* name;
    const char* typename;
    unsigned int minSize;
    unsigned int maxSize;
    benchFn benchFn;
    void* benchData;
};


void test_run_one_benchmark(struct testBenchmark* tb) {
    double llb = log(tb->minSize);
    double lub = log(tb->maxSize * bench_size_mult);
    double stepMult = exp((lub - llb) / bench_steps);

    struct benchConfig conf = {
        .iterations = bench_iterations,
        .benchmark = tb->name,
        .type = tb->typename,
        .n = 0,
    };

    double n = tb->minSize;
    for (int i = 0 ; i < bench_steps ; i++ , n *= stepMult) {
        unsigned int nNext = (unsigned int) n;
        if (nNext <= conf.n) continue;
        conf.n = nNext;
        runBenchmark(&conf, tb->benchFn, tb->benchData, benchmarkResults);
    }
}

struct nstring {
    size_t size;
    char* buf;
};

struct nstringgarbage {
    int strno;
    struct nstring nstring;
    struct nstringgarbage* next_nstring;
};

static struct nstringgarbage* first_nstring = NULL;
static int snprintf_strno = 0;

void ssprintf_dispose(void) {
    // fprintf(stderr, "ssprintf_dispose strno: %d\n", snprintf_strno);
    while (first_nstring != NULL) {
        // fprintf(stderr, "ssprintf_dispose: free(%p * %lu) # %d\n", first_nstring, first_nstring->nstring.size, first_nstring->strno);
        struct nstringgarbage* next_nstring = first_nstring->next_nstring;
        free(first_nstring->nstring.buf);
        free(first_nstring);
        first_nstring = next_nstring;
    }
}

struct nstring ssprintf(const char* format, ...) {
    if (first_nstring == NULL) {
        atexit(ssprintf_dispose);
    }
    struct nstringgarbage* next_nstring = first_nstring;
    first_nstring = malloc(sizeof(struct nstringgarbage));
    first_nstring->next_nstring = next_nstring;
    first_nstring->strno = snprintf_strno++;

    // struct nstring result;
    FILE* stream = open_memstream(&first_nstring->nstring.buf, &first_nstring->nstring.size);

    va_list va;
    va_start(va, format);
    vfprintf(stream, format, va);
    va_end(va);
    fflush(stream);

    return first_nstring->nstring;
};


int main(int argc, char** argv)
{
    SplitMix64_initialSeed();

    CU_initialize_registry();


    CU_pSuite ccheckSuite = CU_add_suite("ccheck", 0, 0);
    CCHECK_add_prop_forAll(ccheckSuite, 10, test_assoc_int, 3, &genInt, &genInt, &genInt);
    // CCHECK_add_prop_forAll(suite, 10, test_lessthan_5, 1, &genInt);


    CU_pSuite validatorSuite = CU_add_suite("typedefs", 0, 0);

    CU_add_test_with(validatorSuite, "vtable/LinkedList", (CU_TestFunc1) test_vtable, (void*) &pfds_LinkedList_vtable);
    CU_add_test_with(validatorSuite, "vtable/ArrayList", (CU_TestFunc1) test_vtable, (void*) &pfds_ArrayList_vtable);
    CU_add_test_with(validatorSuite, "vtable/TreeList", (CU_TestFunc1) test_vtable, (void*) &pfds_TreeList_vtable);
    CU_add_test_with(validatorSuite, "vtable/ArrayMap", (CU_TestFunc1) test_vtable, (void*) &pfds_ArrayMap_vtable);


    CU_pSuite gcSuite = CU_add_suite("gc", 0, 0);
    CU_add_test(gcSuite, "new/double", test_gc_new_release_double);
    CU_add_test(gcSuite, "new/string", test_gc_new_release_string);
    CU_add_test(gcSuite, "new/constString", test_gc_new_release_constString);
    CU_add_test(gcSuite, "new/arrayListEmpty", test_gc_new_release_arrayListEmpty);
    CU_add_test(gcSuite, "new/arrayListFromArray0", test_gc_new_release_arrayListFromArray0);
    CU_add_test(gcSuite, "new/arrayListFromArray", test_gc_new_release_arrayListFromArray);
    CU_add_test(gcSuite, "new/arrayListFromArray2", test_gc_new_release_arrayListFromArray2);
    CU_add_test(gcSuite, "new/linkedListNil", test_gc_new_release_linkedListNil);
    CU_add_test(gcSuite, "new/linkedListCons", test_gc_new_release_linkedListCons);

    struct gcObjectMethods {
        CU_TestFunc1 testFn;
        const char* desc;
    } gcObjectMethods [] = {
        { .testFn = test_gc_newrelease, .desc = "new-release", },
        { .testFn = test_gc_debugputstr, .desc = "debugputstr", },
        { .testFn = test_gc_cmp, .desc = "compare", },
        { 0 }
    };

    struct gcObjectInstances gcObjectInstances[] = {
        { .vtable = &pfds_String_vtable, .mkValue = (pfds_object*(*)(void*, int)) mkTestValue_String },
        { .vtable = &pfds_UInt64_vtable, .mkValue = (pfds_object*(*)(void*, int)) mkTestValue_UInt64 },
        { .vtable = &pfds_Double_vtable, .mkValue = (pfds_object*(*)(void*, int)) mkTestValue_Double },
        { 0 }
    };

    for (size_t j = 0; gcObjectInstances[j].vtable ; j++) {
        for(size_t i = 0 ; gcObjectMethods[i].testFn ; i++) {
            CU_add_test_with(
                    gcSuite,
                    ssprintf("%s/%s", gcObjectInstances[j].vtable->typename, gcObjectMethods[i].desc).buf,
                    gcObjectMethods[i].testFn,
                    (void*) &gcObjectInstances[j]);
        }
    }

    struct gcSequenceMethods {
        void (*testFn)(pfds_sequencevtable*);
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

    struct sequenceInstance {
        const pfds_objectvtable* vtable;
    } sequenceInstance[] = {
        { &pfds_ArrayList_vtable },
        { &pfds_LinkedList_vtable },
        { &pfds_TreeList_vtable },
        { 0 }
    };

    for (size_t j = 0; sequenceInstance[j].vtable ; j++) {
        for(size_t i = 0 ; gcSequenceMethods[i].testFn ; i++) {
            CU_add_test_with(
                    gcSuite,
                    ssprintf("%s/%s", sequenceInstance[j].vtable->typename, gcSequenceMethods[i].desc).buf,
                    (CU_TestFunc1) gcSequenceMethods[i].testFn,
                    (void*) sequenceInstance[j].vtable->sequence);
        }
    }


    // TODO: actual property testing, mkValue stuff should be a real generator,
    // enhance ccheck until this is the easy way to write those tests.
    CU_pSuite propSuite = CU_add_suite("properties", 0, 0);

    struct catenableInstance catenableInstance[] = {
        { .name = "String", .dict = pfds_String_vtable.catenable, .mkValue = (pfds_object* (*)(void*, int)) mkTestValue_String, .ud = 0},
        { .name = "sum/size_t", .dict = &pfds_catenable_sum, .mkValue = (pfds_object* (*)(void*, int)) mkTestValue_UInt64, .ud = 0},
        { 0 }
    };

    struct propCatenableMethods propCatenableMethods[] = {
        { .testFn = test_catenable_ident, .desc = "identity" },
        { .testFn = test_catenable_assoc, .desc = "associative" },
        { .testFn = test_catenable_concat, .desc = "concat=append" },
        { 0 }
    };

    for (size_t j = 0; catenableInstance[j].dict ; j++) {
        for(size_t i = 0 ; propCatenableMethods[i].testFn ; i++) {
            CU_add_test_with(
                    propSuite,
                    ssprintf("%s/%s", catenableInstance[j].name, propCatenableMethods[i].desc).buf,
                    (CU_TestFunc1) propCatenableMethods[i].testFn,
                    (void*) &catenableInstance[j]);
        }
    }
    for (size_t j = 0; sequenceInstance[j].vtable ; j++) {
        struct catenableInstance *inst = (struct catenableInstance*) malloc(sizeof(struct catenableInstance));
        inst->name = sequenceInstance[j].vtable->typename;
        inst->dict = sequenceInstance[j].vtable->sequence->catenable;
        inst->mkValue = (pfds_object* (*)(void*, int)) mkTestValue_sequence;
        inst->ud = (void*) sequenceInstance[j].vtable->sequence;

        for(size_t i = 0 ; propCatenableMethods[i].testFn ; i++) {
            CU_add_test_with(
                    propSuite,
                    ssprintf("%s/%s", inst->name, propCatenableMethods[i].desc).buf,
                    (CU_TestFunc1) propCatenableMethods[i].testFn,
                    (void*) inst);
        }
    }

    CU_pSuite miscSuite = CU_add_suite("misc", 0, 0);

    CU_add_test(miscSuite, "TreeList/cmp/case1", test_treelist_cmp_1);
    CU_add_test(miscSuite, "TreeList/lookup/case1", test_treelist_lookup_1);
    CU_add_test(miscSuite, "TreeList/lookup/case2", test_treelist_lookup_2);
    CU_add_test(miscSuite, "TreeList/debugfputs", test_treelist_debugfputs);


    CU_add_test(miscSuite, "test of pfds_String", test_String);
    CU_add_test(miscSuite, "test of pfds_Double", test_Double);
    CU_add_test(miscSuite, "test of pfds_ArrayList", test_ArrayList);

    // CU_add_test(miscSuite, "test of catenable/String", test_catenable_String);
    // CU_add_test(miscSuite, "test of catenable/Sum", test_catenable_sum);
    CU_add_test_with(miscSuite, "test of sequence/ArrayList", (CU_TestFunc1) test_sequence, (void*) pfds_ArrayList_vtable.sequence);
    CU_add_test_with(miscSuite, "test of sequence/LinkedList", (CU_TestFunc1) test_sequence, (void*) pfds_LinkedList_vtable.sequence);
    CU_add_test_with(miscSuite, "test of sequence/TreeList", (CU_TestFunc1) test_sequence, (void*) pfds_TreeList_vtable.sequence);

    CCHECK_add_prop(miscSuite, "test props imperatively", NULL, 10, imperative_prop, NULL);


    // benchmark values are chosen here to run very quickly in an ordinary run
    // with the default size multiplier of 1.0, but also all finish in max a
    // few seconds each with whatever options are specified in `make bench`.
    // pathologically bad benchmarks should only be allowed to run just long
    // enough to show that they are pathological in the benchmark graphs.
    struct testBenchmark testBenchmarks[] = {
        CLASSY_BENCHMARK("lookup", pfds_ArrayList_vtable,   10, 10000, bench_lookup),
        CLASSY_BENCHMARK("lookup", pfds_TreeList_vtable,   10, 10000, bench_lookup),
        CLASSY_BENCHMARK("queueLeft", pfds_TreeList_vtable,   10, 10000, bench_queueLeft),
        CLASSY_BENCHMARK("queueLeft", pfds_ArrayList_vtable,  10, 2000, bench_queueLeft),
        CLASSY_BENCHMARK("queueLeft", pfds_LinkedList_vtable,   10, 1000, bench_queueLeft),
        CLASSY_BENCHMARK("fillFromArray", pfds_TreeList_vtable,   10, 10000, bench_fillFromArray),
        CLASSY_BENCHMARK("fillFromArray", pfds_LinkedList_vtable, 10, 10000, bench_fillFromArray),
        CLASSY_BENCHMARK("fillFromArray", pfds_ArrayList_vtable,  100, 100000, bench_fillFromArray),
        CLASSY_BENCHMARK("fillPushFront", pfds_TreeList_vtable, 10, 10000, bench_fillPushFront),
        CLASSY_BENCHMARK("fillPushFront", pfds_LinkedList_vtable, 10, 10000, bench_fillPushFront),
        CLASSY_BENCHMARK("fillPushFront", pfds_ArrayList_vtable, 10, 1000, bench_fillPushFront),
        CLASSY_BENCHMARK("fillPushBack", pfds_TreeList_vtable, 10, 10000, bench_fillPushBack),
        CLASSY_BENCHMARK("fillPushBack", pfds_LinkedList_vtable, 10, 500, bench_fillPushBack),
        CLASSY_BENCHMARK("fillPushBack", pfds_ArrayList_vtable, 10, 1000, bench_fillPushBack),
        CLASSY_BENCHMARK("shuffle", pfds_ArrayList_vtable, 10, 1000, bench_shuffle),
        CLASSY_BENCHMARK("shuffle", pfds_LinkedList_vtable, 10, 250, bench_shuffle),
        CLASSY_BENCHMARK("shuffle", pfds_TreeList_vtable, 10, 10000, bench_shuffle),
        { 0 },
    };

    CU_pSuite benchSuite = CU_add_suite("bench", 0, 0);
    for (int i = 0 ; testBenchmarks[i].benchFn  ; i++) {
        CU_add_test_with(
                benchSuite,
                ssprintf("%s/%s", testBenchmarks[i].name, testBenchmarks[i].typename).buf,
                (CU_TestFunc1) test_run_one_benchmark,
                &testBenchmarks[i]);
    }



    struct arguments arguments = { 0 };

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    CU_pTestRegistry pRegistry = CU_get_registry();

    if (arguments.preg) {
        for(CU_pSuite pSuite = pRegistry->pSuite; pSuite ; pSuite = pSuite->pNext) {
            if(regexec(arguments.preg, pSuite->pName, 0, 0, 0) == REG_NOMATCH) {
                for(CU_pTest pTest = pSuite->pTest; pTest ; pTest = pTest->pNext) {
                    size_t longNameSz = strlen(pSuite->pName) + strlen(pTest->pName) + 2; // slash and nul byte

                    char* longName = (char*) malloc(longNameSz);
                    snprintf(longName, longNameSz, "%s/%s", pSuite->pName, pTest->pName);
                    if(regexec(arguments.preg, longName, 0, 0, 0) == REG_NOMATCH) {
                        pTest->fActive = CU_FALSE;
                    }
                }
            }
        }
    }
    if (arguments.verbosity > 0) {
        CU_basic_set_mode(CU_BRM_VERBOSE);
    }

    switch(arguments.mode) {
        case TEST_MODE_LIST:
            if (pRegistry) {
                CU_pSuite s = pRegistry->pSuite;
                while (s) {
                    if (s->fActive) {
                        printf("%s\n", s->pName);
                        CU_pTest t = s->pTest;
                        while (t) {
                            if (t->fActive) printf("  %s\n", t->pName);
                            t = t->pNext;
                        }
                    }
                    s = s->pNext;
                }
            }
            ssprintf_dispose();
            return 0;
        default:

            if (arguments.benchmarkResultsPath) {
                if (strcmp(arguments.benchmarkResultsPath, "-") == 0) {
                    benchmarkResults = stdout;
                } else {
                    benchmarkResults = fopen(arguments.benchmarkResultsPath, "w");
                    if(!benchmarkResults) {
                        perror(arguments.benchmarkResultsPath);
                        exit(2);
                    }
                }

                writeBenchmarkHeader(benchmarkResults);
            }

            // struct mallinfo2 mi_start = mallinfo2();
            CU_basic_run_tests();
            // struct mallinfo2 mi_end = mallinfo2();
            // display_mallinfodelta(stderr, &mi_start, &mi_end);

            unsigned int result = CU_get_number_of_failures();
            CU_cleanup_registry();
            if (benchmarkResults) {
                fclose(benchmarkResults);
            }
            ssprintf_dispose();
            return result;
    }
}
