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

#include "pfds/pfds-fingertree.h"
#include "pfds/pfds-object-intl.h"
#include "pfds/pfds-sequence.h"
#include "pfds/pfds-catenable.h"
#include "pfds/pfds-num.h"
#include "misc.h"

#include <stdbool.h>
#include <stdarg.h>

/** extract the "measure" of a desegnated type, which must be of the correct, catenable type.
 * \param x
 * \invariant give(return) borrow(x)
 */
typedef pfds_object* (*pfds_measure)(pfds_object*);

enum DigitTag : char {
    DIGIT1 = 1,
    DIGIT2 = 2,
    DIGIT3 = 3,
    DIGIT4 = 4,
};

enum NodeTag : char {
    NODE2 = 2,
    NODE3 = 3,
};

typedef struct Digit {
    pfds_object object;
    enum DigitTag tag;
    pfds_object* elements[];
} Digit;

typedef struct Node /* v a */ {
    pfds_object self;
    pfds_object /* v */ *measure;
    enum NodeTag tag;
    pfds_object* elements[3];
} Node;

enum FingerTreeTag : char {
    FINGERTREE_EMPTY = 0,
    FINGERTREE_SINGLE,
    FINGERTREE_DEEP,
};

struct FingerTreeDeep /* v a */ {
    pfds_object /* v */ *measure;
    struct Digit /* a */ *prefix;
    pfds_FingerTree* /* v (Node v a) */ m;
    struct Digit /* a */ *suffix;
};

struct pfds_FingerTree /* a */ {
    pfds_object self;
    enum FingerTreeTag tag;
    union {
        struct {} empty;
        pfds_object* single;
        struct FingerTreeDeep deep;
    };
    // union FingerTree_fields fields;
};


struct WrappedMeasure {
    pfds_catenablevtable* cat;
    pfds_measure mm;
};


#define ARRAYVIEW_3(n1, a1, n2, a2, n3, a3, ix) ( \
    ix < n1 ? a1[ix] : \
    ix < (n1+n2) ? a2[ix-n1] : \
    a3[ix-n1-n2] )


Digit* Digit_new(enum DigitTag sz, pfds_object* elements[]);
Digit* Digit_vnew(enum DigitTag sz, ...);

pfds_FingerTree* /* v a */ FingerTree_deep(
        pfds_catenablevtable* cat, pfds_measure mm,
        Digit* /* a */ pr,
        pfds_FingerTree* /* v (Node v a) */ m,
        Digit* /* a */ sf);



void Digit_destroy(Digit *self) {
    for(enum DigitTag i = 0; i < self->tag; i++) {
        pfds_release(self->elements[i]);
    }
}

pfds_object* Digit_front (Digit* self) {
    return self->elements[0];
}

bool Digit_popFront(pfds_object** head, Digit** tail, Digit* self) {
    bool result = self->tag != DIGIT1;
    if (head != NULL) {
        *head = self->elements[0];
        pfds_retain(*head);
    }
    if (tail != NULL) {
        if (result) {
            pfds_retain_array(self->tag - 1, self->elements + 1);
            *tail = Digit_new(self->tag - 1, self->elements + 1);
        } else {
            *tail = NULL;
        }
    }
    pfds_release(self);
    return result;
}

pfds_object* Digit_back (Digit* self) {
    return self->elements[self->tag - 1];
}

bool Digit_popBack(Digit** init, pfds_object** last, Digit* self) {
    bool result = self->tag != DIGIT1;
    if (last != NULL) {
        *last = self->elements[self->tag - 1];
        pfds_retain(*last);
    }
    if (init != NULL) {
        if (result) {
            pfds_retain_array(self->tag - 1, self->elements);
            *init = Digit_new(self->tag - 1, self->elements);
        } else {
            *init = NULL;
        }
    }
    pfds_release(self);
    return result;
}

pfds_object* Digit_reduceLeft(binop op, void* ud, pfds_object* x0, Digit* self) {
    for(enum DigitTag i = 0 ; i < self->tag ; i++) {
        pfds_retain(self->elements[i]);
        x0 = op(ud, x0, self->elements[i]);
    }
    pfds_release(self);
    return x0;
}

pfds_object* Digit_reduceRight(binop op, void* ud, Digit* self, pfds_object* x0) {
    for(enum DigitTag i = self->tag ; i> 0 ; i--) {
        pfds_retain(self->elements[i-1]);
        x0 = op(ud, self->elements[i-1], x0);
    }
    pfds_release(self);
    return x0;
}

pfds_sequencevtable Digit_sequence = {
    .front = (pfds_object* (*)(pfds_sequence*)) Digit_front,
    .popFront = (bool (*)(pfds_object**, pfds_sequence**, pfds_sequence*)) Digit_popFront,
    .back = (pfds_object* (*)(pfds_sequence*)) Digit_back,
    .popBack = (bool (*)(pfds_sequence**, pfds_object**, pfds_sequence*)) Digit_popBack,
    .reduceLeft = (pfds_object* (*)(binop, void*, pfds_object*, pfds_sequence*))
        Digit_reduceLeft,
    .reduceRight = (pfds_object* (*)(binop, void*, pfds_sequence*, pfds_object*))
        Digit_reduceRight,
};
const pfds_objectvtable Digit_vtable = {
    .destroy = (void (*)(pfds_object*)) Digit_destroy,
    .typename = "FingerTree_Digit",
    .sequence = &Digit_sequence,
};

