# About Voronota-LT

Voronota-LT (pronounced 'voronota lite') is an alternative version of [Voronota](../README.md) for constructing tessellation-derived atomic contact areas and volumes.
Voronota-LT was written from scratch and does not use any external code, even from the core Voronota.
The primary motivation for creating Voronota-LT was drastically increasing the speed of computing tessellation-based atom-atom contact areas and atom solvent-accessible surface areas.

Like Voronota, Voronota-LT can compute contact areas derived from the additively weighted Voronoi tessellation,
but the main increase in speed comes when utilizing a simpler, radical tessellation variant, also known as Laguerre-Laguerre tessellation or power diagram.
This is the default tessellation variant in Voronota-LT. It considers radii of atoms together with the rolling probe radius to define radical planes as bisectors between atoms.

Voronota-LT is distributed as an expansion part of [the Voronota software package](../README.md),
mainly to enable other Voronota expansions to easily use the Voronota-LT library.

The core functionality of Voronota-LT is also available via the [Voronota-LT web application](./web/index.html) built using Emscripten.

## Benchmarking data and results

Benchmarking data and results are available [here](./benchmark/README.md).

# Quickest install guide

Since Voronota-LT version 1.0.1, universal binary execuitables of Voronota-LT
built with the [Cosmopolitan Libc toolkit](https://github.com/jart/cosmopolitan) are provided.

To download and prepare the latest released cosmopolitan executable, run the following commands in a shell environment (e.g. a Bash shell):

```bash
wget 'https://github.com/kliment-olechnovic/voronota/releases/download/v1.29.4723/cosmopolitan_voronota-lt_v1.1.491.exe'
mv cosmopolitan_voronota-lt_v1.1.491.exe voronota-lt
chmod +x voronota-lt
```

In case of a PowerShell environment in Windows 8, the setup can be done with a single command:

```bash
Invoke-WebRequest -Uri 'https://github.com/kliment-olechnovic/voronota/releases/download/v1.29.4723/cosmopolitan_voronota-lt_v1.1.491.exe' -OutFile voronota-lt.exe
```

# Quick install guide

Please refer to the [core Voronota quick install guide](../README.md#quick-install-guide).

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


    Voronota-LT version 1.1
    
    'voronota-lt' executable constructs a radical Voronoi tessellation (also known as a Laguerre-Voronoi diagram or a power diagram)
    of atomic balls of van der Waals radii constrained inside a solvent-accessible surface defined by a rolling probe.
    The software computes inter-atom contact areas, per-cell solvent accessible surface areas, per-cell constrained volumes.
    'voronota-lt' is very fast when used on molecular data with a not large rolling probe radius (less than 2.0 angstroms, 1.4 is recommended)
    and can be made even faster by running it using multiple processors.
    
    Options:
        --probe                                          number     rolling probe radius, default is 1.4
        --processors                                     number     maximum number of OpenMP threads to use, default is 2 if OpenMP is enabled, 1 if disabled
        --compute-only-inter-residue-contacts                       flag to only compute inter-residue contacts, turns off per-cell summaries
        --compute-only-inter-chain-contacts                         flag to only compute inter-chain contacts, turns off per-cell summaries
        --run-in-aw-diagram-regime                                  flag to run construct a simplified additively weighted Voronoi diagram, turns off per-cell summaries
        --input | -i                                     string     input file path to use instead of standard input, or '_stdin' to still use standard input
        --periodic-box-directions                        numbers    coordinates of three vectors (x1 y1 z1 x2 y2 z2 x3 y3 z3) to define and use a periodic box
        --periodic-box-corners                           numbers    coordinates of two corners (x1 y1 z1 x2 y2 z2) to define and use a periodic box
        --pdb-or-mmcif-exclude-heteroatoms                          flag to exclude heteroatoms when reading input in PDB or mmCIF format
        --pdb-or-mmcif-include-hydrogens                            flag to include hydrogen atoms when reading input in PDB or mmCIF format
        --pdb-or-mmcif-join-models                                  flag to join multiple models into an assembly when reading input in PDB or mmCIF format
        --pdb-or-mmcif-radii-config-file                 string     input file path for reading atom radii assignment rules
        --grouping-directives                            string     string with grouping directives separated by ';'
        --grouping-directives-file                       string     input file path for grouping directives
        --restrict-input-atoms                           string     selection expression to restrict input balls
        --restrict-contacts                              string     selection expression to restrict contacts before construction
        --restrict-contacts-for-output                   string     selection expression to restrict contacts for output
        --restrict-atom-descriptors-for-output           string     selection expression to restrict single-index data (balls, cells, sites) for output
        --print-contacts                                            flag to print table of contacts to stdout
        --print-contacts-residue-level                              flag to print table of residue-level grouped contacts to stdout
        --print-contacts-chain-level                                flag to print table of chain-level grouped contacts to stdout
        --print-cells                                               flag to print table of per-cell summaries to stdout
        --print-cells-residue-level                                 flag to print table of residue-level grouped per-cell summaries to stdout
        --print-cells-chain-level                                   flag to print table of chain-level grouped per-cell summaries to stdout
        --print-sites                                               flag to print table of binding site summaries to stdout
        --print-sites-residue-level                                 flag to print table of residue-level grouped binding site summaries to stdout
        --print-sites-chain-level                                   flag to print table of chain-level grouped binding site summaries to stdout
        --print-everything                                          flag to print everything to stdout, terminate if printing everything is not possible
        --write-input-balls-to-file                                 output file path to write input balls to file
        --write-contacts-to-file                         string     output file path to write table of contacts
        --write-contacts-residue-level-to-file           string     output file path to write table of residue-level grouped contacts
        --write-contacts-chain-level-to-file             string     output file path to write table of chain-level grouped contacts
        --write-cells-to-file                            string     output file path to write table of per-cell summaries
        --write-cells-residue-level-to-file              string     output file path to write table of residue-level grouped per-cell summaries
        --write-cells-chain-level-to-file                string     output file path to write table of chain-level grouped per-cell summaries
        --write-sites-to-file                            string     output file path to write table of binding site summaries
        --write-sites-residue-level-to-file              string     output file path to write table of residue-level grouped binding site summaries
        --write-sites-chain-level-to-file                string     output file path to write table of chain-level grouped binding site summaries
        --write-tessellation-edges-to-file               string     output file path to write generating IDs and lengths of SAS-constrained tessellation edges
        --write-tessellation-vertices-to-file            string     output file path to write generating IDs and positions of SAS-constrained tessellation vertices
        --write-raw-collisions-to-file                   string     output file path to write a table of both true (contact) and false (no contact) collisions
        --plot-contacts-to-file                          string     output file path to write SVG plot of contacts
        --plot-contacts-residue-level-to-file            string     output file path to write SVG plot of residue-level grouped contacts
        --plot-contacts-chain-level-to-file              string     output file path to write SVG plot of chain-level grouped contacts
        --plot-config-flags                              strings    space-separated list of plotting flags, e.g. ylabeled xlabeled gradient dark compact
        --graphics-output-file-for-pymol                 string     output file path to write contacts drawing script for PyMol
        --graphics-output-file-for-chimera               string     output file path to write contacts drawing script for Chimera or ChimeraX
        --graphics-title                                 string     title to use for the graphics objects generated by the contacts drawing script
        --graphics-restrict-representations              strings    space-separated list of representations to output, e.g.: balls faces wireframe sas sasmesh lattice
        --graphics-coloring-config                       string     string with graphics coloring rules separated by ';'
        --graphics-coloring-config-file                  string     input file path for reading graphics coloring rules
        --sites-view-script-for-pymol                    string     output file path to write sites view script for PyMol
        --sites-view-script-for-chimera                  string     output file path to write sites view script for ChimeraX
        --mesh-output-obj-file                           string     output file path to write contacts surfaces mesh .obj file
        --mesh-print-topology-summary                               flag to print mesh topology summary
        --measure-running-time                                      flag to measure and output running times
        --write-log-to-file                              string     output file path to write global log, does not turn off printing log to stderr
        --force-icode-column                                        flag to always add insertion code column to output tables even if there are no insertion codes
        --extra-columns                                             flag to add extra columns with empty identifier parts to output tables
        --quiet | -q                                                flag to suppress printing non-error log messages to stderr
        --help | -h                                                 flag to print help info to stderr and exit
    
    Standard input stream:
        Several input formats are supported:
          a) Space-separated or tab-separated header-less table of balls, one of the following line formats possible:
                 x y z radius
                 chainID x y z radius
                 chainID residueID x y z radius
                 chainID residueID atomName x y z radius
                 chainID residueNum residueName atomName x y z radius
                 chainID residueNum iCode residueName atomName x y z radius
          b) Output of 'voronota get-balls-from-atoms-file' is acceptable, where line format is:
                 x y z radius # atomSerial chainID resSeq resName atomName altLoc iCode
          c) PDB file
          d) mmCIF file
    
    Standard output stream:
        Requested tables with headers, with column values tab-separated
    
    Standard error output stream:
        Log (a name-value pair line), error messages
    
    Usage examples:
    
        cat ./2zsk.pdb | voronota-lt --print-contacts
    
        voronota-lt -i ./2zsk.pdb --print-contacts
    
        voronota-lt --input ./2zsk.pdb --print-contacts-residue-level --compute-only-inter-residue-contacts
    
        voronota-lt --input ./balls.xyzr --processors 8 --write-contacts-to-file ./contacts.tsv --write-cells-to-file ./cells.tsv
    
        voronota-lt -i ./balls.xyzr --probe 2 --periodic-box-corners 0 0 0 100 100 300 --processors 8 --write-cells-to-file ./cells.tsv
    

