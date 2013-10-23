# Overview

Voronota is a tool for constructing the quasi-triangulation
dual of the Voronoi diagram of atomic balls.


# Getting the latest version

Download the latest package from bitbucket.org/kliment/voronota/downloads.


# Building from source code

Voronota has no required external dependencies, only
a standard-compliant C++ compiler is needed to build it.

For example, "voronota" executable can be built from
the sources in "src" directory using GNU C++ compiler:

    g++ -O3 -o voronota src/*.cpp

To enable the usage of OpenMP for parallel processing,
add "-fopenmp" option when buiding:

    g++ -O3 -fopenmp -o voronota src/*.cpp

To enable the usage of MPI for parallel processing, you
can use mpic++ compiler wrapper. You also need to define
"ENABLE_MPI" macro when buiding:

    mpic++ -O3 -DENABLE_MPI -o voronota ./src/*.cpp
    
You can also build using cmake for makefile generation.
Starting in the directory containing "CMakeLists.txt" file,
run the sequence of commands:

    mkdir build
    cd build
    cmake ../
    make


# Basic usage example

Here is a basic example of computing a quasi-triangulation
for a structure in a PDB file:

    ./voronota --mode get-balls-from-atoms-file < input.pdb > input.txt
    ./voronota --mode calculate-triangulation < input.txt > output.txt


# Getting usage help

Usage help is displayed when executing Voronota without any
parameters or with "--help" command line option:

    ./voronota --help


# Contact information

If you experience problems with the software or if you have
questions or suggestions about it, please contact me:

Kliment Olechnovic

kliment@ibt.lt
