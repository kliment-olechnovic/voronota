# About Voronota-JS expansion

Voronota-JS is an expansion of the core Voronota software.
Voronota-JS provides a way to write JavaScript scripts for the comprehensive
analysis of macromolecular structures, including the Voronoi tesselation-based analysis.

Currently, the Voronota-JS package contains three executables:

* "voronota-js" - core engine that executes JavaScript scripts.
* "voronota-js-voromqa" - wrapper to a voronota-js program for computing VoroMQA scores, both old and new (developed for CASP14).
* "voronota-js-membrane-voromqa" - wrapper to a voronota-js program for the VoroMQA-based analysis and assessment of membrane protein structural models.

# Getting the latest version

Download the archive from
[https://kliment-olechnovic.github.io/voronota/expansion_js/voronota-js_release.tar.gz](https://kliment-olechnovic.github.io/voronota/expansion_js/voronota-js_release.tar.gz).

The archive contains ready-to-use statically compiled "voronota-js" program for
64 bit Linux systems. This executable can be rebuilt from the provided
source code to work on any modern Linux, macOS or Windows operating systems.

# Building from source code

## Requirements

Voronota-JS has no required external dependencies, only
a C++14-compliant compiler is needed to build it.

## Using CMake

You can build using CMake for makefile generation.
Starting in the directory containing "CMakeLists.txt" file,
run the sequence of commands:

    cmake ./
    make

Alternatively, to keep files more organized, CMake can be run in a separate "build" directory:

    mkdir build
    cd build
    cmake ../
    make
    cp ./voronota-js ../voronota-js

## Using C++ compiler directly

For example, "voronota-js" executable can be built from
the sources in "src" directory using GNU C++ compiler:

    g++ -std=c++14 -I"./src/expansion_js/src/dependencies" -O3 -o "./voronota-js" $(find ./src/ -name '*.cpp')

    