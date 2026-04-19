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

/** @mainpage libpfds: Persistent Functional Data Structures in C
 *
 * the types in this library follow a particular approach to reference counting
 * garbage collection. in particular, any function that is defined to return
 * the same type as its arguments, will take ownership of those arguments,
 * possibly causing them to be garbage collected earlier than expected.  to
 * prevent this from happening, call pfds_retain on any intermediate values you
 * need to keep.
 *
 * \section Garbage Collection
 *
 * this library makes use of a simple reference counting garbage collection system.
 *
 * most functions in this library will adjust reference counts in a sensible
 * way, taking ownership of arguments when constructing new values out of them,
 * or not if examining them.  the exact details are noted in each function's
 * documentatin under invariants.
 *
 * - `dup(x)` increase ref count by one, allowing one more reference.
 * - `drop(x)` decrease ref count by one, signaling the end of a particular reference.
 * - `borrow(x)` hold a ref without adjusting refcount.  the reference must
 *     stay alive for the duration of the function call.
 * - `lend(x, y)` emit a temporary reference to x who's lifetime is determined by y
 * - `take(x)` consume a reference.  caller must retain the reference
 *      themselves to keep it valid after the function returns.
 * - `give(x)` produce a possibly new value referenced by x.
 *
 */
#ifndef PFDS_HEADER_INCLUDED
#define PFDS_HEADER_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PFDS_INTERNAL
#define panic(msg) { fprintf(stderr, "PANIC %s:%d\n\t%s\n", __FILE__, __LINE__, msg); abort(); }
#endif

typedef struct pfds_object pfds_object;

// #define PFDS_GC_DEBUGREFCOUNT
// #define PFDS_GC_NONE
#if ! (defined (PFDS_GC_DEBUGREFCOUNT) || defined (PFDS_GC_NONE) || defined (PFDS_GC_REFCOUNT))
# define PFDS_GC_REFCOUNT
#endif

#if defined (PFDS_GC_DEBUGREFCOUNT)
#elif defined (PFDS_GC_REFCOUNT)
#elif defined (PFDS_GC_NONE)
#endif

#if defined (PFDS_GC_DEBUGREFCOUNT)
# define PFDS_GC_HEADER size_t retaincount; size_t releasecount; bool finalized;

/** increase reference count of self by 1
 * \public \memberof pfds_object
 *
 * \param self
 * \invariant dup(self)
 */
# define pfds_retain(self) pfds_object_retain((pfds_object*) self, __FILE__, __LINE__)
/** increase reference count of self by 1
 * \public \memberof pfds_object
 *
 * \param self
 * \invariant drop(self)
 */
# define pfds_release(self) pfds_object_release((pfds_object*) self, __FILE__, __LINE__)

/** increase reference count of self by 1
 * \public \memberof pfds_object
 *
 * \param self
 * \param fn file name
 * \param ln line number
 * \invariant dup(self)
 */
void pfds_object_retain(pfds_object* self, char* fn, int ln);

/** decrese reference count of self by 1.  object will be freed if refcount is zero
 * \public \memberof pfds_object
 *
 * \param self
 * \param fn file name
 * \param ln line number
 * \invariant drop(self)
 */
void pfds_object_release(pfds_object* self, char* fn, int ln);


#elif defined (PFDS_GC_REFCOUNT)
# define PFDS_GC_HEADER int refcount;
# define pfds_retain(self) pfds_object_retain((pfds_object*) self)
# define pfds_release(self) pfds_object_release((pfds_object*) self)

/** increase reference count of self by 1
 * \public \memberof pfds_object
 *
 * \param self
 * \invariant dup(self)
 */
void pfds_object_retain(pfds_object* self);

/** increase reference count of self by 1
 * \public \memberof pfds_object
 *
 * \param self
 * \invariant drop(self)
 */
void pfds_object_release(pfds_object* self);

#elif defined (PFDS_GC_NONE)
# define PFDS_GC_HEADER
# define pfds_retain(self)
# define pfds_release(self)