/**
 * \param sz number of elements
 * \param elements
 * \invariant give(return) take(elements)
 */
Digit* Digit_new(enum DigitTag sz, pfds_object* elements[]) {
    assert(sz >= DIGIT1 && sz <= DIGIT4);
    Digit* self = (Digit*) pfds_object_new(sizeof(Digit) + sz * sizeof(pfds_object*), &Digit_vtable);
    self->tag = sz;
    for(enum DigitTag i = 0 ; i < sz ; i++) {
        self->elements[i] = elements[i];
    }
    return self;
}
Digit* Digit_vnew(enum DigitTag sz, ...) {
    assert(sz >= DIGIT1 && sz <= DIGIT4);
    Digit* self = (Digit*) pfds_object_new(sizeof(Digit) + sz * sizeof(pfds_object*), &Digit_vtable);
    self->tag = sz;

    va_list ap;
    va_start(ap, sz);

    for(enum DigitTag i = 0 ; i < sz ; i++) {
        self->elements[i] = va_arg(ap, pfds_object*);
    }
    va_end(ap);
    return self;
}

Digit* Digit_fromNode(Node* node) {
    pfds_object *e0, *e1, *e2;
    switch(node->tag) {
        case NODE2:
            e0 = node->elements[0];
            pfds_retain(e0);
            e1 = node->elements[1];
            pfds_retain(e1);
            pfds_release(node);
            return Digit_vnew(DIGIT2, e0, e1);
        case NODE3:
            e0 = node->elements[0];
            pfds_retain(e0);
            e1 = node->elements[1];
            pfds_retain(e1);
            e2 = node->elements[2];
            pfds_retain(e2);
            pfds_release(node);
            return Digit_vnew(DIGIT3, e0, e1, e2);
        default: panic("illegal Node tag");
    }
}

pfds_object* Digit_measure(pfds_catenablevtable* cat, pfds_measure mm, Digit* self) {
    pfds_object* x = mm(self->elements[0]);
    for(size_t i = 1; i < self->tag; i++) {
        x = cat->mappend(x, mm(self->elements[i]));
    }
    return x;
}

void Node_destroy(pfds_object* self_obj) {
    Node* self = (Node*) self_obj;
    pfds_release(self->measure);
    switch(self->tag) {
        case NODE3:
            pfds_release(self->elements[2]);
        case NODE2:
            pfds_release(self->elements[1]);
            pfds_release(self->elements[0]);
    }
}

const pfds_objectvtable Node_vtable = {
    .destroy = Node_destroy,
    .typename = "FingerTree_Node",
};


Node* Node_new2(pfds_catenablevtable* cat, pfds_measure mm, pfds_object* x, pfds_object* y) {
    Node* self = (Node*) pfds_object_new(sizeof(Node) + 2 * sizeof(pfds_object*), &Node_vtable);
    self->tag = NODE2;
    self->elements[0] = x;
    self->elements[1] = y;
    self->measure = cat->mappend(mm(x), mm(y));
    return self;
}

Node* Node_new3(pfds_catenablevtable* cat, pfds_measure mm, pfds_object* x, pfds_object* y, pfds_object* z) {
    Node* self = (Node*) pfds_object_new(sizeof(Node) + 3 * sizeof(pfds_object*), &Node_vtable);
    self->tag = NODE3;
    self->elements[0] = x;
    self->elements[1] = y;
    self->elements[2] = z;
    self->measure = cat->mappend(cat->mappend(mm(x), mm(y)), mm(z));
    return self;
}

pfds_object* Node_measure(pfds_object* self_obj) {
    Node* self = (Node*) self_obj;
    pfds_retain(self->measure);
    return self->measure;
}

pfds_object* measureTreeOfNodes(pfds_catenablevtable* cat, pfds_FingerTree* self) {
    switch(self->tag) {
        case FINGERTREE_EMPTY:
            return cat->mempty();
        case FINGERTREE_SINGLE:
            Node* single = (Node*) self->single;
            pfds_retain(single->measure);
            return single->measure;
        case FINGERTREE_DEEP:
            pfds_retain(self->deep.measure);
            return self->deep.measure;
        default:
            panic("illegal FingerTree tag field");
    }
}

/**
 *
 * \returns true if the digit was full and a new node was also created.
 *
 */
bool Digit_pushBack(Node** ys, Digit** zs, pfds_catenablevtable* cat, pfds_measure mm, Digit* xs, pfds_object* y) {
    pfds_retain_array(xs->tag, xs->elements);
    switch (xs->tag) {
        case DIGIT1:
            *zs = Digit_vnew(DIGIT2, xs->elements[0], y);
            pfds_release(xs);
            return false;
        case DIGIT2:
            *zs = Digit_vnew(DIGIT3, xs->elements[0], xs->elements[1], y);
            pfds_release(xs);
            return false;
        case DIGIT3:
            *zs = Digit_vnew(DIGIT4, xs->elements[0], xs->elements[1], xs->elements[2], y);
            pfds_release(xs);
            return false;
        case DIGIT4:
            *ys = Node_new3(cat, mm, xs->elements[0], xs->elements[1], xs->elements[2]);
            *zs = Digit_vnew(DIGIT2, xs->elements[3], y);
            pfds_release(xs);
            return true;
        default:
            panic("illegal Digit tag field");
    }
}

