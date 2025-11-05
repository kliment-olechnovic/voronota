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
chainID residueNumber residueName atomName x y z radius
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