#endif


typedef struct pfds_orderedvtable pfds_orderedvtable;
typedef struct pfds_objectvtable pfds_objectvtable;
typedef struct pfds_catenablevtable pfds_catenablevtable;
typedef struct pfds_sequencevtable pfds_sequencevtable;

/** base "class" for most types in this library.
 *
 * fixed header for all values that are managed by the garbage collector.
 * this must always be the first field in any structure that uses it.
 *
 * \class pfds_object
 *
 * All containers require that the elements they contain have pfds_object as
 * first member.  in Normal use types based on object start with a reference
 * count of 1 and must be `retain` / `release`ed as needed, when references are
 * passed around.
 *
 */
struct pfds_object {
    PFDS_GC_HEADER
    const pfds_objectvtable* vtable;
};

/** assortment of general purpose overloadable methods for the types defined in this lib. 
 * all fields may be left null for reasonable default behavior
 */
struct pfds_objectvtable {
    const char* typename;
    /** finalizer for object.
     * \private \memberof pfds_object
     *
     * if not null, called by the garbage collector just before an object is freed
     * use this method to free resources and decrease refcounts of owned members
     *
     * \param self
     * \invariant invalid(self)
     *
     */
    const void (*destroy)(pfds_object*);

    /** convenience function to display an object
     * \private \memberof pfds_object
     *
     * \param stream File pointer open for writing.
     * \param self
     * \invariant borrow(self)
     *
     */
    const void (*debugfputs)(FILE*, pfds_object*);

    /** object is a catenable */
    const pfds_catenablevtable *catenable;
    const pfds_orderedvtable *ordering;
    const pfds_sequencevtable *sequence;
};

/** generic placeholder for a function with 2 arguments
 *
 * \param userData arbitrary user data to be passed to the function.
 * \param l
 * \param r
 * \returns result of the selected 2 argument closure.
 * \invariant give(return) take(l) take(r)
 *
 */
typedef pfds_object* (*binop)(void* userData, pfds_object* l, pfds_object* r);

#ifdef PFDS_INTERNAL
/** create a new uninitialised object
 *
 * \protected \memberof pfds_object
 * \param size size of object as returned by sizeof
 * \param vtable method dispatch table
 * \invariant give(return)
 *
 * */
pfds_object* pfds_object_new(size_t size, const pfds_objectvtable* vtable);
#endif


#define pfds_debugfputs(file, self) pfds_object_debugfputs(file, (pfds_object*) self)
#define pfds_debugputs(self) pfds_object_debugfputs(stdout, (pfds_object*) self)

/** convenience function to display an object to a file stream.
 *
 * \param stream File pointer open for writing.
 * \param self
 * \invariant borrow(self)
 *
 */
void pfds_object_debugfputs(FILE* stream, pfds_object* self);

/** convenience macro to print a pfds object to stderr */
#define pfds_logstderr(msg, obj) { \
    fprintf(stderr, "%s: \n\t", msg ); \
    pfds_debugfputs(stderr, obj); \
    fprintf(stderr, "\n"); \
}

typedef struct pfds_String
#ifdef PFDS_INTERNAL
{
    pfds_object object;
    bool owner;
    size_t size;
    const char* buf;
}
#endif
pfds_String;

/** Conveninece method to turn any object into its string representation.
 *
 * \param self
 * \invariant give(return) borrow(self)
 */
pfds_String* pfds_object_toString(pfds_object* self);
#define pfds_toString(self) pfds_object_toString((pfds_object*) self)

/** construct a new String from an existing buffer.  the buffer must be nul
 * terminated, size is as returned by strlen. if owner is true the buffer will
 * be freed when the string is destroyed
 */
pfds_String* pfds_String_new(const char* buf, size_t size, bool owner);

pfds_String* pfds_String_fromCstring(char* buf);
pfds_String* pfds_String_fromConstCstring(char* buf);

pfds_String* pfds_String_concat(size_t n, pfds_String* chunks[]);

