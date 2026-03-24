# Introduction to CAD-score-LT

CAD-score-LT software computes CAD-score (Contact Area Difference score): a superposition-free similarity measure based on contact areas.
CAD-score-LT is based on the [Voronota-LT software](../expansion_lt/index.html).
CAD-score-LT is much faster and more versatile than the previous CAD-score implementations.
It is designed to work for both global comparisons and for localized analyses such as interfaces and binding sites.

CAD-score-LT is open source (MIT license) and hosted [on GitHub as a project in the Voronota monorepo](https://github.com/kliment-olechnovic/voronota/tree/master/expansion_lt_cadscore).

## CAD-score definition for residue-residue contacts

Given reference structure $T$ (target) and structure to be compared, $M$ (model),
let $G$ denote the set of all the pairs of residues $(i,j)$ that have
a non-zero contact area $T_{(i,j)}$ in the target structure.
Then for every residue pair $(i,j) \in G$ the corresponding contact area
$M_{(i,j)}$ in the model is calculated.
$M_{(i,j)}$ is assigned zero if there is no contact between residues $i$ and $j$ in the model
or if either residue ($i$ or $j$) is missing from the model.
The CAD-score for the model structure is then defined as:

$$\text{CAD-score}(G)=1-\frac {\sum_{(i,j) \in G} \min(|T_{(i,j)}-M_{(i,j)}|,T_{(i,j)})} {\sum_{(i,j) \in G}T_{(i,j)} }$$

CAD-score values are always within the $[0,1]$ range.
If model and target structures are identical, $\text{CAD-score}(G)=1$.
At the other extreme, if not a single contact is reproduced with sufficient accuracy, $\text{CAD-score}(G)=0$.

More local scores (e.g. interface-scores or per-residue scores) are calculated by applying the CAD-score formula
to a restricted subset of $G$.

## CAD-score definition for other types of areas

If areas in a target and in a model have identifiers, and those identifiers can be matched, then CAD-score can be calculated.
For example, if we want to compare solvent-accessible (SAS) areas, we can assign identifiers to such areas.
Let us formulate the CAD-score definition for per-residue SAS areas in a manner similar to the CAD-score definition for residue-residue contacts.

Given reference structure $T$ (target) and structure to be compared, $M$ (model),
let $G$ denote the set of all the residues $i$ that have
a non-zero SAS area $T_{(i)}$ in the target structure.
Then for every residue $i \in G$ the corresponding SAS area
$M_{(i)}$ in the model is calculated.
$M_{(i)}$ is assigned zero if there is no SAS for residue $i$ in the model
or if residue $i$ is missing from the model.
The SAS-based CAD-score for the model structure is then defined as:

$$\text{CAD-score}(G)=1-\frac {\sum_{i \in G} \min(|T_{(i)}-M_{(i)}|,T_{(i)})} {\sum_{i \in G}T_{(i)} }$$

Similarly we can calculate CAD-score for per-residue binding site areas produced by accumulating contact areas.

Overall, on CAD-score-LT, three types of areas can be analyzed:

* _"contact"_ - Voronoi tessellation-derived iteratomic contact areas.
* _"SAS"_ - solvent-accessible surface areas.
* _"site"_ - Voronoi tessellation-derived binding site areas.

Areas of every type can be assessed on three levels of detail:

* _"atom"_ - atom-atom contact areas, atom-level site areas, atom-level SAS areas
* _"residue"_ - residue-residue contact areas, residue-level site areas, residue-level SAS areas
* _"chain"_ - chain-chain contact areas, residue-level site areas, residue-level SAS areas

All the areas are intitially computed on the atom level. Then, if needed, the atom-level areas are aggregated on residue or/and chain levels.

# Quickest install guide

Since CAD-score-LT version 0.8.106, universal binary execuitables of CAD-score-LT
built with the [Cosmopolitan Libc toolkit](https://github.com/jart/cosmopolitan) are provided.

To download and prepare the latest released cosmopolitan executable, run the following commands in a shell environment (e.g. a Bash shell):

```bash
wget 'https://github.com/kliment-olechnovic/voronota/releases/download/vLATEST_VORONOTA_RELEASE_VERSION/cosmopolitan_cadscore-lt_vLATEST_CADSCORELT_RELEASE_VERSION.exe'
mv cosmopolitan_cadscore-lt_vLATEST_CADSCORELT_RELEASE_VERSION.exe cadscore-lt
chmod +x cadscore-lt
```

In case of a PowerShell environment in Windows 8, the setup can be done with a single command:

```bash
Invoke-WebRequest -Uri 'https://github.com/kliment-olechnovic/voronota/releases/download/vLATEST_VORONOTA_RELEASE_VERSION/cosmopolitan_cadscore-lt_vLATEST_CADSCORELT_RELEASE_VERSION.exe' -OutFile cadscore-lt.exe
```

# Building the command-line tool from source code

Download the latest CAD-score-LT source archive from the official downloads page at
[https://github.com/kliment-olechnovic/voronota/releases](https://github.com/kliment-olechnovic/voronota/releases),
e.g. [cadscore-lt_vLATEST_CADSCORELT_RELEASE_VERSION.tar.gz](https://github.com/kliment-olechnovic/voronota/releases/download/vLATEST_VORONOTA_RELEASE_VERSION/cadscore-lt_vLATEST_CADSCORELT_RELEASE_VERSION.tar.gz).

## Requirements

The CAD-score-LT executable can be built from the provided
source code to work on any modern Linux, macOS or Windows operating systems.

CAD-score-LT has no required external library dependencies, only
a C++17-compliant compiler is needed to build it.

## Using CMake

You can build using CMake for makefile generation.

Change to the CAD-score-LT directory with ``CMakeLists.txt`` file,
then run the sequence of commands:

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
cp ./cadscore-lt ../cadscore-lt
```

## Using C++ compiler directly

For example, "cadscore-lt" executable can be built using GNU C++ compiler.

Then run the CAD-score-LT directory and run the compilation command:

```bash
g++ -std=c++17 -O3 -fopenmp -I ./src -o ./cadscore-lt ./src/cadscore_lt.cpp
```

Performance-boosting compiler flags can be included:

```bash
g++ -std=c++17 -Ofast -march=native -fopenmp -I ./src -o ./cadscore-lt ./src/cadscore_lt.cpp
```

# Basic usage

## Listing command-line options

Running

```bash
cadscore-lt -h
```

prints an overview of the command line interface:

```
CAD-score-LT version 0.9

'cadscore-lt' calculates CAD-score (Contact Area Difference score).

Options:
    --targets | -t                                   string     input file or directory paths for target (reference) structure files
    --models | -m                                    string     input file or directory paths for model structure files
    --processors                                     number     maximum number of OpenMP threads to use, default is 2 if OpenMP is enabled, 1 if disabled
    --recursive-directory-search                                flag to search directories recursively
    --include-heteroatoms                                       flag to include heteroatoms when reading input in PDB or mmCIF format
    --read-inputs-as-assemblies                                 flag to join multiple models into an assembly when reading a file in PDB or mmCIF format
    --radii-config-file                              string     input file path for reading atom radii assignment rules
    --probe                                          number     rolling probe radius, default is 1.4
    --restrict-raw-input                             string     selection expression to restrict input atoms before any chain renaming or residue renumbering
    --reference-sequences-file                       string     input file path for reference sequences in FASTA format
    --reference-stoichiometry                        numbers    list of stoichiometry values to apply when mapping chains to reference sequences
    --restrict-processed-input                       string     selection expression to restrict input atoms after all chain renaming and residue renumbering
    --save-processed-inputs-mmcif                               flag to save processed input structures in mmCIF format to the output directory
    --save-processed-inputs-pdb                                 flag to save processed input structures in PDB format to the output directory
    --save-sequence-alignments                                  flag to save best alignments with reference sequences into a file in the output directory
    --quit-before-scoring                                       flag to exit before scoring but after all the input processing and saving
    --subselect-contacts                             string     selection expression to restrict contact area descriptors to score, default is '[-min-sep 1]'
    --subselect-atoms                                string     selection expression to restrict atom SAS and site area descriptors to score, default is '[]'
    --conflate-atom-types                                       flag to conflate known equivalent atom types
    --conflation-config-file                         string     input file path for reading atom name conflation rules
    --scoring-types                                  strings    scoring types ('contacts', 'SAS', 'sites'), default is 'contacts'
    --scoring-levels                                 strings    scoring levels ('atom', 'residue', 'chain'), default is 'residue'
    --local-output-formats                           strings    list of formats (can include 'table', 'pdb', 'mmcif', 'contactmap', 'graphics-pymol', 'graphics-chimera')
    --local-output-levels                            strings    list of output levels (can include 'atom', 'residue', 'chain'), default is 'residue'
    --consider-residue-names                                    flag to include residue names in residue and atom identifiers, making mapping more strict
    --binarize-areas                                            flag to binarize (convert to 0 or 1) all area values before scoring
    --remap-chains                                              flag to automatically rename chains in models to maximize residue-residue contacts global score
    --max-chains-to-fully-permute                    number     limit of chain combinations to chech exhaustively when remapping chains, default is 200
    --clustering-thresholds                          numbers    clustering thresholds for Taylor-Butina-like clustering if in all-to-all comparison mode
    --max-renaming-cache-size                        number     max number of contact sets to cache when doing comparisons to multiple targets, default is 400
    --print-paths-in-output                                     flag to print file paths instead of file base names in output
    --output-with-f1                                            flag to output area-based F1 scores along with CAD-scores
    --output-with-areas                                         flag to output all recorded types of areas in tables of global and local scores
    --output-with-identities                                    flag to output identity percentages (for input atoms, residues, chains) along with CAD-scores
    --compact-output                                            flag to reduce size of output global scores table without removing rows
    --extremely-compact-output                                  flag to reduce size of output global scores by writing them as an integer matrix
    --output-global-scores                           string     path to output table of global scores, default is '_stdout' to print to standard output 
    --output-dir                                     string     path to output directory for all result files
    --help | -h                                                 flag to print help info to stderr and exit

Standard output stream:
    Global scores

Standard error output stream:
    Error messages

Usage examples:

    cadscore-lt -t ./target.pdb -m ./model1.pdb ./model2.pdb

    cadscore-lt -t ./target.pdb -m ./model1.pdb ./model2.pdb --subselect-contacts '[-inter-chain]'

```

## Choosing scoring types and scoring levels

CAD-score-LT supports several scoring types and scoring levels.

Scoring types (``--scoring-types``):

* ``contacts`` (default): contact-area-based CAD-score (the classic CAD-score concept)
* ``sas``: solvent-accessible surface area descriptors
* ``sites``: binding site area descriptors


Scoring levels (``--scoring-levels``):

* ``atom``
* ``residue`` (default)
* ``chain``

Residue-level contacts is the default mode. That is, running

```bash
cadscore-lt \
  -t ./target.pdb \
  -m ./model1.pdb
```

is equivalent to running:

```bash
cadscore-lt \
  -t ./target.pdb \
  -m ./model1.pdb \
  --scoring-types contacts \
  --scoring-levels residue
```

Multiple options (even all of them) can be used together, for example:

```bash
cadscore-lt \
  -t ./target.pdb \
  -m ./model1.pdb \
  --scoring-types contacts sas sites \
  --scoring-levels atom residue chain
```

## Providing inputs

For input, CAD-score-LT accepts files in PDB or mmCIF formats.

Basic usage (one target, one model):

```bash
cadscore-lt -t target.pdb -m model1.pdb
```

One target, multiple models:

```bash
cadscore-lt -t target.pdb -m model1.pdb model2.pdb
```

Read models from stdin (convenient for ``find``):

```bash
find ./input/ -name 'model*.pdb' | cadscore-lt -t ./input/target.pdb
```

All-vs-all mode (no ``--target`` or ``-t`` given) is entered when no targets are provided,
then CAD-score-LT sets targets = models and compares all pairs (excluding self-pairs). For example:

```bash
find ./input/ -name '*.pdb' | cadscore-lt
```

All-vs-all mode is convenient for clustering structures or their parts (e.g. inter-chain interfaces).

## Outputting a global scores table

By default, ``cadscore-lt`` prints a tab-separated table of global scores to stdout.

Useful flags:

* ``--output-with-f1`` - adds area-based F1-like metrics alongside CAD scores.
* ``--output-all-details`` - produces a more verbose table (more columns / details).
* ``--compact-output`` - keeps the table smaller (without removing rows), writing auxiliary files into the directoru specified by ``--output-dir``.

Useful tip: use ``column -t`` command to align column value in a pretty way. 

Examples:

```bash
cadscore-lt -t target.pdb -m model1.pdb model2.pdb --output-with-f1 | column -t
```

```bash
cadscore-lt -t target.pdb -m model1.pdb model2.pdb --output-all-details | column -t
```

## Selecting what to score

CAD-score-LT lets you:

* restrict raw input atoms (atoms before any optional chain renaming and residue renumbering)
* restrict processed input atoms ( atoms after optional chain renaming and residue renumbering)
* select which contacts contribute to the contacts score
* select which atoms contribute to SAS and sites scoring

Both atom-focused and contact focused expressions should be provided in the system described in the next section.

## Filtering (selection) system

Some optional arguments of CAD-score-LT expect selection expressions:

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

## Examples of using selection expressions in CAD-score-LT

### Restricting input atoms

Example of restricting atoms before any chain renaming and residue renumbering:

```bash
cadscore-lt -t target.pdb -m model1.pdb model2.pdb \
  --output-all-details \
  --restrict-raw-input "[-chain A]" \
| column -t
```

Example of restricting atoms after any chain renaming and residue renumbering:

```bash
cadscore-lt -t target.pdb -m model1.pdb model2.pdb \
  --output-all-details \
  --reference-sequences-file ./sequences.fasta \
  --reference-stoichiometry 2 2 1 \
  --restrict-processed-input "[-chain A,B]" \
| column -t
```

### Selecting contacts

Using ``--subselect-contacts`` is the most common way to focus CAD-score.

Default contact selection is ``[-min-sep 1]`` (to discard contacts between atoms in the same residue).

Example of sidechain–sidechain contact scoring:

```bash
cadscore-lt -t target.pdb -m model1.pdb model2.pdb \
  --output-all-details \
  --subselect-contacts "[-min-sep 1 -atom1 [-protein-sidechain] -atom2 [-protein-sidechain]]" \
| column -t
```

Example of scoring contacts between chain A and chain B:

```bash
cadscore-lt -t target.pdb -m model1.pdb model2.pdb \
  --output-all-details \
  --subselect-contacts "[-a1 [-chain A] -a2 [-chain B]]" \
| column -t
```

Example of scoring contacts between any different chains: 

```bash
cadscore-lt -t target.pdb -m model1.pdb model2.pdb \
  --output-all-details \
  --subselect-contacts "[-inter-chain]" \
| column -t
```

Example of scoring contacts between residue sets (example with ranges):

```bash
cadscore-lt -t target.pdb -m model1.pdb model2.pdb \
  --output-all-details \
  --subselect-contacts "[-a1 [-chain B -rnum 39:51] -a2 [-chain B -rnum 39:66,75:87]]" \
| column -t
```

Selections can be combined with boolean expressions, as shown below:

```bash
cadscore-lt -t target.pdb -m model1.pdb model2.pdb \
  --output-all-details \
  --subselect-contacts "(([-a1 [-chain B -rnum 39:51] -a2 [-chain B -rnum 39:66]]) or ([-a1 [-chain B -rnum 39:51] -a2 [-chain B -rnum 75:87]]))" \
| column -t
```

### Select atoms for SAS/sites

When scoring SAS or binding sites, you can select which atoms are considered using the ``--subselect-atoms`` option:

```bash
cadscore-lt -t target.pdb -m model1.pdb model2.pdb \
  --subselect-contacts "[-inter-chain]" \
  --scoring-types "sites" \
  --subselect-atoms "[-chain A]" \
  --output-all-details \
| column -t
```

## Local scoring outputs

CAD-score-LT can write local scores and visualization helpers into an output directory.

Options to use:

* ``--output-dir`` DIR
* ``--local-output-formats`` ...

Formats (``--local-output-formats``) can include:

* ``table``
* ``pdb``
* ``mmcif``
* ``contactmap``
* ``graphics-pymol``
* ``graphics-chimera``

Example of writing local per-residue scores as as tables and as PDB B-factors:

```bash
cadscore-lt -t target.pdb -m model1.pdb model2.pdb \
  --output-dir ./results \
  --local-output-formats table pdb \
| column -t
```

Example of writing interface local scores:

```bash
cadscore-lt -t target.pdb -m model1.pdb model2.pdb \
  --subselect-contacts "[-inter-chain]" \
  --output-dir ./results_inter_chain \
  --local-output-formats table mmcif contactmap graphics-pymol \
| column -t
```

PDB/mmCIF outputs are typically easiest to inspect in molecular viewers by coloring residues by B-factor.
CAD-score also sets occupancy values - 1 for every atom that contributed to any areas involved in scoring, 0 for all the other atoms.
For example, when scoring an inter-chain interface only atoms involved in inter-chain contacts will have occupancy value of 1.

## Chain remapping for complexes 

When scoring multimeric complexes, model chain IDs may not match target chain IDs (especially homomers).

CAD-score-LT can automatically rename chains in models to try to maximize the global residue–residue contacts score.
For that, use the ``--remap-chains`` flag:

```bash
cadscore-lt -t target.pdb -m model1.pdb model2.pdb \
  --subselect-contacts "[-inter-chain]" \
  --remap-chains \
| column -t
```

## Clustering of models

When running in all-versus-all comparison mode, CAD-score-LT can cluster models with a Taylor-Butina-like algorithm - the same algorithm as used for clustering in [PPI3D](https://bioinformatics.lt/ppi3d/).
Multiple CAD-score thresholds can be provided to obtain multiple clusterings in one run.

For example:

```bash
find ./input/protein_rna/ -type f -name '*model*' \
| cadscore-lt \
  --scoring-levels residue \
  --scoring-types contacts \
  --subselect-contacts '[-a1 [-protein] -a2 [-nucleic]]' \
  --output-dir ./output_dir_v1 \
  --clustering-thresholds 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0
```

This will produce both all-versus-all global scores and clustering info in the following files in the output directory:

* `./output_dir_v1/numbered_input_files.tsv`
* `./output_dir_v1/global_scores.tsv`
* `./output_dir_v1/residue_residue_cadscore_clusters.tsv`
* `./output_dir_v1/residue_residue_cadscore_cluster_representatives.tsv`

An example of a `input_model_files.tsv` file:

```
0  6FPQ_model1a.pdb
1  6FPQ_model1b.pdb
2  6FPQ_model2a.pdb
3  6FPQ_model2b.pdb
4  6FPQ_model3a.pdb
5  6FPQ_model3b.pdb
```

An example of a `global_scores.tsv` file:

```
target            model             t_id  m_id  residue_residue_cadscore
6FPQ_model3a.pdb  6FPQ_model3b.pdb  4     5     0.878349
6FPQ_model3b.pdb  6FPQ_model3a.pdb  5     4     0.877782
6FPQ_model1a.pdb  6FPQ_model1b.pdb  0     1     0.854266
6FPQ_model1b.pdb  6FPQ_model1a.pdb  1     0     0.849624
6FPQ_model2b.pdb  6FPQ_model2a.pdb  3     2     0.84137
6FPQ_model2a.pdb  6FPQ_model2b.pdb  2     3     0.837263
6FPQ_model1a.pdb  6FPQ_model2a.pdb  0     2     0.377615
6FPQ_model1a.pdb  6FPQ_model2b.pdb  0     3     0.373931
6FPQ_model2a.pdb  6FPQ_model1a.pdb  2     0     0.368113
6FPQ_model1b.pdb  6FPQ_model2b.pdb  1     3     0.36464
6FPQ_model1b.pdb  6FPQ_model2a.pdb  1     2     0.35661
6FPQ_model2a.pdb  6FPQ_model1b.pdb  2     1     0.344747
6FPQ_model2b.pdb  6FPQ_model1a.pdb  3     0     0.344366
6FPQ_model2b.pdb  6FPQ_model1b.pdb  3     1     0.332512
6FPQ_model3a.pdb  6FPQ_model2a.pdb  4     2     0.201901
6FPQ_model2b.pdb  6FPQ_model3a.pdb  3     4     0.188873
6FPQ_model3a.pdb  6FPQ_model2b.pdb  4     3     0.188156
6FPQ_model3b.pdb  6FPQ_model2a.pdb  5     2     0.183383
6FPQ_model2a.pdb  6FPQ_model3a.pdb  2     4     0.183358
6FPQ_model3b.pdb  6FPQ_model2b.pdb  5     3     0.173589
6FPQ_model2b.pdb  6FPQ_model3b.pdb  3     5     0.16057
6FPQ_model2a.pdb  6FPQ_model3b.pdb  2     5     0.152587
6FPQ_model1a.pdb  6FPQ_model3a.pdb  0     4     0.13624
6FPQ_model1b.pdb  6FPQ_model3b.pdb  1     5     0.129939
6FPQ_model1b.pdb  6FPQ_model3a.pdb  1     4     0.129754
6FPQ_model1a.pdb  6FPQ_model3b.pdb  0     5     0.125677
6FPQ_model3a.pdb  6FPQ_model1a.pdb  4     0     0.089375
6FPQ_model3b.pdb  6FPQ_model1b.pdb  5     1     0.087488
6FPQ_model3b.pdb  6FPQ_model1a.pdb  5     0     0.084623
6FPQ_model3a.pdb  6FPQ_model1b.pdb  4     1     0.078454

```

When dealing with large sets of models (e.g. thousands of models), global scores in the default vebose format may take a lot of space.
In such cases it is recommended to use the `--extremely-compact-output` output flag, for example:

```bash
find ./input/protein_rna/ -type f -name '*model*' \
| cadscore-lt \
  --scoring-levels residue \
  --scoring-types contacts \
  --subselect-contacts '[-a1 [-protein] -a2 [-nucleic]]' \
  --output-dir ./output_dir_v2 \
  --extremely-compact-output \
  --clustering-thresholds 0.3 0.4 0.5 0.6 0.7 0.8 0.9
```

This will output global 0-1 scores converted to integer 0-100 values and save the in the `output_dir_v2/residue_residue_cadscore_global.tsv` file.
That file holds a matrix, where rows and columns correspond to model ordering in the `output_dir_v2/input_model_files.tsv` file.

Overall, the "extremely compact" regime will produce the following files relevant to the clustering:

* `./output_dir_v2/input_model_files.tsv`
* `./output_dir_v2/residue_residue_cadscore_global.tsv`
* `./output_dir_v2/residue_residue_cadscore_clusters.tsv`
* `./output_dir_v2/residue_residue_cadscore_cluster_representatives.tsv`

An example of a `input_model_files.tsv` matrix file:

```
6FPQ_model1a.pdb
6FPQ_model1b.pdb
6FPQ_model2a.pdb
6FPQ_model2b.pdb
6FPQ_model3a.pdb
6FPQ_model3b.pdb
```

An example of a `residue_residue_cadscore_global.tsv` matrix file:

```
100  85   37   34   9    8
85   100  34   33   8    9
38   36   100  84   20   18
37   36   84   100  19   17
14   13   18   19   100  88
13   13   15   16   88   100
```

The generated clustering data files are the same for both the default and the "extremely compact" regimes.

An example of a `residue_residue_cadscore_clusters.tsv` file:

```
name              threshold_30  threshold_40  threshold_50  threshold_60  threshold_70  threshold_80  threshold_90
6FPQ_model1a.pdb  1             1             1             1             1             1             1
6FPQ_model1b.pdb  1             1             1             1             1             1             2
6FPQ_model2a.pdb  1             2             2             2             2             2             3
6FPQ_model2b.pdb  1             2             2             2             2             2             4
6FPQ_model3a.pdb  2             3             3             3             3             3             5
6FPQ_model3b.pdb  2             3             3             3             3             3             6
```

An example of a `residue_residue_cadscore_cluster_representatives.tsv` file:

```
threshold_percents  cluster_id  representative    cluster_size

30                  1           6FPQ_model1a.pdb  4
30                  2           6FPQ_model3a.pdb  2

40                  1           6FPQ_model1a.pdb  2
40                  2           6FPQ_model2a.pdb  2
40                  3           6FPQ_model3a.pdb  2

50                  1           6FPQ_model1a.pdb  2
50                  2           6FPQ_model2a.pdb  2
50                  3           6FPQ_model3a.pdb  2

60                  1           6FPQ_model1a.pdb  2
60                  2           6FPQ_model2a.pdb  2
60                  3           6FPQ_model3a.pdb  2

70                  1           6FPQ_model1a.pdb  2
70                  2           6FPQ_model2a.pdb  2
70                  3           6FPQ_model3a.pdb  2

80                  1           6FPQ_model1a.pdb  2
80                  2           6FPQ_model2a.pdb  2
80                  3           6FPQ_model3a.pdb  2

90                  1           6FPQ_model1a.pdb  1
90                  2           6FPQ_model1b.pdb  1
90                  3           6FPQ_model2a.pdb  1
90                  4           6FPQ_model2b.pdb  1
90                  5           6FPQ_model3a.pdb  1
90                  6           6FPQ_model3b.pdb  1
```

## Pre-processing small-molecule ligand input

For consistent naming of input small-molecule ligand atoms, and for determining small-molecule atom equivalence classes,
[extra data-preparation utilities for CAD-score-LT](https://github.com/kliment-olechnovic/cadscorelt-extra-data-utilities)
are provided in a separate repository.

The available tools are:

* [prepare-canonical-receptor-ligand-mmcif.py](https://github.com/kliment-olechnovic/cadscorelt-extra-data-utilities/blob/master/prepare-canonical-receptor-ligand-mmcif/README.md) is a command-line utility for preparing molecular structure files with consistent ligand atom naming.

* [group-equivalent-ligand-atoms.py](https://github.com/kliment-olechnovic/cadscorelt-extra-data-utilities/blob/master/group-equivalent-ligand-atoms/README.md) is a command-line utility that reads a molecular structure from file, identifies unique ligands, computes RDKit atom symmetry classes, and outputs standardized equivalence-based atom names.

The usage of those tools is not required, but suggested in order to ensure appropriate handling of ligand atom names to not penalize atom-level CAD-score.

### Equivalence classes data

The folloving optional argument of CAD-score-LT accepts atom equivalence information input:

```
--conflation-config-file                         string     input file path for reading atom name conflation rules
```

The expected file format is tab-separated table with three columns: residue name, atom name, generated equivalence class name.
Such table files can be generated by the [group-equivalent-ligand-atoms.py](https://github.com/kliment-olechnovic/cadscorelt-extra-data-utilities/blob/master/group-equivalent-ligand-atoms/README.md) script.

For example, run the script and save the output to a file:

```bash
python3 ./group-equivalent-ligand-atoms.py ./1CNW.cif > ./table.tsv
```

The file `table.tsv` should contain a table where each row corresponds to an atom that belongs to a multi-member equivalence class within a ligand:

```
EG1    O1    OX4
EG1    O2    OX4
EG1    C2    CX12
EG1    C3    CX13
EG1    C5    CX13
EG1    C6    CX
```
