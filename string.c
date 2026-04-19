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

#include <string.h>

#define PFDS_INTERNAL

#include "pfds.h"


void String_destroy(pfds_String* self) {
    if (self->owner) {
        free((char*) self->buf); // the struct has it as const char but we know it's heapy
    }
}

void String_debugfputs(FILE* stream, pfds_String* self) {
    fputs(self->buf, stream);
}
pfds_object* String_mappend(pfds_object* l_object, pfds_object* r_object) {
    pfds_String* l = (pfds_String*) l_object;
    pfds_String* r = (pfds_String*) r_object;
    char* buf = malloc(l->size + r->size + 1);
    memcpy(buf, l->buf, l->size);
    memcpy(buf+l->size, r->buf, r->size);
    buf[l->size+r->size] = '\0';
    pfds_String* self = pfds_String_new(buf, l->size + r->size, true);
    pfds_release(l_object);
    pfds_release(r_object);
    return (pfds_object*) self;
}

pfds_String* pfds_String_concat(size_t n, pfds_String* chunks[]) {
    size_t size = 0;
    for (int i = 0; i < n ; i ++) {
        size += chunks[i]->size;
    }
    char * buf = (char*) malloc(size);
    size_t pos = 0;
    for (int i = 0; i < n ; i ++) {
        memcpy(buf + pos, chunks[i]->buf, chunks[i]->size);
        pos += chunks[i]->size;
        pfds_release(chunks[i]);
    }
    buf[pos] = '\0';
    return pfds_String_new((const char*) buf, size, true);
}

pfds_ordering String_cmp(pfds_ordered *l, pfds_ordered *r) {
    return strcmp(((pfds_String*)l)->buf, ((pfds_String*) r)->buf);
}

static pfds_orderedvtable String_orderedvtable = {
    .cmp = String_cmp
};
static pfds_catenablevtable String_catenablevtable = {
    .mempty = (pfds_object* (*)(void)) pfds_String_empty,
    .mappend = String_mappend,
    .concat = (pfds_object* (*)(size_t, pfds_object**)) pfds_String_concat,
};

const pfds_objectvtable pfds_String_vtable = {
    .typename = "String",
    .destroy = (void (*)(pfds_object*)) String_destroy,
    .debugfputs = (void (*)(FILE*, pfds_object*)) String_debugfputs,
    .catenable = &String_catenablevtable,
    .ordering = &String_orderedvtable,
};

extern pfds_String* pfds_String_new(const char* buf, size_t size, bool owner) {
    pfds_String* self = (pfds_String*) pfds_object_new(sizeof(pfds_String), &pfds_String_vtable);
    self->owner = owner;
    self->size = size;
    self->buf = buf;
    return self;

}

extern pfds_String* pfds_String_empty(void) {
    static const char* nullStr = "";
    return pfds_String_new(nullStr, 0, false);
}

extern pfds_String* pfds_String_fromCstring(char* buf) {
    return pfds_String_new(strdup(buf), strlen(buf), true);
}
extern pfds_String* pfds_String_fromConstCstring(char* buf) {
    return pfds_String_new(buf, strlen(buf), false);
}

extern const char* pfds_String_toCstring(pfds_String* self) {
    return self->buf;
}