bool Digit_pushFront(Digit** zs, Node** ws, pfds_catenablevtable* cat, pfds_measure mm, pfds_object* x, Digit* ys) {
    pfds_retain_array(ys->tag, ys->elements);
    switch (ys->tag) {
        case DIGIT1:
            *zs = Digit_vnew(DIGIT2, x, ys->elements[0]);
            pfds_release(ys);
            return false;
        case DIGIT2:
            *zs = Digit_vnew(DIGIT3, x, ys->elements[0], ys->elements[1]);
            pfds_release(ys);
            return false;
        case DIGIT3:
            *zs = Digit_vnew(DIGIT4, x, ys->elements[0], ys->elements[1], ys->elements[2]);
            pfds_release(ys);
            return false;
        case DIGIT4:
            *zs = Digit_vnew(DIGIT2, x, ys->elements[0]);
            *ws = Node_new3(cat, mm, ys->elements[1], ys->elements[2], ys->elements[3]);
            pfds_release(ys);
            return true;
        default:
            panic("illegal digit tag field");
    }
}

void FingerTree_destroy(pfds_FingerTree* self) {
    switch(self->tag) {
        case FINGERTREE_EMPTY:
            return;
        case FINGERTREE_SINGLE:
            pfds_release(self->single);
            return;
        case FINGERTREE_DEEP:
            pfds_release(self->deep.measure);
            pfds_release(self->deep.prefix);
            pfds_release(self->deep.m);
            pfds_release(self->deep.suffix);
            return;
    }
}

pfds_objectvtable FingerTree_vtable = {
    .destroy = (void (*)(pfds_object*)) FingerTree_destroy,
    .typename = "FingerTree",
};

pfds_FingerTree* pfds_FingerTree_pushFront(
        pfds_catenablevtable* cat, pfds_measure mm,
        pfds_object* head,
        pfds_FingerTree *tail) {
    switch (tail->tag) {
        case FINGERTREE_EMPTY:
            pfds_release(tail);
            return pfds_FingerTree_singleton(head);
        case FINGERTREE_SINGLE:
            pfds_object* single = tail->single;
            pfds_retain(single);
            pfds_release(tail);
            return FingerTree_deep(cat, mm,
                Digit_vnew(DIGIT1, head),
                pfds_FingerTree_empty(),
                Digit_vnew(DIGIT1, single));
        case FINGERTREE_DEEP:
            Digit* prefix = tail->deep.prefix;
            pfds_retain(prefix);
            pfds_FingerTree* m = tail->deep.m;
            pfds_retain(m);
            Digit* suffix = tail->deep.suffix;
            pfds_retain(suffix);
            pfds_release(tail);
            // [head, prefix, m, suffix]
            Digit* ys;
            Node* mPrime;
            if (Digit_pushFront(&ys, &mPrime, cat, mm, head, prefix)) {
                // [ys, mPrime, m, suffix]
                return FingerTree_deep(
                        cat, mm,
                        ys,
                        pfds_FingerTree_pushFront(cat, Node_measure,
                            (pfds_object*) mPrime,
                            m),
                        suffix);
            } else {
                // [ys, m, suffix]
                return FingerTree_deep(cat, mm, ys, m, suffix);
            }
        default:
            panic("illegal FingerTree tag field");
    }
}

pfds_FingerTree* FingerTree_pushFrontArray(
        pfds_catenablevtable* cat, pfds_measure mm,
        size_t n, pfds_object* heads[],
        pfds_FingerTree *tail) {
    for (size_t i = 0; i < n; i++) {
        tail = pfds_FingerTree_pushFront(cat, mm, heads[n-i-1], tail);
    }
    return tail;
}


pfds_FingerTree* pfds_FingerTree_pushBack(
        pfds_catenablevtable* cat, pfds_measure mm,
        pfds_FingerTree* init,
        pfds_object* last) {
    // [init, last]
    switch(init->tag) {
        case FINGERTREE_EMPTY:
            pfds_release(init);
            return pfds_FingerTree_singleton(last);
        case FINGERTREE_SINGLE:
            pfds_object* single = init->single;
            pfds_retain(single);
            pfds_release(init);
            // [single, last]
            return FingerTree_deep(cat, mm,
                    Digit_vnew(DIGIT1, single),
                    pfds_FingerTree_empty(),
                    Digit_vnew(DIGIT1, last));
        case FINGERTREE_DEEP:
            Digit* prefix = init->deep.prefix;
            pfds_retain(prefix);
            pfds_FingerTree* m = init->deep.m;
            pfds_retain(m);
            Digit* suffix = init->deep.suffix;
            pfds_retain(suffix);
            pfds_release(init);
            // [prefix, m, suffix, last]

            Node* mPrime;
            Digit* xs;
            if (Digit_pushBack(&mPrime, &xs, cat, mm, suffix, last)) {
                // [mPrime, xs, prefix, m]
                return FingerTree_deep(cat, mm,
                        prefix,
                        pfds_FingerTree_pushBack(cat, Node_measure,
                            m,
                            (pfds_object*) mPrime),
                        xs);
            } else {
                // [xs, prefix, m]
                return FingerTree_deep(cat, mm, prefix, m, xs);
            }
        default:
            panic("illegal FingerTree tag field");
    }
}

pfds_FingerTree* pfds_FingerTree_pushBackArray(
        pfds_catenablevtable* cat, pfds_measure mm,
        pfds_FingerTree* init,
        size_t n, pfds_object* lasts[]) {
    for(size_t i = 0; i < n; i++) {
        init = pfds_FingerTree_pushBack(cat, mm, init, lasts[i]);
    }
    return init;
}