## Example of a command to write main basic descriptors to files

The following example command outputs main basic descriptors into separate files:

```bash
./voronota-lt \
  --probe 1.4 \
  --input ./input.pdb \
  --write-contacts-to-file ./interatomic_contacts.tsv \
  --write-cells-to-file ./atomic_cells.tsv \
  --write-tessellation-vertices-to-file ./tessellation_vertices.tsv
```

The main acceptable input file formats are:

* PDB file
* mmCIF file
* Space-separated or tab-separated header-less table of balls, one of the following line formats possible:

```
x y z radius
chainID x y z radius
chainID residueID x y z radius
chainID residueID atomName x y z radius
chainID residueID atomName x y z radius
chainID residueNum residueName atomName x y z radius
chainID residueNum iCode residueName atomName x y z radius
```

The output file `interatomic_contacts.tsv` has named columns, below is an example of its first 10 lines:

```
ia_header  ID1_chain  ID1_rnum  ID1_rname  ID1_atom  ID2_chain  ID2_rnum  ID2_rname  ID2_atom  ID1_index  ID2_index  area     arc_legth  distance
ia         A          1         MET        N         A          1         MET        CA        0          1          16.5137  6.06483    1.4962
ia         A          1         MET        N         A          1         MET        CB        0          4          8.78012  6.97738    2.47926
ia         A          1         MET        N         A          1         MET        C         0          2          2.26495  1.78545    2.51605
ia         A          1         MET        N         A          1         MET        O         0          3          5.95355  5.4563     2.86488
ia         A          1         MET        CA        A          1         MET        CB        1          4          14.3565  1.04322    1.53664
ia         A          1         MET        CA        A          1         MET        C         1          2          9.42943  0.82315    1.53635
ia         A          1         MET        CA        A          2         LYS        N         1          5          5.50647  1.01891    2.43603
ia         A          1         MET        CA        A          224       ALA        O         1          1783       2.13187  0          3.58016
ia         A          1         MET        CA        A          226       GLU        OXT       1          1800       5.78086  3.06435    4.80279
```

The output file `atomic_cells.tsv` has named columns, below is an example of its first 10 lines:

```
ac_header  ID_chain  ID_rnum  ID_rname  ID_atom  ID_index  sas_area   volume
ac         A         1        MET       N        0         53.3426    63.8989
ac         A         1        MET       CA       1         4.70418    30.7944
ac         A         1        MET       C        2         2.62416    15.6203
ac         A         1        MET       O        3         11.1833    27.3368
ac         A         1        MET       CB       4         27.5103    75.9687
ac         A         2        LYS       N        5         2.52533    14.778
ac         A         2        LYS       CA       6         2.06606    23.2376
ac         A         2        LYS       C        7         0.0340018  12.7911
ac         A         2        LYS       O        8         1.12595    15.7616
```

The output file `tessellation_vertices.tsv` has unnamed columns,
the values in every row are the following:

("atom index 1", "atom index 2", "atom index 3", "atom index 4", "Voronoi vertex position x coordinate", "Voronoi vertex position y coordinate", "Voronoi vertex position z coordinate", "minimum distance to the surface of any relevant atom ball", "maximum distance to the surface of any relevant atom ball")

The value of "atom index 4" (and in some cases "atom index 3") can be equal -1, indicating that the Voronoi vertex is not inside the solvent accessible surface.
Below is an example of the first 10 lines of `tessellation_vertices.tsv`:

```
0  1  2  3     21.3073  56.4249  13.7346  -1.49455  -1.18217
0  1  2  -1    20.1445  57.7992  11.0521  0         0
0  1  3  4     22.217   55.5682  14.8128  -0.61821  -0.52342
0  1  4  -1    24.4023  55.03    10.3079  0         0
0  2  3  -1    20.1775  58.3249  12.7279  0         0
0  3  4  -1    22.597   55.6138  15.4236  0         0
1  2  3  4     21.523   54.9988  14.4133  -1.26086  -1.01695
1  2  4  5     20.4112  53.4852  12.286   -1.41883  -1.26735
1  2  5  -1    19.8423  57.283   10.4901  0         0
1  4  5  1783  20.4614  53.2701  11.6814  -1.352    -1.08078
```

Note about the atom indices - they correspond to the ordering of the atoms in the input.
They also can be extracted from the "ID_index" column of the `atomic_cells.tsv` output file.
Optionally, to be able check what atoms were read from the input in PDB or mmCIF formats,
the input atomic balls can be written to a file using the `--write-input-balls-to-file` option.


# Filtering (selection) system

Some optional arguments of Voronota-LT expect selection expressions:

