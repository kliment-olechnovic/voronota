# About Voronota-JS expansion

Voronota-JS is an expansion of the core Voronota software.
Voronota-JS provides a way to write JavaScript scripts for the comprehensive
analysis of macromolecular structures, including the Voronoi tesselation-based analysis.

Currently, the Voronota-JS package contains four executables:

* "voronota-js" - core engine that executes JavaScript scripts.
* "voronota-js-voromqa" - wrapper to a voronota-js program for computing VoroMQA scores, both old and new (developed for CASP14).
* "voronota-js-membrane-voromqa" - wrapper to a voronota-js program for the VoroMQA-based analysis and assessment of membrane protein structural models.
* "voronota-js-ifeatures-voromqa" - wrapper to a voronota-js program for the computation of multiple VoroMQA-based features of protein-protein complexes.

# Getting the latest version

Download the archive from
[https://kliment-olechnovic.github.io/voronota/expansion_js/voronota-js_release.tar.gz](https://kliment-olechnovic.github.io/voronota/expansion_js/voronota-js_release.tar.gz).

The archive contains ready-to-use statically compiled "voronota-js" program for
64 bit Linux systems. This executable can be rebuilt from the provided
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

    g++ -std=c++14 -I"./src/expansion_js/src/dependencies" -O3 -o "./voronota-js" $(find ./src/ -name '*.cpp')

    
# Usage

## VoroMQA dark and light methods

'voronota-js-voromqa' script provides an interface to VoroMQA dark (newer) and light (classic) methods.

### Script interface

    
    Options:
        --input | -i              string  *  input file path or '_list' to read file paths from stdin
        --restrict-input          string     query to restrict input atoms, default is '[]'
        --sequence-file           string     input sequence file
        --select-contacts         string     query to select contacts, default is '[-min-seq-sep 2]'
        --inter-chain             string     set query to select contacts to '[-inter-chain]'
        --constraints-file        string     input distance constraints file
        --output-table-file       string     output table file path, default is '_stdout' to print to stdout
        --output-dark-scores      string     output PDB file with dark scores as B-factors
        --output-light-scores     string     output PDB file with light scores as B-factors
        --output-alignment        string     output alignment sequence alignment file
        --cache-dir               string     path to cache directory
        --tour-sort               string     tournament sorting mode, default is '_mono', options are '_homo', '_hetero' or custom
        --sbatch-parameters       string     parameters to run Slurm sbatch if input is '_list'
        --split-models-prefix     string     name prefix for splitting input PDB file by models
        --smoothing-window        number     residue scores smoothing window size, default is 0
        --processors              number     maximum number of processors to use, default is 1
        --casp-b-factors                     flag to write CASP-required B-factors in output PDB files
        --use-scwrl                          flag to use Scwrl4 to rebuild side-chains
        --guess-chain-names                  flag to guess chain names based on sequence nubmering
        --order-by-residue-id                flag to order atoms by residue ids
        --as-assembly                        flag to treat input file as biological assembly
        --help | -h                          flag to display help message and exit
    
    Standard output:
        space-separated table of scores
        
    Examples:
    
        voronota-js-voromqa --input model.pdb
        
        voronota-js-voromqa --cache-dir ./cache --input model.pdb
        
        ls *.pdb | voronota-js-voromqa --cache-dir ./cache --input _list
        
        ls *.pdb | voronota-js-voromqa --cache-dir ./cache --input _list | column -t
        
        ls *.pdb | voronota-js-voromqa --cache-dir ./cache --input _list \
          --processors 8 \
          --inter-chain \
          --tour-sort _hetero
          
        ls *.pdb | voronota-js-voromqa --cache-dir ./cache --input _list \
          --processors 8 \
          --select-contacts '[-a1 [-chain A -rnum 1:500] -a2 [-chain B -rnum 1:500]]' \
          --tour-sort '-columns full_dark_score sel_energy -multipliers 1 -1 -tolerances 0.02 0.0'
    

## VoroMQA-based membrane protein structure assessment

'voronota-js-membrane-voromqa' script provides an interface to the VoroMQA-based method for assessing membrane protein structures.

### Script interface

    
    Options:
        --input | -i               string  *  input file path
        --restrict-input           string     query to restrict input atoms, default is '[]'
        --membrane-width           number     membrane width or list of widths to use, default is 25.0
        --output-local-scores      string     prefix to output PDB files with local scores as B-factors
        --as-assembly                         flag to treat input file as biological assembly
        --help | -h                           flag to display help message and exit
    
    Standard output:
        space-separated table of scores
        
    Examples:
    
        voronota-js-membrane-voromqa --input model.pdb --membrane-width 30.0
        
        voronota-js-membrane-voromqa --input model.pdb --membrane-width 20.0,25.0,30.0
        
        voronota-js-membrane-voromqa --input model.pdb \
          --membrane-width 20,25,30 \
          --output-local-scores ./local_scores/
    

## VoroMQA-based collection of protein-protein complex features

'voronota-js-ifeatures-voromqa' script computes multiple VoroMQA-based features of protein-protein complexes.

### Script interface

    
    Options:
        --input | -i              string  *  input file path or '_list' to read file paths from stdin
        --output-table-file       string     output table file path, default is '_stdout' to print to stdout
        --processors              number     maximum number of processors to use, default is 1
        --use-scwrl                          flag to use Scwrl4 to rebuild side-chains
        --as-assembly                        flag to treat input file as biological assembly
        --help | -h                          flag to display help message and exit
    
    Standard output:
        space-separated table of scores
        
    Examples:
    
        voronota-js-ifeatures-voromqa --input model.pdb
        
        ls *.pdb | voronota-js-ifeatures-voromqa --input _list --processors 8 | column -t
    
