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
void test_gc_mapping_toString(const pfds_objectvtable*);
void test_gc_mapping_dfltToString(const pfds_objectvtable*);
void test_gc_mapping_cmp(const pfds_objectvtable*);
void test_gc_mapping_insertNew(const pfds_objectvtable*);
void test_gc_mapping_insertReplace(const pfds_objectvtable*);
void test_gc_mapping_insertPastEnd(const pfds_objectvtable*);
void test_gc_mapping_lookup(const pfds_objectvtable*);
void test_gc_mapping_lookupMiss(const pfds_objectvtable*);

bool prop_mapping_empty_isEmpty(const pfds_objectvtable *vtable);
bool prop_mapping_singletonSize(const pfds_objectvtable *vtable, pfds_object_pair);
bool prop_mapping_singletonGet(const pfds_objectvtable *vtable, pfds_object_pair);
bool prop_mapping_popMin(const pfds_objectvtable*, pfds_ArrayMap*);
bool prop_mapping_debugfputs(const pfds_objectvtable*, pfds_ArrayMap* xsArray);
bool prop_mapping_size(const pfds_objectvtable*, pfds_ArrayMap* xsArray);
bool prop_mapping_cmpSize(const pfds_objectvtable*, pfds_ArrayMap* xsArray, pfds_ArrayMap* ysArray);
bool prop_mapping_cmpKeys(const pfds_objectvtable*, pfds_ArrayMap* xsArray,
        pfds_object* k1, pfds_object* k2, pfds_object* val);
bool prop_mapping_cmpValues(const pfds_objectvtable*, pfds_ArrayMap* xsArray,
        pfds_object* k1, pfds_object* k2, pfds_object* v1, pfds_object* v2);
bool prop_mapping_lookup(const pfds_objectvtable *vtable, pfds_ArrayMap*, pfds_object* k);

const CCHECK_Gen* genObjectPair(const CCHECK_Gen* k, const CCHECK_Gen* v);
const CCHECK_Gen* genArrayMap(const CCHECK_Gen* k, const CCHECK_Gen* v);

