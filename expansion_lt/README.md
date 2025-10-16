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


    
    Voronota-LT version 0.9.5
    
    'voronota-lt' executable constructs a radical Voronoi tessellation (also known as a Laguerre-Voronoi diagram or a power diagram)
    of atomic balls of van der Waals radii constrained inside a solvent-accessible surface defined by a rolling probe.
    The software computes inter-atom contact areas, per-cell solvent accessible surface areas, per-cell constrained volumes.
    'voronota-lt' is very fast when used on molecular data with a not large rolling probe radius (less than 2.0 angstroms, 1.4 is recommended)
    and can be made even faster by running it using multiple processors.
    
    Options:
        --probe                                          number     rolling probe radius, default is 1.4
        --processors                                     number     maximum number of OpenMP threads to use, default is 2
        --compute-only-inter-residue-contacts                       flag to only compute inter-residue contacts, turns off per-cell summaries
        --compute-only-inter-chain-contacts                         flag to only compute inter-chain contacts, turns off per-cell summaries
        --run-in-aw-diagram-regime                                  flag to run construct a simplified additively weighted Voronoi diagram, turns off per-cell summaries
        --input | -i                                     string     input file path to use instead of standard input, or '_stdin' to still use standard input
        --periodic-box-directions                        numbers    coordinates of three vectors (x1 y1 z1 x2 y2 z2 x3 y3 z3) to define and use a periodic box
        --periodic-box-corners                           numbers    coordinates of two corners (x1 y1 z1 x2 y2 z2) to define and use a periodic box
        --pdb-or-mmcif-heteroatoms                                  flag to include heteroatoms when reading input in PDB or mmCIF format
        --pdb-or-mmcif-hydrogens                                    flag to include hydrogen atoms when reading input in PDB or mmCIF format
        --pdb-or-mmcif-join-models                                  flag to join multiple models into an assembly when reading input in PDB or mmCIF format
        --print-contacts                                            flag to print table of contacts to stdout
        --print-contacts-residue-level                              flag to print residue-level grouped contacts to stdout
        --print-contacts-chain-level                                flag to print chain-level grouped contacts to stdout
        --print-cells                                               flag to print table of per-cell summaries to stdout
        --print-cells-residue-level                                 flag to print residue-level grouped per-cell summaries to stdout
        --print-cells-chain-level                                   flag to print chain-level grouped per-cell summaries to stdout
        --print-everything                                          flag to print everything to stdout, terminate if printing everything is not possible
        --write-input-balls-to-file                                 output file path to write input balls to file
        --write-contacts-to-file                         string     output file path to write table of contacts
        --write-contacts-residue-level-to-file           string     output file path to write residue-level grouped contacts
        --write-contacts-chain-level-to-file             string     output file path to write chain-level grouped contacts
        --write-cells-to-file                            string     output file path to write of per-cell summaries
        --write-cells-residue-level-to-file              string     output file path to write residue-level grouped per-cell summaries
        --write-cells-chain-level-to-file                string     output file path to write chain-level grouped per-cell summaries
        --write-tessellation-edges-to-file               string     output file path to write generating IDs and lengths of SAS-constrained tessellation edges
        --write-tessellation-vertices-to-file            string     output file path to write generating IDs and positions of SAS-constrained tessellation vertices
        --graphics-output-file                           string     output file path to write contacts drawing .py script to run in PyMol
        --graphics-title                                 string     title to use for the graphics objects generated by the contacts drawing script
        --graphics-restrict-representations              strings    space-separated list of representations to output, e.g.: balls faces wireframe xspheres lattice
        --graphics-restrict-chains                       strings    space-separated list of chain IDs to include in the output, e.g.: A B
        --graphics-restrict-chain-pairs                  strings    space-separated list of pairs of chain IDs to include in the output, e.g.: A B A C B C
        --graphics-color-balls                           string     hex-coded color for balls, default is '0x00FFFF'
        --graphics-color-faces                           string     hex-coded color for faces, default is '0xFFFF00'
        --graphics-color-wireframe                       string     hex-coded color for wireframe, default is '0x808080'
        --graphics-color-xspheres                        string     hex-coded color for xspheres (expanded spheres), default is '0x00FF00'
        --graphics-color-lattice                         string     hex-coded color for lattice (periodic boundaries), default is '0x00FF00'
        --mesh-output-obj-file                           string     output file path to write contacts surfaces mesh .obj file
        --mesh-print-topology-summary                               flag to print mesh topology summary
        --measure-running-time                                      flag to measure and output running times
        --write-log-to-file                              string     output file path to write global log, does not turn off printing log to stderr
        --help-full                                                 flag to print full help (for all options) to stderr and exit
        --help | -h                                                 flag to print help (for basic options) to stderr and exit
    
    Standard input stream:
        Several input formats are supported:
          a) Space-separated or tab-separated header-less table of balls, one of the following line formats possible:
                 x y z radius
                 chainID x y z radius
                 chainID residueID x y z radius
                 chainID residueID atomName x y z radius
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

