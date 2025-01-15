# About Voronota-LT

Voronota-LT (pronounced 'voronota lite') is an alternative version of [Voronota](../index.html) for constructing tessellation-derived atomic contact areas and volumes.
Voronota-LT was written from scratch and does not use any external code, even from the core Voronota.
The primary motivation for creating Voronota-LT was drastically increasing the speed of computing tessellation-based atom-atom contact areas and atom solvent-accessible surface areas.

Like Voronota, Voronota-LT can compute contact areas derived from the additively weighted Voronoi tessellation,
but the main increase in speed comes when utilizing a simpler, radical tessellation variant, also known as Laguerre-Laguerre tessellation or power diagram.
This is the default tessellation variant in Voronota-LT. It considers radii of atoms together with the rolling probe radius to define radical planes as bisectors between atoms.

Voronota-LT is distributed as an expansion part of [the Voronota software package](../index.html),
mainly to enable other Voronota expansions to easily use the Voronota-LT library.

The core functionality of Voronota-LT is also available via the [Voronota-LT web application](./web/index.html) built using Emscripten.

## Benchmarking data and results

Benchmarking data and results are available [here](./benchmark/index.html).

# Quick install guide

Please refer to the [core Voronota quick install guide](../index.html#quick-install-guide).

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

Change to the 'expansion_lt' directory:

```bash
cd expansion_lt
```

Then run the sequence of commands:

```bash
cmake ./
make
```

Alternatively, to keep files more organized, CMake can be run in a separate "build" directory:

```bash
mkdir build
cd build
cmake ../
make
cp ./voronota-lt ../voronota-lt
```

## Using C++ compiler directly

For example, "voronota-lt" executable can be built using GNU C++ compiler.

Change to the 'expansion_lt' directory:

```bash
cd expansion_lt
```

Then run the compiler:

```bash
g++ -std=c++14 -O3 -fopenmp -o ./voronota-lt ./src/voronota_lt.cpp
```

Performance-boosting compiler flags can be included:

```bash
g++ -std=c++14 -Ofast -march=native -fopenmp -o ./voronota-lt ./src/voronota_lt.cpp
```

## Compiling on Windows

### Using Windows Subsystem for Linux

When using Windows Subsystem for Linux, Voronota-LT can be compiled using the same instructions as described above, that is, using CMake or g++ directly.

### Using Microsoft Visual C++ command-line compiler

If you have installed Visual Studio 2017 or later on Windows 10 or later,
open 'Developer Command Prompt for VS' from start menu,
navigate to the 'expansion_lt' folder,
and run the following command that produces 'voronota-lt.exe' program:

```
cl /Ox /openmp:llvm .\src\voronota_lt.cpp
```

# Running the command-line tool

The overview of command-line options, as well as input and output, is printed when running the "voronota-lt" executable with "--help" or "-h" flags:

```bash
voronota-lt --help

voronota-lt -h
```

The overview text is the following:

