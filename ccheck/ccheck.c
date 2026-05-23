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
#include <float.h>
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

void generateIntImpl(int* sample, void*, int size, SplitMix64* randGen) {
    *sample = SplitMix64_nextInt(randGen);
}

int showIntImpl(FILE* stream, void* userData, int* sample) {
    return fprintf(stream, "%d", *sample);
}

const struct CCHECK_Gen genInt = {
    .genType = &ffi_type_sint,
    .generate = (void (*)(void* , void*, int , SplitMix64* ))
        generateIntImpl,
    .show = (int (*)(FILE*, void*, void*))
        showIntImpl,
};


int showIntArray(FILE* stream, void*, struct CCHECK_IntArray* sample) {
    int n = 0;
    n += fprintf(stream, "[");
    for (size_t i = 0; i < sample->size; i++) {
        n += fprintf(stream, "%s%d", (i > 0 ? ", " : ""), sample->elements[i]);
    }
    n += fprintf(stream, "]");
    return n;
}

void disposeIntArray(void*, struct CCHECK_IntArray* sample) {
    if (sample->elements != NULL) {
        free(sample->elements);
    }
}

void generateSeries(struct CCHECK_IntArray* sample, void*, int size, SplitMix64* ) {
    sample->size = size;
    if (size > 0) {
        sample->elements = (int*) calloc(sizeof(int), size);
        for (size_t i = 0; i < size ; i++) {
            sample->elements[i] = i;
        }
    } else {
        sample->elements = NULL;
    }

}

ffi_type* ffi_array_fields[] = {
    &ffi_type_ulong,
    &ffi_type_pointer,
    NULL,
};

ffi_type ffi_array_type = {
    .size = 0, // this will be initialised by libffi when first used.
    .alignment = 0, // set by libffi when first used
    .type = FFI_TYPE_STRUCT,
    .elements = ffi_array_fields,
};

const CCHECK_Gen genSeries = {
    .genType = &ffi_array_type,
    .generate = (void (*)(void* , void*, int , SplitMix64* ))
        generateSeries,
    .show = (int (*)(FILE*, void*, void*))
        showIntArray,
    .dispose = (void (*)(void*, void*))
        disposeIntArray,
};

void generateDoubleImpl(double* sample, void*, int size, SplitMix64* randGen) {
    *sample = SplitMix64_nextDouble(randGen);
}

int showDoubleImpl(FILE* stream, void* userData, double* sample) {
    return fprintf(stream, "%.*e", DBL_DECIMAL_DIG, *sample);
}


const struct CCHECK_Gen genDouble = {
    .genType = &ffi_type_double,
    .generate = (void (*)(void* , void*, int , SplitMix64* ))
        generateDoubleImpl,
    .show = (int (*)(FILE*, void*, void*))
        showDoubleImpl,
};