/** format an object to a pfds_String.
 *
 * \param self
 * \invariant borrow(self)
 */
const char* pfds_String_toCstring(pfds_String* self);

typedef struct pfds_Double
#ifdef PFDS_INTERNAL
{
    pfds_object object;
    double value;
}
#endif
pfds_Double;

/** A boxed double
 *
 * \invariant give(return)
 */
pfds_Double* pfds_Double_new(double);
double pfds_Double_get(pfds_Double*);

typedef struct pfds_UInt64
#ifdef PFDS_INTERNAL
{
    pfds_object object;
    unsigned long value;
}
#endif
pfds_UInt64;

/** A boxed unsigned long
 *
 * \invariant give(return)
 */
pfds_UInt64* pfds_UInt64_new(unsigned long);
unsigned long pfds_UInt64_get(pfds_UInt64*);

extern pfds_catenablevtable pfds_catenable_sum;


/** the identity element of the group
 *
 * \invariant give(return)
 */
typedef pfds_object* (*pfds_catenablevtable_mempty)(void);

/** combine two elements of the group
 *
 * \param l
 * \param r
 * \invariant give(return) take(l) take (r)
 */
typedef pfds_object* (*pfds_catenablevtable_mappend)(pfds_object* l, pfds_object* r);

struct pfds_catenablevtable {
    /** the identity element of the group
     *
     * \invariant give(return)
     */
    pfds_object* (*mempty)(void);

    /** combine two elements of the group
     *
     * \param l
     * \param r
     * \invariant give(return) take(l) take (r)
     */
    pfds_object* (*mappend)(pfds_object* l, pfds_object* r);

    /** construct a member of the group concatenating an array of several
     * elements of a suitable type.
     *
     * \param n number of elements
     * \param xs
     * \invariant give(return) take(xs[0..n])
     *
     */
    pfds_object* (*concat)(size_t, pfds_object**);
};

/** combine two elements of the same type
 *
 * \param l
 * \param r
 * \invariant give(return) take(l) take (r)
 */
#define pfds_mappend(l, r) ((typeof(l))pfds_object_mappend((pfds_object*)l, (pfds_object*)r))

/** combine two elements of the same type
 *
 * \param l
 * \param r
 * \invariant give(return) take(l) take (r)
 */
pfds_object* pfds_object_mappend(pfds_object* l, pfds_object* r);

/** a simple, generally suboptimal implementation of a sequential container implemented as a simple array of pointers. */
typedef struct pfds_ArrayList
#ifdef PFDS_INTERNAL
{
    pfds_object object;
    size_t size;
    pfds_object **elements;
}
#endif
pfds_ArrayList;

/** return a new, empty ArrayList
 *
 * \invariant give(return)
 * */
pfds_ArrayList* pfds_ArrayList_empty();

/** return a new ArrayList with one element
 *
 * \param elem
 * \invariant give(return) take(elem)
 */
pfds_ArrayList* pfds_ArrayList_singleton(pfds_object* elem);

/** append one element to an ArrayList
 *
 * \param init
 * \param last
 * \invariant give(return) take(init) take(last)
 */
pfds_ArrayList* pfds_ArrayList_pushBack(pfds_ArrayList* init, pfds_object* last);


/** concatenate two ArrayLists
 *
 * \param l
 * \param r
 * \invariant give(return) take(l) take (r)
 */
pfds_ArrayList* pfds_ArrayList_mappend(pfds_ArrayList* l, pfds_ArrayList* r);

typedef struct pfds_ordered
#ifdef PFDS_INTERNAL
{
    pfds_object object;
}
#endif
pfds_ordered;

typedef enum pfds_ordering {
    PFDS_LT = -1,
    PFDS_EQ = 0,
    PFDS_GT = 1,
} pfds_ordering;


struct pfds_orderedvtable {
    /** compare two objects
     *
     * @param l
     * @param r
     * @return zero if both objects are the same, negative if the first is smaller and positive if the first is larger
     * \invariant borrow(l) borrow(r)
     */
    pfds_ordering (*cmp)(pfds_ordered*, pfds_ordered*);
};

