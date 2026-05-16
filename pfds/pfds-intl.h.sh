#! /usr/bin/env bash

cat << EOF
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

EOF

set -eu
shopt -s execfail


DIR="$(dirname $0)"
CONF="$(basename $DIR)/../config.inc.sh"

if [ ! -f "$CONF" ] ; then
    echo "run ./configure first" >&2
    exit 1
else
    source "$CONF"
fi

# ATTENTION: The following warning will appear in the generated file.
# ATTENTION: you are editing the correct file
cat << "EOF"
/* ATTENTION: this file was generated from ./pfds-intl.h.in
 * ATTENTION: any changes you make here will be lost
 */

#ifndef PFDS_INTL_HEADER_INCLUDED
#define PFDS_INTL_HEADER_INCLUDED

#include <stddef.h>

EOF


cat << EOF

/** libpfds was configured with reference counting gc $( pfds_gcconfig \
    "ENABLED" "DEBUGGING ENABLED" "DISABLED")).
 *
 * This option cannot be changed once libpfds is built.  This macro can only be
 * used to determined which mode libpfds has been configured in.
 */
#define PFDS_GC_CONFIG "${PFDS_CONFIG_GC}"
$(pfds_gcconfig \
    '#define PFDS_GC_REFCOUNT' \
    '#define PFDS_GC_DEBUGREFCOUNT' \
    '#define PFDS_GC_NONE')

typedef struct pfds_object pfds_object;

#define PFDS_GC_HEADER $(pfds_gcconfig \
    'int refcount;' \
    'size_t retaincount; size_t releasecount; bool finalized;' \
    '')

/** increase reference count of self by 1
 * \\public \\memberof pfds_object
 *
 * \\param self
 * \\invariant dup(self)
 */
#define pfds_retain(self) $(pfds_gcconfig \
        'pfds_object_retain((pfds_object*) self)' \
        'pfds_object_retain((pfds_object*) self, __FILE__, __LINE__)' \
        '')

/** decrease reference count of self by 1
 * \\public \\memberof pfds_object
 *
 * \\param self
 * \\invariant drop(self)
 */

#define pfds_release(self) $(pfds_gcconfig \
        'pfds_object_release((pfds_object*) self)' \
        'pfds_object_release((pfds_object*) self, __FILE__, __LINE__)' \
        '')

/** increase reference count of self by 1
 * \\public \\memberof pfds_object
 *
 * \\param self $(pfds_gcdebugconfig '
 * \param fn file name
 * \param ln line number')
 * \\invariant dup(self)
 */
void pfds_object_retain(pfds_object* self$(pfds_gcdebugconfig ', char* fn, int ln'));

/** decrese reference count of self by 1.  object will be freed if refcount is zero
 * \\public \\memberof pfds_object
 *
 * \\param self $(pfds_gcdebugconfig '
 * \param fn file name
 * \param ln line number')
 * \\invariant drop(self)
 */
void pfds_object_release(pfds_object* self$(pfds_gcdebugconfig ', char* fn, int ln'));

struct pfds_gcinfo {$(pfds_gcenabledconfig '
    size_t births;
    size_t deaths;
    size_t retaincount;
    size_t releasecount;
    ')
};
struct pfds_gcinfo pfds_getgcinfo(void);

#endif
EOF
# vim: set syntax=bash sw=4 ts=4 et :
