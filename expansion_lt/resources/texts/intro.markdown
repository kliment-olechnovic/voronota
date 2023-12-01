# About Voronota-LT

Voronota-LT (pronounced 'voronota lite') is an alternative version of Voronota for contructing tessellation-derived atomic contact areas and volumes.
Voronota-LT was written from scratch and does not use any external code, even from the core Voronota.
The primary motivation for creating Voronota-LT was drastically increasing the speed of computing tessellation-based atom-atom contact areas and atom solvent-accessible surface areas.
Like Voronota, Voronota-LT can compute contact areas derived from the additively weighted Voronoi tessellation,
but the main increase in speed comes when utilizing a simpler, radical tessellation variant, also known as Laguerre tessellation.
This is the default tessellation variant in Voronota-LT. It considers radii of atoms together with the rolling probe radius to define radical planes as bisectors between atoms.

# Getting the latest version

Download the latest archive from the official downloads page:
[https://github.com/kliment-olechnovic/voronota/releases](https://github.com/kliment-olechnovic/voronota/releases).

The archive contains the Voronota-LT software in the 'expansion_lt' subdirectory.
This executable can be built from the provided
source code to work on any modern Linux, macOS or Windows operating systems.

# Building the command-line tool from source code

## Requirements

Voronota-LT has no required external dependencies, only
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
    cp ./voronota-lt ../voronota-lt

## Using C++ compiler directly

For example, "voronota-lt" executable can be built using GNU C++ compiler:

    g++ -std=c++14 -O3 -fopenmp -o ./voronota-lt ./src/voronota_lt.cpp

Performance-boosting compiler flags can be included:

    g++ -std=c++14 -Ofast -march=native -fopenmp -o ./voronota-lt ./src/voronota_lt.cpp

# Running the command-line tool

The overview of command-line options, as well as input and output, is printed when running the "voronota-lt" executable with "--help" or "-h" flags:

    voronota-lt --help
    
    voronota-lt -h

The overview text is the following:

