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

#ifndef TESTPFDS_HEADER_INCLUDED
#define TESTPFDS_HEADER_INCLUDED

#include "pfds.h"
#include "ccheck.h"

#define ASSERT_PFDS_STRING_EQUALS(actual, expected) \
    { pfds_String* assertPfdsStringEquals_toString_##__LINE__ = pfds_toString(actual) ; \
      CU_assertImplementation(!(strcmp((const char*)(pfds_String_toCstring( assertPfdsStringEquals_toString_##__LINE__) ), (const char*)(expected))), \
              __LINE__, \
              ("ASSERT_PFDS_STRING_EQUALS(" #actual ","  #expected ")"), \
              __FILE__, \
              "", CU_TRUE); \
        pfds_release( assertPfdsStringEquals_toString_##__LINE__ );}

#define ASSERT_PFDS_EQUALS(actual, expected) \
  { CU_assertImplementation( \
          (pfds_cmp(actual, expected) == PFDS_EQ), \
          __LINE__, \
          ("ASSERT_PFDS_EQUALS(" #actual ","  #expected ")"), \
          __FILE__, "", CU_FALSE);\
  }

#if defined (PFDS_GC_DEBUGREFCOUNT)
# define ASSERT_ONE_REF(self) { \
    ptrdiff_t assertOneRef_refcount##__LINE__ = 1 + ((pfds_object*) self)->retaincount - ((pfds_object*) self)->releasecount; \
      CU_assertImplementation( assertOneRef_refcount##__LINE__ == 1 , \
              __LINE__, \
              ("ASSERT_ONE_REF(" #self ")"), \
              __FILE__, \
              "", CU_FALSE); }
#elif defined (PFDS_GC_REFCOUNT)
# define ASSERT_ONE_REF(self) { \
    ptrdiff_t assertOneRef_refcount##__LINE__ = ((pfds_object*) self)->refcount; \
    CU_assertImplementation( assertOneRef_refcount##__LINE__ == 1 , \
            __LINE__, \
            ("ASSERT_ONE_REF(" #self ")"), \
            __FILE__, \
            "", CU_TRUE); }
#elif defined (PFDS_GC_NONE)
# define ASSERT_ONE_REF(self)
#endif

struct gc_counter { size_t totalRefs; size_t totalObjs; };

#if defined (PFDS_GC_DEBUGREFCOUNT) || defined (PFDS_GC_REFCOUNT)
# define PREPARE_GC_COUNTS(nm) struct pfds_gcinfo nm = pfds_getgcinfo() /* ; struct mallinfo2 nm##mi = mallinfo2() */
# define ASSERT_GC_COUNTS(nm, exRefs, exObjs) { \
    struct pfds_gcinfo nm##__LINE__ = pfds_getgcinfo(); \
    /* struct mallinfo2 nm##mi_end = mallinfo2(); \
     if (nm##mi_end.uordblks != nm##mi.uordblks) display_mallinfodelta(stderr, &nm##mi, &nm##mi_end); \
    */ size_t acRefs##__LINE__ = (nm##__LINE__.retaincount - nm.retaincount) - (nm##__LINE__.releasecount - nm.releasecount); \
    size_t acObjs##__LINE__ = (nm##__LINE__.births - nm.births) - (nm##__LINE__.deaths - nm.deaths); \
    CU_assertImplementation( acRefs##__LINE__ == exRefs && acObjs##__LINE__ == exObjs , \
            __LINE__, \
            ("ASSERT_GC_COUNTS(" #nm ", " #exRefs ", " #exObjs ")"), \
            __FILE__, \
            "", CU_FALSE); }
#elif defined (PFDS_GC_NONE)
# define PREPARE_GC_COUNTS(nm)
# define ASSERT_GC_COUNTS(nm, exRefs, exObjs)
#endif

struct catenableInstance {
    const char* name;
    const pfds_catenablevtable *dict;
    pfds_object*(*mkValue)(void*, int);
    void* ud;
};

struct propCatenableMethods {
    void (*testFn)(struct catenableInstance*);
    const char* desc;
};

struct testGC {
    const char* name;
    const char* typename;
    CU_TestFunc1 testFn;
    void* userData;
    struct testGC* nextTestGC;
};

struct testProperty {
    const char* name;
    const char* typename;
    unsigned int iterations;
    CCHECK_PROP propFn;
    void* propData;
    size_t nargs;
    const CCHECK_Gen ** gens;
    struct testProperty* nextProp;
};

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


void bench_setup(struct benchState *bs);
void bench_begin(struct benchState *bs, int iter);
void bench_end(struct benchState *bs, int iter);
void bench_teardown(struct benchState *bs);
void bench_done(struct benchState *bs);

typedef void (*benchFn)(struct benchState *, void*);

#define CLASSY_BENCHMARK(nm, dict, min, max, fn) { .name = nm, .typename = dict.typename, .minSize = min, .maxSize = max, .benchFn = (benchFn) fn, .benchData = (void*) &dict, }


struct testBenchmark {
    const char* name;
    const char* typename;
    unsigned int minSize;
    unsigned int maxSize;
    benchFn benchFn;
    void* benchData;
    struct testBenchmark* nextBenchmark;
};


struct testModule {
    // test category
    const char* testModule;

    // tests that verify correct/confignrming refcount behavior
    struct testGC *testGC, *lastTestGC;

    // property tests.
    struct testProperty *testProperties, *lastProp;
    struct testBenchmark *testBenchmarks, *lastBenchmark;
};

/* register a ccheck property.
 *
 * pfds types and ccheck interact in a way that means that the refcount of arguments must be 1 at function exit.
 *
 * @param m
 * @param nm
 * @param dict
 * @param iterations
 * @param propFn
 * @param gens null terminated list of CCHECK_Gen generators.  any generator
 *  may be used, not just generators of pfds types.  number fo generators must
 *  match the number of extra arguments taken by propFn.
 *
 */
void classyProp(
        struct testModule* m,
        const char* nm,
        const pfds_objectvtable *dict,
        int iterations,
        int (*propFn)(const pfds_objectvtable*, ...),
        const CCHECK_Gen ** gens);

void classyBenchmark(
        struct testModule* m,
        const char* nm,
        const pfds_objectvtable *dict,
        int min,
        int max,
        void (*fn)(struct benchState*, const pfds_objectvtable*));

void classyGCTest(
        struct testModule* m,
        const char* nm,
        void (*fn)( const pfds_objectvtable*),
        const pfds_objectvtable *dict);


// struct testModule newTestModule(const char* moduleName);

struct commonSuites {
    CU_pSuite validatorSuite;
    CU_pSuite gcSuite;
    CU_pSuite propSuite;
    CU_pSuite miscSuite;
    CU_pSuite benchSuite;
};

struct testModule getTestSequenceModule ();

pfds_String * test_binop(void* ud, pfds_object* l, pfds_object * r);

pfds_sequence* mkTestValue_sequence(const pfds_sequencevtable* vtable, int seed);
pfds_String* mkTestValue_String(void* ud, int seed);
pfds_Double* mkTestValue_Double(void* ud, int seed);
pfds_UInt64* mkTestValue_UInt64(void* ud, int seed);


int pfds_defaultGenShow(FILE* stream, void* userData, void* sample);
void pfds_defaultGenDispose(void* userData, void* sample);

#endif