The main acceptable input file formats formats:

* PDB file
* mmCIF file
* Space-separated or tab-separated header-less table of balls, one of the following line formats possible:

```
x y z radius
chainID x y z radius
chainID residueID x y z radius
chainID residueID atomName x y z radius
```

The output file `interatomic_contacts.tsv` has named columns, below is an example of its first 10 lines:

```
ca_header  ID1_chain  ID1_residue  ID1_atom  ID2_chain  ID2_residue  ID2_atom  ID1_index  ID2_index  area     arc_legth  distance
ca         A          1|MET        N         A          1|MET        CA        0          1          16.5137  6.06483    1.4962
ca         A          1|MET        N         A          1|MET        CB        0          4          8.78012  6.97738    2.47926
ca         A          1|MET        N         A          1|MET        C         0          2          2.26495  1.78545    2.51605
ca         A          1|MET        N         A          1|MET        O         0          3          5.95355  5.4563     2.86488
ca         A          1|MET        CA        A          1|MET        CB        1          4          14.3565  1.04322    1.53664
ca         A          1|MET        CA        A          1|MET        C         1          2          9.42943  0.82315    1.53635
ca         A          1|MET        CA        A          2|LYS        N         1          5          5.50647  1.01891    2.43603
ca         A          1|MET        CA        A          224|ALA      O         1          1783       2.13187  0          3.58016
ca         A          1|MET        CA        A          226|GLU      OXT       1          1800       5.78086  3.06435    4.80279
```

The output file `atomic_cells.tsv` has named columns, below is an example of its first 10 lines:

```
sa_header  ID_chain  ID_residue  ID_atom  ID_index  sas_area   volume
sa         A         1|MET       N        0         53.3426    63.8989
sa         A         1|MET       CA       1         4.70418    30.7944
sa         A         1|MET       C        2         2.62416    15.6203
sa         A         1|MET       O        3         11.1833    27.3368
sa         A         1|MET       CB       4         27.5103    75.9687
sa         A         2|LYS       N        5         2.52533    14.778
sa         A         2|LYS       CA       6         2.06606    23.2376
sa         A         2|LYS       C        7         0.0340018  12.7911
sa         A         2|LYS       O        8         1.12595    15.7616
```

The output file `tessellation_vertices.tsv` has unnnamed columns,
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

Voronota-LT Python interface PyPI package is hosted at [https://pypi.org/project/voronotalt/](https://pypi.org/project/voronotalt/).

## Installation

Install with pip using this command:

```bash
pip install voronotalt
```

## Usage example, basic

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

rt = voronotalt.RadicalTessellation(balls, probe=1.0)

contacts=list(rt.contacts)

print("contacts:")

for contact in contacts:
    print("contact", contact.index_a, contact.index_b, contact.area, contact.arc_length);

cells=list(rt.cells)

print("cells:")

for i, cell in enumerate(cells):
    print("cell", i, cell.sas_area, cell.volume);

```


## Usage example, using Biotite to provide input

Voronota-LT can be used in Python code with Biotite as in the following example:

```py
import argparse
from voronotalt.biotite_interface import radical_tessellation_from_atom_array
from biotite.structure.io import load_structure

# Parse command-line arguments
parser = argparse.ArgumentParser(description="Compute radical tessellation using Voronota-LT for an input PDB file.")
parser.add_argument("input_file", help="Path to the input PDB file")
args = parser.parse_args()

# Load AtomArray from a PDB file
structure = load_structure(args.input_file)

# Compute tessellation
rt = radical_tessellation_from_atom_array(structure, probe=1.4)

# Print input balls
for i, ball in enumerate(rt.balls):
    print("ball", i, ball.x, ball.y, ball.z, ball.r)

# Print contacts
print("contacts:")
for contact in rt.contacts:
    print("contact", contact.index_a, contact.index_b, contact.area, contact.arc_length)

# Print cells
print("cells:")
for i, cell in enumerate(rt.cells):
    print("cell", i, cell.sas_area, cell.volume)
```

To run this example, make sure you have intalled Biotite:

```bash
pip install biotite
```

Then run by providing either PDB or mmCIF file path:

```bash
python3 example_script.py 2zsk.pdb
python3 example_script.py 2zsk.cif
```


# Voronota-LT Rust bindings

Thanks to Mikael Lund, there is also Rust interface for Voronota-LT at [https://github.com/mlund/voronota-rs](https://github.com/mlund/voronota-rs).