const CCHECK_Gen** gens(size_t nargs, ...) {
    CCHECK_Gen** gens = (CCHECK_Gen**) calloc(sizeof(CCHECK_Gen*), nargs+1);

    va_list args;
    va_start(args, (size_t) nargs);
    for(int i = 0; i < nargs; ++i) {
        gens[i] = va_arg(args, struct CCHECK_Gen*);
    }
    va_end(args);
    gens[nargs] = NULL;
    return (const CCHECK_Gen**) gens;
}

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
        { .testFn = test_gc_mapping_toString, .desc = "toString", },
        { .testFn = test_gc_mapping_dfltToString, .desc = "defaultToString", },
        { .testFn = test_gc_mapping_cmp, .desc = "cmp", },
        { .testFn = test_gc_mapping_insertNew, .desc = "insertNew", },
        { .testFn = test_gc_mapping_insertReplace, .desc = "insertReplace", },
        { .testFn = test_gc_mapping_insertPastEnd, .desc = "insertPastEnd", },

        { .testFn = test_gc_mapping_lookup, .desc = "lookup" },
        { .testFn = test_gc_mapping_lookupMiss, .desc = "lookupMiss" },

        { 0 }
    };

    const CCHECK_Gen* genKey = &genBoxUInt64;
    // const CCHECK_Gen* genSzKey = genBoxUInt64With(genSize(&genSizedInt, clampBelow, (void*) 1));
    const CCHECK_Gen* genSzKey = genBoxUInt64With(&genSizedInt);
    const CCHECK_Gen* genValue = &genBoxDouble;

    const CCHECK_Gen* genItem = genObjectPair(genKey, genValue);
    const CCHECK_Gen* genMap = genArrayMap(genKey, genValue);

    // the idea is that generating with a size like this increases the
    // propability of colissions between parallel maps.
    const CCHECK_Gen* genSzMap = genArrayMap(genSzKey, genValue);

    const CCHECK_Gen** gen_Item = gens(1, genItem);
    const CCHECK_Gen** gen_Map = gens(1, genMap);

    // static CCHECK_Gen * gen_Item[2];
    // gen_Item[0] = (CCHECK_Gen*) ;
    // gen_Item[1] = 0;

    // static CCHECK_Gen * gen_Map[2];
    // gen_Map[0] = (CCHECK_Gen*) genArrayMap(&genBoxUInt64, &genBoxDouble);
    // gen_Map[1] = 0;

    // static CCHECK_Gen * gen_Map_Map[2];
    // static CCHECK_Gen * gen_Map[2];
    // static CCHECK_Gen * gen_Map[2];

    typedef bool (*propMappingFn)(const pfds_objectvtable*, ...);
    struct propMappingMethods {
        propMappingFn propFn;
        const char* desc;
        const CCHECK_Gen ** gens;
    } propMappingMethods[] = {
        { .propFn = (propMappingFn) prop_mapping_empty_isEmpty, .desc = "isEmpty(empty)", .gens = 0 },
        { .propFn = (propMappingFn) prop_mapping_singletonSize, .desc = "singletonSize",
            .gens = gen_Item },
        { .propFn = (propMappingFn) prop_mapping_singletonGet, .desc = "singletonGet",
            .gens = gen_Item },
        { .propFn = (propMappingFn)  prop_mapping_popMin, .desc = "popMin",
            .gens = gen_Map },
        { .propFn = (propMappingFn)  prop_mapping_debugfputs, .desc = "debugfputs",
            .gens = gen_Map },

        { .propFn = (propMappingFn) prop_mapping_size, .desc = "size", .gens = gen_Map },
        { .propFn = (propMappingFn) prop_mapping_cmpSize, .desc = "cmpSize",
            .gens = gens(2, genSzMap, genSzMap) },
        { .propFn = (propMappingFn) prop_mapping_cmpKeys, .desc = "cmpKeys",
            .gens = gens(4, genMap, genKey, genKey, genValue) },
        { .propFn = (propMappingFn) prop_mapping_cmpValues, .desc = "cmpValues",
            .gens = gens(5, genMap, genKey, genKey, genValue, genValue) },
        { .propFn = (propMappingFn) prop_mapping_lookup, .desc = "lookup", .gens = gens(2, genSzMap, genSzKey) },

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

void test_gc_mapping_empty(const pfds_objectvtable* vtable) {
    pfds_mapping* xs = vtable->mapping->empty();
    pfds_release(xs);
}

void test_gc_mapping_isEmpty(const pfds_objectvtable* vtable) {
    pfds_mapping* xs = vtable->mapping->empty();
    CU_ASSERT(pfds_mapping_isEmpty(xs));
    pfds_release(xs);
}

void test_gc_mapping_size(const pfds_objectvtable* vtable) {
    pfds_mapping* xs = vtable->mapping->empty();
    CU_ASSERT_EQUAL(pfds_mapping_size(xs), 0);
    pfds_release(xs);

}

void test_gc_mapping_singleton(const pfds_objectvtable* vtable) {
    pfds_mapping* xs = vtable->mapping->singleton(
            (pfds_object*) pfds_String_fromConstCstring("x"),
            (pfds_object*) pfds_Double_new(5));
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
    pfds_mapping *xs = vtable->mapping->singleton(item.key, item.value);
    bool result = pfds_mapping_size(xs) == 1;
    pfds_release(xs);
    return result;
}
bool prop_mapping_singletonGet(const pfds_objectvtable *vtable, pfds_object_pair item) {
    pfds_retain(item.key);
    pfds_retain(item.value);
    pfds_mapping *xs = vtable->mapping->singleton(item.key, item.value);
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

const CCHECK_Gen* genObjectPair(const CCHECK_Gen* k, const CCHECK_Gen* v) {
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

    return (const CCHECK_Gen*) gen;
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

const CCHECK_Gen* genArrayMap(const CCHECK_Gen* k, const CCHECK_Gen* v) {

    struct Gen_ObjectPair *gen = (struct Gen_ObjectPair*) malloc(sizeof(struct Gen_ObjectPair));
    gen->gen.userData = (void*) gen;
    gen->gen.genType = &ffi_type_pointer;
    gen->gen.generate = (void (*)(void*, void*, int,  SplitMix64 *))
        generateArrayMap;
    gen->gen.show = pfds_defaultGenShow;
    gen->gen.dispose = pfds_defaultGenDispose;
    gen->key = k;
    gen->value = v;

    return (const CCHECK_Gen*) gen;
}

void test_gc_mapping_popMin(const pfds_objectvtable* vtable) {
    pfds_mapping* xs = vtable->mapping->singleton(
            (pfds_object*) pfds_String_fromConstCstring("x"),
            (pfds_object*) pfds_Double_new(5));
    // xs

    pfds_object_pair item;
    CU_ASSERT(pfds_mapping_popMin(&item, &xs, xs));
    // item.key item.value xs

    pfds_release(item.key);
    pfds_release(item.value);
    pfds_release(xs);
}

void test_gc_mapping_popMinEmpty(const pfds_objectvtable* vtable) {
    pfds_object_pair item;
    pfds_mapping* xs = vtable->mapping->empty();

    CU_ASSERT_FALSE(pfds_mapping_popMin(&item, &xs, xs));
}

bool prop_mapping_debugfputs(const pfds_objectvtable* vtable, pfds_ArrayMap* xsArray) {
    pfds_retain(xsArray);
    pfds_mapping* xs = pfds_mapping_fromArrayMap(vtable, xsArray);

    char* buf;
    size_t size = 0;
    FILE* stream = open_memstream(&buf, &size);
    pfds_object_debugfputs(stream, (pfds_object*) xs);
    fclose(stream);

    char* buf2;
    size_t size2 = 0;
    FILE* stream2 = open_memstream(&buf2, &size2);
    pfds_mapping_defaultDebugfputs(stream2, xs);
    fclose(stream2);

    bool result = strcmp(buf, buf2);

    free(buf); free(buf2);
    pfds_release(xs);

    return result == 0;
}

void test_gc_mapping_toString(const pfds_objectvtable* vtable) {
    pfds_mapping* xs = vtable->mapping->singleton(
            (pfds_object*) pfds_String_fromConstCstring("x"),
            (pfds_object*) pfds_Double_new(5));
    pfds_String* xsStr = pfds_object_toString((pfds_object*) xs);
    pfds_release(xs);
    pfds_release(xsStr);
}

void test_gc_mapping_dfltToString(const pfds_objectvtable* vtable) {
    pfds_mapping* xs = vtable->mapping->singleton(
            (pfds_object*) pfds_String_fromConstCstring("x"),
            (pfds_object*) pfds_Double_new(5));

    char* buf2;
    size_t size2 = 0;
    FILE* stream2 = open_memstream(&buf2, &size2);
    pfds_mapping_defaultDebugfputs(stream2, xs);
    fclose(stream2);
    free(buf2);

    pfds_release(xs);
}

void test_gc_mapping_cmp(const pfds_objectvtable* vtable) {
    pfds_mapping* xs = vtable->mapping->singleton(
            (pfds_object*) pfds_String_fromConstCstring("x"),
            (pfds_object*) pfds_Double_new(5));
    pfds_mapping* zs = vtable->mapping->singleton(
            (pfds_object*) pfds_String_fromConstCstring("x"),
            (pfds_object*) pfds_Double_new(6));

    pfds_cmp(xs, zs);
    pfds_mapping_defaultCmp(xs, zs);
    pfds_release(xs);
    pfds_release(zs);
}

bool prop_mapping_popMin(const pfds_objectvtable* vtable, pfds_ArrayMap* xsArray) {
    pfds_retain(xsArray);
    pfds_mapping* xs = pfds_mapping_fromArrayMap(vtable, xsArray);

    pfds_object_pair currentItem;
    if(!pfds_mapping_popMin(&currentItem, &xs, xs)) {
        return true;
    }
    pfds_object_pair nextItem;
    while(pfds_mapping_popMin(&nextItem, &xs, xs)) {
        if (pfds_cmp(currentItem.key, nextItem.key) != PFDS_LT) {
            pfds_release(currentItem.key); pfds_release(currentItem.value);
            pfds_release(nextItem.key); pfds_release(nextItem.value);
            pfds_release(xs);
            return false;
        }
        pfds_release(currentItem.key); pfds_release(currentItem.value);
        currentItem = nextItem;
    }

    pfds_release(currentItem.key);
    pfds_release(currentItem.value);
    return true;

}

void test_gc_mapping_insertNew(const pfds_objectvtable* vtable) {

    pfds_mapping* xs = vtable->mapping->singleton(
            (pfds_object*) pfds_String_fromConstCstring("b"),
            (pfds_object*) pfds_Double_new(5));

    xs = pfds_mapping_insert(
            xs,
            (pfds_object*) pfds_String_fromConstCstring("a"),
            (pfds_object*) pfds_Double_new(6));

    pfds_release(xs);
}
void test_gc_mapping_insertReplace(const pfds_objectvtable* vtable) {

    pfds_mapping* xs = vtable->mapping->singleton(
            (pfds_object*) pfds_String_fromConstCstring("x"),
            (pfds_object*) pfds_Double_new(5));

    xs = pfds_mapping_insert(
            xs,
            (pfds_object*) pfds_String_fromConstCstring("x"),
            (pfds_object*) pfds_Double_new(6));

    pfds_release(xs);
}

void test_gc_mapping_insertPastEnd(const pfds_objectvtable* vtable) {

    pfds_mapping* xs = vtable->mapping->singleton(
            (pfds_object*) pfds_String_fromConstCstring("a"),
            (pfds_object*) pfds_Double_new(5));

    xs = pfds_mapping_insert(
            xs,
            (pfds_object*) pfds_String_fromConstCstring("b"),
            (pfds_object*) pfds_Double_new(5));

    pfds_release(xs);
}

bool prop_mapping_size(const pfds_objectvtable* vtable, pfds_ArrayMap* xsArray) {

    pfds_retain(xsArray);
    pfds_mapping* xs = pfds_mapping_fromArrayMap(vtable, xsArray);

    size_t size1 = pfds_mapping_size(xs);
    size_t size2 = 0;
    pfds_object_pair item;
    while(pfds_mapping_popMin(&item, &xs, xs)) {
        size2++;
        pfds_release(item.key);
        pfds_release(item.value);
    }

    return size1 == size2;
}

pfds_ordering int2ordering(int x) {
    return
        x < 0 ? PFDS_LT :
        x == 0 ? PFDS_EQ :
        PFDS_GT;
}

bool prop_mapping_cmpSize(const pfds_objectvtable* vtable, pfds_ArrayMap* xsArray, pfds_ArrayMap* ysArray) {

    pfds_retain(xsArray);
    pfds_mapping* xs = pfds_mapping_fromArrayMap(vtable, xsArray);
    pfds_retain(ysArray);
    pfds_mapping* ys = pfds_mapping_fromArrayMap(vtable, ysArray);

    int xSize = (int) pfds_mapping_size(xs);
    int ySize = (int) pfds_mapping_size(ys);
    pfds_ordering result = pfds_cmp(xs, ys);
    pfds_ordering resultDflt = pfds_mapping_defaultCmp(xs, ys);
    pfds_ordering resultSize = int2ordering(xSize - ySize);

    pfds_release(xs);
    pfds_release(ys);

    if (resultSize == PFDS_EQ) {
        // TODO ccheck neesds a prune option.
        return result == resultDflt;
    }
    return resultSize == result && result == resultDflt;
}

bool prop_mapping_cmpKeys(const pfds_objectvtable* vtable, pfds_ArrayMap* xsArray,
        pfds_object* k1, pfds_object* k2, pfds_object* val) {

    pfds_ordering kOrder = pfds_cmp(k1, k2);
    if (kOrder == PFDS_EQ) {
        // todo: return PRUNE;
        return true;
    }

    pfds_retain(xsArray);
    pfds_mapping* xs = pfds_mapping_fromArrayMap(vtable, xsArray);
    pfds_retain(k1); pfds_retain(k2); pfds_retain(val);
    xs = pfds_mapping_insert(pfds_mapping_erase(xs, k2), k1, val);

    pfds_retain(xsArray);
    pfds_mapping* ys = pfds_mapping_fromArrayMap(vtable, xsArray);
    pfds_retain(k1); pfds_retain(k2); pfds_retain(val);
    ys = pfds_mapping_insert(pfds_mapping_erase(ys, k1), k2, val);

    pfds_ordering mapOrder = pfds_cmp(xs, ys);
    pfds_release(xs);
    pfds_release(ys);

    return mapOrder == kOrder;
}

bool prop_mapping_cmpValues(const pfds_objectvtable* vtable, pfds_ArrayMap* xsArray,
        pfds_object* k1, pfds_object* k2, pfds_object* v1, pfds_object* v2) {

    pfds_ordering kOrder = pfds_cmp(k1, k2);
    pfds_ordering vOrder = pfds_cmp(v1, v2);
    if (kOrder == PFDS_EQ || vOrder == PFDS_EQ) {
        // todo: return PRUNE;
        return true;
    }

    pfds_retain(xsArray);
    pfds_mapping* xs = pfds_mapping_fromArrayMap(vtable, xsArray);
    pfds_retain(k1); pfds_retain(k2); pfds_retain(v1); pfds_retain(v2);
    xs = pfds_mapping_insert(pfds_mapping_insert(xs, k1, v1), k2, v2);

    pfds_retain(xsArray);
    pfds_mapping* ys = pfds_mapping_fromArrayMap(vtable, xsArray);
    pfds_retain(k1); pfds_retain(k2); pfds_retain(v1); pfds_retain(v2);
    ys = pfds_mapping_insert(pfds_mapping_insert(ys, k1, v2), k2, v1);

    pfds_ordering mapOrder = pfds_cmp(xs, ys);
    pfds_release(xs);
    pfds_release(ys);

    return mapOrder == (kOrder == PFDS_LT ? vOrder : -vOrder);
}

void test_gc_mapping_lookup(const pfds_objectvtable* vtable) {
    pfds_object* k1 = (pfds_object*) pfds_String_fromConstCstring("a");
    pfds_object* k2 = (pfds_object*) pfds_String_fromConstCstring("a");
    pfds_mapping* xs = vtable->mapping->singleton(k1, (pfds_object*) pfds_Double_new(5));

    CU_ASSERT(NULL != pfds_mapping_lookup(xs, k2));
    pfds_release(xs);
    pfds_release(k2);
}

void test_gc_mapping_lookupMiss(const pfds_objectvtable* vtable) {
    pfds_object* k1 = (pfds_object*) pfds_String_fromConstCstring("a");
    pfds_object* k2 = (pfds_object*) pfds_String_fromConstCstring("b");
    pfds_mapping* xs = vtable->mapping->singleton(k1, (pfds_object*) pfds_Double_new(5));

    CU_ASSERT_EQUAL(NULL, pfds_mapping_lookup(xs, k2));
    pfds_release(xs);
    pfds_release(k2);
}

bool prop_mapping_lookup(const pfds_objectvtable *vtable, pfds_ArrayMap* xsArray, pfds_object* k) {

    pfds_retain(xsArray);
    pfds_mapping* xs = pfds_mapping_fromArrayMap(vtable, xsArray);

    pfds_object* valArray = pfds_mapping_lookup((pfds_mapping*) xsArray, k);
    pfds_object* val = pfds_mapping_lookup(xs, k);

    if (valArray == NULL) {
        // TODO: ccheck histogram "miss"

        pfds_release(xs);
        return val == NULL;
    }

    // TODO: ccheck histogram "hit"
    bool result = val != NULL && pfds_cmp(valArray, val) == PFDS_EQ;
    pfds_release(xs);
    return result;

}