pfds_FingerTree* WrappedFingerTree_pushFront(struct WrappedMeasure *mm, pfds_object* head, pfds_FingerTree *tail) {
    return pfds_FingerTree_pushFront(mm->cat, mm->mm, head, tail);
}


pfds_FingerTree* FingerTree_pushFrontRange(
        pfds_catenablevtable* cat, pfds_measure mm,
        pfds_sequence* xs,
        pfds_FingerTree* y) {
    struct WrappedMeasure wrappedMm = {cat, mm};
    return (pfds_FingerTree*) pfds_sequence_reduceRight(
            (binop) WrappedFingerTree_pushFront, &wrappedMm,
            xs,
            (pfds_object*) y );
}

pfds_FingerTree* WrappedFingerTree_pushBack(struct WrappedMeasure *mm, pfds_FingerTree *init, pfds_object* last) {
    return pfds_FingerTree_pushBack(mm->cat, mm->mm, init, last);
}

pfds_FingerTree* FingerTree_pushBackRange(
        pfds_catenablevtable* cat, pfds_measure mm,
        pfds_FingerTree* x,
        pfds_sequence* ys) {
    struct WrappedMeasure wrappedMm = {cat, mm};
    return (pfds_FingerTree*) pfds_sequence_reduceLeft(
            (binop) WrappedFingerTree_pushBack, &wrappedMm,
            (pfds_object*) x,
            ys);
}

extern pfds_FingerTree* pfds_FingerTree_empty(void) {
    pfds_FingerTree* self = (pfds_FingerTree*) pfds_object_new(sizeof(pfds_FingerTree), &FingerTree_vtable);
    self->tag = FINGERTREE_EMPTY;
    return self;
}


pfds_FingerTree* pfds_FingerTree_singleton (pfds_object* head) {
    pfds_FingerTree* self = (pfds_FingerTree*) pfds_object_new(sizeof(pfds_FingerTree), &FingerTree_vtable);
    self->tag = FINGERTREE_SINGLE;
    self->single = head;
    return self;
}

pfds_FingerTree* FingerTree_fromArray (
        pfds_catenablevtable* cat, pfds_measure mm,
        size_t n, pfds_object* heads[]) {
        return FingerTree_pushFrontArray(cat, mm, n, heads, pfds_FingerTree_empty());
}

/*
 *
 * ```haskell
 * deep :: Measured a v -> Digit a -> FingerTree v (Node v a) -> Digit a -> NEFingerTree v a
 * deep mm pr m sf = Deep
 *   (append mm (append mm
 *     (abs (measureDigit mm) pr)
 *     (abs (measureNodeTree mm) m))
 *     (abs (measureDigit mm) sf)
 *   ) pr m sf
 * ```
 */
pfds_FingerTree* /* v a */ FingerTree_deep(
        pfds_catenablevtable* cat, pfds_measure mm,
        Digit* /* a */ pr,
        pfds_FingerTree* /* v (Node v a) */ m,
        Digit* /* a */ sf) {
    pfds_FingerTree* self = (pfds_FingerTree*) pfds_object_new(sizeof(pfds_FingerTree), &FingerTree_vtable);
    self->tag = FINGERTREE_DEEP;
    self->deep.measure =
        cat->mappend(
            cat->mappend(
                Digit_measure(cat, mm, pr),
                measureTreeOfNodes(cat, m)
            ),
            Digit_measure(cat, mm, sf)
        );
    self->deep.prefix = pr;
    self->deep.m = m;
    self->deep.suffix = sf;
    return self;
}

pfds_object* pfds_FingerTree_measure(pfds_catenablevtable* cat, pfds_measure mm, pfds_FingerTree* self) {
    switch (self->tag) {
        case FINGERTREE_EMPTY:
            return cat->mempty();
        case FINGERTREE_SINGLE:
            return mm(self->single);
        case FINGERTREE_DEEP:
            pfds_retain(self->deep.measure);
            return self->deep.measure;
        default:
            panic("invalid tag in FingerTree_measure");
    }
}

// read the measure from a non-root FingerTree; who's elements are nodes.
pfds_object* FingerTree_Node_measure(pfds_catenablevtable* cat, pfds_FingerTree* self) {
    switch (self->tag) {
        case FINGERTREE_EMPTY:
            return cat->mempty();
        case FINGERTREE_SINGLE:
            Node* s = (Node*) self->single;
            pfds_retain(s->measure);
            return s->measure;
        case FINGERTREE_DEEP:
            pfds_retain(self->deep.measure);
            return self->deep.measure;
        default:
            panic("invalid tag in FingerTree_measure");
    }
}


bool FingerTree_popFront(
        pfds_object** head, pfds_FingerTree** tail,
        pfds_catenablevtable* cat, pfds_measure mm,
        pfds_FingerTree* self);

pfds_FingerTree* FingerTree_deepL(
        pfds_catenablevtable* cat, pfds_measure mm,
        Digit* /* Maybe (Digit a) */ prefix,
        pfds_FingerTree* /* Node v a */ self,
        Digit* /* a */ suffix);

bool FingerTree_popBack(
        pfds_FingerTree** init, pfds_object** last,
        pfds_catenablevtable* cat, pfds_measure mm,
        pfds_FingerTree* self);

pfds_FingerTree* FingerTree_deepR(
        pfds_catenablevtable* cat, pfds_measure mm,
        Digit* /* a */ prefix,
        pfds_FingerTree* /* Node v a */ self,
        Digit* /* Maybe (Digit a) */ suffix);

bool FingerTree_popFront(
        pfds_object** head, pfds_FingerTree** tail,
        pfds_catenablevtable* cat, pfds_measure mm,
        pfds_FingerTree* self) {
    switch (self->tag) {
        case FINGERTREE_EMPTY:
            pfds_release(self);
            OPTIONAL_OUTPARAM(head, NULL);
            OPTIONAL_OUTPARAM(tail, NULL);
            return false;
        case FINGERTREE_SINGLE:
            pfds_object* single = self->single;
            pfds_retain(single);
            pfds_release(self);
            OPTIONAL_OUTPARAM2(head, single, pfds_release(single));
            OPTIONAL_OUTPARAM(tail, pfds_FingerTree_empty())
            return true;
        case FINGERTREE_DEEP:
            Digit* prefix = self->deep.prefix;
            pfds_retain(prefix);
            pfds_FingerTree* m = self->deep.m;
            pfds_retain(m);
            Digit* suffix = self->deep.suffix;
            pfds_retain(suffix);
            pfds_release(self);

            Digit* prefixTail = NULL;
            Digit_popFront(head, &prefixTail, prefix);
            OPTIONAL_OUTPARAM2(tail,
                    FingerTree_deepL(cat, mm, prefixTail, m, suffix),
                    (pfds_release(prefixTail), pfds_release(m), pfds_release(suffix)));
            return true;
        default:
            panic("illegal FingerTree tag");
    }
}



pfds_FingerTree* FingerTree_deepL(
        pfds_catenablevtable* cat, pfds_measure mm,
        Digit* /* Maybe (Digit a) */ prefix,
        pfds_FingerTree* /* Node v a */ m,
        Digit* /* a */ suffix) {
    // [prefix, m, suffix]
    if (prefix == NULL) {
        // [m, suffix] (assumption)
        pfds_object* a;
        pfds_FingerTree* mPrime;
        // [m, m, suffix] (assumption)
        if(FingerTree_popFront(&a, &mPrime, cat, Node_measure, m)) {
            // [a, mPrime, suffix]
            return FingerTree_deep(cat, mm, Digit_fromNode((Node*) a), mPrime, suffix);
            // [return]
        } else {
            // [suffix]
            // if popFront failed, m was the empty tree.
            return FingerTree_pushFrontRange(cat, mm, (pfds_sequence*) suffix, pfds_FingerTree_empty());
            // [return]
        }
    } else {
        return FingerTree_deep(cat, mm, prefix, m, suffix);
    }
}

pfds_FingerTree* FingerTree_fromDigit(
        pfds_catenablevtable* cat, pfds_measure mm,
        Digit* self) {
    if (self == NULL) {
        // paper returns lists from splitDigit.  In C i'll simply tolerate nulls.
        return pfds_FingerTree_empty();
    } else {
        return FingerTree_deepL(cat, mm, NULL, pfds_FingerTree_empty(), self);
    }
};

bool FingerTree_popBack(
        pfds_FingerTree** init, pfds_object** last,
        pfds_catenablevtable* cat, pfds_measure mm,
        pfds_FingerTree* self) {
    switch (self->tag) {
        case FINGERTREE_EMPTY:
            *init = NULL;
            *last = NULL;
            pfds_release(self);
            return false;
        case FINGERTREE_SINGLE:
            *init = pfds_FingerTree_empty();
            *last = self->single;
            pfds_retain(self->single);
            pfds_release(self);
            return true;
        case FINGERTREE_DEEP:
            Digit* suffixInit = NULL;
            Digit* prefix = self->deep.prefix;
            pfds_retain(prefix);
            pfds_FingerTree* m = self->deep.m;
            pfds_retain(m);
            Digit* suffix = self->deep.suffix;
            pfds_retain(suffix);
            pfds_release(self);
            Digit_popBack(&suffixInit, last, suffix);
            *init = FingerTree_deepR(cat, mm, prefix, m, suffixInit);
            return true;
        default:
            panic("illegal FingerTree tag");
    }
}


pfds_FingerTree* FingerTree_deepR(
        pfds_catenablevtable* cat, pfds_measure mm,
        Digit* /* a */ prefix,
        pfds_FingerTree* /* Node v a */ m,
        Digit* /* Maybe (Digit a) */ suffix) {
    // prefix, m, suffix
    if (suffix == NULL) {
        // prefix, m
        pfds_FingerTree* mPrime;
        pfds_object* a;
        if(FingerTree_popBack(&mPrime, &a, cat, Node_measure, m)) {
            // mPrime, a, prefix
            return FingerTree_deep(cat, mm, prefix, mPrime, Digit_fromNode((Node*) a));
        } else {
            // prefix
            return FingerTree_pushBackRange(cat, mm, pfds_FingerTree_empty(), (pfds_sequence*) prefix);
        }
    } else {
        return FingerTree_deep(cat, mm, prefix, m, suffix);
    }
}

#define NODEAPP3_VIEW(xs, n, ts, ys, idx) ARRAYVIEW_3(xs->tag, xs->elements, n, ts, ys->tag, ys->elements, idx)