/** compare two objects
 *
 * @param l
 * @param r
 * @return zero if both objects are the same, negative if the first is smaller and positive if the first is larger
 * \invariant borrow(l) borrow(r)
 */
#define pfds_cmp(l, r) pfds_ordered_cmp((pfds_ordered*) l, (pfds_ordered*) r)

/** compare two objects
 *
 * @param l
 * @param r
 * @return zero if both objects are the same, negative if the first is smaller and positive if the first is larger
 * \invariant borrow(l) borrow(r)
 */
pfds_ordering pfds_ordered_cmp(pfds_ordered* l, pfds_ordered* r);

typedef struct pfds_sequence
#ifdef PFDS_INTERNAL
{
    pfds_object object;
}
#endif
pfds_sequence;


typedef struct pfds_LinkedList pfds_LinkedList;

#ifdef PFDS_INTERNAL
struct pfds_LinkedList
{
    pfds_object object;
    size_t size;
    pfds_object* head;
    pfds_LinkedList* tail;
};
#endif

/** return a new, empty LinkedList
 *
 * \invariant give(return)
 */
pfds_LinkedList* pfds_LinkedList_empty();

/** construct a new linked list by prepending a given element to a given list.
 *
 * this is the conventional way of building up LinkedList.
 *
 * \param head
 * \param tail
 * \invariant give(return) take(head) take(tail)
 */
pfds_LinkedList* pfds_LinkedList_pushFront(pfds_object* head, pfds_LinkedList* tail);

struct pfds_sequencevtable {
    /** a sequence is required to be a monoid in a standard way. */
    const pfds_catenablevtable *catenable;

    /** construct a new sequence of the selected type from an array of the given objects.
     * the pfds_sequence_fromArray() global function returns the default implementation.
     *
     * \param n
     * \param xs
     * \invariant give(return) take(xs[0..n])
     */
    pfds_sequence* (*fromArray)(size_t, pfds_object**);

    /** test if a sequence has elements
     *
     * \param self
     * \return true if self is the empty sequence, false otherwise.
     * \invariant borrow(self)
     *
     */
    bool (*isEmpty)(pfds_sequence*);

    /** return the number of elementst in a sequence
     *
     * \param self
     * \return number of elements in self
     * \invariant borrow(self)
     */
    size_t (*size)(pfds_sequence*);

    /** split sequence into first element and remaing sequence. outputs NULL and returns false on empty sequence
     *
     * \param head[out] first element of a sequence
     * \param tail[out] remaining elements of a sequence
     * \param self[in] sequence to be examined.
     * \return true if there was an element to be popped.
     * \invariant when(return == true) give(head) give(tail) take(self)
     *      | when(return == false) borrow(self)
     */
    bool (*popFront)(pfds_object**, pfds_sequence**, pfds_sequence*);
    /** split sequence into intial sequence and last element.  outputs NULL and returns false on empty sequence
     *
     * \param init[out] remaining elements of a sequence
     * \param last[out] first element of a sequence
     * \param self[in] sequence to be examined.
     * \return true if there was an element to be popped.
     * \invariant when(return == true) give(init) give(last) take(self)
     *      | when(return == false) borrow(self)
     */
    bool (*popBack)(pfds_sequence**, pfds_object**, pfds_sequence*);

    /** split sequence into intial sequence, selected element, and remaining sequence.  outputs NULL and returns false if sequence does not contain selected element
     *
     * \param init[out] first n-1 elements
     * \param link[out] nth element
     * \param last[out] n+1 thru last elements
     * \param self[in] sequence to be examined.
     * \return true if sequence is large enough to contain nth element.
     * \invariant when(return == true) give(init) give(link) give(tail) take(self)
     *      | when(return == false) borrow(self)
     */
    bool (*split)(pfds_sequence**, pfds_object**, pfds_sequence**, pfds_sequence*, size_t);

