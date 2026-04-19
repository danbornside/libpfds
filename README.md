## libpfds C Library implementation of Persistent Functional Data Structures

This library is a plain C implementation of persistent data structures.

As of this version, libpfds is unsutied to any uses, presented in a pre-alpha
state for educational purposes only.

> libpfds is free software: you can redistribute it and/or modify it under the
> terms of the GNU General Public License as published by the Free Software
> Foundation, either version 3 of the License, or (at your option) any later
> version.
>
> libpfds is distributed in the hope that it will be useful, but WITHOUT ANY
> WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
> FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
> details.
>
> You should have received a copy of the GNU General Public License along with
> libpfds. If not, see <https://www.gnu.org/licenses/>.


### implementation notes

types here are implemented using a simple reference counting garbage collection
mechanism.  many functions that return modified values will "claim ownership"
of their arguments, either using them directly to construct the new value, or
decreasing their reference count if that isn't helpful.  To avoid freeing
function arguments too early, users of this library will need to increment the
ref counts
