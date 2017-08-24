## OpenVMS

This directory contains mainly a build script for OpenVMS.

## Requirements

* OpenVMS 8.4 or later (Alpha or IA64), VAX is not supported
* DEC CC 6.5 or later
* DEC CXX 7.1 or later (for tests only)
* Boost 1.34 or later (for tests only)

## Building

Library can be built with 32-bit or 64-bit pointers. By default 64-bit
pointers are used. Use `32` parameter to switch behaviour.

Test suit is optional (and requires quite a long time to be compiled on
an emulator) and is not built by default. Use `TESTS` parameter to
enable tests. Tests require _Boost Unit Test Framefork_ which you must
point out with the `BOOST_ROOT` parameter. On some systems _Boost_
loves either 32-bit or 64-bit mode only (not the both).

There are other build parametes available, plese look inside the
`build_vms.com` (a DCL-based script) to see the detailed description for
all of them.

Object library (.OLB) and shareable image (.EXE) are built. An object
library acts like a widely-known static library, and a shareable image
acts like a shared library. All libraries and tests are placed inside the
`[.ALPHA]` or `[.IA64]` directory depending on a host architecture.

Do not forget to define a logical name for a shareable image of the library
before running programs which use it:

`DEFINE UNIC SYS$SYSROOT:[BUILD_DIR]UNIC.EXE`

You can also place an image into the `SYS$SHARE` directory instead of
defining a logical name.

Here are some examples of the build commands:

* `@build_vms.com` builds only the libraries (64-bit pointers).
* `@build_vms.com 32 TESTS BOOST_ROOT=/SYS$SYSROOT/BOOST_1_34_1` builds
libraries (32-bit pointers) and all the tests.
* `@build_vms.com NOLIB RUN_TESTS` only runs already built tests.
* `@build_vms.com CLEAN` cleans all the files produced during a build.

You can use _Boost_ reduced mini-version supplied with the library in
`boost` directory. Version 1.34.1 was checked and works fine.

## More

OpenVMS can mangle long (> 31 characters) symbol names in a compiled object
to fit the limit. Sometimes it is useful to know the mangled name of a
symbol. Use the `vms_shorten_symbol.c` program to get a mangled name. See
details inside.
