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

#ifndef CCHECK_HEADER_DEFINED
#define CCHECK_HEADER_DEFINED
#include <stdarg.h>
#include <ffi.h>

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "splitmix.h"

#define PROP_COUNTEREXAMPLE CU_FALSE
#define PROP_SAT CU_TRUE

#define CCHECK_add_prop_forAll(pSuite, iterations, fn, nargs, ...) \
    CCHECK_add_prop_forall_impl(pSuite, iterations, __FILE__, __LINE__ ,#fn, (CCHECK_PROP) fn, nargs, __VA_ARGS__)

/** Signature for a test function accepting one argument cast to void* */
typedef void (*CU_TestFunc1)(void*);

/** a version of CU_add_test that passes an extra parameter to the test */
CU_pTest CU_add_test_with(CU_pSuite pSuite, char * strName, CU_TestFunc1 pTestFunc, void* ud);


typedef void (*CCHECK_PROP)(void);

/** a generator for random values of a particular type.  */
typedef struct CCHECK_Gen {
    /** user supplied argument will be passed to each method */
    void* userData;

    /** the type of the generated value. */
    ffi_type* genType;

    /** generate a random value cast to void*
     * @param userData
     * @param randGen the value generated must be deterministically computed from.
     * @returns a pointer to the generated value cast to void*
     */
    void* (*generate)(void* userData, SplitMix64 *randGen);
    /** render the value to the supplied stream.
     *
     * used to display counterexamples for failing properties.
     *
     * @param stream FILE* to print the sample to.
     * @param userData userData
     * @param sample the value to be stringified previously returned by `generate()`
     * @returns the number of characters written
     */
    int (*show)(FILE* stream, void* userData, void* sample);
    /** clean up a generated sample.
     *
     * if this field is null then the returned value will be freed with
     * `free(3)`.  if it is set to something then the dispose method is
     * responsible for freeing the sample.
     *
     * @param userData userData
     * @param sample a value previously returned by `generate()` to be disposed
     */
    void (*dispose)(void* userData, void* sample);
} CCHECK_Gen;

extern const struct CCHECK_Gen genInt;
extern const struct CCHECK_Gen genDouble;

void genIntConstant(CCHECK_Gen* gen, int);
void genIntRange(CCHECK_Gen* gen, int, int);

void genDoubleConstant(CCHECK_Gen* gen, double);
void genDoubleRange(CCHECK_Gen* gen, double, double);

/** add a test that checks a simple property
 *
 * @param pSuite      Test suite to which to add new test (non-NULL).
 * @param iterations  number of times the property should be checked with generated inputs before accepting.
 * @param fileName    file name
 * @param lineNo      lineNumber
 * @param strName     Name for the new test case (non-NULL).
 * @param propFn      Function to call when running the test (non-NULL).
 * @param nargs       the number of generators that follow
 * @param ...         generators for the arguments to the test function.  They
 *                    must exactly match the number and type of arguments to propFn
 * @return A pointer to the newly-created test (NULL if creation failed)
 * @see CU_add_test
 */
extern CU_pTest CCHECK_add_prop_forall_impl(CU_pSuite pSuite, int iterations,
        char* fileName, int lineNo, char * strName,
        CCHECK_PROP propFn, size_t nargs, ...);


typedef struct CCHECK_Context CCHECK_Context;

extern void CCHECK_test_forall_impl(
    SplitMix64* seedGen,
    int iterations,
    int lineNo,
    char* fileName,
    char* strName,
    CCHECK_PROP propFn,
    size_t nargs,
    struct CCHECK_Gen** gens);

extern void* CCHECK_generate(CCHECK_Context* ctx, const CCHECK_Gen* gen, char* hint);

/** check a particular property holds.
 *
 * if the property fails, the current context will be unwound and all generated
 * values displayed to the user as a counterexample
 *
 * @param ctx the CCHECK_Context passed to the property function
 * @param prop a boolean expression to assert.
 * */
#define CCHECK_assert_prop(ctx, prop) CCHECK_assert_prop_impl(ctx, __LINE__, __FILE__, #prop, prop)


/** add an imperative style property to a CUnit test suite as its own unit test.
 *
 * @param pSuite      Test suite to which to add new test (non-NULL).
 * @param strName     Name for the new test case (non-NULL).
 * @param source for test seeds.  may be null.
 * @param iterations number of attempts to find a counterexample
 * @param propFn function which tests properties using a CCHECK_Context
 * @param userData userData passed to propFn
 * */
extern void CCHECK_add_prop(CU_pSuite pSuite, char * strName,
        SplitMix64 *seedGen, int iterations,
        void(*propFn)(CCHECK_Context*, void*), void* userData);

/** Implementation of CCHECK_assert_prop
 *
 * if the property fails, the current context will be unwound and all generated
 * values displayed to the user as a counterexample
 *
 * @param ctx the CCHECK_Context passed to the property function
 * @param lineNo line number
 * @param fileName file name
 * @param strName string display name of property being checked.
 * @param prop a boolean expression to assert.
 * */
extern void CCHECK_assert_prop_impl(CCHECK_Context* ctx, int lineNo, char* fileName, char* strName, CU_BOOL prop);

/** implementation of CHECK_test_prop.
 *
 * @param source for test seeds.  may be null.
 * @param iterations number of attempts to find a counterexample
 * @param propFn function which tests properties using a CCHECK_Context
 * @param userData userData passed to propFn
 * */
extern void CCHECK_test_prop_impl(
        SplitMix64 *seedGen, int iterations,
        void(*propFn)(CCHECK_Context*, void*), void* userData);

#endif
