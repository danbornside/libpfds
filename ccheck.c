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

#include <stdlib.h>
#include "ccheck.h"
#include "splitmix.h"

#define panic(msg) { fprintf(stderr, "PANIC %s:%d\n\t%s\n", __FILE__, __LINE__, msg); abort(); }

struct testHelperInfo {
    void (*pTestFunc)(void*);
    void* ud;

};

void withTestHelperBinding(ffi_cif *cif, void *ret, void* args[], void* closureInfo) {
    struct testHelperInfo* info = (struct testHelperInfo*) closureInfo;
    (info->pTestFunc)(info->ud);
}

extern CU_pTest CU_add_test_with(CU_pSuite pSuite, char * strName, CU_TestFunc1 pTestFunc, void* ud) {
    static int setup = 0;
    static ffi_cif cif;
    static ffi_type *ffi_args[0] = {};

    if (!setup) {
        if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 0, &ffi_type_void, ffi_args) != FFI_OK) panic("ffi setup failed");
        setup = 1;
    }

    struct testHelperInfo* info = (struct testHelperInfo*) malloc(sizeof(struct testHelperInfo));
    info->pTestFunc = pTestFunc;
    info->ud = ud;

    ffi_closure *closure;
    void* boundTestFunc;

    closure = ffi_closure_alloc(sizeof(ffi_closure), &boundTestFunc);

    if (ffi_prep_closure_loc(closure, &cif, withTestHelperBinding, info, boundTestFunc) != FFI_OK) {
        panic("ffi closure setup failed");
    }
    return CU_add_test(pSuite, strName, (CU_TestFunc) boundTestFunc);
}

void* generateIntImpl(void*, SplitMix64* randGen) {
    int *sample = (int*) malloc(sizeof(int));
    *sample = SplitMix64_nextInt(randGen);
    return (void*) sample;
}

int showIntImpl(FILE* stream, void* userData, void* sample) {
    return fprintf(stream, "%d", * (int*) sample);
}

const struct CCHECK_Gen genInt = {
    .genType = &ffi_type_sint,
    .generate = generateIntImpl,
    .show = showIntImpl,
};

void CCHECK_Gen_dispose(const CCHECK_Gen* gen, void* sample) {
    if (sample) {
        if (gen->dispose) {
            gen->dispose(gen->userData, sample);
        } else {
            free(sample); // if you show up on a sunday.
        }
    } else {
        fputs("CCHECK_Gen_dispose called on NULL", stderr);
    }
}

struct testForallClosure {
    int iterations;
    size_t nargs;
    SplitMix64 seedGen;
    struct CCHECK_Gen** gens;
    CCHECK_PROP propFn;
    char* strName;
    int lineNo;
    char* fileName;
};

// uh general sketch for a monadic property check.
// outer loop has an iteration counter.  makes a new seed generator for each loop
// set up a test context with {assoc list of generated values}
// call the prop function with the ctx
//   prop can use a `genValue(ctx, gen, name)` to tell the test runner what its doing.
//   prop can do a `assertProp(ctx, cond, hint)` to tell the runner something that should be true.
// if an assert fails the test runner can long jump out of the test or wait for the prop to return
// failed properties are reported to the user.

void testForAllHelper(ffi_cif* _cif, void* _ret, void* _args[], void* testForAllClosureVoidP) {
    struct testForallClosure* info = (struct testForallClosure*) testForAllClosureVoidP;
    CCHECK_test_forall_impl(&info->seedGen, info->iterations,
         info->lineNo, info->fileName, info->strName,
         info->propFn, info->nargs, info->gens);
}

struct samplesList {
    char* sampleHint;
    void* sample;
    const CCHECK_Gen* gen;

    struct samplesList* next;
};

struct CCHECK_Context {
    int iteration;
    int64_t seed;
    SplitMix64 seedGen;
    struct samplesList* samples;
    struct samplesList* sampleTail;

};

