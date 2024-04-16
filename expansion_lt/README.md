# About Voronota-LT

Voronota-LT (pronounced 'voronota lite') is an alternative version of [Voronota](../README.md) for constructing tessellation-derived atomic contact areas and volumes.
Voronota-LT was written from scratch and does not use any external code, even from the core Voronota.
The primary motivation for creating Voronota-LT was drastically increasing the speed of computing tessellation-based atom-atom contact areas and atom solvent-accessible surface areas.

Like Voronota, Voronota-LT can compute contact areas derived from the additively weighted Voronoi tessellation,
but the main increase in speed comes when utilizing a simpler, radical tessellation variant, also known as Laguerre-Laguerre tessellation or power diagram.
This is the default tessellation variant in Voronota-LT. It considers radii of atoms together with the rolling probe radius to define radical planes as bisectors between atoms.

Voronota-LT is distributed an expansion part of [the Voronota software package](../README.md) for two reasons:
to enable other Voronota expansions to easily use the Voronota-LT library;
to bundle the Voronota-LT command-line tool with the vanilla Voronota command-line tools that can produce input for Voronota-LT
by parsing molecular structure files and assigning van der Waals radii to atoms.

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


    Voronota-LT version 0.9.2
    
    'voronota-lt' executable constructs a radical Voronoi tessellation (also known as a Laguerre-Voronoi diagram or a power diagram)
    of atomic balls of van der Waals radii constrained inside a solvent-accessible surface defined by a rolling probe.
    The software computes inter-atom contact areas, per-cell solvent accessible surface areas, per-cell constrained volumes.
    'voronota-lt' is very fast when used on molecular data with a not large rolling probe radius (less than 2.0 angstroms, 1.4 is recommended)
    and can be made even faster by running it using multiple processors.
    
    Options:
        --probe                                          number     rolling probe radius, default is 1.4
        --processors                                     number     maximum number of OpenMP threads to use, default is 1
        --compute-only-inter-residue-contacts                       flag to only compute inter-residue contacts, turns off per-cell summaries
        --compute-only-inter-chain-contacts                         flag to only compute inter-chain contacts, turns off per-cell summaries
        --run-in-aw-diagram-regime                                  flag to run construct a simplified additively weighted Voronoi diagram, turns off per-cell summaries
        --periodic-box-corners                           numbers    coordinates of a periodic bounding box corners, by default no periodicity is applied
        --measure-running-time                                      flag to measure and output running times
        --print-contacts                                            flag to print table of contacts to stdout
        --print-contacts-residue-level                              flag to print residue-level grouped contacts to stdout
        --print-contacts-chain-level                                flag to print chain-level grouped contacts to stdout
        --print-cells                                               flag to print table of per-cell summaries to stdout
        --print-cells-residue-level                                 flag to print residue-level grouped per-cell summaries to stdout
        --print-cells-chain-level                                   flag to print chain-level grouped per-cell summaries to stdout
        --print-everything                                          flag to print everything to stdout, terminate if printing everything is not possible
        --write-contacts-to-file                         string     output file path to write table of contacts
        --write-contacts-residue-level-to-file           string     output file path to write residue-level grouped contacts
        --write-contacts-chain-level-to-file             string     output file path to write chain-level grouped contacts
        --write-cells-to-file                            string     output file path to write of per-cell summaries
        --write-cells-residue-level-to-file              string     output file path to write residue-level grouped per-cell summaries
        --write-cells-chain-level-to-file                string     output file path to write chain-level grouped per-cell summaries
        --write-contacts-graphics-to-file                string     output file path to write contacts drawing .py script to run in PyMol
        --graphics-title                                 string     title to use for the graphics object generated by the contacts drawing script
        --write-log-to-file                              string     output file path to write global log, does not turn off printing log to stderr
        --help | -h                                                 flag to print help to stderr and exit
    
    Standard input stream:
        Space-separated or tab-separated header-less table of balls, one of the following line formats possible:
            x y z radius
            chainID x y z radius
            chainID residueID x y z radius
            chainID residueID atomName x y z radius
        Alternatively, output of 'voronota get-balls-from-atoms-file' is acceptable, where line format is:
            x y z radius # atomSerial chainID resSeq resName atomName altLoc iCode
    
    Standard output stream:
        Requested tables with headers, with column values tab-separated
    
    Standard error output stream:
        Log (a name-value pair line), error messages
    
    Usage examples:
    
        cat ./2zsk.pdb | voronota get-balls-from-atoms-file | voronota-lt --print-contacts-residue-level --compute-only-inter-residue-contacts
    
        cat ./2zsk.pdb | voronota get-balls-from-atoms-file --radii-file ../radii | voronota-lt --print-contacts-residue-level --compute-only-inter-residue-contacts
    
        cat ./balls.xyzr | voronota-lt --processors 8 --write-contacts-to-file ./contacts.tsv --write-cells-to-file ./cells.tsv
    
        cat ./balls.xyzr | voronota-lt --probe 2 --periodic-box-corners 0 0 0 100 100 300 --processors 8 --write-cells-to-file ./cells.tsv

# Using Voronota-LT as a C++ library

Voronota-LT can be used as a header-only C++ library.
The need headers are all in "./src/voronotalt" folder.
The only header file needed to be included is "voronotalt.h".

Below is a detailed example for both basic and periodic boc modes:

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
                voronotalt::get_simple_points_from_points(periodic_box_corners),
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
            const std::size_t index=static_cast<std::size_t>(result.cells_summaries[i].id);
            cells[index].index=static_cast<int>(result.cells_summaries[i].id);
            cells[index].sas_area=result.cells_summaries[i].sas_area;
            cells[index].volume=result.cells_summaries[i].sas_inside_volume;
            cells[index].included=true;
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

# Using Voronota-LT Python bindings

## Compiling Python bindings

Python bindings of Voronota-LT can be built using SWIG, in the "expansion_lt/swig" directory:

```bash
swig -python -c++ voronotalt_python.i

g++ -fPIC -shared -O3 -fopenmp voronotalt_python_wrap.cxx -o _voronotalt_python.so $(python3-config --includes)
```

This produces "_voronotalt_python.so" and "voronotalt_python.py" that are needed to call Voronota-LT from Python code.

## Using Python bindings

When "_voronotalt_python.so" and "voronotalt_python.py" are generated, the "voronotalt_python" module can be made findable by python by adding its directory to the PYTHONPATH environmental variable:

```bash
export PYTHONPATH="${PYTHONPATH}:/path/to/voronota/expansion_lt/swig"
```

Then Voronota-LT can be used in Python code as in the following example:

```py
import voronotalt_python as voronotalt

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