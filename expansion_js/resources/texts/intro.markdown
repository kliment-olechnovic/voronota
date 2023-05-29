# About Voronota-JS expansion

Voronota-JS is an expansion of [the core Voronota software](../index.html).
Voronota-JS provides a way to write JavaScript scripts for the comprehensive
analysis of macromolecular structures, including the Voronoi tesselation-based analysis.

Currently, the Voronota-JS package contains several executables:

 * "voronota-js" - core engine that executes JavaScript scripts.
 * "voronota-js-voromqa" - wrapper to a voronota-js program for computing VoroMQA scores, both old and new (developed for CASP14).
 * "voronota-js-only-global-voromqa" - wrapper to a voronota-js program for computing only global VoroMQA scores with fast caching.
 * "voronota-js-membrane-voromqa" - wrapper to a voronota-js program for the VoroMQA-based analysis and assessment of membrane protein structural models.
 * "voronota-js-ifeatures-voromqa" - wrapper to a voronota-js program for the computation of multiple VoroMQA-based features of protein-protein complexes.
 * "voronota-js-fast-iface-voromqa" - wrapper to a voronota-js program for the very fast computation of the inter-chain interface VoroMQA energy.
 * "voronota-js-fast-iface-cadscore" - wrapper to a voronota-js program for the very fast computation of the inter-chain interface CAD-score.
 * "voronota-js-fast-iface-cadscore-matrix" - wrapper to a voronota-js program for the very fast computation of the inter-chain interface CAD-score matrix.
 * "voronota-js-fast-iface-contacts" - wrapper to a voronota-js program for the very fast computation of the inter-chain interface contact areas.
 * "voronota-js-fast-iface-data-graph" - wrapper to a voronota-js program for the computation of interface graphs used by the VoroIF-GNN method.
 * "voronota-js-voroif-gnn" - wrapper to a voronota-js program and GNN inference scripts that run the VoroIF-GNN method for scoring models of protein-protein complexes (developed for CASP15).
 * "voronota-js-ligand-cadscore" - wrapper to a voronota-js program for the computation of protein-ligand variation of CAD-score (developed to analyze protein-ligand models from CASP15).

## Usage of externally developed software

Voronota-JS relies on several externally developed software projects (big thanks to their authors):

* Duktape - [https://duktape.org/](https://duktape.org/)
* PStreams - [https://github.com/jwakely/pstreams](https://github.com/jwakely/pstreams)
* TM-align - [https://zhanggroup.org/TM-align/](https://zhanggroup.org/TM-align/)
* FASPR - [https://zhanggroup.org/FASPR/](https://zhanggroup.org/FASPR/)
* linenoise - [https://github.com/antirez/linenoise](https://github.com/antirez/linenoise)
* PicoSHA2 - [https://github.com/okdshin/PicoSHA2](https://github.com/okdshin/PicoSHA2)
* frugally-deep - [https://github.com/Dobiasd/frugally-deep](https://github.com/Dobiasd/frugally-deep)
    * FunctionalPlus - [https://github.com/Dobiasd/FunctionalPlus](https://github.com/Dobiasd/FunctionalPlus)
    * Eigen - [http://eigen.tuxfamily.org/](http://eigen.tuxfamily.org/)
    * json  - [https://github.com/nlohmann/json](https://github.com/nlohmann/json)
* tinf - [https://github.com/jibsen/tinf](https://github.com/jibsen/tinf)
* LodePNG - [https://github.com/lvandeve/lodepng](https://github.com/lvandeve/lodepng)

# Getting the latest version

Download the latest archive from the official downloads page:
[https://github.com/kliment-olechnovic/voronota/releases](https://github.com/kliment-olechnovic/voronota/releases).

The archive contains the Voronota-JS software in the 'expansion_js' subdirectory.
There is a ready-to-use statically compiled 'voronota-js' program for 64 bit Linux systems.
This executable can be rebuilt from the provided
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

    g++ -std=c++14 -I"./src/dependencies" -O3 -o "./voronota-js" $(find ./src/ -name '*.cpp')