pfds_FingerTree* pfds_FingerTree_app3(
        pfds_catenablevtable* cat, pfds_measure mm,
        pfds_FingerTree* xs,
        size_t n, pfds_object** ts,
        pfds_FingerTree* ys) {
    if(xs->tag == FINGERTREE_EMPTY) {
        pfds_release(xs);
        return FingerTree_pushFrontArray(cat, mm, n, ts, ys);
    } else if(ys->tag == FINGERTREE_EMPTY) {
        pfds_release(ys);
        return pfds_FingerTree_pushBackArray(cat, mm, xs, n, ts);
    } else if(xs->tag == FINGERTREE_SINGLE) {
        pfds_object* single = xs->single;
        pfds_retain(single);
        pfds_release(xs);
        if(n == 0) {
            return pfds_FingerTree_pushFront(cat, mm, single, ys);
        } else {
            return pfds_FingerTree_pushFront(cat, mm,
                    single,
                    FingerTree_pushFrontArray(cat, mm, n, ts, ys));
        }
    } else if(ys->tag == FINGERTREE_SINGLE) {
        pfds_object* single = ys->single;
        pfds_retain(single);
        pfds_release(ys);
        if(n == 0) {
            return pfds_FingerTree_pushBack(cat, mm, xs, single);
        } else {
            return pfds_FingerTree_pushBack(cat, mm,
                    pfds_FingerTree_pushBackArray(cat, mm, xs, n, ts),
                    single);
        }
    } else {

        Digit* xPrefix = xs->deep.prefix;
        pfds_FingerTree* xM = xs->deep.m;
        Digit* xSuffix = xs->deep.suffix;
        pfds_retain(xPrefix); pfds_retain(xM); pfds_retain(xSuffix);
        pfds_release(xs);

        Digit* yPrefix = ys->deep.prefix;
        pfds_FingerTree* yM = ys->deep.m;
        Digit* ySuffix = ys->deep.suffix;
        pfds_retain(yPrefix); pfds_retain(yM); pfds_retain(ySuffix);
        pfds_release(ys);

        // size_t numNodes = Node_app3(&nodes, cat, mm, xSuffix, n, ts, yPrefix);

        // ArrayList* size_t Node_app3(
        // pfds_catenablevtable* cat, pfds_measure mm,
        // Digit* xSuffix,
        // size_t n, pfds_object** ts,
        // Digit* yPrefix) {
        size_t numElts = xSuffix->tag + n + yPrefix->tag;
        size_t numNodes = (numElts + 2) / 3;
        Node** nodes = (Node**) alloca(sizeof(Node*) * numNodes);

        for(size_t i = 0, j = 0; i < numNodes; i++) {
            switch(numElts - j) {
                case 2:
                    nodes[i] = Node_new2(cat, mm,
                            NODEAPP3_VIEW(xSuffix, n, ts, yPrefix, j),
                            NODEAPP3_VIEW(xSuffix, n, ts, yPrefix, j+1));
                    pfds_retain((nodes)[i]->elements[0]);
                    pfds_retain((nodes)[i]->elements[1]);
                    j += 2;
                    break;
                case 3:
                    nodes[i] = Node_new3(cat, mm,
                            NODEAPP3_VIEW(xSuffix, n, ts, yPrefix, j),
                            NODEAPP3_VIEW(xSuffix, n, ts, yPrefix, j+1),
                            NODEAPP3_VIEW(xSuffix, n, ts, yPrefix, j+2));
                    pfds_retain((nodes)[i]->elements[0]);
                    pfds_retain((nodes)[i]->elements[1]);
                    pfds_retain((nodes)[i]->elements[2]);
                    j += 3;
                    break;
                case 4:
                    nodes[i] = Node_new2(cat, mm,
                            NODEAPP3_VIEW(xSuffix, n, ts, yPrefix, j),
                            NODEAPP3_VIEW(xSuffix, n, ts, yPrefix, j+1));
                    pfds_retain((nodes)[i]->elements[0]);
                    pfds_retain((nodes)[i]->elements[1]);
                    i++;
                    nodes[i] = Node_new2(cat, mm,
                            NODEAPP3_VIEW(xSuffix, n, ts, yPrefix, j+2),
                            NODEAPP3_VIEW(xSuffix, n, ts, yPrefix, j+3));
                    pfds_retain((nodes)[i]->elements[0]);
                    pfds_retain((nodes)[i]->elements[1]);
                    j += 4;
                    break;
                default:
                    nodes[i] = Node_new3(cat, mm,
                            NODEAPP3_VIEW(xSuffix, n, ts, yPrefix, j),
                            NODEAPP3_VIEW(xSuffix, n, ts, yPrefix, j+1),
                            NODEAPP3_VIEW(xSuffix, n, ts, yPrefix, j+2));
                    pfds_retain((nodes)[i]->elements[0]);
                    pfds_retain((nodes)[i]->elements[1]);
                    pfds_retain((nodes)[i]->elements[2]);
                    j += 3;
                    break;
            }
        }
        pfds_release(xSuffix);
        pfds_release(yPrefix);

        return FingerTree_deep(cat, mm,
                xPrefix,
                pfds_FingerTree_app3(cat, Node_measure, xM, numNodes, (pfds_object**) nodes, yM),
                ySuffix);
    }
}


/** split a digit
 *
 * \param init[out] leading digit or null
 * \param pivot[out] matching element
 * \param tail[out] matching suffix
 * \param cat[in]
 * \param mm[in]
 * \param p[in]
 * \param ud[in]
 * \param i[in]
 * \param self[in]
 *
 * \invariant give(init) give(pivot) give(tail) take(i) take(self)
 *
 */
