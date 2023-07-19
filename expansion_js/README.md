# About Voronota-JS expansion

Voronota-JS is an expansion of [the core Voronota software](../README.md).
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
 * "voronota-js-fast-iface-contacts" - wrapper to a voronota-js program for the very fast computation of the inter-chain interface contact areas.
 * "voronota-js-fast-iface-data-graph" - wrapper to a voronota-js program for the computation of interface graphs used by the VoroIF-GNN method.
 * "voronota-js-voroif-gnn" - wrapper to a voronota-js program and GNN inference scripts that run the VoroIF-GNN method for scoring models of protein-protein complexes (developed for CASP15).
 * "voronota-js-ligand-cadscore" - wrapper to a voronota-js program for the computation of protein-ligand variation of CAD-score (developed to analyze protein-ligand models from CASP15).

## Usage of externally developed software

Voronota-JS relies on several externally developed software projects (big thanks to their authors):

* Duktape - [https://duktape.org/](https://duktape.org/)
* PStreams - [https://github.com/jwakely/pstreams](https://github.com/jwakely/pstreams)
* TM-align - [https://zhanggroup.org/TM-align/](https://zhanggroup.org/TM-align/)
* FASPR - [https://zhanggroup.org/FASPR/](https://zhanggroup.org/FASPR/)
* QCP - [https://theobald.brandeis.edu/qcp/](https://theobald.brandeis.edu/qcp/)
* linenoise - [https://github.com/antirez/linenoise](https://github.com/antirez/linenoise)
* PicoSHA2 - [https://github.com/okdshin/PicoSHA2](https://github.com/okdshin/PicoSHA2)
* frugally-deep - [https://github.com/Dobiasd/frugally-deep](https://github.com/Dobiasd/frugally-deep)
    * FunctionalPlus - [https://github.com/Dobiasd/FunctionalPlus](https://github.com/Dobiasd/FunctionalPlus)
    * Eigen - [http://eigen.tuxfamily.org/](http://eigen.tuxfamily.org/)
    * json  - [https://github.com/nlohmann/json](https://github.com/nlohmann/json)
* tinf - [https://github.com/jibsen/tinf](https://github.com/jibsen/tinf)
* LodePNG - [https://github.com/lvandeve/lodepng](https://github.com/lvandeve/lodepng)

# Getting the latest version

Download the latest archive from the official downloads page:
[https://github.com/kliment-olechnovic/voronota/releases](https://github.com/kliment-olechnovic/voronota/releases).

The archive contains the Voronota-JS software in the 'expansion_js' subdirectory.
There is a ready-to-use statically compiled 'voronota-js' program for 64 bit Linux systems.
This executable can be rebuilt from the provided
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

    g++ -std=c++14 -I"./src/dependencies" -O3 -o "./voronota-js" $(find ./src/ -name '*.cpp')


# More about protein-ligand variation of CAD-score

The script 'voronota-js-ligand-cadscore' for the protein-ligand variation of CAD-score
is included to [the Voronota-JS expansion](./README.md) of Voronota.

## Example of how it works for CASP15 models

The 'voronota-js-ligand-cadscore' scripts has a special input mode for CASP15 protein-ligand model files: when using --casp15-* options for specifying input, the input files are split into receptor and ligand parts and passed to the same script with basic input options. The target input in the CASP15 mode should be formatted in the same as the model input. The basic input mode option is more flexible, but it requires all the receptor and ligand files available separately, and the ligand IDs (names) to be specified explicitly.

For example, let us take two files with protein-ligand models from CASP15: 'H1135LG035_1' and 'H1135LG046_1'.
Then let us execute the following command:

    voronota-js-ligand-cadscore \
      --casp15-target ./T1118v1LG035_1 \
      --casp15-model ./T1118v1LG046_1 \
      --casp15-target-pose 1 \
      --casp15-model-pose 2 \
    | column -t

which outputs:

    target                model                 contacts_set             CAD_score  target_area  model_area
    T1118v1LG035_1_pose1  T1118v1LG046_1_pose2  interface                0.568301   648.036      808.139
    T1118v1LG035_1_pose1  T1118v1LG046_1_pose2  interface_plus_adjacent  0.629109   1781.9       2189.88

Note, that CAD-score values will be different if we swap target and model inputs. The script can do the swap automatically and output additional results when the '--and-swap true' is added to the arguments, then the output will look as follows:

    target                model                 contacts_set             CAD_score  target_area  model_area
    T1118v1LG035_1_pose1  T1118v1LG046_1_pose2  interface                0.568301   648.036      808.139
    T1118v1LG035_1_pose1  T1118v1LG046_1_pose2  interface_plus_adjacent  0.629109   1781.9       2189.88
    T1118v1LG046_1_pose2  T1118v1LG035_1_pose1  interface                0.528076   808.139      648.036
    T1118v1LG046_1_pose2  T1118v1LG035_1_pose1  interface_plus_adjacent  0.519405   2189.88      1781.9

### Output explanation

The output table columns mean the following

 * 'target' - name of the target protein-ligand complex structure.
 * 'model' - name of the model protein-ligand complex structure.
 * 'contacts_set' - indicates what contacts were compared: only the direct interface contacts ('interface'), or the expanded set that also contains the contacts adjacent to the interface ('interface_plus_adjacent').
 * 'CAD_score' - CAD-score value computed using the standard CAD-score formula (the formula can be seen on [https://bioinformatics.lt/cad-score/theory](https://bioinformatics.lt/cad-score/theory)).
 * 'target_area' - total area of the relevant contacts in the target complex
 * 'model_area' - total area of the relevant contacts in the model complex

## More info about the involved contacts

There are some important aspects:

 * All the contact areas are computed on the atom-atom level, but then bundled (summed) to the residue-residue/residue-ligand level.
 * When defining the contact uniqueness, residue names and numbers (and chain ID, if it is present) are used for the protein side, but only ligand names are used for the ligand side. Therefore, the script treats same-named ligands as a single ligand when summarizing contact areas. This was done to avoid doing permutations of ligand numbers for identical ligands.
 * The 'interface' set includes only the protein-ligand contacts.
 * The 'interface_plus_adjacent' set includes the following contacts: all the 'interface' set contacts; protein-protein contacts between residues that have contacts with ligands.
 * You can use the '--details-dir' option to look at the used contacts lists.
 * You can use the '--drawing-dir' option to generate PDB files (with interface participants) and contact drawing scripts for PyMol.

### Details example

Examples of files generated using the '--details-dir' option:

 * 'interface' contacts:
     * [T1118v1LG035_1_pose1_interface_contacts.txt](./tests/jobs_output/ligand_cadscore_script/details/T1118v1LG035_1_pose1_interface_contacts.txt)
     * [T1118v1LG046_1_pose2_interface_contacts.txt](./tests/jobs_output/ligand_cadscore_script/details/T1118v1LG046_1_pose2_interface_contacts.txt)
 * 'interface_plus_adjacent' contacts:
     * [T1118v1LG035_1_pose1_interface_and_adjacent_contacts.txt](./tests/jobs_output/ligand_cadscore_script/details/T1118v1LG035_1_pose1_interface_and_adjacent_contacts.txt)
     * [T1118v1LG046_1_pose2_interface_and_adjacent_contacts.txt](./tests/jobs_output/ligand_cadscore_script/details/T1118v1LG046_1_pose2_interface_and_adjacent_contacts.txt)

### Drawing example

Example pymol call for displaying files generated using the '--drawing-dir' option:
 
    pymol \
      draw_T1118v1LG035_1_pose1_interface_and_adjacent_contacts_in_pymol.py \
      T1118v1LG035_1_pose1_cutout_interface_residues_ligand.pdb \
      T1118v1LG035_1_pose1_cutout_interface_residues_receptor.pdb

Example of visualized contacts (with direct interface contacts in green, adjacent contacts in yellow, ligand atoms in red, receptor atoms in cyan): [protein_ligand_3D.png](./resources/texts/protein_ligand_3D.png).


# Wrapper scripts

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
        --input-is-script                    flag to treat input file as vs script
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
    

## VoroMQA for only global scores with fast caching

'voronota-js-only-global-voromqa' script computes global VoroMQA scores and can use fast caching.

### Script interface

    
    Options:
        --input | -i              string  *  input file path or '_list' to read file paths from stdin
        --restrict-input          string     query to restrict input atoms, default is '[]'
        --output-table-file       string     output table file path, default is '_stdout' to print to stdout
        --output-dark-pdb         string     output path for PDB file with VoroMQA-dark scores, default is ''
        --output-light-pdb        string     output path for PDB file with VoroMQA-light scores, default is ''
        --processors              number     maximum number of processors to run in parallel, default is 1
        --sbatch-parameters       string     sbatch parameters to run in parallel, default is ''
        --cache-dir               string     cache directory path to store results of past calls
        --run-faspr               string     path to FASPR binary to rebuild side-chains
        --input-is-script                    flag to treat input file as vs script
        --as-assembly                        flag to treat input file as biological assembly
        --help | -h                          flag to display help message and exit
    
    Standard output:
        space-separated table of scores
        
    Examples:
    
        voronota-js-only-global-voromqa --input model.pdb
        
        ls *.pdb | voronota-js-only-global-voromqa --input _list --processors 8 | column -t
    

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
    

## Fast inter-chain interface VoroMQA energy

'voronota-js-fast-iface-voromqa' script rapidly computes VoroMQA-based interface energy of protein complexes.

### Script interface

    
    Options:
        --input | -i                string  *  input file path or '_list' to read file paths from stdin
        --restrict-input            string     query to restrict input atoms, default is '[]'
        --subselect-contacts        string     query to subselect inter-chain contacts, default is '[]'
        --constraints-required      string     query to check required contacts, default is ''
        --constraints-banned        string     query to check banned contacts, default is ''
        --constraint-clashes        number     max allowed clash score, default is 0.9
        --output-table-file         string     output table file path, default is '_stdout' to print to stdout
        --output-ia-contacts-file   string     output inter-atom contacts file path, default is ''
        --output-ir-contacts-file   string     output inter-residue contacts file path, default is ''
        --processors                number     maximum number of processors to run in parallel, default is 1
        --sbatch-parameters         string     sbatch parameters to run in parallel, default is ''
        --stdin-file                string     input file path to replace stdin
        --run-faspr                 string     path to FASPR binary to rebuild side-chains
        --input-is-script                      flag to treat input file as vs script
        --as-assembly                          flag to treat input file as biological assembly
        --detailed-times                       flag to output detailed times
        --score-symmetry                       flag to score interface symmetry
        --blanket                              flag to keep nucleic acids and use blanket potential
        --help | -h                            flag to display help message and exit
    
    Standard output:
        space-separated table of scores
        
    Examples:
    
        voronota-js-fast-iface-voromqa --input model.pdb
        
        ls *.pdb | voronota-js-fast-iface-voromqa --input _list --processors 8 | column -t
    

## Fast inter-chain interface CAD-score

'voronota-js-fast-iface-cadscore' script rapidly computes interface CAD-score for two protein complexes.

### Script interface

    
    Options:
        --target | -t             string  *  target file path
        --model | -m              string  *  model file path or '_list' to read file paths from stdin
        --restrict-input          string     query to restrict input atoms, default is '[]'
        --subselect-contacts      string     query to subselect inter-chain contacts, default is '[]'
        --output-table-file       string     output table file path, default is '_stdout' to print to stdout
        --processors              number     maximum number of processors to run in parallel, default is 1
        --sbatch-parameters       string     sbatch parameters to run in parallel, default is ''
        --stdin-file              string     input file path to replace stdin
        --run-faspr               string     path to FASPR binary to rebuild side-chains
        --as-assembly                        flag to treat input files as biological assemblies
        --remap-chains                       flag to calculate and use optimal chains remapping
        --ignore-residue-names               flag to ignore residue names in residue identifiers
        --test-common-ids                    flag to fail quickly if there are no common residues
        --crude                              flag to enable very crude faster mode
        --help | -h                          flag to display help message and exit
    
    Standard output:
        space-separated table of scores
        
    Examples:
    
        voronota-js-fast-iface-cadscore --target target.pdb --model model.pdb
        
        ls *.pdb | voronota-js-fast-iface-cadscore --target target.pdb --model _list --processors 8 | column -t
    

## Fast inter-chain interface CAD-score matrix

'voronota-js-fast-iface-cadscore-matrix' script rapidly computes interface CAD-score between complexes.

### Script interface

    
    Options:
        --restrict-input          string     query to restrict input atoms, default is '[]'
        --subselect-contacts      string     query to subselect inter-chain contacts, default is '[]'
        --output-table-file       string     output table file path, default is '_stdout' to print to stdout
        --processors              number     maximum number of processors to run in parallel, default is 1
        --sbatch-parameters       string     sbatch parameters to run in parallel, default is ''
        --stdin-file              string     input file path to replace stdin
        --as-assembly                        flag to treat input files as biological assemblies
        --remap-chains                       flag to calculate and use optimal chains remapping
        --ignore-residue-names               flag to ignore residue names in residue identifiers
        --crude                              flag to enable very crude faster mode
        --help | -h                          flag to display help message and exit
    
    Standard output:
        space-separated table of scores
        
    Examples:
    
        ls *.pdb | voronota-js-fast-iface-cadscore-matrix | column -t
        
        find ./complexes/ -type f -name '*.pdb' | voronota-js-fast-iface-cadscore-matrix > "full_matrix.txt"
        
        (find ./group1/ -type f | awk '{print $1 " a"}' ; find ./group2/ -type f | awk '{print $1 " b"}') | voronota-js-fast-iface-cadscore-matrix > "itergroup_matrix.txt"
    

## Fast inter-chain interface contacts

'voronota-js-fast-iface-contacts' script rapidly computes contacts of inter-chain interface in a molecular complex.

### Script interface

    
    Options:
        --input                   string  *  input file path or '_list' to read file paths from stdin
        --restrict-input          string     query to restrict input atoms, default is '[]'
        --subselect-contacts      string     query to subselect inter-chain contacts, default is '[]'
        --output-contacts-file    string     output table file path, default is '_stdout' to print to stdout
        --output-bsite-file       string     output binding site table file path, default is ''
        --output-drawing-script   string     output PyMol drawing script file path, default is ''
        --processors              number     maximum number of processors to run in parallel, default is 1
        --sbatch-parameters       string     sbatch parameters to run in parallel, default is ''
        --stdin-file              string     input file path to replace stdin
        --run-faspr               string     path to FASPR binary to rebuild side-chains
        --custom-radii-file       string     path to file with van der Waals radii assignment rules
        --with-sas-areas                     flag to also compute and output solvent-accessible areas of interface residue atoms
        --coarse-grained                     flag to output inter-residue contacts
        --input-is-script                    flag to treat input file as vs script
        --as-assembly                        flag to treat input file as biological assembly
        --use-hbplus                         flag to run 'hbplus' to tag H-bonds
        --expand-ids                         flag to output expanded IDs
        --og-pipeable                        flag to format output to be pipeable to 'voronota query-contacts'
        --help | -h                          flag to display help message and exit
    
    Standard output:
        tab-separated table of contacts
        
    Examples:
    
        voronota-js-fast-iface-contacts --input "./model.pdb" --expand-ids > "./contacts.tsv"
        
        voronota-js-fast-iface-contacts --input "./model.pdb" --with-sas-areas --coarse-grained --og-pipeable | voronota query-contacts --summarize-by-first
        
        cat "./model.pdb" | voronota-js-fast-iface-contacts --input _stream --with-sas-areas --coarse-grained --og-pipeable | voronota query-contacts --summarize
        
        ls *.pdb | voronota-js-fast-iface-contacts --input _list --processors 8 --output-contacts-file "./output/-BASENAME-.tsv"
    

## Computation of inter-chain interface graphs

'voronota-js-fast-iface-data-graph' script generates interface data graphs of protein complexes.

### Script interface

    
    Options:
        --input                   string  *  input file path or '_list' to read file paths from stdin
        --restrict-input          string     query to restrict input atoms, default is '[]'
        --subselect-contacts      string     query to subselect inter-chain contacts, default is '[]'
        --with-reference          string     input reference complex structure file path, default is ''
        --output-data-prefix      string  *  output data files prefix
        --output-table-file       string     output table file path, default is '_stdout' to print to stdout
        --processors              number     maximum number of processors to run in parallel, default is 1
        --sbatch-parameters       string     sbatch parameters to run in parallel, default is ''
        --stdin-file              string     input file path to replace stdin
        --run-faspr               string     path to FASPR binary to rebuild side-chains
        --coarse-grained                     flag to output a coarse-grained graph
        --input-is-script                    flag to treat input file as vs script
        --as-assembly                        flag to treat input file as biological assembly
        --help | -h                          flag to display help message and exit
    
    Standard output:
        space-separated table of generated file paths
        
    Examples:
    
        voronota-js-fast-iface-data-graph --input model.pdb --output-prefix ./data_graphs/
        
        ls *.pdb | voronota-js-fast-iface-data-graph --input _list --processors 8 --output-prefix ./data_graphs/ | column -t
    

## VoroIF-GNN method for scoring models of protein-protein complexes

'voronota-js-voroif-gnn' scores protein-protein interfaces using the VoroIF-GNN method

### Script interface

    
    Options:
        --input                   string  *  input file path, or '_list' to read multiple input files paths from stdin
        --gnn                     string     GNN package file or directory with package files, default is '${SCRIPT_DIRECTORY}/voroif/gnn_packages/v1'
        --gnn-add                 string     additional GNN package file or directory with package files, default is ''
        --restrict-input          string     query to restrict input atoms, default is '[]'
        --subselect-contacts      string     query to subselect inter-chain contacts, default is '[]'
        --as-assembly             string     flag to treat input file as biological assembly
        --input-is-script         string     flag to treat input file as vs script
        --conda-path              string     conda installation path, default is ''
        --conda-env               string     conda environment name, default is ''
        --faspr-path              string     path to FASPR binary, default is ''
        --run-faspr               string     flag to rebuild sidechains using FASPR, default is 'false'
        --processors              number     maximum number of processors to run in parallel, default is 1
        --sbatch-parameters       string     sbatch parameters to run in parallel, default is ''
        --stdin-file              string     input file path to replace stdin, default is '_stream'
        --local-column            string     flag to add per-residue scores to the global output table, default is 'false'
        --cache-dir               string     cache directory path to store results of past calls, default is ''
        --output-dir              string     output directory path for all results, default is ''
        --output-pdb-file         string     output path for PDB file with interface residue scores, default is ''
        --output-pdb-mode         string     mode to write b-factors ('overwrite_all', 'overwrite_iface' or 'combine'), default is 'overwrite_all'
        --help | -h                          flag to display help message and exit
    
    Standard output:
        space-separated table of global scores
        
    Important note about output interpretation:
        higher GNN scores are better, lower GNN scores are worse (with VoroMQA energy it is the other way around)
    
    Examples:
    
        voronota-js-voroif-gnn --conda-path ~/miniconda3 --input './model.pdb'
        
        voronota-js-voroif-gnn --input './model.pdb' --gnn "${HOME}/git/voronota/expansion_js/voroif/gnn_packages/v1"
        
        find ./models/ -type f | voronota-js-voroif-gnn --conda-path ~/miniconda3 --input _list
    
    Requirements installation example using Miniconda (may need more than 10 GB of disk space):
    
        wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
        bash Miniconda3-latest-Linux-x86_64.sh
        source ~/miniconda3/bin/activate
        conda install pytorch torchvision torchaudio pytorch-cuda=11.7 -c pytorch -c nvidia # using instructions from 'https://pytorch.org/get-started/locally/'
        conda install pyg -c pyg # using instructions from 'https://pytorch-geometric.readthedocs.io/en/latest/install/installation.html'
        conda install pandas
        conda install r # may skip this if you have R already and do not want it in Miniconda
    

## Protein-ligand interface variation of CAD-score

'voronota-js-ligand-cadscore' script computes protein-ligand variation of CAD-score.

### Script interface

    
    Input options, basic:
        --target-receptor             string  *  target receptor file path
        --target-ligands              string  *  list of target ligand file paths
        --target-ligand-ids           string  *  list of target ligand IDs
        --model-receptor              string  *  model receptor file path
        --model-ligands               string  *  list of model ligand file paths
        --model-ligand-ids            string  *  list of model ligand IDs
        --target-name                 string     target name to use for output, default is 'target_complex'
        --model-name                  string     model name to use for output, default is 'model_complex'
    
    Input options, alternative:
        --casp15-target               string  *  target data file in CASP15 format, alternative to --target-* options
        --casp15-target-pose          string  *  pose number to select from the target data file in CASP15 format
        --casp15-model                string  *  model data file in CASP15 format, alternative to --model-* options
        --casp15-model-pose           string  *  pose number to select from the model data file in CASP15 format
    
    Other options:
        --table-dir                   string     directory to output scores table file named '${TARGET_NAME}_vs_${MODEL_NAME}.txt'
        --details-dir                 string     directory to output lists of contacts used for scoring
        --drawing-dir                 string     directory to output files to visualize with pymol
        --and-swap                    string     flag to compute everything after swapping target and model, default is 'false'
        --ignore-ligand-headers       string     flag to ignore title header in ligand files
        --help | -h                              display help message and exit
        
    Standard output:
        space-separated table of scores
        
    Examples:
    
        voronota-js-ligand-cadscore --casp15-target ./T1118v1LG035_1 --casp15-target-pose 1 --casp15-model ./T1118v1LG046_1 --casp15-model-pose 1
        
        voronota-js-ligand-cadscore \
          --target-receptor ./t_protein.pdb --target-ligands './t_ligand1.mol ./t_ligand2.mol ./t_ligand3.mol' --target-ligand-ids 'a a b' \
          --model-receptor ./m_protein.pdb --model-ligands './m_ligand1.mol ./m_ligand2.mol ./m_ligand3.mol' --model-ligand-ids 'a a b'
    
