# mi-programoptions

This library is a small command line argument parser library for C++11
and later:

- it only handles strings
- it supports long options with --, short options with -, and
  positional arguments
- it avoids option name strings repetition by using option objects
  such that the compiler can find some typos

The library has no runtime dependencies beyond the standard C++
library.

The library uses mi-cpptest for the optional unit tests. In the CMake
build, unit tests may be disabled by setting `ENABLE_TESTS` to `0`.

## Use with CMake

1. either include this as a subproject with `ADD_SUBDIRECTORY(...)`,
   which will build a static library
2. or build and install and use `FIND_PACKAGE(mi-programoptions)`,
   which will build a shared library

In both cases, link to `mi-programoptions`.

## Use without CMake

As the library consists of only 3 code files, 1 of which tiny, it
should be easy to use with other build systems.