void Digit_split(Digit** init, pfds_object** pivot, Digit** tail,
        pfds_catenablevtable * cat, pfds_measure mm,
        bool (*p)(void*, pfds_object*), void* ud,
        pfds_object* i,
        Digit* self) {
    size_t lSize;
    for(lSize = 0; lSize < self->tag; lSize++) {
        i = cat->mappend(i, mm(self->elements[(size_t) lSize]));
        if (p(ud, i)) {
            pfds_release(i);

            if (init != NULL) {
                pfds_retain_array(lSize, self->elements);
                *init = lSize > 0
                    ? Digit_new(lSize, self->elements)
                    : NULL;
            }

            *pivot = self->elements[(size_t) lSize];
            pfds_retain(*pivot);

            if (tail != NULL) {
                pfds_retain_array(self->tag - lSize - 1, self->elements + 1 + lSize);
                *tail = lSize + 1 < self->tag
                    ? Digit_new(self->tag - lSize - 1, self->elements + 1 + lSize)
                    : NULL;
            }
            pfds_release(self);
            return;
        }
    }
    panic("invalid predicate in Digit_split");
}

/** divide a tree into three parts.
 *
 * \param init[out] non-matching prefix
 * \param pivot[out] first matching element
 * \param tail[out] matching suffix
 * \param cat[in] catenable instance
 * \param mm[in] measure function
 * \param p[in] monotone predicate
 * \param ud[in] first argument to p
 * \param i[in] prefix weight
 * \param self[in] tree to be split
 *
 * \invariant give(init) give(pivot) give(tail) take(i) take(self)
 *
 */
void FingerTree_splitTree(
        pfds_FingerTree** init,
        pfds_object** pivot,
        pfds_FingerTree** tail,
        pfds_catenablevtable* cat, pfds_measure mm,
        bool (*p)(void*, pfds_object*), void* ud,
        pfds_object* i,
        pfds_FingerTree* self
        ) {
    // [i, self]
    switch(self->tag) {
        case FINGERTREE_EMPTY:
            panic("invalid split, tree is empty");
        case FINGERTREE_SINGLE:
            pfds_object* single = self->single;
            pfds_retain(single);
            pfds_release(self);
            pfds_release(i);

            OPTIONAL_OUTPARAM(init, pfds_FingerTree_empty());
            *pivot = single;
            OPTIONAL_OUTPARAM(tail, pfds_FingerTree_empty());

            return;
        case FINGERTREE_DEEP:
            // [i, self]
            struct Digit *prefix = self->deep.prefix;
            pfds_retain(prefix);
            pfds_FingerTree* m = self->deep.m;
            pfds_retain(m);
            struct Digit *suffix = self->deep.suffix;
            pfds_retain(suffix);
            pfds_release(self);
            // UNPACK (prefix, m, suffix) <<< self 
            // [i, prefix, m, suffix]

            pfds_retain(i);
            // [i:2, prefix, m, suffix]

            pfds_object *vpr = cat->mappend(i, Digit_measure(cat, mm, prefix));
            // [vpr, i, prefix, m, suffix]
            if (p(ud, vpr)) {
                // [vpr, i, prefix, m, suffix]
                pfds_release(vpr);
                Digit* l;
                Digit* r;
                Digit_split(&l, pivot, &r, cat, mm, p, ud, i, prefix);
                // [l, r, m, suffix] (uses: i, prefix)
                if (init != NULL && l != NULL) {
                    *init = FingerTree_fromDigit(cat, mm, l);
                    // [r, m, suffix] (uses l)
                } else if (init != NULL && l == NULL) {
                    *init = pfds_FingerTree_empty();
                    // [r, m, suffix] (l is already invalid)
                } else if (init == NULL && l != NULL) {
                    pfds_release(l);
                    // [r, m, suffix]
                }
                OPTIONAL_OUTPARAM2(tail,
                        FingerTree_deepL(cat, mm, r, m, suffix),
                        (pfds_release(r) , pfds_release(m), pfds_release(suffix))
                        );
                // []
                return;
            } else {
                // [vpr, i, prefix, m, suffix]
                pfds_release(i);
                pfds_retain(vpr);
                pfds_object* vm = cat->mappend(vpr, FingerTree_Node_measure(cat, m));
                if(p(ud, vm)) {
                    // [vpr, vm, prefix, m, suffix]
                    pfds_retain(vpr);
                    pfds_release(vm);
                    // [vpr:2, prefix, m, suffix]

                    // we always need mInit because we need to compute the
                    // measure up to the pivot node
                    pfds_FingerTree* mInit;
                    Node* mPivot;
                    pfds_FingerTree* mTail;
                    pfds_FingerTree** mTailP = tail != NULL ? &mTail : NULL;
                    FingerTree_splitTree(
                            &mInit, (pfds_object**) &mPivot, mTailP,
                            cat, Node_measure,
                            p, ud, vpr, m);
                    // [mInit, mPivot, mTail, vpr, prefix, suffix] uses (vpr, m)
                    Digit* l;
                    Digit* r;
                    Digit** lp = init != NULL ? &l : NULL;
                    Digit** rp = tail != NULL ? &r : NULL;
                    Digit_split(lp, pivot, rp,
                            cat, mm, p, ud,
                            cat->mappend(vpr, FingerTree_Node_measure(cat, mInit)),
                            Digit_fromNode(mPivot));
                    // [l, r, mInit, mTail, prefix, suffix] uses (mPivot, vpr)
                    OPTIONAL_OUTPARAM2(init,
                            FingerTree_deepR(cat, mm, prefix, mInit, l),
                            (pfds_release(prefix), pfds_release(mInit))
                            );
                    // [r, mTail, suffix] uses (prefix, mInit, l)

                    OPTIONAL_OUTPARAM2(tail,
                            FingerTree_deepL(cat, mm, r, mTail, suffix),
                            (pfds_release(suffix))
                            );
                    // [] uses (r, mTail, suffix)
                    return;
                } else {
                    // [vpr, vm, prefix, m, suffix]
                    pfds_release(vpr);
                    // [vm, prefix, m, suffix]
                    Digit* l;
                    Digit* r;
                    Digit** lp = init != NULL ? &l : NULL;
                    Digit** rp = tail != NULL ? &r : NULL;
                    Digit_split(lp, pivot, rp,
                            cat, mm, p, ud,
                            vm,
                            suffix);
                    // [l, r, prefix, m] uses (vm, suffix)

                    OPTIONAL_OUTPARAM2(init,
                            FingerTree_deepR(cat, mm, prefix, m, l),
                            ( pfds_release(prefix), pfds_release(m)) );
                    // [r] uses (prefix, m, l)

                    OPTIONAL_OUTPARAM(tail,
                            FingerTree_fromDigit(cat, mm, r));
                    // [] uses (r)
                    return;
                }
            }
        default:
            panic("invalid tag in FingerTree_splitTree");
    }
}