    /** add an element to the front of a sequence.
     *
     * \param head
     * \param tail
     * \invariant give(return) take(head) take(tail)
     */
    pfds_sequence* (*pushFront)(pfds_object* head, pfds_sequence* tail);

    /** add an element to the back of a sequence.
     *
     * \param last
     * \param init
     * \invariant give(return) take(init) take(last)
     */
    pfds_sequence* (*pushBack)(pfds_sequence* init, pfds_object* last);

    /** return the nth element or NULL if the container does not contain enough elements
     *
     * \param self
     * \param n
     * \returns the nth element of self if it exists, NULL otherwise
     * \invariant when(return != NULL) lend(return, self)
     *
     */
    pfds_object* (*get)(pfds_sequence* self, size_t n);

    /** return the first element or NULL if the container is empty
     *
     * \param self
     * \returns the first element of self if it exists, NULL otherwise
     * \invariant when(return != NULL) lend(return, self)
     *
     */
    pfds_object* (*front)(pfds_sequence* self);

    /** return the last element or NULL if the container is empty
     *
     * \param self
     * \returns the last element of self if it exists, NULL otherwise
     * \invariant when(return != NULL) lend(return, self)
     *
     */
    pfds_object* (*back)(pfds_sequence* self);

    /** return a duplicate of the sequence with elements in reverse order
     *
     * \param self
     * \invariant give(return) take(self)
     */
    pfds_sequence* (*reverse)(pfds_sequence*);

    /** collect all elements of a sequence into a right associative running
     * total with user provided function.
     *
     * \param fn
     * \param ud
     * \param self
     * \param initial
     * \invariant give(return) take(self) take(initial)
     *
     * */
    pfds_object* (*reduceRight)(binop fn, void* ud, pfds_sequence* self, pfds_object* initial);

    /** collect all elements of a sequence into a left associative running
     * total with user provided function.
     *
     * \param fn
     * \param ud
     * \param initial
     * \param self
     * \invariant give(return) take(self) take(initial)
     *
     * */
    pfds_object* (*reduceLeft)(binop, void*, pfds_object*, pfds_sequence*);
};

/** convenient default implementation of isEmpty that uses size.
 *
 * \param self
 * \return true if self is the empty sequence, false otherwise.
 * \invariant borrow(self)
 *
 */
bool pfds_sequence_defaultIsEmpty(pfds_sequence* self);

/** convenient default implementation of popFront that uses split
 *
 * \param head[out] first element of a sequence
 * \param tail[out] remaining elements of a sequence
 * \param self[in] sequence to be examined.
 * \return true if there was an element to be popped.
 * \invariant when(return == true) give(head) give(tail) take(self)
 *      | when(return == false) borrow(self)
 */
bool pfds_sequence_defaultPopFront(pfds_object** head, pfds_sequence** tail, pfds_sequence* self);

/** convenient default implementation of popBack that uses split
 *
 * \param init[out] remaining elements of a sequence
 * \param last[out] first element of a sequence
 * \param self[in] sequence to be examined.
 * \return true if there was an element to be popped.
 * \invariant when(return == true) give(init) give(last) take(self)
 *      | when(return == false) borrow(self)
 */
bool pfds_sequence_defaultPopBack(pfds_sequence** init, pfds_object** last, pfds_sequence* self);

/** convenient default implementation of front that uses get
 *
 * \param self
 * \returns the first element of self if it exists, NULL otherwise
 * \invariant when(return != NULL) lend(return, self)
 *
 */
pfds_object* pfds_sequence_defaultFront(pfds_sequence*);

/** convenient default implementation of back that uses size and get
 *
 * \param self
 * \returns the last element of self if it exists, NULL otherwise
 * \invariant when(return != NULL) lend(return, self)
 *
 */
pfds_object* pfds_sequence_defaultBack(pfds_sequence*);


/** convenience default implementation of debugfputs based on popFront
 * \protected \memberof pfds_sequence
 *
 * \param stream File pointer open for writing.
 * \param self sequence.  must implement popFront
 * \invariant borrow(self)
 *
 */
