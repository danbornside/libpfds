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


/* miscelaneious macros used throughout the implementation of pfds that are inappropriate to install globally */

/* assign to an outparameter pointer if it is not null and perform a fallback otherwise  */
#define OPTIONAL_OUTPARAM2(var, val, garbage) if (var != NULL) { *var = val; } else { garbage ; }

/* assign to an outparameter pointer if it is not null  */
#define OPTIONAL_OUTPARAM(var, val) if (var != NULL) { *var = val; }

#define panic(msg) { fprintf(stderr, "PANIC %s:%d\n\t%s\n", __FILE__, __LINE__, msg); abort(); }