void CCHECK_Context_init(CCHECK_Context *ctx) {
    ctx->samples = ctx->sampleTail = NULL;
}
void CCHECK_Context_reset(CCHECK_Context *ctx) {
    struct samplesList *cur, *prev;
    cur = ctx->samples;
    while (cur) {
        CCHECK_Gen_dispose(cur->gen, cur->sample);
        cur->sample = NULL;
        prev = cur;
        cur = cur->next;
        free(prev);
    }
    CCHECK_Context_init(ctx);
}

extern void CCHECK_assert_prop_impl(CCHECK_Context* ctx, int lineNo, char* fileName, char* strName, CU_BOOL prop) {
    if(!prop) {
        char* buf;
        size_t size = 0;
        FILE* stream = open_memstream(&buf, &size);
        fprintf(stream, "Property %s failed after %d iterations (seed:%ld)\n",
                strName,
                ctx->iteration, ctx->seed);
        struct samplesList* curSample = ctx->samples;
        while(curSample != NULL) {
            fprintf(stream, "\t%s: ", curSample->sampleHint);
            curSample->gen->show(stream, curSample->gen->userData, curSample->sample);
            fprintf(stream, "\n");
            curSample = curSample->next;
        }
        fflush(stream);
        CCHECK_Context_reset(ctx);
        CU_assertImplementation( CU_FALSE, lineNo, buf, fileName, strName, CU_TRUE );
    }
}

extern void* CCHECK_generate(CCHECK_Context* ctx, const CCHECK_Gen* gen, char* hint) {
    struct samplesList* newSample = (struct samplesList*) malloc(sizeof(struct samplesList));
    newSample->next = NULL;
    newSample->sampleHint = hint;
    newSample->gen = gen;
    newSample->sample = gen->generate(gen->userData, &ctx->seedGen);
    if (ctx->samples == NULL) {
        ctx->samples = newSample;
        ctx->sampleTail = newSample;
    } else {
        ctx->sampleTail->next = newSample;
        ctx->sampleTail = newSample;
    }
    return newSample->sample;
}

struct TestPropArgs {
    SplitMix64 *seedGen;
    int iterations;
    void(*propFn)(CCHECK_Context*, void*);
    void* userData;
};

void test_prop_wrapper(struct TestPropArgs* testPropArgs) {
    CCHECK_test_prop_impl(testPropArgs->seedGen, testPropArgs->iterations, testPropArgs->propFn, testPropArgs->userData);
}

extern void CCHECK_add_prop(CU_pSuite pSuite, char * strName,
        SplitMix64 *seedGen, int iterations,
        void(*propFn)(CCHECK_Context*, void*), void* userData) {
    struct TestPropArgs* testPropArgs = (struct TestPropArgs*) malloc(sizeof(struct TestPropArgs));
    testPropArgs->seedGen = seedGen;
    testPropArgs->iterations = iterations;
    testPropArgs->propFn = propFn;
    testPropArgs->userData = userData;
    CU_add_test_with(pSuite, strName, (void (*)(void*)) test_prop_wrapper, testPropArgs);
}


extern void CCHECK_test_prop_impl(
        SplitMix64 *seedGen, int iterations,
        void(*propFn)(CCHECK_Context*, void*), void* userData) {
    SplitMix64 mySeedGen;
    CCHECK_Context ctx;
    CCHECK_Context_init(&ctx);
    if (seedGen) {
        SplitMix64_split(&mySeedGen, seedGen);
    } else {
        SplitMix64_initDefault(&mySeedGen);
    }

    for(ctx.iteration = 0; ctx.iteration < iterations; ctx.iteration++) {
        ctx.seed = SplitMix64_nextInt64(&mySeedGen);
        SplitMix64_init(&ctx.seedGen, ctx.seed);
        propFn(&ctx, userData);
        CCHECK_Context_reset(&ctx);
    }
}


