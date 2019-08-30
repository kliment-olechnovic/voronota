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
[https://github.com/kliment-olechnovic/voronota/releases](https://github.com/kliment-olechnovic/voronota/releases).

The archive contains ready-to-use statically compiled 'voronota' program for
64 bit Linux systems. This executable can be rebuilt from the provided
source code to work on any modern Linux, Mac OS X or Windows operating systems.

On Ubuntu 18.04 and newer it is possible to install Voronota using 'apt' command:

    sudo apt install voronota


# Building from source code

## Using C++ compiler directly

Voronota has no required external dependencies, only
a standard-compliant C++ compiler is needed to build it.

For example, "voronota" executable can be built from
the sources in "src" directory using GNU C++ compiler:

    g++ -O3 -o voronota $(find ./src/ -name '*.cpp')

## Using make

Inside the unpacked downloaded Voronota archive,
you can generate and use a makefile:

    ./configure
    make

It allows to install and uninstall Voronota executables:

    make install
    make uninstall

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

## Enabling OpenMP

To allow the usage of OpenMP when calling the "calculate-vertices-in-parallel" command,
the "-fopenmp" flag needs to be set when building.

When building using C++ compiler directly, just add "-fopenmp":

    g++ -O3 -fopenmp -o voronota $(find ./src/ -name '*.cpp')

When generating a makefile with the "configure" script,
set the CXXFLAGS variable:

    ./configure CXXFLAGS="-O3 -fopenmp"
    make

When using CMake, set the CMAKE_CXX_FLAGS variable:

    cmake -DCMAKE_CXX_FLAGS="-fopenmp" ./
    make

## Enabling MPI

To allow the usage of MPI when calling the "calculate-vertices-in-parallel" command,
you can use mpic++ compiler wrapper and define "ENABLE_MPI" macro when buiding:

    mpic++ -O3 -DENABLE_MPI -o voronota ./$(find ./src/ -name '*.cpp')

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

# Command reference
## List of all commands

* get-balls-from-atoms-file
* calculate-vertices
* calculate-vertices-in-parallel
* calculate-contacts
* query-balls
* query-contacts
* draw-contacts
* score-contacts-energy
* score-contacts-quality
* score-contacts-potential
* compare-contacts
* write-balls-to-atoms-file
* query-balls-clashes
* expand-descriptors

## Command 'get-balls-from-atoms-file'

### Command line arguments:

    Name                            Type        Description
    ------------------------------- ------ ---- ------------------------------------------------------------------------
    --annotated                                 flag to enable annotated mode
    --include-heteroatoms                       flag to include heteroatoms
    --include-hydrogens                         flag to include hydrogen atoms
    --multimodel-chains                         flag to read multiple models in PDB format and rename chains accordingly
    --input-format                  string      input format, variants are: 'pdb' (default), 'mmcif', 'detect'
    --radii-file                    string      path to radii configuration file
    --default-radius                number      default atomic radius
    --only-default-radius                       flag to make all radii equal to the default radius
    --hull-offset                   number      positive offset distance enables adding artificial hull balls
    --help                                      flag to print usage help to stdout and exit

### Input stream:

    file in PDB or mmCIF format

### Output stream:

    list of balls

        default mode line format: 'x y z r # atomSerial chainID resSeq resName atomName altLoc iCode'

        annotated mode line format: 'annotation x y z r tags adjuncts'

## Command 'calculate-vertices'

### Command line arguments:

    Name                            Type        Description
    ------------------------------- ------ ---- ------------------------------------------------------------------------
    --print-log                                 flag to print log of calculations
    --exclude-hidden-balls                      flag to exclude hidden input balls
    --include-surplus-quadruples                flag to include surplus quadruples
    --link                                      flag to output links between vertices
    --init-radius-for-BSH           number      initial radius for bounding sphere hierarchy
    --check                                     flag to slowly check the resulting vertices (used only for testing)
    --help                                      flag to print usage help to stdout and exit

### Input stream:

    list of balls (line format: 'x y z r')

### Output stream:

    list of Voronoi vertices, i.e. quadruples with tangent spheres (line format: 'q1 q2 q3 q4 x y z r')

## Command 'calculate-vertices-in-parallel'

### Command line arguments:

    Name                            Type        Description
    ------------------------------- ------ ---- ------------------------------------------------------------------------
    --method                        string   *  parallelization method name, variants are: 'simulated'
    --parts                         number   *  number of parts for splitting, must be power of 2
    --print-log                                 flag to print log of calculations
    --include-surplus-quadruples                flag to include surplus quadruples
    --link                                      flag to output links between vertices
    --init-radius-for-BSH           number      initial radius for bounding sphere hierarchy
    --help                                      flag to print usage help to stdout and exit

### Input stream:

    list of balls (line format: 'x y z r')

### Output stream:

    list of Voronoi vertices, i.e. quadruples with tangent spheres (line format: 'q1 q2 q3 q4 x y z r')

## Command 'calculate-contacts'

### Command line arguments:

    Name                            Type        Description
    ------------------------------- ------ ---- ------------------------------------------------------------------------
    --annotated                                 flag to enable annotated mode
    --probe                         number      probe radius
    --exclude-hidden-balls                      flag to exclude hidden input balls
    --step                          number      curve step length
    --projections                   number      curve optimization depth
    --sih-depth                     number      spherical surface optimization depth
    --add-mirrored                              flag to add mirrored contacts to non-annnotated output
    --draw                                      flag to output graphics for annotated contacts
    --tag-centrality                            flag to tag contacts centrality
    --tag-peripherial                           flag to tag peripherial contacts
    --old-contacts-output           string      file path to output spherical contacts
    --volumes-output                string      file path to output constrained cells volumes
    --help                                      flag to print usage help to stdout and exit

### Input stream:

    list of balls

        default mode line format: 'x y z r'

        annotated mode line format: 'annotation x y z r tags adjuncts'

### Output stream:

    list of contacts

        default mode line format: 'b1 b2 area'

        annotated mode line format: 'annotation1 annotation2 area distance tags adjuncts [graphics]'

## Command 'query-balls'

### Command line arguments:

    Name                            Type        Description
    ------------------------------- ------ ---- ------------------------------------------------------------------------
    --match                         string      selection
    --match-not                     string      negative selection
    --match-tags                    string      tags to match
    --match-tags-not                string      tags to not match
    --match-adjuncts                string      adjuncts intervals to match
    --match-adjuncts-not            string      adjuncts intervals to not match
    --match-external-annotations    string      file path to input matchable annotations
    --invert                                    flag to invert selection
    --whole-residues                            flag to select whole residues
    --drop-atom-serials                         flag to drop atom serial numbers from input
    --drop-altloc-indicators                    flag to drop alternate location indicators from input
    --drop-tags                                 flag to drop all tags from input
    --drop-adjuncts                             flag to drop all adjuncts from input
    --set-tags                      string      set tags instead of filtering
    --set-dssp-info                 string      file path to input DSSP file
    --set-adjuncts                  string      set adjuncts instead of filtering
    --set-external-adjuncts         string      file path to input external adjuncts
    --set-external-adjuncts-name    string      name for external adjuncts
    --rename-chains                             flag to rename input chains to be in interval from 'A' to 'Z'
    --guess-chain-names                         flag to assign input chain names based on residue numbering
    --renumber-from-adjunct         string      adjunct name to use for input residue renumbering
    --renumber-positively                       flag to increment residue numbers to make them positive
    --reset-serials                             flag to reset atom serial numbers
    --set-seq-pos-adjunct                       flag to set normalized sequence position adjunct
    --set-ref-seq-num-adjunct       string      file path to input reference sequence
    --ref-seq-alignment             string      file path to output alignment with reference
    --seq-output                    string      file path to output query result sequence string
    --chains-summary-output         string      file path to output chains summary
    --chains-seq-identity           number      sequence identity threshold for chains summary, default is 0.9
    --help                                      flag to print usage help to stdout and exit

### Input stream:

    list of balls (line format: 'annotation x y z r tags adjuncts')

### Output stream:

    list of balls (line format: 'annotation x y z r tags adjuncts')

## Command 'query-contacts'

### Command line arguments:

    Name                            Type        Description
    ------------------------------- ------ ---- ------------------------------------------------------------------------
    --match-first                   string      selection for first contacting group
    --match-first-not               string      negative selection for first contacting group
    --match-second                  string      selection for second contacting group
    --match-second-not              string      negative selection for second contacting group
    --match-min-seq-sep             number      minimum residue sequence separation
    --match-max-seq-sep             number      maximum residue sequence separation
    --match-min-area                number      minimum contact area
    --match-max-area                number      maximum contact area
    --match-min-dist                number      minimum distance
    --match-max-dist                number      maximum distance
    --match-tags                    string      tags to match
    --match-tags-not                string      tags to not match
    --match-adjuncts                string      adjuncts intervals to match
    --match-adjuncts-not            string      adjuncts intervals to not match
    --match-external-first          string      file path to input matchable annotations
    --match-external-second         string      file path to input matchable annotations
    --match-external-pairs          string      file path to input matchable annotation pairs
    --no-solvent                                flag to not include solvent accessible areas
    --no-same-chain                             flag to not include same chain contacts
    --invert                                    flag to invert selection
    --drop-tags                                 flag to drop all tags from input
    --drop-adjuncts                             flag to drop all adjuncts from input
    --set-tags                      string      set tags instead of filtering
    --set-hbplus-tags               string      file path to input HBPLUS file
    --set-distance-bins-tags        string      list of distance thresholds
    --inter-residue-hbplus-tags                 flag to set inter-residue H-bond tags
    --set-adjuncts                  string      set adjuncts instead of filtering
    --set-external-adjuncts         string      file path to input external adjuncts
    --set-external-adjuncts-name    string      name for external adjuncts
    --set-external-means            string      file path to input external values for averaging
    --set-external-means-name       string      name for external means
    --renaming-map                  string      file path to input atoms renaming map
    --inter-residue                             flag to convert input to inter-residue contacts
    --inter-residue-after                       flag to convert output to inter-residue contacts
    --summing-exceptions            string      file path to input inter-residue summing exceptions annotations
    --summarize                                 flag to output only summary of matched contacts
    --summarize-by-first                        flag to output only summary of matched contacts by first identifier
    --preserve-graphics                         flag to preserve graphics in output
    --help                                      flag to print usage help to stdout and exit

### Input stream:

    list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts [graphics]')

### Output stream:

    list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts [graphics]')

## Command 'draw-contacts'

### Command line arguments:

    Name                            Type        Description
    ------------------------------- ------ ---- ------------------------------------------------------------------------
    --drawing-for-pymol             string      file path to output drawing as pymol script
    --drawing-for-jmol              string      file path to output drawing as jmol script
    --drawing-for-scenejs           string      file path to output drawing as scenejs script
    --drawing-name                  string      graphics object name for drawing output
    --default-color                 string      default color for drawing output, in hex format, white is 0xFFFFFF
    --adjunct-gradient              string      adjunct name to use for gradient-based coloring
    --adjunct-gradient-blue         number      blue adjunct gradient value
    --adjunct-gradient-red          number      red adjunct gradient value
    --adjuncts-rgb                              flag to use RGB color values from adjuncts
    --random-colors                             flag to use random color for each drawn contact
    --alpha                         number      alpha opacity value for drawing output
    --use-labels                                flag to use labels in drawing if possible
    --help                                      flag to print usage help to stdout and exit

### Input stream:

    list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts graphics')

### Output stream:

    list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts graphics')

## Command 'score-contacts-energy'

### Command line arguments:

    Name                            Type        Description
    ------------------------------- ------ ---- ------------------------------------------------------------------------
    --potential-file                string   *  file path to input potential values
    --ignorable-max-seq-sep         number      maximum residue sequence separation for ignorable contacts
    --inter-atom-scores-file        string      file path to output inter-atom scores
    --atom-scores-file              string      file path to output atom scores
    --depth                         number      neighborhood normalization depth
    --help                                      flag to print usage help to stdout and exit

### Input stream:

    list of contacts (line format: 'annotation1 annotation2 conditions area')

### Output stream:

    global scores

## Command 'score-contacts-quality'

### Command line arguments:

    Name                            Type        Description
    ------------------------------- ------ ---- ------------------------------------------------------------------------
    --default-mean                  number      default mean parameter
    --default-sd                    number      default standard deviation parameter
    --means-and-sds-file            string      file path to input atomic mean and sd parameters
    --mean-shift                    number      mean shift in standard deviations
    --external-weights-file         string      file path to input external weights for global scoring
    --smoothing-window              number      window to smooth residue quality scores along sequence
    --atom-scores-file              string      file path to output atom scores
    --residue-scores-file           string      file path to output residue scores
    --help                                      flag to print usage help to stdout and exit

### Input stream:

    list of atom energy descriptors

### Output stream:

    weighted average local score

## Command 'score-contacts-potential'

### Command line arguments:

    Name                            Type        Description
    ------------------------------- ------ ---- ------------------------------------------------------------------------
    --input-file-list                           flag to read file list from stdin
    --input-contributions           string      file path to input contact types contributions
    --input-fixed-types             string      file path to input fixed types
    --input-seq-pairs-stats         string      file path to input sequence pairings statistics
    --potential-file                string      file path to output potential values
    --probabilities-file            string      file path to output observed and expected probabilities
    --single-areas-file             string      file path to output single type total areas
    --contributions-file            string      file path to output contact types contributions
    --multiply-areas                number      coefficient to multiply output areas
    --toggling-list                 string      list of toggling subtags
    --help                                      flag to print usage help to stdout and exit

### Input stream:

    list of contacts (line format: 'annotation1 annotation2 conditions area')

### Output stream:

    line of contact type area summaries (line format: 'annotation1 annotation2 conditions area')

## Command 'compare-contacts'

### Command line arguments:

    Name                            Type        Description
    ------------------------------- ------ ---- ------------------------------------------------------------------------
    --target-contacts-file          string   *  file path to input target contacts
    --inter-atom-scores-file        string      file path to output inter-atom scores
    --inter-residue-scores-file     string      file path to output inter-residue scores
    --atom-scores-file              string      file path to output atom scores
    --residue-scores-file           string      file path to output residue scores
    --depth                         number      local neighborhood depth
    --smoothing-window              number      window to smooth residue scores along sequence
    --smoothed-scores-file          string      file path to output smoothed residue scores
    --ignore-residue-names                      flag to consider just residue numbers and ignore residue names
    --residue-level-only                        flag to output only residue-level results
    --detailed-output                           flag to enable detailed output
    --chains-renaming-file          string      file path to input chains renaming
    --remap-chains                              flag to calculate optimal chains remapping
    --remap-chains-log                          flag output remapping progress to stderr
    --remapped-chains-file          string      file path to output calculated chains remapping
    --help                                      flag to print usage help to stdout and exit

### Input stream:

    list of model contacts (line format: 'annotation1 annotation2 area')

### Output stream:

    global scores (atom-level and residue-level)

## Command 'write-balls-to-atoms-file'

### Command line arguments:

    Name                            Type        Description
    ------------------------------- ------ ---- ------------------------------------------------------------------------
    --pdb-output                    string      file path to output query result in PDB format
    --pdb-output-b-factor           string      name of adjunct to output as B-factor in PDB format
    --pdb-output-template           string      file path to input template for B-factor insertions
    --add-chain-terminators                     flag to add TER lines after chains
    --help                                      flag to print usage help to stdout and exit

### Input stream:

    list of balls (line format: 'annotation x y z r tags adjuncts')

### Output stream:

    list of balls (line format: 'annotation x y z r tags adjuncts')

## Command 'query-balls-clashes'

### Command line arguments:

    Name                            Type        Description
    ------------------------------- ------ ---- ------------------------------------------------------------------------
    --clash-distance                number      clash distance threshold in angstroms, default is 3.0
    --min-seq-sep                   number      minimum residue sequence separation, default is 2
    --init-radius-for-BSH           number      initial radius for bounding sphere hierarchy
    --help                                      flag to print usage help to stdout and exit

### Input stream:

    list of balls (line format: 'annotation x y z r')

### Output stream:

    list of clashes (line format: 'annotation1 annotation2 distance min-distance-between-balls')

## Command 'expand-descriptors'

### Command line arguments:

    Name                            Type        Description
    ------------------------------- ------ ---- ------------------------------------------------------------------------
    --help                                      flag to print usage help to stdout and exit

### Input stream:

    any text containing atom descriptors

### Output stream:

    text with each atom descriptor expanded to 'chainID resSeq iCode serial altLoc resName name'

# Wrapper scripts

## VoroMQA method script

'voronota-voromqa' script is an implementation of VoroMQA (Voronoi diagram-based Model Quality Assessment) method using Voronota.
The script interface is presented below:
    
    Basic options:
        --input | -i                   string      input structure file in PDB or mmCIF format
        --input-filter-query           string      input atoms filtering query parameters
        --output-atom-scores           string      output text file with atom scores
        --output-atom-scores-pdb       string      output PDB file with atom scores as B-factors
        --output-residue-scores        string      output text file with residue scores
        --output-residue-scores-pdb    string      output PDB file with residue scores as B-factors
        --output-residue-scores-plot   string      output PNG image file with residue scores plot, requires R
        --help | -h                                flag to display help message and exit
    
    Advanced options:
        --cache-dir                    string      path to cache directory
        --smoothing-window             number      residue scores smoothing window size, default is 5
        --atoms-query                  string      atoms query parameters to define selection
        --contacts-query               string      contacts query parameters to define selection
        --output-selected-scores       string      output text file with selected atom scores
        --reference-sequence           string      sequence file or string for residue renumbering
        --output-sequence-alignment    string      output text file with sequence alignment
        --multiple-models                          flag to handle multiple models in PDB file
        --list-all-options                         flag to display list of all command line options and exit
    
    Standard output (one line):
        {input file path} {global score} {number of residues} {number of atoms} [ {selection score} {number of selected atoms} ]
    

## CAD-score method script

'voronota-cadscore' script is an implementation of CAD-score (Contact Area Difference score) method using Voronota.
The script interface is presented below:
    
    Basic options:
        --input-target | -t             string   *  input target structure file in PDB or mmCIF format
        --input-model | -m              string   *  input model structure file in PDB format
        --input-filter-query            string      input atoms filtering query parameters
        --filter-model-by-target                    flag to filter model residues by the set of target residue IDs
        --output-residue-scores         string      output text file with residue scores
        --output-residue-scores-pdb-t   string      output target PDB file with residue scores as B-factors
        --output-residue-scores-pdb-m   string      output model PDB file with residue scores as B-factors
        --smoothing-window              number      residue scores smoothing window size, default is 0
        --contacts-query                string      contacts query parameters
        --contacts-query-inter-chain                flag to consider only inter-chain contacts
        --contacts-query-by-code        string      contacts query code, possible codes are AA, AS, SS, AM, MM, MS
        --use-all-query-codes                       flag to output global scores for all query codes, one line per code
        --cache-dir                     string      path to cache directory
        --output-header                             flag to output header before result line
        --help | -h                                 flag to display help message and exit
    
    Advanced options:
        --ignore-residue-names                      flag to consider just residue numbers and ignore residue names
        --enable-site-based-scoring                 falg to enable site-based scoring
        --multiple-models                           flag to handle multiple models in PDB file as an assembly
        --input-model-chains-renaming               input text file with renaming rules for chains
        --remap-chains                              flag to automatically rearrange chain names for higher scores
        --remap-chains-output           string      output file with chain names rearrangement
        --old-regime                                flag to calculate areas as in pre-Voronota CAD-score
    
    Standard output (one line):
        {target file path} {model file path} {query code} {number of residues} {global score} {target total area} {corresponding model total area}
        [ {site-based number of residues} {site-based global score} {site-based target total area} {corresponding site-based model total area} ]
    

## Contacts calculation convenience script

'voronota-contacts' script provides a way for calculating and querying interatomic contacts with just one command (without the need to construct a pipeline from 'voronota' calls).
    
    Basic options:
        --input | -i                   string   *  input structure file in PDB or mmCIF format
        --input-filter-query           string      input atoms filtering query parameters
        --contacts-query               string      contacts query parameters
        --contacts-query-additional    string      additional contacts query parameters
        --cache-dir                    string      path to cache directory
        --sum-at-end                               flag to print sum of areas as the last line in output
        --help | -h                                flag to display help message and exit
    
    Advanced options:
        --output-drawing               string      output file with drawing script for PyMol
        --drawing-parameters           string      drawing parameters
        --wireframe-drawing                        flag to draw wireframe representation of contacts
        --multiple-models                          flag to handle multiple models in PDB file
    
    Standard output (multiple lines):
        {contacting atom} {contacting atom} {contact area} {distance between atoms centers} {tags} {adjunct values}
    

## Volumes calculation convenience script

'voronota-volumes' script provides a way for calculating and querying atomic volumes with just one command (without the need to construct a pipeline from 'voronota' calls).
    
    Basic options:
        --input | -i                   string   *  input structure file in PDB or mmCIF format
        --input-filter-query           string      input atoms filtering query parameters
        --cache-dir                    string      path to cache directory
        --sum-at-end                               flag to print sum of areas as the last line in output
        --help | -h                                flag to display help message and exit
    
    Advanced options:
        --atoms-query                  string      atoms query parameters
        --per-residue                              flag to output per-residue results
        --multiple-models                          flag to handle multiple models in PDB file
    
    Standard output (multiple lines):
        {name} {volume}
    
