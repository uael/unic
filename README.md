
[![](https://api.travis-ci.org/saprykin/plib.svg?branch=master)](https://travis-ci.org/saprykin/plib)
[![](https://ci.appveyor.com/api/projects/status/github/saprykin/plib?branch=appveyor_test&svg=true)](https://ci.appveyor.com/project/saprykin/plib)
[![](https://scan.coverity.com/projects/8333/badge.svg)](https://scan.coverity.com/projects/saprykin-plib)
[![](https://codecov.io/gh/saprykin/plib/branch/master/graph/badge.svg)](https://codecov.io/gh/saprykin/plib)
[![](http://img.shields.io/:license-gpl2-blue.svg?style=flat)](http://www.gnu.org/licenses/gpl-2.0.html)

======= About PLibSYS =======

PLibSYS is a cross-platform system C library with some helpful routines.

======= Features =======

PLibSYS gives you:

* Platform independent data types
* Threads: POSIX, Solaris and Win32
* Mutexes: POSIX, Solaris and Win32
* Conditional variables: POSIX, Solaris and Win32
* System-wide semaphores: POSIX, System V and Win32
* System-wide shared memory: POSIX, System V and Win32
* Atomic operations
* Sockets support (UDP, TCP, SCTP)
* Hash functions: MD5, SHA-1, GOST (R 34.11-94)
* Binary trees: BST, red-black, AVL
* INI files parser
* High resolution time profiler
* Files and directories
* Dynamic library loading
* Useful routines for linked lists, strings, hash tables
* Macros for OS and compiler detection
* Fully covered with unit tests

======= Platforms =======

PLibSYS is a cross-platform, highly portable library, it was tested on
the following platforms:

* GNU/Linux
* OS X
* Windows
* Cygwin
* FreeBSD, NetBSD, OpenBSD
* DragonFlyBSD
* Solaris
* AIX
* HP-UX
* IRIX64
* QNX Neutrino
* UnixWare 7
* SCO OpenServer 5

It should also work on other *nix systems with or without minimal
efforts.

======= Compilers =======

PLibSYS was tested with the following compilers:

* MSVC (x86, x64) 2003 and above
* MinGW (x86, x64)
* Open Watcom (x86)
* Borland (x86)
* GCC (x86, x64, PPC32be, PPC64be, IA-64/32, IA-64, HPPA2.0-32, MIPS32, AArch32, SPARCv9)
* Clang (x86, x64)
* Intel (x86, x64)
* QCC (x86)
* Oracle Solaris Studio (x86, x64, SPARCv9)
* MIPSpro (MIPS32)

======= Building =======

Use CMake to build PLibSYS for any target platform.

======= License =======

PLibSYS is distributed under the terms of GNU GPLv2 license.