```
--restrict-input-atoms                           string     selection expression to restrict input balls
--restrict-contacts                              string     selection expression to restrict contacts before construction
--restrict-contacts-for-output                   string     selection expression to restrict contacts for output
--restrict-atom-descriptors-for-output           string     selection expression to restrict single-index data (balls, cells, sites) for output
```

The expressions need to be specified using the filtering (selection) language described in this section.
The language is used to select
**atoms** or atom-based descriptors (like **cells** and **sites**) and **contacts**.

The language supports explicit boolean logic with round brackets to avoid
any ambiguity in operator precedence.
It is recommended to always use round brackets when combining filters with logical operators.
Even when redundant, over-bracketing makes expressions easier to read and safer for users unfamiliar with precedence rules.

## General syntax

### Filter blocks

A filter block is written in square brackets:

```
[ <clause> <clause> ... ]
```

- Clauses are space-separated.
- Clause arguments are single tokens.
- Lists use commas (`,`).
- Numeric ranges use colons (`:`).

### Boolean expressions

Filter blocks can be combined into boolean expressions.

Supported operators:

| Logical meaning | Accepted forms      |
|-----------------|---------------------|
| AND             | `and`, `&`, `&&`    |
| OR              | `or`, `|`, `||`     |
| NOT             | `not`, `!`          |

Round brackets `(` `)` may be nested freely and are encouraged.

### Examples

```
[ ... ]
```

```
( [ ... ] and [ ... ] )
```

```
( ( [ ... ] ) or ( not [ ... ] ) )
```

```
( ( ( [ ... ] and [ ... ] ) or ( [ ... ] ) ) and ( not [ ... ] ) )
```

The same boolean expression syntax applies to **atom selection** and
**contact selection**.

## Atom (or atom-based descriptor) selection

Atom filters match **atoms** based on chain, residue, atom name, element,
and biochemical category.

### Chain selection

```
-chain <list>        or  -c <list>
-chain-not <list>    or  -c! <list>
```

Examples:

```
[ -chain A ]
[ -c A,B ]
[ -c! C ]
```

### Residue number (ranges use ':')

```
-residue-number <intervals>        or  -rnum <intervals>
-residue-number-not <intervals>    or  -rnum! <intervals>
```

Examples:

```
[ -rnum 42 ]
[ -rnum 10:20 ]
[ -rnum! 1:5 ]
```

### Residue name

```
-residue-name <list>        or  -rname <list>
-residue-name-not <list>    or  -rname! <list>
```

Examples:

```
[ -rname ALA ]
[ -rname ALA,GLY,SER ]
[ -rname! PRO ]
```

### Atom name

```
-atom-name <list>        or  -aname <list>
-atom-name-not <list>    or  -aname! <list>
```

Examples:

```
[ -aname CA ]
[ -aname CA,CB ]
[ -aname! H ]
```

### Element

```
-element <list>        or  -elem <list>
-element-not <list>    or  -elem! <list>
```

Examples:

```
[ -elem C ]
[ -elem N,O ]
[ -elem! H ]
```

### Residue ID (combined identifier)


```
-residue-id <list>        or  -rid <list>
-residue-id-not <list>   or  -rid! <list>
```

Supported formats:

- `42` where `42` is the residue number
- `42/A`, where `A` is the insertion code
- `42|ALA`,  where `ALA` is the residue name
- `42/A|ALA`

Examples:

```
[ -rid 42|ALA ]
[ -rid 101/B ]
```

### Protein-specific macros

These predefined macros expand internally to standard residue and atom sets.

| Macro                 | Description                   |
|-----------------------|-------------------------------|
| `-protein`            | All standard protein residues |
| `-protein-backbone`   | Backbone atoms (N, CA, C, O)  |
| `-protein-sidechain`  | Side-chain atoms              |

Examples:

```
[ -protein ]
[ -protein-backbone ]
[ -protein-sidechain ]
```

### Nucleic-acid–specific macros

These macros select standard nucleic-acid residues and optionally restrict
atoms to backbone or base (side-chain) atoms.

| Macro                     | Description                                   |
|---------------------------|-----------------------------------------------|
| `-nucleic`                | All nucleic acids (DNA and RNA)               |
| `-nucleic-dna`            | DNA residues only                             |
| `-nucleic-rna`            | RNA residues only                             |
| `-nucleic-backbone`       | Sugar-phosphate backbone atoms (DNA or RNA)   |
| `-nucleic-sidechain`      | Base atoms (DNA or RNA)                       |
| `-nucleic-dna-backbone`   | DNA backbone atoms                            |
| `-nucleic-dna-sidechain`  | DNA base atoms                                |
| `-nucleic-rna-backbone`   | RNA backbone atoms                            |
| `-nucleic-rna-sidechain`  | RNA base atoms                                |

Conceptually:

- Backbone = phosphate + sugar atoms
- Sidechain = nucleobase atoms

### Atom-selection examples

ALA CB atom:

```
[ -rname ALA -aname CB ]
```

C-alpha atoms in chain A:

```
[ -chain A -aname CA ]
```

Protein side-chain atoms in residues 50–100, excluding PRO:

```
( [ -protein-sidechain -rnum 50:100 ] ) and ( not [ -rname PRO ] )
```

## Contact selection

Contact filters match a pair of atom groups (`atom1`, `atom2`) and optional
relationship constraints.

### Selecting atom1 and atom2

Positive forms:

```
-atom1 <atom-filter>   or  -a1 <atom-filter>
-atom2 <atom-filter>   or  -a2 <atom-filter>
```

Negation forms:

```
-atom1-not <atom-filter>   or  -a1!
-atom2-not <atom-filter>   or  -a2!
```

Example:

```
[ -a1 [ -rname ALA -aname CB ] -a2 [ -protein-backbone ] ]
```

### Chain and residue relationships

| Clause           | Meaning                       |
|------------------|-------------------------------|
| `-inter-chain`   | Atoms from different chains   |
| `-intra-chain`   | Atoms from the same chain     |
| `-inter-residue` | Atoms from different residues |

Examples:

```
[ -inter-chain ]
[ -inter-residue ]
```

### Sequence separation (same chain)

```
-min-sep <int>
-max-sep <int>
```

Examples:

```
[ -min-sep 5 ]
[ -max-sep 10 ]
```

### Distance constraint

```
-max-dist <float>
```

Example:

```
[ -max-dist 3.5 ]
```

### Contact-selection examples

ALA CB with protein backbone:

```
[ -a1 [ -rname ALA -aname CB ] -a2 [ -protein-backbone ] ]
```

Inter-chain protein–protein contacts:

```
[ -a1 [ -protein ] -a2 [ -protein ] -inter-chain ]
```

Short side-chain contacts:

```
( [ -a1 [ -protein-sidechain ] -a2 [ -protein-sidechain ] -max-dist 3.5 ] )
```

Complex example:

```
(
  (
    ( [ -a1 [ -aname CA ] -a2 [ -protein-sidechain ] ] )
    and
    ( [ -inter-residue ] )
  )
  or
  ( [ -max-dist 3.0 ] )
)
```


# Using Voronota-LT as a C++ library

## Stateless C++ API

Voronota-LT can be used as a header-only C++ library.
The needed headers are all in "./src/voronotalt" folder.
The only header file needed to be included is "voronotalt.h".

Below is a detailed example for both basic and periodic box modes:

