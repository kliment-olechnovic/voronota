# About Voronota

The analysis of macromolecular structures often requires
a comprehensive definition of atomic neighborhoods.
Such a definition can be based on the Voronoi diagram of balls,
where each ball represents an atom of some van der Waals radius.
Voronota is a software tool for finding all the vertices
of the Voronoi diagram of balls. Such vertices correspond to
the centers of the empty tangent spheres defined by quadruples of balls.
Voronota is especially suitable for processing three-dimensional
structures of biological macromolecules such as proteins and RNA.

Since version 1.2 Voronota also uses the Voronoi vertices to construct
inter-atom contact surfaces and solvent accessible surfaces.
Voronota provides tools to query contacts, generate contacts graphics,
compare contacts and evaluate quality of protein structural models using contacts.

Voronota is developed by Kliment Olechnovic (kliment@ibt.lt).


# Getting the latest version

Download the latest archive from the official downloads page:
[https://bitbucket.org/kliment/voronota/downloads](https://bitbucket.org/kliment/voronota/downloads).

The archive contains ready-to-use statically compiled 'voronota' program for
64 bit Linux systems. This executable can be rebuilt from the provided
source code to work on any modern Linux, Mac OS X or Windows operating systems.

Packages in .deb or .rpm formats are currently not available. However,
installing Voronota in Linux or Mac OS X is easy: just copy Voronota
executable files ('voronota' program and, if needed, the wrapper scripts)
to one of the directories listed in $PATH variable.


# Building from source code

## Using C++ compiler directly

Voronota has no required external dependencies, only
a standard-compliant C++ compiler is needed to build it.

For example, "voronota" executable can be built from
the sources in "src" directory using GNU C++ compiler:

    g++ -O3 -o voronota src/*.cpp

## Using CMake

You can also build using CMake for makefile generation.
Starting in the directory containing "CMakeLists.txt" file,
run the sequence of commands:

    mkdir build ; cd build ; cmake ../ ; make ; cd ../ ; mv build/voronota voronota

## Enabling OpenMP

To enable the usage of OpenMP for parallel processing when
building using C++ compiler directly, add "-fopenmp" option:

    g++ -O3 -fopenmp -o voronota src/*.cpp

When using CMake, OpenMP usage is enabled automatically if it is possible.

## Enabling MPI

To enable the usage of MPI for parallel processing, you
can use mpic++ compiler wrapper. You also need to define
"ENABLE_MPI" macro when buiding:

    mpic++ -O3 -DENABLE_MPI -o voronota ./src/*.cpp


# Basic usage example

## Computing Voronoi vertices

Here is a basic example of computing Voronoi vertices
for a structure in a PDB file:

    ./voronota get-balls-from-atoms-file < input.pdb > balls.txt
    ./voronota calculate-vertices < balls.txt > vertices.txt

The first command reads a PDB file "input.pdb" and outputs a file "balls.txt"
that contains balls corresponding to the atoms in "input.pdb"
(by default, Voronota ignores all heteroatoms and all hydrogen atoms
when reading PDB files: this behavior can be altered using command-line options).
The second command reads "balls.txt" and outputs a file "vertices.txt"
that contains a quadruples and empty tangent spheres that correspond
to the vertices of the Voronoi diagram of the input balls.
The formats of "balls.txt" and "vertices.txt" are described below.

In "balls.txt" the line format is "x y z r # comments".
The first four values (x, y, z, r) are atomic ball coordinates and radius.
Comments are not needed for further calculations, they are to assist human readers.
For example, below is a part of some possible "balls.txt":

    28.888 9.409 52.301 1.7 # 1 A 2 SER N
    27.638 10.125 52.516 1.9 # 2 A 2 SER CA
    26.499 9.639 51.644 1.75 # 3 A 2 SER C
    26.606 8.656 50.915 1.49 # 4 A 2 SER O
    27.783 11.635 52.378 1.91 # 5 A 2 SER CB
    27.69 12.033 51.012 1.54 # 6 A 2 SER OG

In "vertices.txt" the line format is "q1 q2 q3 q4 x y z r".
The first four numbers (q1, q2, q3, q4) are numbers
of atomic records in "balls.txt", starting from 0.
The remaining four values (x, y, z, r) are the coordinates and the radius of
an empty tangent sphere of the quadruple of atoms.
For example, below is a part of some possible "vertices.txt":

    0 1 2 3 27.761 8.691 51.553 -0.169
    0 1 2 23 28.275 9.804 50.131 0.588
    0 1 3 1438 24.793 -3.225 60.761 14.047
    0 1 4 5 28.785 10.604 50.721 0.283
    0 1 4 1453 30.018 10.901 55.386 1.908
    0 1 5 23 28.544 10.254 50.194 0.595

## Getting help in command line

The list of all available Voronota commands is displayed when executing Voronota without any parameters.

Command help is shown when "--help" command line option is present, for example:

    ./voronota calculate-vertices --help

Using "--help" option without specific command results in printing help for all commands:

    ./voronota --help

