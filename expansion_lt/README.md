# About Voronota-LT

Voronota-LT (pronounced 'voronota lite') is an alternative version of Voronota for contructing tessellation-derived atomic contact areas and volumes.
Voronota-LT was written from scratch and does not use any external code, even from the core Voronota.
The primary motivation for creating Voronota-LT was drastically increasing the speed of computing tessellation-based atom-atom contact areas and atom solvent-accessible surface areas.
Like Voronota, Voronota-LT can compute contact areas derived from the additively weighted Voronoi tessellation,
but the main increase in speed comes when utilizing a simpler, radical tessellation variant, also known as Laguerre tessellation.
This is a default tessellation variant in Voronota-LT. It considers radii of atoms together with the rolling probe radius to define radical planes as bisectors between atoms.

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
Starting in the directory containing "CMakeLists.txt" file,
run the sequence of commands:

    cmake ./
    make

Alternatively, to keep files more organized, CMake can be run in a separate "build" directory:

    mkdir build
    cd build
    cmake ../
    make
    cp ./voronota-lt ../voronota-lt

## Using C++ compiler directly

For example, "voronota-lt" executable can be built using GNU C++ compiler:

    g++ -std=c++14 -O3 -fopenmp -o ./voronota-lt ./src/voronota_lt.cpp

Performance-boosting compiler flags can be included:

    g++ -std=c++14 -Ofast -march=native -fopenmp -o ./voronota-lt ./src/voronota_lt.cpp

# Running the command-line tool

The overview of command-line options, as well as input and output, is printed when running the "voronota-lt" executable with "--help" or "-h" flags:

    voronota-lt --help
    
    voronota-lt -h

The overview text is the following:


    Voronota-LT version 0.9
    
    'voronota-lt' executable constructs a radical Voronoi tessellation (also known as a Laguerre-Voronoi diagram or a power diagram)
    of atomic balls of van der Waals radii constrained inside a solvent-accessible surface defined by a rolling probe.
    The software computes inter-atom contact areas, per-cell solvent accessible surface areas, per-cell constrained volumes.
    'voronota-lt' is very fast when used on molecular data with a not large rolling probe radius (less than 2.0 angstroms, 1.4 is recommended)
    and and can be made even faster by running it using multiple processors.
    
    Options:
        --probe                                          number     rolling probe radius, default is 1.4
        --processors                                     number     maximum number of OpenMP threads to use, default is 1
        --compute-only-inter-residue-contacts                       flag to only compute inter-residue contacts, turns off per-cell summaries
        --compute-only-inter-chain-contacts                         flag to only compute inter-chain contacts, turns off per-cell summaries
        --run-in-aw-diagram-regime                                  flag to run construct a simplified additively weighted Voronoi diagram, turns off per-cell summaries
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
    
        cat ~/2zsk.pdb | voronota get-balls-from-atoms-file | voronota-lt --print-contacts-residue-level --compute-only-inter-residue-contacts
    
        cat ~/2zsk.pdb | voronota get-balls-from-atoms-file | voronota-lt --processors 8 --write-contacts-to-file ./contacts.tsv --write-cells-to-file ./cells.tsv

# Using Voronota-LT as a C++ library

Voronota-LT can be used as a header-only C++ library.
The need headers are all in "./src/voronotalt" folder.
The only header file needed to be included is "voronotalt.h".

Below is a detailed example:

    #include <stdexcept> // this example uses exceptions, but the Voronota-LT code does not
    
    include "voronotalt.h" // assuming that the "voronotalt" directory is in the include path
    
    //user-defined structure for a ball
    struct Ball
    {
        Ball() : x(0.0), y(0.0), z(0.0), r(0.0) {}
    
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
        Cell() : sas_area(0.0), volume(0.0), included(false) {}
    
        double sas_area;
        double volume;
        bool included;
    };
    
    //user-defined function that uses voronotalt::RadicalTessellation to fill vectors of contact and cell descriptors
    void compute_contact_and_cell_descriptors(
            const std::vector<Ball>& balls,
            const double probe,
            std::vector<Contact>& contacts,
            std::vector<Cell>& cells)
    {
        contacts.clear();
        cells.clear();
    
        if(balls.empty())
        {
            throw std::runtime_error("No balls to compute the tessellation for.");
        }
    
        // computing Voronota-LT radical tessellation results
        voronotalt::RadicalTessellation::Result result;
        voronotalt::RadicalTessellation::construct_full_tessellation(voronotalt::get_spheres_from_balls(balls, probe), result);
    
        if(result.contacts_summaries.empty())
        {
            throw std::runtime_error("No contacts constructed for the provided balls and probe.");
        }
    
        if(result.cells_summaries.empty())
        {
            throw std::runtime_error("No cells constructed for the provided balls and probe.");
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
            cells[index].sas_area=result.cells_summaries[i].sas_area;
            cells[index].volume=result.cells_summaries[i].sas_inside_volume;
            cells[index].included=true;
        }
    }


# Using Voronota-LT Python bindings

## Compiling Python bindings

Python bindings of Voronota-LT can be built using SWIG, in the "expansion_lt/swig" directory:

    swig -python -c++ voronotalt_python.i
    
    g++ -fPIC -shared -O3 -fopenmp voronotalt_python_wrap.cxx -o _voronotalt_python.so -I/usr/include/python3.11

This produces "_voronotalt_python.so" and "voronotalt_python.py" that are needed to call Voronota-LT from Python code.

## Using Python bindings

When "_voronotalt_python.so" and "voronotalt_python.py" are prepared, Voronota-LT can be used in Python code as in the following example:

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
    