```cpp
    #include <iostream>

    #include "voronotalt.h" // assuming that the "voronotalt" directory is in the include path

    //user-defined structure for a ball
    struct Ball
    {
        Ball(const double x, const double y, const double z, const double r) : x(x), y(y), z(z), r(r) {}

        double x;
        double y;
        double z;
        double r;
    };

    //user-defined structure for a contact descriptor
    struct Contact
    {
        Contact() : index_a(0), index_b(0), area(0.0), arc_length(0.0) {}

        int index_a;
        int index_b;
        double area;
        double arc_length;
    };

    //user-defined structure for a cell descriptor
    struct Cell
    {
        Cell() : index(0), sas_area(0.0), volume(0.0), included(false) {}

        int index;
        double sas_area;
        double volume;
        bool included;
    };

    //user-defined structure for a point, to define optonal periodic box corners
    struct Point
    {
        Point(const double x, const double y, const double z) : x(x), y(y), z(z) {}

        double x;
        double y;
        double z;
    };

    //user-defined function that uses voronotalt::RadicalTessellation to fill vectors of contact and cell descriptors
    bool compute_contact_and_cell_descriptors_with_optional_periodic_box_conditions(
            const std::vector<Ball>& balls,
            const double probe,
            const std::vector<Point>& periodic_box_corners,
            std::vector<Contact>& contacts,
            std::vector<Cell>& cells)
    {
        contacts.clear();
        cells.clear();

        if(balls.empty())
        {
            std::cerr << "No balls to compute the tessellation for." << std::endl;
            return false;
        }

        if(!periodic_box_corners.empty() && periodic_box_corners.size()<2)
        {
            std::cerr << "Invalid number of provided periodic box corners, there must be either none or more than one corners." << std::endl;
            return false;
        }

        // computing Voronota-LT radical tessellation results
        voronotalt::RadicalTessellation::Result result;
        voronotalt::RadicalTessellation::construct_full_tessellation(
                voronotalt::get_spheres_from_balls(balls, probe),
                voronotalt::PeriodicBox::create_periodic_box_from_corners(voronotalt::get_simple_points_from_points(periodic_box_corners)),
                result);

        if(result.contacts_summaries.empty())
        {
            std::cerr << "No contacts constructed for the provided balls and probe." << std::endl;
            return false;
        }

        if(result.cells_summaries.empty())
        {
            std::cerr << "No cells constructed for the provided balls and probe.";
            return false;
        }

        // using the result data about contacts
        contacts.resize(result.contacts_summaries.size());
        for(std::size_t i=0;i<result.contacts_summaries.size();i++)
        {
            contacts[i].index_a=result.contacts_summaries[i].id_a;
            contacts[i].index_b=result.contacts_summaries[i].id_b;
            contacts[i].area=result.contacts_summaries[i].area;
            contacts[i].arc_length=result.contacts_summaries[i].arc_length;
        }

        // using the result data about cells
        cells.resize(balls.size());
        for(std::size_t i=0;i<result.cells_summaries.size();i++)
        {
            cells[i].index=static_cast<int>(i);
            if(result.cells_summaries[i].stage>0)
            {
                cells[i].sas_area=result.cells_summaries[i].sas_area;
                cells[i].volume=result.cells_summaries[i].sas_inside_volume;
                cells[i].included=true;
            }
        }

        return true;
    }

    //user-defined convenience function that redirects to the previously defined function with an empty vector of periodic box corners
    bool compute_contact_and_cell_descriptors(
            const std::vector<Ball>& balls,
            const double probe,
            std::vector<Contact>& contacts,
            std::vector<Cell>& cells)
    {
        return compute_contact_and_cell_descriptors_with_optional_periodic_box_conditions(balls, probe, std::vector<Point>(), contacts, cells);
    }

    //user-defined function to print input balls
    void print_balls(const std::vector<Ball>& balls)
    {
        std::cout << "balls:\n";
        for(std::size_t i=0;i<balls.size();i++)
        {
            const Ball& ball=balls[i];
            std::cout << "ball " << i << " " << ball.x << " " << ball.y << " " << ball.z << " " << ball.r << "\n";
        }
        std::cout << "\n";
    }

    //user-defined function to print resulting contacts and cells
    void print_contacts_and_cells(const std::vector<Contact>& output_contacts, const std::vector<Cell>& output_cells)
    {
        std::cout << "contacts:\n";
        for(const Contact& contact : output_contacts)
        {
            std::cout << "contact " << contact.index_a << " " << contact.index_b << " " << contact.area << " " << contact.arc_length << "\n";
        }
        std::cout << "\n";

        std::cout << "cells:\n";
        for(const Cell& cell : output_cells)
        {
            if(cell.included)
            {
                std::cout << "cell " << cell.index << " " << cell.sas_area << " " << cell.volume << "\n";
            }
        }
        std::cout << "\n";
    }

    int main(const int, const char**)
    {
        std::vector<Ball> input_balls;

        input_balls.push_back(Ball(0, 0, 2, 1));
        input_balls.push_back(Ball(0, 1, 0, 0.5));
        input_balls.push_back(Ball(0.382683, 0.92388, 0, 0.5));
        input_balls.push_back(Ball(0.707107, 0.707107, 0, 0.5));
        input_balls.push_back(Ball(0.92388, 0.382683, 0, 0.5));
        input_balls.push_back(Ball(1, 0, 0, 0.5));
        input_balls.push_back(Ball(0.92388, -0.382683, 0, 0.5));
        input_balls.push_back(Ball(0.707107, -0.707107, 0, 0.5));
        input_balls.push_back(Ball(0.382683, -0.92388, 0, 0.5));
        input_balls.push_back(Ball(0, -1, 0, 0.5));
        input_balls.push_back(Ball(-0.382683, -0.92388, 0, 0.5));
        input_balls.push_back(Ball(-0.707107, -0.707107, 0, 0.5));
        input_balls.push_back(Ball(-0.92388, -0.382683, 0, 0.5));
        input_balls.push_back(Ball(-1, 0, 0, 0.5));
        input_balls.push_back(Ball(-0.92388, 0.382683, 0, 0.5));
        input_balls.push_back(Ball(-0.707107, 0.707107, 0, 0.5));
        input_balls.push_back(Ball(-0.382683, 0.92388, 0, 0.5));

        std::cout << "Input:\n\n";

        print_balls(input_balls);

        const double probe=1.0;

        {
            std::cout << "Output in basic mode:\n\n";

            std::vector<Contact> output_contacts;
            std::vector<Cell> output_cells;

            if(compute_contact_and_cell_descriptors(input_balls, probe, output_contacts, output_cells))
            {
                print_contacts_and_cells(output_contacts, output_cells);
            }
            else
            {
                std::cerr << "Failed to compute contact and cell descriptors in basic mode." << std::endl;
                return 1;
            }
        }

        {
            std::cout << "Output in periodic box mode:\n\n";

            std::vector<Point> periodic_box_corners;
            periodic_box_corners.push_back(Point(-1.6, -1.6, -0.6));
            periodic_box_corners.push_back(Point(1.6, 1.6, 3.1));

            std::vector<Contact> output_contacts;
            std::vector<Cell> output_cells;

            if(compute_contact_and_cell_descriptors_with_optional_periodic_box_conditions(input_balls, probe, periodic_box_corners, output_contacts, output_cells))
            {
                print_contacts_and_cells(output_contacts, output_cells);
            }
            else
            {
                std::cerr << "Failed to compute contact and cell descriptors in periodic box mode." << std::endl;
                return 1;
            }
        }

        return 0;
    }

```