void CCHECK_Gen_dispose(const CCHECK_Gen* gen, void* sample) {
    if (sample) {
        if (gen->dispose) {
            gen->dispose(gen->userData, sample);
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

extern int CCHECK_Context_iteration(CCHECK_Context *ctx) {
    return ctx->iteration;
}

void CCHECK_Context_reset(CCHECK_Context *ctx) {
    struct samplesList *cur, *prev;
    cur = ctx->samples;
    while (cur) {
        CCHECK_Gen_dispose(cur->gen, cur->sample);
        free(cur->sample);
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

extern void* CCHECK_generate(CCHECK_Context* ctx, const CCHECK_Gen* gen, int size, char* hint) {
    struct samplesList* newSample = (struct samplesList*) malloc(sizeof(struct samplesList));
    newSample->next = NULL;
    newSample->sampleHint = hint;
    newSample->gen = gen;
    newSample->sample = malloc(gen->genType->size);
    gen->generate(newSample->sample, gen->userData, size, &ctx->seedGen);
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
    void** values = (void**) alloca(sizeof(void*) * nargs);

    for (size_t argn = 0; argn < nargs; argn++) {
        ffi_args[argn] = gens[argn]->genType;
    }

    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, nargs, &ffi_type_sint, ffi_args) != FFI_OK) panic("ffi setup failed");

    // we do this step after the call to ffi_prep_cif in case any of the gen type fields need to be set up with a correct size.
    for (size_t argn = 0; argn < nargs; argn++) {
        values[argn] = (void*) alloca(gens[argn]->genType->size);
    }

    ffi_arg rc;

    int iteration;
    for (iteration = 0; iteration < iterations; iteration++) {
        // TODO: come up with a better way to give the size
        int size = iteration >> 4;

        for (int argn = 0; argn < nargs; argn++) {
            SplitMix64 argGen;
            SplitMix64_split(&argGen, seedGen);
            gens[argn]->generate(values[argn], gens[argn]->userData, size, &argGen);
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
                }

                CU_assertImplementation(CU_FALSE, lineNo ,buf, fileName , "FORALL", CU_FALSE);
                return;
            case PROP_SAT:
            default:
                for (int argn = 0; argn < nargs; argn++) {
                    CCHECK_Gen_dispose(gens[argn], values[argn]);
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

void generateConstIntImpl(int* sample, void* ud, int size, SplitMix64* seed) {
    *sample = (int) (long) ud;
}


CCHECK_Gen* genIntConstant(int k) {
    CCHECK_Gen* myGen = (CCHECK_Gen*) malloc(sizeof(CCHECK_Gen));
    myGen->genType = &ffi_type_sint;
    myGen->generate = (void (*)(void* , void*, int , SplitMix64* ))
            generateConstIntImpl;
    myGen->show = (int (*)(FILE*, void*, void*))
            showIntImpl;
    myGen->userData = (void*) (long) k;

    return myGen;
}

struct Gen_ConstantDouble {
    CCHECK_Gen gen;
    double k;
};

void generateConstDoubleImpl(double* sample, struct Gen_ConstantDouble* ud, int size, SplitMix64* seed) {
    *sample = ud->k;
}

CCHECK_Gen* genDoubleConstant(double k) {
    struct Gen_ConstantDouble* myGen = (struct Gen_ConstantDouble*) malloc(sizeof(struct Gen_ConstantDouble));
    myGen->gen.genType = &ffi_type_double;
    myGen->gen.generate = (void (*)(void* , void*, int , SplitMix64* ))
            generateConstDoubleImpl;
    myGen->gen.show = (int (*)(FILE*, void*, void*))
            showDoubleImpl;
    myGen->gen.userData = (void*) myGen;
    myGen->k = k;

    return (CCHECK_Gen*) myGen;
}


struct Gen_IntRange {
    CCHECK_Gen gen;
    int lb;
    int ub;
};

void generateIntRange(int* sample, struct Gen_IntRange* ud, int size, SplitMix64* seed) {
    *sample = (int) SplitMix64_nextInt64Range(seed, ud->lb, ud->ub);
}

CCHECK_Gen* genIntRange(int lb, int ub) {
    struct Gen_IntRange* myGen = (struct Gen_IntRange*) malloc(sizeof(struct Gen_IntRange));
    myGen->gen.genType = &ffi_type_sint;
    myGen->gen.generate = (void (*)(void* , void*, int , SplitMix64* ))
            generateIntRange;
    myGen->gen.show = (int (*)(FILE*, void*, void*))
            showIntImpl;
    myGen->gen.userData = (void*) myGen;
    myGen->lb = lb;
    myGen->ub = ub;

    return (CCHECK_Gen*) myGen;
}

struct Gen_DoubleRange {
    CCHECK_Gen gen;
    double lb;
    double ub;
};

void generateDoubleRange(double* sample, struct Gen_DoubleRange* ud, int size, SplitMix64* seed) {
    *sample = SplitMix64_nextDoubleRange(seed, ud->lb, ud->ub);
}

extern CCHECK_Gen* genDoubleRange(double lb, double ub) {
    struct Gen_DoubleRange* myGen = (struct Gen_DoubleRange*) malloc(sizeof(struct Gen_DoubleRange));
    myGen->gen.genType = &ffi_type_double;
    myGen->gen.generate = (void (*)(void* , void*, int , SplitMix64* ))
            generateDoubleRange;
    myGen->gen.show = (int (*)(FILE*, void*, void*))
            showDoubleImpl;
    myGen->gen.userData = (void*) myGen;
    myGen->lb = lb;
    myGen->ub = ub;

    return (CCHECK_Gen*) myGen;
}

size_t asize(size_t s, size_t a) {
    size_t sa = s & ~(a - 1);
    return s == sa ? sa : sa + a;
}

void generateArray(CCHECK_Array* sample, CCHECK_Gen* elementGen, int size, SplitMix64* seed) {
    sample->size = size;
    if (size > 0) {
        sample->elements = calloc(elementGen->genType->size, size);
        size_t elemSize = asize(elementGen->genType->size, elementGen->genType->alignment);
        for (size_t i = 0; i < size; i++) {
            elementGen->generate(sample->elements + (i * elemSize), elementGen->userData, size, seed);
        }
    } else {
        sample->elements = NULL;
    }
}

int showArray(FILE* stream, CCHECK_Gen* elementGen, CCHECK_Array* sample) {
    int n = 0;
    size_t elemSize = asize(elementGen->genType->size, elementGen->genType->alignment);
    n += fprintf(stream, "[");
    for (size_t i = 0; i < sample->size; i++) {
        n += fprintf(stream, "%s", (i > 0 ? ", " : ""));
        void* thisSample = sample->elements + (i*elemSize);
        n += elementGen->show(stream, elementGen->userData, thisSample);
    }
    n += fprintf(stream, "]");
    return n;
}

void disposeArray(CCHECK_Gen* elementGen, CCHECK_Array* sample) {
    size_t elemSize = asize(elementGen->genType->size, elementGen->genType->alignment);
    if (elementGen->dispose) {
        for (size_t i = 0; i < sample->size; i++) {
            void* thisSample = sample->elements + (i*elemSize);
            elementGen->dispose(elementGen->userData, thisSample);
        }
    }
    if (sample->elements != NULL) {
        free(sample->elements);
    }
}

extern CCHECK_Gen* genArray(CCHECK_Gen* elementGen) {
    struct CCHECK_Gen* myGen = (struct CCHECK_Gen*) malloc(sizeof(struct CCHECK_Gen));
    myGen->userData = elementGen;
    myGen->genType = &ffi_array_type;
    myGen->generate = (void (*)(void* , void*, int , SplitMix64* ))
        generateArray;
    myGen->show = (int (*)(FILE*, void*, void*))
        showArray;
    myGen->dispose = (void (*)(void*, void*))
        disposeArray;

    // ensure the size/alignment fields of the parent generator are set properly.
    if (elementGen->genType->size == 0 || elementGen->genType->alignment == 0) {
        ffi_cif cif;
        ffi_type * args[1] = { elementGen->genType };
        if(ffi_prep_cif(&cif, 0, 1, &ffi_type_void, args) != FFI_OK) {
            panic("ffi setup failed");
        }
    }
    return myGen;
}
