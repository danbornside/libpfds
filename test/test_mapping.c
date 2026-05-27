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

#include "test_pfds.h"

#include "pfds.h"
#include "pfds/pfds-arraymap.h"

#include <stdlib.h>


void test_gc_mapping_empty(const pfds_objectvtable*);
void test_gc_mapping_isEmpty(const pfds_objectvtable*);
void test_gc_mapping_size(const pfds_objectvtable*);
void test_gc_mapping_singleton(const pfds_objectvtable*);

void test_gc_mapping_popMin(const pfds_objectvtable*);
void test_gc_mapping_popMinEmpty(const pfds_objectvtable*);

bool prop_mapping_empty_isEmpty(const pfds_objectvtable *vtable);
bool prop_mapping_singletonSize(const pfds_objectvtable *vtable, pfds_object_pair);
bool prop_mapping_singletonGet(const pfds_objectvtable *vtable, pfds_object_pair);

CCHECK_Gen* genObjectPair(const CCHECK_Gen* k, const CCHECK_Gen* v);
CCHECK_Gen* genArrayMap(const CCHECK_Gen* k, const CCHECK_Gen* v);

struct testModule getTestMappingModule () {
    struct testModule testSequenceModule = {
        .testModule = "mapping",
    };

    // benchmark values are chosen here to run very quickly in an ordinary run
    // with the default size multiplier of 1.0, but also all finish in max a
    // few seconds each with whatever options are specified in `make bench`.
    // pathologically bad benchmarks should only be allowed to run just long
    // enough to show that they are pathological in the benchmark graphs.
    //classyBenchmark(&testSequenceModule, "lookup",        &pfds_ArrayList_vtable,   10, 10000, bench_lookup);

    struct gcMappingMethods {
        void (*testFn)(const pfds_objectvtable*);
        const char* desc;
    } gcMappingMethods[] = {
        { .testFn = test_gc_mapping_empty, .desc = "empty", },
        { .testFn = test_gc_mapping_isEmpty, .desc = "isEmpty", },
        { .testFn = test_gc_mapping_size, .desc = "size", },
        { .testFn = test_gc_mapping_singleton, .desc = "singleton", },
        { .testFn = test_gc_mapping_popMin, .desc = "popMin", },
        { .testFn = test_gc_mapping_popMinEmpty, .desc = "popMin empty", },
        { 0 }
    };


    static CCHECK_Gen * gen_Item[2];
    gen_Item[0] = (CCHECK_Gen*) genObjectPair(&genBoxUInt64, &genBoxDouble);
    gen_Item[1] = 0;

    typedef bool (*propMappingFn)(const pfds_objectvtable*, ...);
    struct propMappingMethods {
        // void (*propFn)(const pfds_objectvtable*, ...);
        propMappingFn propFn;
        const char* desc;
        CCHECK_Gen ** gens;
    } propMappingMethods[] = {
        { .propFn = (propMappingFn) prop_mapping_empty_isEmpty, .desc = "isEmpty(empty)", .gens = 0 },
        { .propFn = (propMappingFn) prop_mapping_singletonSize, .desc = "prop_mapping_singletonSize",
            gen_Item },
        { .propFn = (propMappingFn) prop_mapping_singletonGet, .desc = "prop_mapping_singletonGet",
            gen_Item },
        { 0 },
    };

    struct mappingInstances {
        const pfds_objectvtable* vtable;
    } mappingInstances[] = {
        { &pfds_ArrayMap_vtable },
        { 0 }
    };

    for (size_t j = 0; mappingInstances[j].vtable ; j++) {
        for(size_t i = 0 ; propMappingMethods[i].propFn ; i++) {
            classyProp(&testSequenceModule,
                    propMappingMethods[i].desc,
                    mappingInstances[j].vtable,
                    100,
                    propMappingMethods[i].propFn,
                    (const CCHECK_Gen**) propMappingMethods[i].gens);
        }
    }
    for (size_t j = 0; mappingInstances[j].vtable ; j++) {
        for(size_t i = 0 ; gcMappingMethods[i].testFn ; i++) {
            classyGCTest(
                    &testSequenceModule,
                    gcMappingMethods[i].desc,
                    gcMappingMethods[i].testFn,
                    (void*) mappingInstances[j].vtable);
        }
    }


    return testSequenceModule;
}

void test_gc_mapping_empty(const pfds_objectvtable* dict) {
    pfds_mapping* xs = dict->mapping->empty();
    pfds_release(xs);
}

void test_gc_mapping_isEmpty(const pfds_objectvtable* dict) {
    pfds_mapping* xs = dict->mapping->empty();
    CU_ASSERT(pfds_mapping_isEmpty(xs));
    pfds_release(xs);
}

void test_gc_mapping_size(const pfds_objectvtable* dict) {
    pfds_mapping* xs = dict->mapping->empty();
    CU_ASSERT_EQUAL(pfds_mapping_size(xs), 0);
    pfds_release(xs);

}

void test_gc_mapping_singleton(const pfds_objectvtable* dict) {
    pfds_object *x = (pfds_object*) pfds_String_fromConstCstring("x");
    pfds_object *y = (pfds_object*) pfds_Double_new(5);
    pfds_object_pair item = {.key = x, .value = y};
    pfds_mapping* xs = dict->mapping->singleton(item);
    pfds_release(xs);
}