## Stateless C++ API additional features

In addition to calculatig contacts and cell descriptors,
Voronota-LT can be used to compute and output vertices of the Voronoi cells
constrained inside the solvent-accessible surface.

Below is a detailed example for both basic and periodic box modes.
It calls the ``voronotalt::RadicalTessellation::construct_full_tessellation`` function that computes contacts, cell descriptors, and cell vertices.
In this example cell vertices are being outputted, because outputting contacts and cell descritors is showcased in the previous example.


```cpp
    #include <iostream>

    #include "voronotalt.h" // assuming that the "voronotalt" directory is in the include path

    //user-defined structure for a ball
    struct Ball
    {
        Ball(const double x, const double y, const double z, const double r) : x(x), y(y), z(z), r(r) {}

        double x;
        double y;
        double z;
        double r;
    };

    //user-defined structure for a point, used for both defining a periodic box and for recording cell vertices
    struct Point
    {
        Point() : x(0), y(0), z(0) {}

        Point(const double x, const double y, const double z) : x(x), y(y), z(z) {}

        double x;
        double y;
        double z;
    };

    //user-defined structure for a cell vertex
    struct CellVertex
    {
        CellVertex() {}

        int ball_indices[4];
        Point position;

        //checks if the vertex lies on the solvent accessible surface
        bool is_on_sas() const
        {
            return (ball_indices[3]<0);
        }
    };

    //user-defined function that uses voronotalt::RadicalTessellation to fill a vectors cell vertices
    bool compute_cell_vertices_with_optional_periodic_box_conditions(
            const std::vector<Ball>& balls,
            const double probe,
            const std::vector<Point>& periodic_box_corners,
            std::vector<CellVertex>& cell_vertices)
    {
        cell_vertices.clear();

        if(balls.empty())
        {
            std::cerr << "No balls to compute the tessellation for." << std::endl;
            return false;
        }

        if(!periodic_box_corners.empty() && periodic_box_corners.size()<2)
        {
            std::cerr << "Invalid number of provided periodic box corners, there must be either none or more than one corners." << std::endl;
            return false;
        }

        // computing Voronota-LT radical tessellation results
        voronotalt::RadicalTessellation::Result result;
        voronotalt::RadicalTessellation::construct_full_tessellation(
                voronotalt::get_spheres_from_balls(balls, probe),
                voronotalt::PeriodicBox::create_periodic_box_from_corners(voronotalt::get_simple_points_from_points(periodic_box_corners)),
                true,
                result);

        if(result.tessellation_net.tes_vertices.empty())
        {
            std::cerr << "No tessellation cell vertices constructed for the provided balls and probe." << std::endl;
            return false;
        }

        // using the result data about tessellation cell vertices
        cell_vertices.resize(result.tessellation_net.tes_vertices.size());
        for(std::size_t i=0;i<result.tessellation_net.tes_vertices.size();i++)
        {
            const voronotalt::RadicalTessellationContactConstruction::TessellationVertex& tv=result.tessellation_net.tes_vertices[i];
            CellVertex& cv=cell_vertices[i];
            for(int j=0;j<4;j++)
            {
                if(tv.ids_of_spheres[j]==voronotalt::null_id())
                {
                    cv.ball_indices[j]=-1;
                }
                else
                {
                    cv.ball_indices[j]=static_cast<int>(tv.ids_of_spheres[j]);
                }
            }
            cv.position=Point(tv.position.x, tv.position.y, tv.position.z);
        }

        return true;
    }

    //user-defined convenience function that redirects to the previously defined function with an empty vector of periodic box corners
    bool compute_cell_vertices(
            const std::vector<Ball>& balls,
            const double probe,
            std::vector<CellVertex>& cell_vertices)
    {
        return compute_cell_vertices_with_optional_periodic_box_conditions(balls, probe, std::vector<Point>(), cell_vertices);
    }

    //user-defined function to print input balls
    void print_balls(const std::vector<Ball>& balls)
    {
        std::cout << "balls:\n";
        for(std::size_t i=0;i<balls.size();i++)
        {
            const Ball& ball=balls[i];
            std::cout << "ball index=" << i << " center=(" << ball.x << ", " << ball.y << ", " << ball.z << ") radius=" << ball.r << "\n";
        }
        std::cout << "\n";
    }

    //user-defined function to print resulting contacts and cells
    void print_cell_vertices(const std::vector<CellVertex>& output_cell_vertices)
    {
        std::cout << "tessellation cell vertices:\n";
        for(const CellVertex& cv : output_cell_vertices)
        {
            std::cout << "cell_vertex " << (cv.is_on_sas() ? "on_SAS" : "not_on_SAS");
            std::cout << " ball_indices=(" << cv.ball_indices[0] << ", " << cv.ball_indices[1] << ", " << cv.ball_indices[2] << ", " << cv.ball_indices[3] << ")";
            std::cout << " position=(" << cv.position.x << ", " << cv.position.y << ", " << cv.position.z << ")\n";
        }
        std::cout << "\n";
    }

    int main(const int, const char**)
    {
        std::vector<Ball> input_balls;

        input_balls.push_back(Ball(0, 0, 2, 1));
        input_balls.push_back(Ball(0, 1, 0, 0.5));
        input_balls.push_back(Ball(0.382683, 0.92388, 0, 0.5));
        input_balls.push_back(Ball(0.707107, 0.707107, 0, 0.5));
        input_balls.push_back(Ball(0.92388, 0.382683, 0, 0.5));
        input_balls.push_back(Ball(1, 0, 0, 0.5));
        input_balls.push_back(Ball(0.92388, -0.382683, 0, 0.5));
        input_balls.push_back(Ball(0.707107, -0.707107, 0, 0.5));
        input_balls.push_back(Ball(0.382683, -0.92388, 0, 0.5));
        input_balls.push_back(Ball(0, -1, 0, 0.5));
        input_balls.push_back(Ball(-0.382683, -0.92388, 0, 0.5));
        input_balls.push_back(Ball(-0.707107, -0.707107, 0, 0.5));
        input_balls.push_back(Ball(-0.92388, -0.382683, 0, 0.5));
        input_balls.push_back(Ball(-1, 0, 0, 0.5));
        input_balls.push_back(Ball(-0.92388, 0.382683, 0, 0.5));
        input_balls.push_back(Ball(-0.707107, 0.707107, 0, 0.5));
        input_balls.push_back(Ball(-0.382683, 0.92388, 0, 0.5));

        std::cout << "Input:\n\n";

        print_balls(input_balls);

        const double probe=1.0;

        {
            std::cout << "Output in basic mode:\n\n";

            std::vector<CellVertex> output_cell_vertices;

            if(compute_cell_vertices(input_balls, probe, output_cell_vertices))
            {
                print_cell_vertices(output_cell_vertices);
            }
            else
            {
                std::cerr << "Failed to compute tessellation cell vertices in basic mode." << std::endl;
                return 1;
            }
        }

        {
            std::cout << "Output in periodic box mode:\n\n";

            std::vector<Point> periodic_box_corners;
            periodic_box_corners.push_back(Point(-1.6, -1.6, -0.6));
            periodic_box_corners.push_back(Point(1.6, 1.6, 3.1));

            std::vector<CellVertex> output_cell_vertices;

            if(compute_cell_vertices_with_optional_periodic_box_conditions(input_balls, probe, periodic_box_corners, output_cell_vertices))
            {
                print_cell_vertices(output_cell_vertices);
            }
            else
            {
                std::cerr << "Failed to compute tessellation cell vertices in periodic box mode." << std::endl;
                return 1;
            }
        }

        return 0;
    }

```

