# More about generating tessellation-based data graphs from structures

Two utility scripts for generating Voronoi tessellation-based data graphs are included in [the Voronota-JS expansion](./index.html) of Voronota:

* `voronota-js-receptor-data-graph` - for generating graphs with an extended set of annotations
* `voronota-js-lt-data-graph` - for generating basic graphs, but much faster, using the Voronota-LT algorithm

These scripts are available in the latest version of [the Voronota package](../index.html#voronota-on-github), in the `expanson_js` subdirectory.


## Generating graphs with an extended set of annotations

The command-line interface of the `voronota-js-receptor-data-graph` script is described below:
	
	'voronota-js-receptor-data-graph' script describes a receptor protein structure as an annotated graph.
	
	Options:
	    --input                   string  *  path to input protein file
	    --probe-min               number     scanning probe radius minimum, default is 2.0
	    --probe-max               number     scanning probe radius maximum, default is 30.0
	    --buriedness-core         number     buriedness minimum for pocket start, default is 0.7
	    --buriedness-rim          number     buriedness maximum for pocket end, default is 0.4
	    --subpockets              number     number of sorted subpockets to include, default is 999999
	    --output-dir              string  *  output directory path
	    --output-naming           string     output files naming mode, default is 'BASENAME/name', other possibilities are 'BASENAME_name' and 'BASENAME/BASENAME_name'
	    --help | -h                          flag to display help message and exit
	    
	Standard output:
	    Information messages in stdout, error messages in stderr
	    
	Examples:
	    voronota-js-receptor-data-graph --input "./2zsk.pdb" --output-dir "./output"
	    
	    voronota-js-receptor-data-graph --input "./2zsk.pdb" --output-dir "./output" --probe-min 1 --probe-max 5 --buriedness-core 0.8 --buriedness-rim 0.7 --subpockets 1

### Example of execution and output

Running

	voronota-js-receptor-data-graph --input "./2zsk.pdb" --output-dir "./output"
	
generates four .TSV files:

* `./output/2zsk/atom_graph_nodes.tsv` - atom-level graph nodes table file, one row per node, every node represents an atom
* `./output/2zsk/atom_graph_links.tsv` - atom-level graph links (edges) table file, one row per link, every link represents an atom-atom contact
* `./output/2zsk/residue_graph_nodes.tsv` - residue-level graph nodes table file, one row per node, every node represents a residue
* `./output/2zsk/residue_graph_links.tsv` - residue-level graph links (edges) table file, one row per link, every link represents a residue-residue contact

It also generates .PDB files with "buriedness" and "pocketness" values written as b-factors - in case a user wants to visualize those values in a 3D viewer.

### Data format of the atom graph nodes file

Example (first 10 lines) from the file `./output/2zsk/atom_graph_nodes.tsv`:

	ID_chainID  ID_resSeq  ID_iCode  ID_serial  ID_altLoc  ID_resName  ID_name  atom_index  residue_index  atom_type  residue_type  center_x  center_y  center_z  radius  sas_area  solvdir_x   solvdir_y   solvdir_z   voromqa_sas_energy  voromqa_depth  voromqa_score_a  voromqa_score_r  volume   volume_vdw  ev14       ev28      ev56       ufsr_a1  ufsr_a2  ufsr_a3   ufsr_b1  ufsr_b2  ufsr_b3   ufsr_c1  ufsr_c2  ufsr_c3  buriedness  pocketness
	A           1          .         .          .          MET         N        0           0              97         12            22.493    56.29     12.4      1.7     53.351    0.559085    0.82758     -0.0503525  57.9168             1              1.01688e-07      0.167578         66.5845  15.242      0.125304   0.124587  0.121036   30.0876  112.815  -196.313  41.8069  256.541  -1727.84  37.0975  249.607  439.762  0.132536    0
	A           1          .         .          .          MET         CA       1           0              93         12            21.658    55.076    12.14     1.9     4.72457   0.135243    0.432065    -0.891644   6.17629             1              0.015941         0.167578         26.134   13.3463     0.0448295  0.039199  0.0265035  30.0876  112.815  -196.313  41.8069  256.541  -1727.84  37.0975  249.607  439.762  0.0847213   0
	A           1          .         .          .          MET         C        2           0              92         12            20.271    55.177    12.793    1.75    2.62013   -0.250994   0.910664    -0.328166   5.18333             1              0.292879         0.167578         14.0446  8.87448     0.601112   0.563979  0.457316   30.0876  112.815  -196.313  41.8069  256.541  -1727.84  37.0975  249.607  439.762  0.13229     0
	A           1          .         .          .          MET         O        3           0              98         12            20.135    55.689    13.912    1.49    11.2317   0.372405    0.737202    0.56378     8.18408             1              0.361824         0.167578         31.8395  9.38393     0.661347   0.635732  2          30.0876  112.815  -196.313  41.8069  256.541  -1727.84  37.0975  249.607  439.762  0.139913    0
	A           1          .         .          .          MET         CB       4           0              94         12            22.393    53.825    12.646    1.91    27.6946   0.886864    -0.0929711  0.452581    17.787              1              0.167248         0.167578         74.006   23.068      0.63118    0.590042  0.584411   30.0876  112.815  -196.313  41.8069  256.541  -1727.84  37.0975  249.607  439.762  0.121678    0
	A           2          .         .          .          LYS         N        5           1              89         11            19.254    54.687    12.079    1.7     2.52782   -0.0429483  0.809307    -0.585814   3.47919             1              0.052448         0.472209         17.3995  10.0202     0.55985    0.535622  0.457316   29.1886  119.733  -71.1108  41.8069  256.541  -1727.84  37.0975  249.607  439.762  0.0643745   0
	A           2          .         .          .          LYS         CA       6           1              84         11            17.871    54.701    12.555    1.9     2.06927   0.0828678   0.93132     -0.354648   1.32819             1              0.342294         0.472209         18.5844  12.7854     0.62449    0.611648  0.645691   29.1886  119.733  -71.1108  41.8069  256.541  -1727.84  37.0975  249.607  439.762  0.0846022   0
	A           2          .         .          .          LYS         C        7           1              83         11            17.712    53.714    13.699    1.75    0.033581  0.809845    -0.165563   0.562797    0.0571512           1              0.698036         0.472209         11.1993  8.68042     0.965086   2         2          29.1886  119.733  -71.1108  41.8069  256.541  -1727.84  37.0975  249.607  439.762  2           0
	A           2          .         .          .          LYS         O        8           1              91         11            18.114    52.551    13.6      1.49    1.14708   0.796577    -0.0907621  0.597685    0.209446            1              0.571392         0.472209         21.023   9.3442      0.965086   2         2          29.1886  119.733  -71.1108  41.8069  256.541  -1727.84  37.0975  249.607  439.762  2           0

Description of the columns:

* __ID_chainID__ - chain name in PDB file, given just for reference
* __ID_resSeq__ - residue number in PDB file, given just for reference
* __ID_iCode__ - insertion code in PDB file, usually null and written as '.', given just for reference
* __ID_altLoc__ - atom alternate location indicator in PDB file, usually null and written as '.', given just for reference
* __ID_serial__ - atom serial number in PDB file, usually null and written as '.', given just for reference
* __ID_resName__ - residue name
* __ID_name__ - atom name
* __atom_index__ - atom index (starting from 0), used to describe atom-atom links in the corresponding links.tsv file
* __residue_index__ - residue index (starting from 0), to be used for pooling (from atom-level to residue-level)
* __atom_type__ - atom type encoded as a number from 0 to 159
* __residue_type__ - amino acid residue type encoded as a number from 0 to 19
* __center_x__, __center_y__, __center_z__ - atom center coordinates, to be either ignored or used with special care ensuring rotational and translational invariance
* __radius__ - atom van der Waals radius
* __sas_area__ - solvent-accessible surface area: larger area means that the atom is less buried and more exposed
* __solvdir_x__, __solvdir_y__, __solvdir_z__ - mean solvent-accessible surface direction vector, to be either ignored or used with special care ensuring rotational and translational invariance
* __voromqa_sas_energy__ - observed atom-solvent VoroMQA energy value
* __voromqa_depth__ - atom topological distance from the surface, starts with 1 for surface atoms
* __voromqa_score_a__ - atom-level VoroMQA score
* __voromqa_score_r__ - residue-level VoroMQA score, same for all the  atoms in the same residue
* __volume__ - volume of the Voronoi cell of an atom constrained inside the solvent-accessible surface
* __volume_vdw__ - volume of the Voronoi cell of an atom constrained inside the van der Waals surface
* __ev14__, __ev28__, __ev56__ - geometric buriedness values for different minimum probing radii (1.4, 2.8, 5.6), range from 0 (most exposed) to 1 (most buried), value of 2 is assigned to all atoms not accessible by external probes
* __ufsr_a1__, __ufsr_a2__, ... , __ufsr_c2__, __ufsr_c3__ - geometric descriptors calculated using the Ultra-fast Shape Recognition algorithm adapted for polymers
* __buriedness__ - geometric buriedness value, calculated using the provided parameters
* __pocketness__ - pocket number, calculated using the provided parameters

### Data format of the atom graph links file

Example (first 10 lines) from the file `./output/2zsk/atom_graph_links.tsv`:

	ID1_chainID  ID1_resSeq  ID1_iCode  ID1_serial  ID1_altLoc  ID1_resName  ID1_name  ID2_chainID  ID2_resSeq  ID2_iCode  ID2_serial  ID2_altLoc  ID2_resName  ID2_name  atom_index1  atom_index2  area      boundary  distance  voromqa_energy  seq_sep_class  covalent_bond
	A            1           .          .           .           MET          N         A            1           .          .           .           MET          CA        0            1            15.4593   6.06454   1.4962    0               0              1
	A            1           .          .           .           MET          N         A            1           .          .           .           MET          C         0            2            2.49966   1.78477   2.51605   0               0              0
	A            1           .          .           .           MET          N         A            1           .          .           .           MET          O         0            3            6.83024   5.45489   2.86488   0               0              0
	A            1           .          .           .           MET          N         A            1           .          .           .           MET          CB        0            4            9.44152   6.97601   2.47926   0               0              0
	A            1           .          .           .           MET          CA        A            1           .          .           .           MET          C         1            2            8.27879   0.824826  1.53635   0               0              1
	A            1           .          .           .           MET          CA        A            1           .          .           .           MET          O         1            3            0.751241  0         2.41563   0               0              0
	A            1           .          .           .           MET          CA        A            1           .          .           .           MET          CB        1            4            12.4635   1.04702   1.53664   0               0              1
	A            1           .          .           .           MET          CA        A            2           .          .           .           LYS          N         1            5            4.95624   1.021     2.43603   0               1              0
	A            1           .          .           .           MET          CA        A            224         .          .           .           ALA          O         1            1783         2.2368    0         3.58016   -0.886374       5              0

Description of the columns:

* __ID1_chainID__, __ID1_resSeq__, __ID1_iCode__, __ID1_serial__, __ID1_altLoc__, __ID1_resName__, __ID1_name__ -  general info about the first atom participating in the link, see descriptions of ID columns of the nodes table
* __ID2_chainID__, __ID2_resSeq__, __ID2_iCode__, __ID2_serial__, __ID2_altLoc__, __ID2_resName__, __ID2_name__ -  general info about the second atom participating in the link, see descriptions of ID columns of the nodes table
* __atom_index1__ - node index of the first atom participating in the link
* __atom_index2__ - node index of the second atom participating in the link
* __area__ - tessellation-derived contact area
* __boundary__ - length of the contact-solvent boundary, 0 if contact is not adjacent to the solvent-accessible surface
* __distance__ - distance between two atoms
* __voromqa_energy__ - contact VoroMQA-energy value
* __seq_sep_class__ - residue sequence separation class, ranging from 0 (sequence separation = 0) to 5 (sequence separation >= 5)
* __covalent_bond__ - covalent bond indicator (0 or 1)

### Data format of the residue graph nodes file

Example (first 10 lines) from the file `./output/2zsk/residue_graph_nodes.tsv`:

	ID_chainID  ID_resSeq  ID_iCode  ID_serial  ID_altLoc  ID_resName  ID_name  residue_index  residue_type  residue_sas_area  residue_volume  residue_voromqa_sas_energy  residue_mean_voromqa_depth  voromqa_score_r  residue_min_ev14  residue_max_ev14  residue_mean_ev14  residue_min_ev28  residue_max_ev28  residue_mean_ev28  residue_min_ev56  residue_max_ev56  residue_mean_ev56  residue_min_buriedness  residue_max_buriedness  residue_mean_buriedness
	A           1          .         .          .          MET         .        0              12            99.622            212.609         95.2474                     1                           0.167578         0.0448295         0.661347          0.335069           0.039199          0.635732          0.319117           0.0265035         0.584411          0.240568           0.0847213               0.139913                0.128075
	A           2          .         .          .          LYS         .        1              11            26.6093           132.673         2.76791                     1                           0.472209         0.502281          0.965086          0.537789           0.498853          0.611648          0.488983           0.457316          0.645691          0.452329           0.0643745               0.0846022               0.0788722
	A           3          .         .          .          LYS         .        2              11            66.3108           253.264         -53.6353                    1.44444                     0.613378         0.337858          0.965086          0.366175           0.337066          0.621141          0.36095            0.334376          0.473626          0.351902           0.337636                0.34682                 0.337817
	A           4          .         .          .          ILE         .        3              9             0                 167.223         0                           2.5                         0.643901         2                 2                 2                  2                 2                 2                  2                 2                 2                  2                       2                       2
	A           5          .         .          .          GLY         .        4              7             0                 63.6288         0                           2.25                        0.996122         2                 2                 2                  2                 2                 2                  2                 2                 2                  2                       2                       2
	A           6          .         .          .          ILE         .        5              9             1.9557            175.105         1.55475                     1.75                        0.913145         0.978935          0.994222          0.981069           2                 2                 2                  2                 2                 2                  2                       2                       2
	A           7          .         .          .          ILE         .        6              9             0.997957          170.309         0.912408                    2                           0.895152         0.990166          0.990742          0.990307           2                 2                 2                  2                 2                 2                  2                       2                       2
	A           8          .         .          .          GLY         .        7              7             0                 69.5537         0                           2                           0.970268         2                 2                 2                  2                 2                 2                  2                 2                 2                  2                       2                       2
	A           9          .         .          .          GLY         .        8              7             0                 63.8229         0                           2                           0.848999         2                 2                 2                  2                 2                 2                  2                 2                 2                  2                       2                       2

Description of the columns: they are __per-residue summaries of the columns from the atom graph nodes file__.

### Data format of the residue graph links file

Example (first 10 lines) from the file `./output/2zsk/residue_graph_links.tsv`:

	ID1_chainID  ID1_resSeq  ID1_iCode  ID1_serial  ID1_altLoc  ID1_resName  ID1_name  ID2_chainID  ID2_resSeq  ID2_iCode  ID2_serial  ID2_altLoc  ID2_resName  ID2_name  area     boundary  distance  voromqa_energy
	A            1           .          .           .           MET          .         A            2           .          .           .           LYS          .         26.7853  5.60234   1.33573   0
	A            1           .          .           .           MET          .         A            3           .          .           .           LYS          .         11.6844  4.40933   3.46409   4.30572
	A            1           .          .           .           MET          .         A            34          .          .           .           PHE          .         7.33832  7.65326   5.72583   -12.2026
	A            1           .          .           .           MET          .         A            36          .          .           .           PRO          .         10.5094  4.20942   5.41294   -4.09459
	A            1           .          .           .           MET          .         A            37          .          .           .           GLU          .         2.13388  6.22514   5.51268   2.25328
	A            1           .          .           .           MET          .         A            75          .          .           .           GLU          .         4.96855  3.02668   3.82408   6.90425
	A            1           .          .           .           MET          .         A            224         .          .           .           ALA          .         9.9518   0         3.34954   -0.823021
	A            1           .          .           .           MET          .         A            225         .          .           .           SER          .         10.3855  6.90767   4.80928   3.00817
	A            1           .          .           .           MET          .         A            226         .          .           .           GLU          .         6.02953  3.85763   4.80279   -0.209601

Description of the columns: they are __per-residue-residue contact summaries of the columns from the atom graph links file__.

## Fast generation of basic graphs

The command-line interface of the `voronota-js-lt-data-graph` script is described below:

	'voronota-js-lt-graph' script describes a molecular structure as a graph.
	
	Options:
	    --input                   string  *  path to input protein file
	    --probe                   number     probe radius is 1.4
	    --output-dir              string  *  output directory path
	    --output-naming           string     output files naming mode, default is 'BASENAME/name', other possibilities are 'BASENAME_name' and 'BASENAME/BASENAME_name'
	    --help | -h                          flag to display help message and exit
	    
	Standard output:
	    Information messages in stdout, error messages in stderr
	    
	Examples:
	    voronota-js-lt-graph --input "./2zsk.pdb" --output-dir "./output"
	    
	    voronota-js-lt-graph --input "./2zsk.pdb" --output-dir "./output" --probe 1.4

### Example of execution and output

Running

	voronota-js-lt-data-graph --input "./2zsk.pdb" --output-dir "./output"
	
generates four .TSV files:

* `./output/2zsk/atom_graph_nodes.tsv` - atom-level graph nodes table file, one row per node, every node represents an atom
* `./output/2zsk/atom_graph_links.tsv` - atom-level graph links (edges) table file, one row per link, every link represents an atom-atom contact
* `./output/2zsk/residue_graph_nodes.tsv` - residue-level graph nodes table file, one row per node, every node represents a residue
* `./output/2zsk/residue_graph_links.tsv` - residue-level graph links (edges) table file, one row per link, every link represents a residue-residue contact

### Data format of the atom graph nodes file

Example (first 10 lines) from the file `./output/2zsk/atom_graph_nodes.tsv`:

	ID_chainID  ID_resSeq  ID_iCode  ID_serial  ID_altLoc  ID_resName  ID_name  atom_index  residue_index  atom_type  residue_type  center_x  center_y  center_z  radius  sas_area   volume
	A           1          .         .          .          MET         N        0           0              97         12            22.493    56.29     12.4      1.7     53.3426    63.8989
	A           1          .         .          .          MET         CA       1           0              93         12            21.658    55.076    12.14     1.9     4.70418    30.7944
	A           1          .         .          .          MET         C        2           0              92         12            20.271    55.177    12.793    1.75    2.62416    15.6203
	A           1          .         .          .          MET         O        3           0              98         12            20.135    55.689    13.912    1.49    11.1833    27.3368
	A           1          .         .          .          MET         CB       4           0              94         12            22.393    53.825    12.646    1.91    27.5103    75.9687
	A           2          .         .          .          LYS         N        5           1              89         11            19.254    54.687    12.079    1.7     2.52533    14.778
	A           2          .         .          .          LYS         CA       6           1              84         11            17.871    54.701    12.555    1.9     2.06606    23.2376
	A           2          .         .          .          LYS         C        7           1              83         11            17.712    53.714    13.699    1.75    0.0340018  12.7911
	A           2          .         .          .          LYS         O        8           1              91         11            18.114    52.551    13.6      1.49    1.12595    15.7616

Description of the columns:

* __ID_chainID__ - chain name in PDB file, given just for reference
* __ID_resSeq__ - residue number in PDB file, given just for reference
* __ID_iCode__ - insertion code in PDB file, usually null and written as '.', given just for reference
* __ID_altLoc__ - atom alternate location indicator in PDB file, usually null and written as '.', given just for reference
* __ID_serial__ - atom serial number in PDB file, usually null and written as '.', given just for reference
* __ID_resName__ - residue name
* __ID_name__ - atom name
* __atom_index__ - atom index (starting from 0), used to describe atom-atom links in the corresponding links.tsv file
* __residue_index__ - residue index (starting from 0), to be used for pooling (from atom-level to residue-level)
* __atom_type__ - atom type encoded as a number from 0 to 159
* __residue_type__ - amino acid residue type encoded as a number from 0 to 19
* __center_x__, __center_y__, __center_z__ - atom center coordinates, to be either ignored or used with special care ensuring rotational and translational invariance
* __radius__ - atom van der Waals radius
* __sas_area__ - solvent-accessible surface area: larger area means that the atom is less buried and more exposed
* __volume__ - volume of the Voronoi cell of an atom constrained inside the solvent-accessible surface

### Data format of the atom graph links file

Example (first 10 lines) from the file `./output/2zsk/atom_graph_links.tsv`:

	ID1_chainID  ID1_resSeq  ID1_iCode  ID1_serial  ID1_altLoc  ID1_resName  ID1_name  ID2_chainID  ID2_resSeq  ID2_iCode  ID2_serial  ID2_altLoc  ID2_resName  ID2_name  atom_index1  atom_index2  area     boundary  distance
	A            1           .          .           .           MET          N         A            1           .          .           .           MET          CA        0            1            16.5137  6.06483   1.4962
	A            1           .          .           .           MET          N         A            1           .          .           .           MET          CB        0            4            8.78012  6.97738   2.47926
	A            1           .          .           .           MET          N         A            1           .          .           .           MET          C         0            2            2.26495  1.78545   2.51605
	A            1           .          .           .           MET          N         A            1           .          .           .           MET          O         0            3            5.95355  5.4563    2.86488
	A            1           .          .           .           MET          CA        A            1           .          .           .           MET          CB        1            4            14.3565  1.04322   1.53664
	A            1           .          .           .           MET          CA        A            1           .          .           .           MET          C         1            2            9.42943  0.82315   1.53635
	A            1           .          .           .           MET          CA        A            2           .          .           .           LYS          N         1            5            5.50647  1.01891   2.43603
	A            1           .          .           .           MET          CA        A            224         .          .           .           ALA          O         1            1783         2.13187  0         3.58016
	A            1           .          .           .           MET          CA        A            226         .          .           .           GLU          OXT       1            1800         5.78086  3.06435   4.80279

Description of the columns:

* __ID1_chainID__, __ID1_resSeq__, __ID1_iCode__, __ID1_serial__, __ID1_altLoc__, __ID1_resName__, __ID1_name__ -  general info about the first atom participating in the link, see descriptions of ID columns of the nodes table
* __ID2_chainID__, __ID2_resSeq__, __ID2_iCode__, __ID2_serial__, __ID2_altLoc__, __ID2_resName__, __ID2_name__ -  general info about the second atom participating in the link, see descriptions of ID columns of the nodes table
* __atom_index1__ - node index of the first atom participating in the link
* __atom_index2__ - node index of the second atom participating in the link
* __area__ - tessellation-derived contact area
* __boundary__ - length of the contact-solvent boundary, 0 if contact is not adjacent to the solvent-accessible surface
* __distance__ - distance between two atoms

### Data format of the residue graph nodes file

Example (first 10 lines) from the file `./output/2zsk/residue_graph_nodes.tsv`:

	ID_chainID  ID_resSeq  ID_iCode  ID_serial  ID_altLoc  ID_resName  ID_name  residue_type  residue_sas_area  residue_volume
	A           1          .         .          .          MET         .        12            99.3646           213.619
	A           2          .         .          .          LYS         .        11            26.5311           132.432
	A           3          .         .          .          LYS         .        11            66.24             256.468
	A           4          .         .          .          ILE         .        9             0                 168.399
	A           5          .         .          .          GLY         .        7             0                 62.1498
	A           6          .         .          .          ILE         .        9             1.93343           175.439
	A           7          .         .          .          ILE         .        9             0.989463          171.733
	A           8          .         .          .          GLY         .        7             0                 69.6737
	A           9          .         .          .          GLY         .        7             0                 63.6192

Description of the columns: they are __per-residue summaries of the columns from the atom graph nodes file__.

### Data format of the residue graph links file

Example (first 10 lines) from the file `./output/2zsk/residue_graph_links.tsv`:

	ID1_chainID  ID1_resSeq  ID1_iCode  ID1_serial  ID1_altLoc  ID1_resName  ID1_name  ID2_chainID  ID2_resSeq  ID2_iCode  ID2_serial  ID2_altLoc  ID2_resName  ID2_name  area     boundary  distance
	A            1           .          .           .           MET          .         A            2           .          .           .           LYS          .         27.3555  5.59216   1.33573
	A            1           .          .           .           MET          .         A            3           .          .           .           LYS          .         11.287   4.39984   3.46409
	A            1           .          .           .           MET          .         A            34          .          .           .           PHE          .         7.35286  7.65587   5.72583
	A            1           .          .           .           MET          .         A            36          .          .           .           PRO          .         10.5861  4.18204   5.41294
	A            1           .          .           .           MET          .         A            37          .          .           .           GLU          .         2.13882  6.27953   5.51268
	A            1           .          .           .           MET          .         A            75          .          .           .           GLU          .         4.52381  3.02635   3.82408
	A            1           .          .           .           MET          .         A            224         .          .           .           ALA          .         9.38126  0         3.34954
	A            1           .          .           .           MET          .         A            225         .          .           .           SER          .         11.0272  6.90226   4.80928
	A            1           .          .           .           MET          .         A            226         .          .           .           GLU          .         6.14355  3.85731   4.80279

Description of the columns: they are __per-residue-residue contact summaries of the columns from the atom graph links file__.