extern void CCHECK_test_forall_impl(
    SplitMix64* seedGen,
    int iterations,
    int lineNo,
    char* fileName,
    char* strName,
    CCHECK_PROP propFn,
    size_t nargs,
    struct CCHECK_Gen** gens) {

    ffi_cif cif;
    ffi_type **ffi_args = (ffi_type**) calloc(sizeof(ffi_type*), nargs);
    void** values = (void**) calloc(sizeof(void*), nargs);

    for (size_t argn = 0; argn < nargs; argn++) {
        ffi_args[argn] = gens[argn]->genType;
    }
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, nargs, &ffi_type_sint, ffi_args) != FFI_OK) panic("ffi setup failed");

    ffi_arg rc;

    int iteration;
    for (iteration = 0; iteration < iterations; iteration++) {

        for (int argn = 0; argn < nargs; argn++) {
            SplitMix64 argGen;
            SplitMix64_split(&argGen, seedGen);
            values[argn] = gens[argn]->generate(gens[argn]->userData, &argGen);
        }
        ffi_call(&cif, propFn, &rc, values);
        switch (rc) {
            case PROP_COUNTEREXAMPLE:
                char* buf;
                size_t size = 0;
                FILE* stream = open_memstream(&buf, &size);
                fprintf(stream, "prop %s NOT SATISFIED with counterexample (", strName);
                for (int argn = 0; argn < nargs; argn++) {
                    if (argn != 0) fputs(", ", stream);
                    gens[argn]->show(stream, gens[argn]->userData, values[argn]);
                }
                fputs(")\n", stream);
                fflush(stream);

                for (int argn = 0; argn < nargs; argn++) {
                    CCHECK_Gen_dispose(gens[argn], values[argn]);
                    values[argn] = NULL;
                }

                CU_assertImplementation(CU_FALSE, lineNo ,buf, fileName , "FORALL", CU_FALSE);
                return;
            case PROP_SAT:
            default:
                for (int argn = 0; argn < nargs; argn++) {
                    CCHECK_Gen_dispose(gens[argn], values[argn]);
                    values[argn] = NULL;
                }
                break;
        }
    }

    size_t size = 0;
    size = snprintf(NULL, 0, "prop %s SATISFIED after %d iterations", strName, iterations);
    snprintf(NULL, 0, "prop %s SATISFIED after %d iterations", strName, iterations);
    char* buf = (char*) malloc(size+1);
    snprintf(buf, size+1, "prop %s SATISFIED after %d iterations", strName, iterations);

    CU_assertImplementation(CU_TRUE, lineNo ,buf, fileName , "FORALL", CU_FALSE);

}

extern CU_pTest CCHECK_add_prop_forall_impl(CU_pSuite pSuite, int iterations,
        char* fileName, int lineNo, char * strName,
        CCHECK_PROP propFn, size_t nargs, ...) {
    // we need to use ffi twice.  we need to bundle everything into a closure
    // to match CU_TestFunc, and once we inside the callee of CU_add_test (ie,
    // the closure is called), we need to repeatedly use ffi_call to call the
    // user function with its desired arguments.  the ffi_cif and args in ths
    // func only deal with the first bit, while bundling the other stuff into
    // the prepared void*

    struct testForallClosure* info = (struct testForallClosure*) malloc(sizeof(struct testForallClosure));

    info->fileName = fileName;
    info->lineNo = lineNo;
    info->strName = strName;
    info->iterations = iterations;
    info->nargs = nargs;
    info->propFn = propFn;
    SplitMix64_initDefault(&info->seedGen);
    info->gens = (struct CCHECK_Gen**) calloc(sizeof(struct CCHECK_Gen*), nargs);
    va_list args;
    va_start(args, (int) nargs);
    for(int i = 0; i < nargs; ++i) {
        info->gens[i] = va_arg(args, struct CCHECK_Gen*);
    }
    va_end(args);

    static int setup = 0;
    static ffi_cif cif;
    static ffi_type *ffi_args[0] = {};

    if (!setup) {
        if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 0, &ffi_type_void, ffi_args) != FFI_OK) panic("ffi setup failed");
        setup = 1;
    }
    ffi_closure *closure;
    void* boundTestFunc;
    closure = ffi_closure_alloc(sizeof(ffi_closure), &boundTestFunc);

    if (ffi_prep_closure_loc(closure, &cif, testForAllHelper, info, boundTestFunc) != FFI_OK) {
        panic("ffi closure setup failed");
    }

    return CU_add_test(pSuite, strName, (CU_TestFunc) boundTestFunc);
}