bool pfds_FingerTree_split(
        pfds_FingerTree** init,
        pfds_object** pivot,
        pfds_FingerTree** tail,
        pfds_catenablevtable* cat, pfds_measure mm,
        bool (*p)(void*, pfds_object*), void* ud,
        pfds_FingerTree* self) {
    pfds_object* mmTree = pfds_FingerTree_measure(cat, mm, self);
    bool pmmTree = p(ud, mmTree);
    pfds_release(mmTree);
    if (pmmTree) {
        switch(self->tag) {
            case FINGERTREE_EMPTY:
                panic("invalid FingerTree split, predicate matches empty tree");
                // i'm ambivalent about this case, it maybe should be an error, or maybe its coherent to somebody....
                // *init = NULL;
                // *pivot = NULL;
                // *tail = self;
                // return false;
            case FINGERTREE_SINGLE:
            case FINGERTREE_DEEP:
                FingerTree_splitTree(init, pivot, tail, cat, mm, p, ud, cat->mempty(), self);
                return true;
            default:
                panic("invalid tag in FingerTree_split");
        }
    } else {
        OPTIONAL_OUTPARAM2(init, self, pfds_release(self));
        OPTIONAL_OUTPARAM(pivot, NULL);
        OPTIONAL_OUTPARAM(tail, NULL);
        return false;
    }
}

int Node_debugfputs(FILE* stream, Node* self, size_t depth) {
    int n = fprintf(stream, "N%d[<m:", self->tag);
    n += pfds_debugfputs(stream, self->measure);
    fputs(">", stream); n++;
    for(int i = 0 ; i < self->tag ; i++) {
        fputs(", ", stream); n += 2;
        if (depth <= 1) {
            n += pfds_debugfputs(stream, self->elements[i]);
        } else {
            n += Node_debugfputs(stream, (Node*) self->elements[i], depth - 1);
        }
    }
    fputs("]", stream);
    n++;
    return n;
}

int Digit_debugfputs(FILE* stream, Digit* self, size_t depth) {
    int n = fprintf(stream, "D%d[", self->tag);
    for(int i = 0 ; i < self->tag ; i++) {
        if (i > 0) {
            n += fprintf(stream, ", ");
        }
        if (depth > 0) {
            n += Node_debugfputs(stream, (Node*) self->elements[i], depth);
        } else {
            n += pfds_debugfputs(stream, self->elements[i]);
        }
    }
    fputs("]", stream);
    n ++;
    return n;
}

int pfds_FingerTree_debugfputs(FILE* stream, pfds_FingerTree* self, size_t depth) {
    int n;
    switch (self->tag) {
        case FINGERTREE_EMPTY:
            return fprintf(stream, "EMPTY");
        case FINGERTREE_SINGLE:
            n = fprintf(stream, "SINGLE(");
            if (depth == 0) {
                n += pfds_debugfputs(stream, self->single);
            } else {
                n += Node_debugfputs(stream, (Node*) self->single, depth);
            }
            n += fprintf(stream, ")");
            return n;
        case FINGERTREE_DEEP:
            n = fprintf(stream, "DEEP(<m:");
            n += pfds_debugfputs(stream, self->deep.measure);
            n += fprintf(stream, ">, ");
            n += Digit_debugfputs(stream, self->deep.prefix, depth);
            n += fprintf(stream, ", ");
            n += pfds_FingerTree_debugfputs(stream, self->deep.m, depth + 1);
            n += fprintf(stream, ", ");
            n += Digit_debugfputs(stream, self->deep.suffix, depth);
            n += fprintf(stream, ")");
            return n;
        default:
            panic("invalid fingertree tag");
    }
}

extern bool pfds_FingerTree_isEmpty(pfds_FingerTree* self) {
    return (self->tag == FINGERTREE_EMPTY);
}