## Stateful C++ API for updatable tessellation

In addition to the static functions-based stateless API,
Voronota-LT header-only C++ library also provides a stateful class for constructing and updating a radical Voronoi tessellation.
The needed headers are all in "./src/voronotalt" folder.
The only header file needed to be included is "voronotalt.h".

Below is a detailed example:

```cpp
    #include <iostream>

    #include "voronotalt.h" // assuming that the "voronotalt" directory is in the include path

    //user-defined function to print input spheres
    void print_spheres(const std::vector<voronotalt::SimpleSphere>& spheres)
    {
        std::cout << "spheres (sphere id x y z r):\n";
        for(std::size_t i=0;i<spheres.size();i++)
        {
            const voronotalt::SimpleSphere& sphere=spheres[i];
            std::cout << "sphere "<< i << " " << sphere.p.x << " " << sphere.p.y << " " << sphere.p.z << " " << sphere.r << "\n";
        }
        std::cout << "\n";
    }

    //user-defined function to print tessellation result contacts and cells
    void print_tessellation_result_contacts_and_cells(const voronotalt::UpdateableRadicalTessellation::Result& result)
    {
        std::cout << "contacts (contact id_a id_b area arc_length):\n";
        for(std::size_t i=0;i<result.contacts_summaries.size();i++)
        {
            for(std::size_t j=0;j<result.contacts_summaries[i].size();j++)
            {
                const voronotalt::RadicalTessellation::ContactDescriptorSummary& contact=result.contacts_summaries[i][j];
                if(contact.id_a==i)
                {
                    std::cout << "contact " << contact.id_a << " " << contact.id_b << " " << contact.area << " " << contact.arc_length << "\n";
                }
            }
        }
        std::cout << "\n";

        std::cout << "cells (cell id area volume):\n";
        for(std::size_t i=0;i<result.cells_summaries.size();i++)
        {
            const voronotalt::RadicalTessellation::CellContactDescriptorsSummary& cell=result.cells_summaries[i];
            std::cout << "cell " << i << " " << cell.sas_area << " " << cell.sas_inside_volume << "\n";
        }
        std::cout << "\n";
    }

    //user-defined function to print tessellation result summary
    void print_tessellation_result_summary(const voronotalt::UpdateableRadicalTessellation::ResultSummary& result_summary)
    {
        std::cout << "result_summary (summary contacts_area contacts_count cells_sas_area cells_volume):\n";
        std::cout << "summary " << result_summary.total_contacts_summary.area << " " << result_summary.total_contacts_summary.count << " ";
        std::cout << result_summary.total_cells_summary.sas_area << " " << result_summary.total_cells_summary.sas_inside_volume << "\n";
        std::cout << "\n";
    }

    int main(const int, const char**)
    {
        //Input raw balls

        std::vector<voronotalt::SimpleSphere> input_spheres;

        input_spheres.push_back(voronotalt::SimpleSphere(0, 0, 2, 1));
        input_spheres.push_back(voronotalt::SimpleSphere(0, 1, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(0.382683, 0.92388, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(0.707107, 0.707107, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(0.92388, 0.382683, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(1, 0, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(0.92388, -0.382683, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(0.707107, -0.707107, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(0.382683, -0.92388, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(0, -1, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(-0.382683, -0.92388, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(-0.707107, -0.707107, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(-0.92388, -0.382683, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(-1, 0, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(-0.92388, 0.382683, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(-0.707107, 0.707107, 0, 0.5));
        input_spheres.push_back(voronotalt::SimpleSphere(-0.382683, 0.92388, 0, 0.5));

        //Prepare input spheres by augmenting the radii of the raw balls

        const double probe=1.0;

        for(std::size_t i=0;i<input_spheres.size();i++)
        {
            input_spheres[i].r+=probe;
        }

        //Print prepared input spheres

        std::cout << "Input:\n\n";

        print_spheres(input_spheres);

        //Initialize a periodic box description

        std::vector<voronotalt::SimplePoint> periodic_box_corners;
        periodic_box_corners.push_back(voronotalt::SimplePoint(-1.6, -1.6, -0.6));
        periodic_box_corners.push_back(voronotalt::SimplePoint(1.6, 1.6, 3.1));

        //Initialize an updateable tessellation controller object with automatic backup enabled

        const bool backup_enabled=true;
        voronotalt::UpdateableRadicalTessellation updateable_tessellation(backup_enabled);

        //Compute a tessellation from the input spheres

        if(updateable_tessellation.init(input_spheres, voronotalt::PeriodicBox::create_periodic_box_from_corners(periodic_box_corners)))
        {
            std::cout << "Initialized tessellation." << std::endl;
        }
        else
        {
            std::cerr << "Failed to construct tessellation." << std::endl;
            return 1;
        }

        //Save the tessellation result summary after init

        std::vector<voronotalt::UpdateableRadicalTessellation::ResultSummary> result_summaries;

        result_summaries.push_back(updateable_tessellation.result_summary());

        //Print the tessellation results

        std::cout << "\nResults after init:\n\n";

        print_tessellation_result_contacts_and_cells(updateable_tessellation.result());

        //Iteratively change the input spheres and update the tessellation

        for(int n=1;n<=5;n++)
        {
            //Specify the updated indices of spheres

            std::vector<voronotalt::UnsignedInt> ids_to_update;
            ids_to_update.push_back(0);
            ids_to_update.push_back(1);

            //Update the coordinated of the chosen input spheres

            for(const voronotalt::UnsignedInt& id : ids_to_update)
            {
                input_spheres[id].p.x+=0.1;
            }

            //Update the tessellation

            if(updateable_tessellation.update(input_spheres, ids_to_update))
            {
                std::cout << "Updated tessellation." << std::endl;
            }
            else
            {
                std::cerr << "Failed to update tessellation." << std::endl;
                return 1;
            }

            //Save the tessellation result summary after update

            result_summaries.push_back(updateable_tessellation.result_summary());
        }

        //Print the tessellation results

        std::cout << "\nResults after last update:\n\n";

        print_tessellation_result_contacts_and_cells(updateable_tessellation.result());

        //Print all the save tessellation result summaries

        std::cout << "\nResult summaries for all stages:\n\n";

        for(std::size_t i=0;i<result_summaries.size();i++)
        {
            const voronotalt::UpdateableRadicalTessellation::ResultSummary& rs=result_summaries[i];
            print_tessellation_result_summary(rs);
        }

        //Restore the tessellation from the last backup, i.e. cancel the last update

        if(updateable_tessellation.restore_from_backup())
        {
            //Print the tessellation result summary after restoring the tessellation

            std::cout << "\nResult summary after restoring from backup:\n\n";
            print_tessellation_result_summary(updateable_tessellation.result_summary());
        }
        else
        {
            std::cerr << "Results were not restored from backup because ";
            if(updateable_tessellation.in_sync_with_backup())
            {
                std::cerr << "results are already in sync with backup";
            }
            else
            {
                std::cerr << "backup was not enabled";
            }
            std::cerr << std::endl;
        }

        return 0;
    }

```

