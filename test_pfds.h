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

struct testModule {
    struct catenableInstance *catenableInstance;
    struct propCatenableMethods *propCatenableMethods;
};

#endif