bool prop_mapping_empty_isEmpty(const pfds_objectvtable *vtable) {
    pfds_mapping* xs = vtable->mapping->empty();
    bool result = pfds_mapping_isEmpty(xs);
    pfds_release(xs);
    return result;

}

bool prop_mapping_singletonSize(const pfds_objectvtable *vtable, pfds_object_pair item) {
    pfds_retain(item.key);
    pfds_retain(item.value);
    pfds_mapping *xs = vtable->mapping->singleton(item);
    bool result = pfds_mapping_size(xs) == 1;
    pfds_release(xs);
    return result;
}
bool prop_mapping_singletonGet(const pfds_objectvtable *vtable, pfds_object_pair item) {
    pfds_retain(item.key);
    pfds_retain(item.value);
    pfds_mapping *xs = vtable->mapping->singleton(item);
    pfds_object *y = pfds_mapping_lookup(xs, item.key);
    bool result = y != NULL
        && pfds_cmp(item.value, y) == PFDS_EQ;
    pfds_release(xs);
    return result;
}


struct Gen_ObjectPair {
    CCHECK_Gen gen;
    const CCHECK_Gen* key;
    const CCHECK_Gen* value;
};

void generateObjectPair(pfds_object_pair* sample, struct Gen_ObjectPair* self, int size, SplitMix64* randGen) {
    self->key->generate(&sample->key, self->key->userData, size, randGen);
    self->value->generate(&sample->value, self->value->userData, size, randGen);
}

int showObjectPair(FILE* stream, struct Gen_ObjectPair* self, pfds_object_pair* sample) {
    int n = fprintf(stream, "{.key=");
    n += pfds_debugfputs(stream, sample->key);
    n += fprintf(stream, ", .value=");
    n += pfds_debugfputs(stream, sample->value);
    n += fprintf(stream, "}");

    return n;
}
void disposeObjectPair(struct Gen_ObjectPair* self, pfds_object_pair* sample) {
    pfds_release(sample->key);
    pfds_release(sample->value);
}

CCHECK_Gen* genObjectPair(const CCHECK_Gen* k, const CCHECK_Gen* v) {
    static ffi_type* ffi_objectpair_fields[] = {
        &ffi_type_pointer,
        &ffi_type_pointer,
        NULL,
    };
    static ffi_type ffi_objectpair_type = {
        .size = 0,
        .alignment = 0,
        .type = FFI_TYPE_STRUCT,
        .elements = ffi_objectpair_fields,
    };

    struct Gen_ObjectPair *gen = (struct Gen_ObjectPair*) malloc(sizeof(struct Gen_ObjectPair));
    gen->gen.userData = (void*) gen;
    gen->gen.genType = &ffi_objectpair_type;
    gen->gen.generate = (void (*)(void*, void*, int,  SplitMix64 *))
        generateObjectPair;
    gen->gen.show = (int (*)(FILE *, void*, void*))
        showObjectPair;
    gen->gen.dispose = (void (*)(void*, void*))
        disposeObjectPair;
    gen->key = k;
    gen->value = v;

    return (CCHECK_Gen*) gen;
}



void generateArrayMap(pfds_ArrayMap** sample, struct Gen_ObjectPair* self, int size, SplitMix64* randGen) {
    if (size <= 0) {
        *sample = pfds_ArrayMap_empty();
    } else {
        pfds_object_pair * items = (pfds_object_pair*) calloc(sizeof(pfds_object_pair), size);
        for (size_t i = 0 ; i < size ; i++) {
            generateObjectPair(&items[i], self, size, randGen);
        }
        *sample = pfds_ArrayMap_fromArray_ex(size, items, AMCF_OWNBUFFER);
    }
}

CCHECK_Gen* genArrayMap(const CCHECK_Gen* k, const CCHECK_Gen* v) {

    struct Gen_ObjectPair *gen = (struct Gen_ObjectPair*) malloc(sizeof(struct Gen_ObjectPair));
    gen->gen.userData = (void*) gen;
    gen->gen.genType = &ffi_type_pointer;
    gen->gen.generate = (void (*)(void*, void*, int,  SplitMix64 *))
        generateArrayMap;
    gen->gen.show = pfds_defaultGenShow;
    gen->gen.dispose = pfds_defaultGenDispose;
    gen->key = k;
    gen->value = v;

    return (CCHECK_Gen*) gen;
}

void test_gc_mapping_popMin(const pfds_objectvtable* dict) {
    pfds_object *x = (pfds_object*) pfds_String_fromConstCstring("x");
    pfds_object *y = (pfds_object*) pfds_Double_new(5);
    pfds_object_pair item = {.key = x, .value = y};
    pfds_mapping* xs = dict->mapping->singleton(item);
    // xs

    CU_ASSERT(pfds_mapping_popMin(&item, &xs, xs));
    // item.key item.value xs

    pfds_release(item.key);
    pfds_release(item.value);
    pfds_release(xs);
}
void test_gc_mapping_popMinEmpty(const pfds_objectvtable* dict) {
    pfds_object_pair item;
    pfds_mapping* xs = dict->mapping->empty();

    CU_ASSERT_FALSE(pfds_mapping_popMin(&item, &xs, xs));
}

