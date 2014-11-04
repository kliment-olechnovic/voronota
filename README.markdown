# About Voronota 1.7

The analysis of macromolecular structures often requires
a comprehensive definition of atomic neighborhoods.
Such a definition can be based on the Voronoi diagram of balls,
where each ball represents an atom of some van der Waals radius.
Voronota is a simple software tool for finding all the vertices
of the Voronoi diagram of balls. Such vertices correspond to
the centers of the empty tangent spheres defined by quadruples of balls.
Voronota is especially suitable for processing three-dimensional
structures of biological macromolecules such as proteins and RNA.
Voronota is developed by Kliment Olechnovic (kliment@ibt.lt).


# Getting the latest version

Download the latest archive from
[the official downloads page](https://bitbucket.org/kliment/voronota/downloads).


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

Here is a basic example of computing Voronoi vertices
for a structure in a PDB file:

    ./voronota get-balls-from-atoms-file < input.pdb > balls.txt
    ./voronota calculate-vertices < balls.txt > vertices.txt

The first command reads a PDB file "input.pdb" and outputs a file "balls.txt"
that contains balls corresponding to the atoms in "input.pdb".
The second command reads "balls.txt" and outputs a file "vertices.txt"
that contains a quadruples and empty tangent spheres that correspond
to the vertices of the Voronoi diagram of the input balls.
The formats of "balls.txt" and "vertices.txt" are described in the next section.

## Formats of the generated files

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

    0 1 2 3 27.761102562843565 8.691997298653096 51.553793549752584 -0.169539877389858
    0 1 2 23 28.275245701294317 9.804833656716982 50.131544414709701 0.588818396229702
    0 1 3 1438 24.793076643633416 -3.225565058946097 60.761180096988049 14.047230829473824
    0 1 4 5 28.785921875922050 10.604515846944295 50.721223702985561 0.283777011742720
    0 1 4 1453 30.018509862360325 10.901712470945974 55.386025520055981 1.908826059705324
    0 1 5 23 28.544757823719145 10.254546149917743 50.194182932428276 0.595962029178680


# Important notes

By default, Voronota ignores all heteroatoms and all hydrogen
atoms when reading PDB files. This behavior can be altered using
"--include-heteroatoms" and "--include-hydrogens" command-line options.

Since version 1.2 Voronota also calculates inter-atom contacts and works with
them in various ways. These features are currently not covered in this document,
but can be viewed using built-in usage help options.


# Getting usage help

The list of Voronota commands is displayed when executing Voronota without any parameters.

Command help is shown when "--help" command line option is present, for example:

    ./voronota calculate-vertices --help

Using "--help" option without specific command results in printing help for all commands:

    ./voronota --help