void pfds_sequence_defaultDebugfputs (FILE*, pfds_sequence*);

/** convenience default implementation of reduceLeft based on popFront
 *
 * \param fn
 * \param ud
 * \param initial
 * \param self
 * \invariant give(return) take(self) take(initial)
 *
 */
pfds_object* pfds_sequence_defaultReduceLeft(binop xy2x, void* ud, pfds_object* x, pfds_sequence* self);

/** convenience default implementation of reduceRight based on popBack
 *
 * \param fn
 * \param ud
 * \param self
 * \param initial
 * \invariant give(return) take(self) take(initial)
 *
 */
pfds_object* pfds_sequence_defaultReduceRight(binop xy2x, void* ud, pfds_sequence* self, pfds_object* x);

/** convenience implementation of everse based on pushFront, popFront and empty
 *
 * \param self
 * \invariant give(return) take(self)
 */
pfds_sequence* pfds_sequence_defaultReverse(pfds_sequence*);

/** test if a sequence has elements
 *
 * \param self
 * \return true if self is the empty sequence, false otherwise.
 * \invariant borrow(self)
 *
 */
bool pfds_sequence_isEmpty (pfds_sequence* self);

/** \see pfds_sequence::pushFront
 */
pfds_sequence* pfds_sequence_pushFront (pfds_object* elem,pfds_sequence* self);
bool pfds_sequence_popFront (pfds_object** head, pfds_sequence** tail, pfds_sequence* self);
pfds_object* pfds_sequence_front(pfds_sequence* self);
pfds_object* pfds_sequence_back(pfds_sequence* self);
pfds_object* pfds_sequence_get(pfds_sequence* self, size_t idx);
size_t pfds_sequence_size(pfds_sequence* self);

pfds_sequence* pfds_sequence_pushBack(pfds_sequence* self, pfds_object* elem);

pfds_object* pfds_sequence_reduceRight(binop xy2y, void* ud, pfds_sequence* xs, pfds_object* y);
pfds_object* pfds_sequence_reduceLeft(binop xy2x, void* ud, pfds_object* x, pfds_sequence* ys);

pfds_sequence* pfds_sequence_reverse(pfds_sequence*);
bool pfds_sequence_split(pfds_sequence**, pfds_object**, pfds_sequence**, pfds_sequence*, size_t);

bool pfds_sequence_popBack(pfds_sequence**, pfds_object**, pfds_sequence*);


/** construct a new ArrayList from an array of the given objects.
 *
 * \param n
 * \param xs
 * \invariant give(return) take(xs[0..n])
 */
pfds_ArrayList* pfds_ArrayList_fromArray(size_t size, pfds_object** elements);

extern const pfds_objectvtable pfds_ArrayList_vtable;
extern const pfds_objectvtable pfds_LinkedList_vtable;
extern const pfds_objectvtable pfds_String_vtable;
extern const pfds_objectvtable pfds_TreeList_vtable;
extern const pfds_objectvtable pfds_UInt64_vtable;
extern const pfds_objectvtable pfds_Double_vtable;


extern pfds_String* pfds_String_empty(void);


/** combine two elements of the same type
 *
 * \param l
 * \param r
 * \invariant give(return) take(l) take (r)
 */
pfds_sequence* pfds_sequence_mappend(pfds_sequence* l, pfds_sequence* r);

#if defined (PFDS_GC_DEBUGREFCOUNT) || defined (PFDS_GC_REFCOUNT)
struct pfds_gcinfo {
    size_t births;
    size_t deaths;
    size_t retaincount;
    size_t releasecount;
};
struct pfds_gcinfo pfds_getgcinfo(void);
#endif

typedef struct pfds_TreeList pfds_TreeList;
pfds_TreeList* pfds_TreeList_fromArray(size_t n, pfds_object** xs);
void pfds_TreeList_debugfputs(FILE* stream, pfds_TreeList* self);

#endif
