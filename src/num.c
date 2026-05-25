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

#define PFDS_INTERNAL

#include "pfds.h"
#include "pfds/pfds-object-intl.h"
#include "pfds/pfds-num.h"

struct pfds_Double {
    pfds_object object;
    double value;
};

struct pfds_UInt64 {
    pfds_object object;
    unsigned long value;
};

pfds_ordering pfds_Double_cmp(pfds_Double *l, pfds_Double *r) {
    // TODO: NaNs are not handled at all.
    double ll = l->value;
    double rr = r->value;
    if (ll < rr) {
        return PFDS_LT;
    } else if (ll > rr) {
        return PFDS_GT;
    } else {
        return PFDS_EQ;
    }

}

int Double_debugfputs(FILE* stream, pfds_object* self_obj) {
    pfds_Double* self = (pfds_Double*) self_obj;
    return fprintf(stream, "%f", self->value);
}

const pfds_objectvtable pfds_Double_vtable = {
    .typename = "Double",
    .debugfputs = Double_debugfputs,
    .cmp = (pfds_ordering (*)(pfds_object*, pfds_object*))
        pfds_Double_cmp,
};

extern pfds_Double* pfds_Double_new(double value) {
    pfds_Double* self = (pfds_Double*) pfds_object_new(sizeof(pfds_Double), &pfds_Double_vtable);
    self->value = value;
    return self;
}
extern double pfds_Double_get(pfds_Double* self) {
    return self->value;
}

pfds_ordering pfds_UInt64_cmp(pfds_UInt64 *l, pfds_UInt64 *r) {
    unsigned long ll = l->value;
    unsigned long rr = r->value;
    // TODO: NaNs are not handled at all.
    if (ll < rr) {
        return PFDS_LT;
    } else if (ll > rr) {
        return PFDS_GT;
    } else {
        return PFDS_EQ;
    }

}

int UInt64_debugfputs(FILE* stream, pfds_object* self_obj) {
    pfds_UInt64* self = (pfds_UInt64*) self_obj;
    return fprintf(stream, "%lu", self->value);
}

const pfds_objectvtable pfds_UInt64_vtable = {
    .typename = "UInt64",
    .debugfputs = UInt64_debugfputs,
    .cmp = (pfds_ordering (*)(pfds_object*, pfds_object*))
        pfds_UInt64_cmp,
};

extern pfds_UInt64* pfds_UInt64_new(unsigned long value) {
    pfds_UInt64* self = (pfds_UInt64*) pfds_object_new(sizeof(pfds_UInt64), &pfds_UInt64_vtable);
    self->value = value;
    return self;
}
extern unsigned long pfds_UInt64_get(pfds_UInt64* self) {
    return self->value;
}

extern pfds_UInt64* pfds_UInt64_zero(void) {
    return pfds_UInt64_new(0);
}
extern pfds_UInt64* pfds_UInt64_add(pfds_UInt64* l, pfds_UInt64* r) {
    unsigned long lr = l->value + r->value;
    pfds_release(l);
    pfds_release(r);
    return pfds_UInt64_new(lr);
}
extern pfds_UInt64* pfds_UInt64_sum(size_t n, pfds_UInt64* xs[]) {
    unsigned long sum = 0;
    for(size_t i = 0; i < n ; i++) {
        sum += xs[i]->value;
        pfds_release(xs[i]);
    }
    return pfds_UInt64_new(sum);
}


pfds_catenablevtable pfds_catenable_sum = {
    .mempty = (pfds_object* (*)(void))
        pfds_UInt64_zero,
    .mappend = (pfds_object* (*)(pfds_object*, pfds_object*))
        pfds_UInt64_add,
    .concat = (pfds_object* (*)(size_t, pfds_object**))
        pfds_UInt64_sum,
};
