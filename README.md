libpfds C Library implementation of Persistent Functional Data Structures
=========================================================================


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

Installation
------------

libpfds is developed using the nix package manager.  If you are also using nix, a `package.nix` is provided that can be used to include libpfds in your own projects, or it can built in with:

```bash
nix-build
```

Also for nix users interested in contributing to libpfds, a `shell.nix` is provided so you can run the following within the libpfds source directory:

```bash
nix-shell

./configure [OPTIONS]
make
```

On other platforms, libpfds can be built with GNU make and a recent C compiler:

```bash
./configure [OPTIONS]
make
make install
```

Documentation is built from sources with doxygen.
