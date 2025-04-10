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

Most of the new developments are happening in the expansions of Voronota: Voronota-JS, Voronota-LT and Voronota-GL.

Voronota and its expansions are developed by Kliment Olechnovic ([https://www.kliment.lt](https://www.kliment.lt)).

# Voronota on GitHub

Voronota source code is hosted on GitHub, at [https://github.com/kliment-olechnovic/voronota](https://github.com/kliment-olechnovic/voronota).

The recommended way to get the latest stable version of Voronota and its expansions is to download the latest archive from the "Releases" page:
[https://github.com/kliment-olechnovic/voronota/releases](https://github.com/kliment-olechnovic/voronota/releases).

The release archive contains ready-to-use statically compiled ‘voronota’ program for 64 bit Linux systems.
The release archive also contains the source code that can be compiled on any modern Linux, macOS or Windows operating systems.

# Quick install guide

Below are several commands that install the latest version of Voronota and its expansions for the command line use in Unix-like systems.

```bash
# download the latest package
wget https://github.com/kliment-olechnovic/voronota/releases/download/vLATEST_RELEASE_VERSION/voronota_LATEST_RELEASE_VERSION.tar.gz

# unpack the package
tar -xf ./voronota_LATEST_RELEASE_VERSION.tar.gz

# change to the package directory
cd ./voronota_LATEST_RELEASE_VERSION

# run CMake
cmake . -DEXPANSION_JS=ON -DEXPANSION_LT=ON

# compile everything
make

# install everything
sudo make install
```

The installed files are listed in the 'install_manifest.txt' file.
They can be uninstalled by running

```bash
sudo xargs rm < ./install_manifest.txt
```

# About Voronota expansions

Currently there are three expansions of Voronota: Voronota-JS, Voronota-LT and Voronota-GL.

The expansions need to be built separately in their subdirectories.
Alternatively, they can be built by appending `-DEXPANSION_JS=ON` and/or `-DEXPANSION_LT=ON` and/or `-DEXPANSION_GL=ON` to the CMake command call.

The expansions have separate dedicated documentation pages.

## Voronota-JS

[Voronota-JS](./expansion_js/index.html) expansion is located in the 'expansion_js' subdirectory of the Voronota package.

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
 * "voronota-js-fast-iface-data-graph" - wrapper to a voronota-js program for the computation of interface graphs used by the VoroIF-GNN method.
 * "voronota-js-voroif-gnn" - wrapper to a voronota-js program and GNN inference scripts that run the VoroIF-GNN method for scoring models of protein-protein complexes (developed for CASP15).
 * "voronota-js-ligand-cadscore" - wrapper to a voronota-js program for the computation of protein-ligand variation of CAD-score (developed to analyze protein-ligand models from CASP15).

## Voronota-LT

[Voronota-LT](./expansion_lt/index.html) expansion is located in the 'expansion_lt' subdirectory of the Voronota package.

Voronota-LT (pronounced 'voronota lite') is an alternative version of Voronota for constructing tessellation-derived atomic contact areas and volumes.
Voronota-LT was written from scratch and does not use any external code, even from the core Voronota.
The primary motivation for creating Voronota-LT was drastically increasing the speed of computing tessellation-based atom-atom contact areas and atom solvent-accessible surface areas.
Like Voronota, Voronota-LT can compute contact areas derived from the additively weighted Voronoi tessellation,
but the main increase in speed comes when utilizing a simpler, radical tessellation variant, also known as Laguerre-Laguerre tessellation or power diagram.

## Voronota-GL

[Voronota-GL](./expansion_gl/index.html) expansion is located in the 'expansion_gl' subdirectory of the Voronota package

Voronota-GL is a visual tool for the comprehensive interactive
analysis of macromolecular structures, including the Voronoi tesselation-based analysis.

# Getting the latest version

Download the latest archive from the official downloads page:
[https://github.com/kliment-olechnovic/voronota/releases](https://github.com/kliment-olechnovic/voronota/releases).

The archive contains ready-to-use statically compiled 'voronota' program for
64 bit Linux systems. This executable can be rebuilt from the provided
source code to work on any modern Linux, macOS or Windows operating systems.

On Ubuntu 18.04 and newer it is possible to install Voronota using 'apt' command:

    sudo apt install voronota

On Windows 10 operating system the easiest way to run Voronota is to use
Windows Subsystem for Linux (WSL).

# Building from source code

## Requirements

Voronota has no required external dependencies, only
a standard-compliant C++ compiler is needed to build it.

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
    cp ./voronota ../voronota

## Using C++ compiler directly

For example, "voronota" executable can be built from
the sources in "src" directory using GNU C++ compiler:

    g++ -O3 -std=c++11 -o voronota $(find ./src/ -name '*.cpp')
    
## Enabling OpenMP

To allow the usage of OpenMP when calling the "calculate-vertices-in-parallel" command,
the "-fopenmp" flag needs to be set when building.

When building using C++ compiler directly, just add "-fopenmp":

    g++ -O3 -std=c++11 -fopenmp -o voronota $(find ./src/ -name '*.cpp')

When using CMake, set the CMAKE_CXX_FLAGS variable:

    cmake -DCMAKE_CXX_FLAGS="-fopenmp" ./
    make

## Enabling MPI

To allow the usage of MPI when calling the "calculate-vertices-in-parallel" command,
you can use mpic++ compiler wrapper and define "ENABLE_MPI" macro when buiding:

    mpic++ -O3 -std=c++11 -DENABLE_MPI -o voronota ./$(find ./src/ -name '*.cpp')

## TR1 usage switch

Voronota can be built with either modern C++ compilers or
pre-C++11 compilers that support C++ Technical Report 1 (TR1) features.
The voronota code has preprocessor-based checks to find out
if C++ TR1 namespace is available and needs to be used.
If compilation fails, it may mean that these checks failed.
To troubleshoot this, try setting the value of the "USE_TR1" macro
to 0 (to not use TR1 and to rely on C++11 standard)
or 1 (to use TR1) when compiling, for example:

    g++ -O3 -DUSE_TR1=1 -o voronota $(find ./src/ -name '*.cpp')

or

    g++ -O3 -std=c++11 -DUSE_TR1=0 -o voronota $(find ./src/ -name '*.cpp')


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

## Computing inter-atom contacts

Taking the "balls.txt" file described in the previous section,
here is a basic example of computing inter-atom contacts:

    ./voronota calculate-contacts < balls.txt > contacts.txt

In "contacts.txt" file the line format is "b1 b2 area".
The first two numbers (b1 and b2) are numbers of atomic records in "balls.txt", starting from 0.
If b1 does not equal b2, then the 'area' value is the area of contact between atoms b1 and b2.
If b1 equals b2, then the 'area' value is the solvent-accessible area of atom b1.
For example, below is a part of some possible "contacts.txt":

    0 0 35.440
    0 1 15.908
    0 2 0.167
    0 3 7.025
    0 4 7.021
    0 5 0.624
    0 23 2.849
    0 25 0.008
    0 26 11.323
    0 1454 0.021
    1 1 16.448
    1 2 11.608
    1 3 0.327
    1 4 14.170
    1 5 0.820
    1 6 3.902
    1 23 0.081
    2 2 3.591
    2 3 11.714
    2 4 0.305
    2 5 2.019

## Computing annotated inter-atom contacts

Here is a basic example of computing annotated inter-atom contacts:

    ./voronota get-balls-from-atoms-file --annotated < input.pdb > annotated_balls.txt
    ./voronota calculate-contacts --annotated < annotated_balls.txt > annotated_contacts.txt

In "annotated_contacts.txt" the line format is
"annotation1 annotation2 area distance tags adjuncts [graphics]".
The strings 'annotation1' and 'annotation2' describe contacting atoms,
the 'area' value is the area of contact between the two atoms,
the 'distance' value is the distance between the centers of the contacting atoms.
If 'annotation2' contains string "solvent", then the 'area' value is
the solvent-accessible area of the atom described by 'annotation1'.
The remaining part of the line is used by Voronota querying
and drawing commands that are not covered in this section.
Below is a part of some possible "annotated_contacts.txt"
(with text fields aligned using "column -t" command):

    c<A>r<2>a<1>R<SER>A<N>   c<A>r<2>a<2>R<SER>A<CA>        15.908  1.456  .  .
    c<A>r<2>a<1>R<SER>A<N>   c<A>r<2>a<3>R<SER>A<C>         0.167   2.488  .  .
    c<A>r<2>a<1>R<SER>A<N>   c<A>r<2>a<4>R<SER>A<O>         7.025   2.774  .  .
    c<A>r<2>a<1>R<SER>A<N>   c<A>r<2>a<5>R<SER>A<CB>        7.021   2.486  .  .
    c<A>r<2>a<1>R<SER>A<N>   c<A>r<2>a<6>R<SER>A<OG>        0.624   3.159  .  .
    c<A>r<2>a<1>R<SER>A<N>   c<A>r<5>a<24>R<GLU>A<CB>       2.849   4.628  .  .
    c<A>r<2>a<1>R<SER>A<N>   c<A>r<5>a<26>R<GLU>A<CD>       0.008   4.792  .  .
    c<A>r<2>a<1>R<SER>A<N>   c<A>r<5>a<27>R<GLU>A<OE1>      11.323  3.932  .  .
    c<A>r<2>a<1>R<SER>A<N>   c<A>r<194>a<1501>R<LEU>A<CD2>  0.021   5.465  .  .
    c<A>r<2>a<1>R<SER>A<N>   c<solvent>                     35.440  5.9    .  .
    c<A>r<2>a<2>R<SER>A<CA>  c<A>r<2>a<3>R<SER>A<C>         11.608  1.514  .  .
    c<A>r<2>a<2>R<SER>A<CA>  c<A>r<2>a<4>R<SER>A<O>         0.327   2.405  .  .
    c<A>r<2>a<2>R<SER>A<CA>  c<A>r<2>a<5>R<SER>A<CB>        14.170  1.523  .  .
    c<A>r<2>a<2>R<SER>A<CA>  c<A>r<2>a<6>R<SER>A<OG>        0.820   2.430  .  .
    c<A>r<2>a<2>R<SER>A<CA>  c<A>r<3>a<7>R<LYS>A<N>         3.902   2.371  .  .
    c<A>r<2>a<2>R<SER>A<CA>  c<A>r<5>a<24>R<GLU>A<CB>       0.081   4.954  .  .
    c<A>r<2>a<2>R<SER>A<CA>  c<solvent>                     16.448  6.1    .  .
    
Each atomic descriptor in the above text can be transformed into a space-separated
list (chainID resSeq iCode serial altLoc resName name) using "voronota expand-descriptors" command:

    cat annotated_contacts.txt | voronota expand-descriptors | column -t > expanded_table_of_contacts.txt

Below is a part of some possible "expanded_table_of_contacts.txt":

    A  2  .  1  .  SER  N   A        2    .  2     .  SER  CA   15.908  1.456  .  .
    A  2  .  1  .  SER  N   A        2    .  3     .  SER  C    0.167   2.488  .  .
    A  2  .  1  .  SER  N   A        2    .  4     .  SER  O    7.025   2.774  .  .
    A  2  .  1  .  SER  N   A        2    .  5     .  SER  CB   7.021   2.486  .  .
    A  2  .  1  .  SER  N   A        2    .  6     .  SER  OG   0.624   3.159  .  .
    A  2  .  1  .  SER  N   A        5    .  24    .  GLU  CB   2.849   4.628  .  .
    A  2  .  1  .  SER  N   A        5    .  26    .  GLU  CD   0.008   4.792  .  .
    A  2  .  1  .  SER  N   A        5    .  27    .  GLU  OE1  11.323  3.932  .  .
    A  2  .  1  .  SER  N   A        194  .  1501  .  LEU  CD2  0.021   5.465  .  .
    A  2  .  1  .  SER  N   solvent  .    .  .     .  .    .    35.440  5.9    .  .
    A  2  .  2  .  SER  CA  A        2    .  3     .  SER  C    11.608  1.514  .  .
    A  2  .  2  .  SER  CA  A        2    .  4     .  SER  O    0.327   2.405  .  .
    A  2  .  2  .  SER  CA  A        2    .  5     .  SER  CB   14.170  1.523  .  .
    A  2  .  2  .  SER  CA  A        2    .  6     .  SER  OG   0.820   2.430  .  .
    A  2  .  2  .  SER  CA  A        3    .  7     .  LYS  N    3.902   2.371  .  .
    A  2  .  2  .  SER  CA  A        5    .  24    .  GLU  CB   0.081   4.954  .  .
    A  2  .  2  .  SER  CA  solvent  .    .  .     .  .    .    16.448  6.1    .  .

## Querying annotated inter-atom contacts

The "voronota query-contacts" command can be used to query and manipulate computed annotated contacts.
For example, below is the command that selects contacts that probably correspond to salt bridges:

    cat annotated_contacts.txt | voronota query-contacts --match-first 'R<ASP,GLU>&A<OD1,OD2,OE1,OE2,OXT>' --match-second 'R<ARG,HIS,LYS>&A<NH1,NH2,ND1,NE2,NZ>' --match-max-dist 4.0

The [support/generate-arguments-for-query-contacts.html](support/generate-arguments-for-query-contacts.html) file in the Voronota package
provides a graphical interface for generating sets of arguments for
the "voronota query-contacts" command: it serves as an interactive documentation.
A similar interface for the "voronota query-balls" command is provided by
the [support/generate-arguments-for-query-balls.html](support/generate-arguments-for-query-balls.html) file.

## Getting help in command line

The list of all available Voronota commands is displayed when executing Voronota without any parameters.

Command help is shown when "--help" command line option is present, for example:

    ./voronota calculate-vertices --help

Using "--help" option without specific command results in printing help for all commands:

    ./voronota --help

