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

#include <float.h>
#include <math.h>

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "ccheck.h"


void asdf() {
    // run through every corner case
    // generate random samples
    // if a counterexample is found, try to shirnk each argument.
    // report the best counterexample.

}


void imperative_prop(CCHECK_Context *ctx, void* userData) {
    int iter = CCHECK_Context_iteration(ctx);
    int sz = iter >> 3;

    int a = *(int*) CCHECK_generate(ctx, &genInt, sz, "a");
    int b = *(int*) CCHECK_generate(ctx, &genInt, sz, "b");

    CCHECK_assert_prop(ctx, a+b == b+a /* commutative */ );

    int c = *(int*) CCHECK_generate(ctx, &genInt, sz, "c");
    CCHECK_assert_prop(ctx, ((a+b)+c) == (a+(b+c)) /* associative */ );
}

void test_props_imperatively(void) {
    CCHECK_test_prop_impl(NULL, 10, imperative_prop, NULL);
}


CU_BOOL prop_assoc_int(int a, int b, int c) {
    return (a + b) + c == a + (b + c);
}

CU_BOOL prop_assoc_double_naive(double a, double b, double c) {
    return (a + b) + c == a + (b + c);
}

// addition of doubles is not exact associative.
CU_BOOL prop_assoc_double(double a, double b, double c) {
    double l = (a + b) + c;
    double r = a + (b + c);
    double e = l - r;
    return fabs(e) <= DBL_EPSILON;
}

CU_BOOL prop_const_int(int a) {
    return a == 5;
}

CU_BOOL prop_const_double(double a) {
    return a == 5;
}

CU_BOOL prop_range_int(int a) {
    return a >= 5 && a < 10;
}

CU_BOOL prop_range_double(int a) {
    return a >= 5 && a < 10;
}


void test_assoc_double_examples(void) {
    double a = 6.38139211535037054e-01, b = 7.69100348964892988e-01, c = 5.33345631684747357e-02;
    CU_ASSERT_FALSE(prop_assoc_double_naive(a, b, c));
    CU_ASSERT(prop_assoc_double(a, b, c));
}

void test_genSeries(CCHECK_Context *ctx, void* userData) {
    size_t sz = CCHECK_Context_iteration(ctx);
    CCHECK_IntArray* series = (CCHECK_IntArray*) CCHECK_generate(ctx, &genSeries, sz, "series");
    CCHECK_assert_prop(ctx, sz == series->size);
    for (int i = 0 ; i < series->size ; i++) {
        CCHECK_assert_prop(ctx, i == series->elements[i]);
    }

}

void test_genArray(CCHECK_Context *ctx, void* userData) {
    static CCHECK_Gen *myGen = NULL;
    if (myGen == NULL) {
        myGen = genArray(genIntRange(0,10));
    }

    size_t sz = CCHECK_Context_iteration(ctx) + 1;
    CCHECK_IntArray* array = (CCHECK_IntArray*) CCHECK_generate(ctx, myGen, sz, "array");
    CCHECK_assert_prop(ctx, sz == array->size);
    int sum = 0;
    for (int i = 0 ; i < array->size ; i++) {
        sum += array->elements[i];
    }
    CCHECK_assert_prop(ctx, sum >= 0 && sum < sz*10);

}


int main(int argc, char** argv) {

    CU_initialize_registry();
    CU_pSuite ccheckSuite = CU_add_suite("ccheck", 0, 0);

    CCHECK_add_prop_forAll(ccheckSuite, 10, prop_assoc_int, 3, &genInt, &genInt, &genInt);
    CCHECK_add_prop_forAll(ccheckSuite, 10, prop_assoc_double, 3, &genDouble, &genDouble, &genDouble);

    CCHECK_add_prop_forAll(ccheckSuite, 10, prop_const_int, 1, genIntConstant(5));
    CCHECK_add_prop_forAll(ccheckSuite, 10, prop_const_double, 1, genDoubleConstant(5));
    CCHECK_add_prop_forAll(ccheckSuite, 10, prop_range_int, 1, genIntRange(5, 10));
    CCHECK_add_prop_forAll(ccheckSuite, 10, prop_range_double, 1, genDoubleRange(5, 10));

    CU_add_test(ccheckSuite, "imperatively", test_props_imperatively);

    CU_add_test(ccheckSuite, "some counterexamples", test_assoc_double_examples);

    CCHECK_add_prop(ccheckSuite, "genSeries", NULL, 10, test_genSeries, NULL);

    CCHECK_add_prop(ccheckSuite, "genSeries", NULL, 10, test_genArray, NULL);

    CU_basic_run_tests();
    unsigned int result = CU_get_number_of_failures();
    CU_cleanup_registry();
    return result;

}