# Voronota-LT Python bindings

The Voronota-LT Python interface PyPI package is hosted at [https://pypi.org/project/voronotalt/](https://pypi.org/project/voronotalt/).

## Installation

Install with pip using this command:

```bash
pip install voronotalt
```

## Basic usage examples

### Basic usage example. generic

Voronota-LT can be used in Python code as in the following example:

```py
import voronotalt

balls = []
balls.append(voronotalt.Ball(0, 0, 2, 1))
balls.append(voronotalt.Ball(0, 1, 0, 0.5))
balls.append(voronotalt.Ball(0.38268343236509, 0.923879532511287, 0, 0.5))
balls.append(voronotalt.Ball(0.707106781186547, 0.707106781186548, 0, 0.5))
balls.append(voronotalt.Ball(0.923879532511287, 0.38268343236509, 0, 0.5))
balls.append(voronotalt.Ball(1, 0, 0, 0.5))
balls.append(voronotalt.Ball(0.923879532511287, -0.38268343236509, 0, 0.5))
balls.append(voronotalt.Ball(0.707106781186548, -0.707106781186547, 0, 0.5))
balls.append(voronotalt.Ball(0.38268343236509, -0.923879532511287, 0, 0.5))
balls.append(voronotalt.Ball(0, -1, 0, 0.5))
balls.append(voronotalt.Ball(-0.38268343236509, -0.923879532511287, 0, 0.5))
balls.append(voronotalt.Ball(-0.707106781186547, -0.707106781186548, 0, 0.5))
balls.append(voronotalt.Ball(-0.923879532511287, -0.38268343236509, 0, 0.5))
balls.append(voronotalt.Ball(-1, 0, 0, 0.5))
balls.append(voronotalt.Ball(-0.923879532511287, 0.38268343236509, 0, 0.5))
balls.append(voronotalt.Ball(-0.707106781186548, 0.707106781186547, 0, 0.5))
balls.append(voronotalt.Ball(-0.38268343236509, 0.923879532511287, 0, 0.5))

for i, ball in enumerate(balls):
    print(f"ball {i} {ball.x:.4f} {ball.y:.4f} {ball.z:.4f} {ball.r:.4f}");

rt = voronotalt.RadicalTessellation(balls, probe=1.0)

contacts=list(rt.contacts)

print("contacts:")

for contact in contacts:
    print(f"contact {contact.index_a} {contact.index_b} {contact.area:.4f} {contact.arc_length:.4f}")

cells=list(rt.cells)

print("cells:")

for i, cell in enumerate(cells):
    print(f"cell {i} {cell.sas_area:.4f} {cell.volume:.4f}");

```

### Basic usage example with converting output to pandas data frames

If the [pandas](https://pandas.pydata.org/) library for data analysis is available in the Python environment,
then the tessellation computation results can also be converted to [pandas data frames](https://pandas.pydata.org/docs/reference/frame.html):

```py
rt = voronotalt.RadicalTessellation(balls, probe=1.0)

df_balls = rt.balls.to_pandas()
df_contacts = rt.contacts.to_pandas()
df_cells = rt.cells.to_pandas()

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_balls)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_contacts)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_cells)

```

To run this example, make sure you have installed pandas:

```bash
pip install pandas
```

### Basic usage example with Biotite to provide input

If [Biotite](https://www.biotite-python.org/) is available in the Python environment,
the Voronota-LT can be used in Python code with Biotite as in the following example:

```py
import voronotalt
import biotite.structure.io

structure = biotite.structure.io.load_structure("./input/assembly_1ctf.cif")

rt = voronotalt.RadicalTessellation.from_biotite_atoms(structure, include_heteroatoms=False, probe=1.4)

print("contacts:")

for contact in rt.contacts:
    if contact.index_a<5:
        print(f"contact {contact.index_a} {contact.index_b} {contact.area:.4f} {contact.arc_length:.4f}")

cells=list(rt.cells)

print("cells:")

for i, cell in enumerate(cells[:20]):
    print(f"cell {i} {cell.sas_area:.4f} {cell.volume:.4f}");

```

To run this example, make sure you have installed Biotite:

```bash
pip install biotite
```

## Biomolecules-focused usage examples

### Biomolecules-focused usage example, generic

Since version 1.0.1, the Voronota-LT Python bindings contain special classes and functions for processing biological macromolecules.
They folow the interface of the Voronota-LT command line software interface.
The main class is `MolecularRadicalTessellation`, a more biomolecules-focused counterpart of the basic `RadicalTessellation` class.
Below is an example of using `MolecularRadicalTessellation`:

```py
import voronotalt

mrt = voronotalt.MolecularRadicalTessellation.from_file(
    input_file="./input/assembly_1ctf.pdb1",
    read_as_assembly=True,
    restrict_contacts_for_output="[-a1 [-chain A] -a2 [-chain A2]]",
    restrict_cells_for_output="[-chain A]"
)

print("inter_residue_contacts:")

for contact in mrt.inter_residue_contact_summaries:
    print(f"ir_contact {contact.ID1_chain} {contact.ID1_residue_seq_number} {contact.ID1_residue_name} {contact.ID2_chain} {contact.ID2_residue_seq_number} {contact.ID2_residue_name} {contact.area:.4f}");

print("residue_cells:")

for cell in mrt.residue_cell_summaries:
    print(f"r_cell {cell.ID_chain} {cell.ID_residue_seq_number} {cell.ID_residue_name} {cell.sas_area:.4f} {cell.volume:.4f}");

print("inter_chain_contacts:")

for contact in mrt.inter_chain_contact_summaries:
    print(f"ic_contact {contact.ID1_chain} {contact.ID2_chain} {contact.area:.4f}");

print("chain_cells:")

for cell in mrt.chain_cell_summaries:
    print(f"c_cell {cell.ID_chain} {cell.sas_area:.4f} {cell.volume:.4f}");

mrt = voronotalt.MolecularRadicalTessellation.from_file(
    input_file="./input/assembly_1ctf.cif",
    record_everything_possible=False,
    record_inter_residue_contact_summaries=True,
    record_inter_chain_contact_summaries=True,
    record_chain_cell_summaries=True,
    restrict_contacts_for_output="[-a1 [-chain A] -a2! [-chain A]]"
)

print("inter_residue_contacts:")

for contact in mrt.inter_residue_contact_summaries:
    print(f"ir_contact {contact.ID1_chain} {contact.ID1_residue_seq_number} {contact.ID1_residue_name} {contact.ID2_chain} {contact.ID2_residue_seq_number} {contact.ID2_residue_name} {contact.area:.4f}");

print("inter_chain_contacts:")

for contact in mrt.inter_chain_contact_summaries:
    print(f"ic_contact {contact.ID1_chain} {contact.ID2_chain} {contact.area:.4f}");

print("chain_cells:")

for cell in mrt.chain_cell_summaries:
    print(f"c_cell {cell.ID_chain} {cell.sas_area:.4f} {cell.volume:.4f}");

```

### Biomolecules-focused usage example with converting output to pandas data frames

Similarly to `RadicalTessellation`, the `MolecularRadicalTessellation` allows
converting the tessellation computation results to [pandas data frames](https://pandas.pydata.org/docs/reference/frame.html)
if the [pandas](https://pandas.pydata.org/) library for data analysis is available in the Python environment.
Below is an example that prints heads of different output data frames that came from the `MolecularRadicalTessellation` object:

```py
import voronotalt

mrt = voronotalt.MolecularRadicalTessellation.from_file(
    input_file="./input/assembly_1ctf.pdb1",
    read_as_assembly=True,
    restrict_contacts_for_output="[-a1 [-chain A] -a2 [-chain A2]]",
    restrict_cells_for_output="[-chain A]"
)

df_atoms = mrt.atom_balls.to_pandas()
df_inter_atom_contacts = mrt.inter_atom_contact_summaries.to_pandas()
df_inter_residue_contacts = mrt.inter_residue_contact_summaries.to_pandas()
df_inter_chain_contacts = mrt.inter_chain_contact_summaries.to_pandas()
df_atom_cells = mrt.atom_cell_summaries.to_pandas()
df_residue_cells = mrt.residue_cell_summaries.to_pandas()
df_chain_cells = mrt.chain_cell_summaries.to_pandas()

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_atoms)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_inter_atom_contacts)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_inter_residue_contacts)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_inter_chain_contacts)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_atom_cells)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_residue_cells)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_chain_cells)

```

To run this example, make sure you have installed pandas:

```bash
pip install pandas
```

### Biomolecules-focused usage example with Biotite to provide input

If [Biotite](https://www.biotite-python.org/) is available in the Python environment,
`MolecularRadicalTessellation` can use [biotite.structure](https://www.biotite-python.org/latest/apidoc/biotite.structure.html) for input:

```py
import voronotalt
import biotite.structure.io

structure = biotite.structure.io.load_structure("./input/assembly_1ctf.cif")

mrt = voronotalt.MolecularRadicalTessellation.from_biotite_atoms(structure, include_heteroatoms=False)

print("inter_residue_contacts:")

for contact in mrt.inter_residue_contact_summaries:
    print(f"ir_contact {contact.ID1_chain} {contact.ID1_residue_seq_number} {contact.ID1_residue_name} {contact.ID2_chain} {contact.ID2_residue_seq_number} {contact.ID2_residue_name} {contact.area:.4f}");

print("residue_cells:")

for cell in mrt.residue_cell_summaries:
    print(f"r_cell {cell.ID_chain} {cell.ID_residue_seq_number} {cell.ID_residue_name} {cell.sas_area:.4f} {cell.volume:.4f}");

```

To run this example, make sure you have installed Biotite:

```bash
pip install biotite
```

### Biomolecules-focused usage example with Gemmi to provide input

If [Gemmi](https://gemmi.readthedocs.io/) is available in the Python environment,
`MolecularRadicalTessellation` can use [gemmi.Model](https://project-gemmi.github.io/python-api/gemmi.Model.html) for input:

```py
import voronotalt
import gemmi

structure=gemmi.read_structure("./input/assembly_1ctf.cif")
model=structure[0]

mrt = voronotalt.MolecularRadicalTessellation.from_gemmi_model_atoms(model, include_heteroatoms=False)

print("inter_residue_contacts:")

for contact in mrt.inter_residue_contact_summaries:
    print(f"ir_contact {contact.ID1_chain} {contact.ID1_residue_seq_number} {contact.ID1_residue_name} {contact.ID2_chain} {contact.ID2_residue_seq_number} {contact.ID2_residue_name} {contact.area:.4f}");

print("residue_cells:")

for cell in mrt.residue_cell_summaries:
    print(f"r_cell {cell.ID_chain} {cell.ID_residue_seq_number} {cell.ID_residue_name} {cell.sas_area:.4f} {cell.volume:.4f}");

```

To run this example, make sure you have installed Gemmi:

```bash
pip install gemmi
```

### Biomolecules-focused usage example with Biopython to provide input

If [Biopython](https://biopython.org/) is available in the Python environment,
`MolecularRadicalTessellation` can use Biopython parsing results for input:

```py
import voronotalt
import Bio.PDB

parser = Bio.PDB.MMCIFParser(QUIET=True)
structure = parser.get_structure("id", "./input/assembly_1ctf.cif")
atoms=structure.get_atoms()

mrt = voronotalt.MolecularRadicalTessellation.from_biopython_atoms(atoms, include_heteroatoms=False)

print("inter_residue_contacts:")

for contact in mrt.inter_residue_contact_summaries:
    print(f"ir_contact {contact.ID1_chain} {contact.ID1_residue_seq_number} {contact.ID1_residue_name} {contact.ID2_chain} {contact.ID2_residue_seq_number} {contact.ID2_residue_name} {contact.area:.4f}");

print("residue_cells:")

for cell in mrt.residue_cell_summaries:
    print(f"r_cell {cell.ID_chain} {cell.ID_residue_seq_number} {cell.ID_residue_name} {cell.sas_area:.4f} {cell.volume:.4f}");

```

To run this example, make sure you have installed Biopython:

```bash
pip install biopython
```

### Biomolecules-focused usage example with custom radii

`MolecularRadicalTessellation` can use a configuration file to specify what van der Waals radii
to assign to different atoms based on their names and their residue names:

```py
import voronotalt

voronotalt.configure_molecular_radii_assignment_rules("./input/custom_radii.txt");

mrt = voronotalt.MolecularRadicalTessellation.from_file(
    input_file="./input/assembly_1ctf.pdb1",
    read_as_assembly=True,
    restrict_contacts_for_output="[-a1 [-chain A] -a2 [-chain A2]]",
    restrict_cells_for_output="[-chain A]"
)

df_atoms = mrt.atom_balls.to_pandas()
df_inter_atom_contacts = mrt.inter_atom_contact_summaries.to_pandas()
df_inter_residue_contacts = mrt.inter_residue_contact_summaries.to_pandas()
df_inter_chain_contacts = mrt.inter_chain_contact_summaries.to_pandas()
df_atom_cells = mrt.atom_cell_summaries.to_pandas()
df_residue_cells = mrt.residue_cell_summaries.to_pandas()
df_chain_cells = mrt.chain_cell_summaries.to_pandas()

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_atoms, n=20)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_inter_chain_contacts)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_chain_cells)

```

A custom radii configuration file format is the same as the one used by the standalone Voronota and Voronota-LT software.
An example of a full radii configuration file is [here](https://github.com/kliment-olechnovic/voronota/blob/master/resources/radii).


# Voronota-LT Rust bindings

Thanks to Mikael Lund, there is also Rust interface for Voronota-LT at [https://github.com/mlund/voronota-rs](https://github.com/mlund/voronota-rs).

