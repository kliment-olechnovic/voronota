# CAD-score-LT Python bindings

The CAD-score-LT Python interface PyPI package is hosted at [https://pypi.org/project/cadscorelt/](https://pypi.org/project/cadscorelt/).

## Installation

Install with pip using this command:

```bash
pip install cadscorelt
```

Additionally, it is recommended to have the [pandas](https://pandas.pydata.org/) library for data analysis available in the Python environment.
This allows the CAD-score result tables to be converted to [pandas data frames](https://pandas.pydata.org/docs/reference/frame.html).
The ``pandas`` library can also be installed using ``pip``:

```bash
pip install pandas
```

CAD-score-LT also provides integration with some common libraries for reading macromolecular files - [Biotite](https://www.biotite-python.org/), [Gemmi](https://gemmi.readthedocs.io/), [Biopython](https://biopython.org/),
if those libraries are available in the Python environment.
They can be installed via ``pip``:

```bash
pip install biotite
pip install gemmi
pip install biopython
```

## Usage examples

### Basic example

Below is an example script that calculates CAD-scores for inter-chain residue-residue contact areas,
produces a table of global scores,
converts that table to ``pandas`` data frame,
and preints the top rows of the data frame:

```py
import cadscorelt

# init a CAD-score computation object
css = cadscorelt.CADScoreComputer.init(subselect_contacts="[-inter-chain]")

# add a target structure
css.add_target_structure_from_file("./input/data/protein_homodimer1/target.pdb")

# add a target structure
css.add_model_structure_from_file("./input/data/protein_homodimer1/model1.pdb")
css.add_model_structure_from_file("./input/data/protein_homodimer1/model2.pdb")

# get a list of global scores and convert it to pandas data frame
df_global_scores_residue_residue = css.get_all_cadscores_residue_residue_summarized_globally().to_pandas()

# print the first rows of the data frame
cadscorelt.print_head_of_pandas_data_frame(df_global_scores_residue_residue)
```

Below is an example of the printed output:

```
target_name model_name  CAD_score  F1_of_areas  target_area  model_area    TP_area    FP_area    FN_area renamed_chains
     target     model2   0.621922     0.774894  1047.807935  941.514533 784.870893 193.071031 262.937041              .
     target     model1   0.507319     0.639249  1047.807935  792.834440 648.098138 331.779276 399.709796              .
```

### Basic example using different structure readers

Below is an example script that is similar to the previous example script, but it shows how to input structures from different sources:

```py
import cadscorelt

# init a CAD-score computation object
csc = cadscorelt.CADScoreComputer.init(subselect_contacts="[-inter-chain]")

# add a target structure read by Biotite
import biotite.structure.io
structure_target = biotite.structure.io.load_structure("./input/data/protein_homodimer1/target.pdb")
csc.add_target_structure_from_biotite(structure_target, "target")

# add a model structure read by Gemmi 
import gemmi
structure_model1 = gemmi.read_structure("./input/data/protein_homodimer1/model1.pdb")
csc.add_model_structure_from_gemmi(structure_model1[0], "model1")

# add a model structure read by Biopython 
import Bio.PDB
parser = Bio.PDB.PDBParser(QUIET=True)
structure_model2 = parser.get_structure("id", "./input/data/protein_homodimer1/model2.pdb").get_atoms()
csc.add_model_structure_from_biopython(structure_model2, "model2")

# get a list of global scores and convert it to pandas data frame
df_global_scores_residue_residue = csc.get_all_cadscores_residue_residue_summarized_globally().to_pandas()

# print the first rows of the data frame
cadscorelt.print_head_of_pandas_data_frame(df_global_scores_residue_residue)
```

Below is an example of the printed output:

```
target_name model_name  CAD_score  F1_of_areas  target_area  model_area    TP_area    FP_area    FN_area renamed_chains
     target     model2   0.621922     0.774894  1047.808013  941.514579 784.870926 193.071041 262.937087              .
     target     model1   0.507319     0.639249  1047.808013  792.834440 648.098141 331.779274 399.709873              .
```

### Advanced example

Below is an example script that that includes:

* residue-residue contact scoring (enabled by default);
* atom-atom contact scoring;
* automatic chain remapping to maximize grobal similarity;
* recording local scores.

```py
import cadscorelt
from pathlib import Path

# to make comparison more strict, globally enable inclusion of residue names into atom and residue identifiers
cadscorelt.enable_considering_residue_names()

# init a CAD-score computation object, enable atom-atom contact scoring, enable automatic chain remapping to maximize grobal similarity, enable recording local scores
csc = cadscorelt.CADScoreComputer.init(subselect_contacts="[-inter-chain]", score_atom_atom_contacts=True, remap_chains=True, record_local_scores=True)

# set reference sequences and stoichiometry for automatic residue renumbering and chain namne assignment
csc.set_reference_sequences_from_file("./input/data/protein_heteromer1/sequences.fasta")
csc.set_reference_stoichiometry([2, 2, 2])

# input structures from all the files in a directory
input_directory = Path("./input/data/protein_heteromer1/structures")
for file_path in input_directory.iterdir():
    if file_path.is_file():
        csc.add_structure_from_file(str(file_path))

# get the table of structure decriptors and print its top rows
df_structure_descriptors = csc.get_all_structure_descriptors().to_pandas()
print("")
print(" # Table of structure decriptors:")
print("")
cadscorelt.print_head_of_pandas_data_frame(df_structure_descriptors)
print("")

# get the table of globals scores based on residue-residue contacts, print top rows
df_global_scores_residue_residue = csc.get_all_cadscores_residue_residue_summarized_globally().to_pandas()
print("")
print(" # Table of globals scores based on residue-residue contacts:")
print("")
cadscorelt.print_head_of_pandas_data_frame(df_global_scores_residue_residue)
print("")

# get the table of globals scores based on atom-atom contacts, print top rows
df_global_scores_atom_atom = csc.get_all_cadscores_atom_atom_summarized_globally().to_pandas()
print("")
print(" # Table of globals scores based on atom-atom contacts:")
print("")
cadscorelt.print_head_of_pandas_data_frame(df_global_scores_atom_atom)
print("")

# set placeholder variable for structure names
target_name="cf_woTemplates_model_3_multimer_v3_pred_47"
model_name="cf_woTemplates_model_2_multimer_v3_pred_26"

# get the table of per-residue scores based on residue-residue contacts, print top rows
df_local_scores_per_residue = csc.get_local_cadscores_residue_residue_summarized_per_residue(target_name, model_name).to_pandas()
print("")
print(" # Table of per-residue scores based on residue-residue contacts:")
print("")
cadscorelt.print_head_of_pandas_data_frame(df_local_scores_per_residue)
print("")

# get the table of scores for every residue-residue contact, print top rows
df_local_scores_residue_residue = csc.get_local_cadscores_residue_residue(target_name, model_name).to_pandas()
print("")
print(" # Table of scores for every residue-residue contact (CAD-score values of -1 idicate that the contact was not present in the target structure):")
print("")
cadscorelt.print_head_of_pandas_data_frame(df_local_scores_residue_residue)
print("")

# get the table of per-atom scores based on atom-atom contacts, print top rows
df_local_scores_per_atom = csc.get_local_cadscores_atom_atom_summarized_per_atom(target_name, model_name).to_pandas()
print("")
print(" # Table of per-atom scores based on atom-atom contacts (CAD-score values of -1 idicate that the atom had no relevant contacts in the target structure):")
print("")
cadscorelt.print_head_of_pandas_data_frame(df_local_scores_per_atom)
print("")

# get the table of scores for every atom-atom contact, print top rows
df_local_scores_atom_atom = csc.get_local_cadscores_atom_atom(target_name, model_name).to_pandas()
print("")
print(" # Table of scores for every atom-atom contact (CAD-score values of -1 idicate that the contact was not present in the target structure):")
print("")
cadscorelt.print_head_of_pandas_data_frame(df_local_scores_atom_atom)
print("")
```

Below is an example of the printed output:

```

 # Table of structure decriptors:

                                                    name  is_target  is_model          renamed_chains reference_alignment
                   afm_basic_model_5_multimer_v1_pred_35       True      True B=A,C=D,D=B,E=E,F=C,G=F           available
            afm_dropout_full_model_1_multimer_v2_pred_42       True      True B=A,C=D,D=B,E=E,F=C,G=F           available
            afm_dropout_full_model_2_multimer_v1_pred_65       True      True B=A,C=D,D=B,E=E,F=C,G=F           available
            afm_dropout_full_model_3_multimer_v3_pred_64       True      True B=A,C=D,D=B,E=E,F=C,G=F           available
            afm_dropout_full_model_3_multimer_v3_pred_66       True      True B=A,C=D,D=B,E=E,F=C,G=F           available
 afm_dropout_full_woTemplates_model_3_multimer_v1_pred_4       True      True B=A,C=D,D=B,E=E,F=C,G=F           available
afm_dropout_full_woTemplates_model_3_multimer_v1_pred_45       True      True B=A,C=D,D=B,E=E,F=C,G=F           available
afm_dropout_full_woTemplates_model_4_multimer_v3_pred_50       True      True B=A,C=D,D=B,E=E,F=C,G=F           available
              cf_woTemplates_model_2_multimer_v3_pred_26       True      True A=A,B=D,C=B,D=E,E=C,F=F           available
              cf_woTemplates_model_3_multimer_v3_pred_47       True      True A=A,B=D,C=B,D=E,E=C,F=F           available


 # Table of globals scores based on residue-residue contacts:

                                 target_name                                               model_name  CAD_score  F1_of_areas  target_area  model_area     TP_area     FP_area     FN_area          renamed_chains
afm_dropout_full_model_1_multimer_v2_pred_42 afm_dropout_full_woTemplates_model_4_multimer_v3_pred_50   0.847662     0.701359  3970.007175 3836.593118 3582.890995 2664.103761  387.116180 A=A;B=B;C=F;D=D;E=E;F=C
  cf_woTemplates_model_3_multimer_v3_pred_47               cf_woTemplates_model_2_multimer_v3_pred_26   0.704688     0.780891  7483.973156 6871.838277 5912.094425 1745.848929 1571.878730 A=D;B=E;C=F;D=A;E=B;F=C
  cf_woTemplates_model_2_multimer_v3_pred_26               cf_woTemplates_model_3_multimer_v3_pred_47   0.699040     0.781187  7657.943354 6855.599895 5914.330867 1569.642289 1743.612487 A=A;B=B;C=C;D=D;E=E;F=F
afm_dropout_full_model_3_multimer_v3_pred_66             afm_dropout_full_model_3_multimer_v3_pred_64   0.657842     0.675283  7313.203511 5519.154809 5124.561234 2739.763621 2188.642277 A=A;B=B;C=C;D=D;E=E;F=F
afm_dropout_full_model_3_multimer_v3_pred_64             afm_dropout_full_model_3_multimer_v3_pred_66   0.620764     0.675283  7864.324855 5420.088717 5124.561234 2188.642277 2739.763621 A=A;B=B;C=C;D=D;E=E;F=F
afm_dropout_full_model_3_multimer_v3_pred_66               cf_woTemplates_model_2_multimer_v3_pred_26   0.575097     0.645935  7313.203511 5696.007986 4835.195167 2822.748187 2478.008344 A=A;B=E;C=C;D=D;E=B;F=F
  cf_woTemplates_model_2_multimer_v3_pred_26             afm_dropout_full_model_3_multimer_v3_pred_66   0.573808     0.645935  7657.943354 5446.078054 4835.195167 2478.008344 2822.748187 A=A;B=E;C=C;D=D;E=B;F=F
afm_dropout_full_model_3_multimer_v3_pred_66               cf_woTemplates_model_3_multimer_v3_pred_47   0.566239     0.627281  7313.203511 5493.415167 4640.996876 2842.976280 2672.206635 A=D;B=B;C=F;D=A;E=E;F=C
  cf_woTemplates_model_3_multimer_v3_pred_47             afm_dropout_full_model_3_multimer_v3_pred_66   0.561418     0.627332  7483.973156 5252.699687 4641.373085 2671.830426 2842.600071 A=A;B=E;C=C;D=D;E=B;F=F
  cf_woTemplates_model_2_multimer_v3_pred_26             afm_dropout_full_model_3_multimer_v3_pred_64   0.550970     0.603414  7657.943354 5255.694013 4683.177891 3181.146964 2974.765463 A=A;B=E;C=C;D=D;E=B;F=F


 # Table of globals scores based on atom-atom contacts:

                                 target_name                                               model_name  CAD_score  F1_of_areas  target_area  model_area     TP_area     FP_area     FN_area          renamed_chains
afm_dropout_full_model_1_multimer_v2_pred_42 afm_dropout_full_woTemplates_model_4_multimer_v3_pred_50   0.694870     0.612790  3970.007175 3631.663716 3130.437751 3116.557004  839.569423 A=A;B=B;C=F;D=D;E=E;F=C
afm_dropout_full_model_3_multimer_v3_pred_66             afm_dropout_full_model_3_multimer_v3_pred_64   0.600300     0.630986  7313.203511 5287.148112 4788.403033 3075.921822 2524.800478 A=A;B=B;C=C;D=D;E=E;F=F
afm_dropout_full_model_3_multimer_v3_pred_64             afm_dropout_full_model_3_multimer_v3_pred_66   0.566073     0.630986  7864.324855 5222.053418 4788.403033 2524.800478 3075.921822 A=A;B=B;C=C;D=D;E=E;F=F
  cf_woTemplates_model_3_multimer_v3_pred_47               cf_woTemplates_model_2_multimer_v3_pred_26   0.559002     0.635331  7483.973156 5931.794420 4810.063036 2847.880319 2673.910120 A=D;B=E;C=F;D=A;E=B;F=C
  cf_woTemplates_model_2_multimer_v3_pred_26               cf_woTemplates_model_3_multimer_v3_pred_47   0.550378     0.635281  7657.943354 5972.073003 4809.683681 2674.289475 2848.259673 A=A;B=B;C=C;D=D;E=E;F=F
afm_dropout_full_model_3_multimer_v3_pred_66               cf_woTemplates_model_2_multimer_v3_pred_26   0.455307     0.527325  7313.203511 5000.847832 3947.330056 3710.613299 3365.873455 A=A;B=E;C=C;D=D;E=B;F=F
  cf_woTemplates_model_2_multimer_v3_pred_26             afm_dropout_full_model_3_multimer_v3_pred_66   0.446372     0.527325  7657.943354 4761.917277 3947.330056 3365.873455 3710.613299 A=A;B=E;C=C;D=D;E=B;F=F
  cf_woTemplates_model_2_multimer_v3_pred_26             afm_dropout_full_model_3_multimer_v3_pred_64   0.445539     0.506868  7657.943354 4677.771912 3933.871243 3930.453612 3724.072111 A=A;B=E;C=C;D=D;E=B;F=F
  cf_woTemplates_model_3_multimer_v3_pred_47             afm_dropout_full_model_3_multimer_v3_pred_64   0.439929     0.489985  7483.973156 4453.132335 3760.215370 4104.109485 3723.757786 A=D;B=B;C=F;D=A;E=E;F=C
afm_dropout_full_model_3_multimer_v3_pred_66               cf_woTemplates_model_3_multimer_v3_pred_47   0.439601     0.499394  7313.203511 4526.494827 3694.810365 3789.162791 3618.393146 A=D;B=B;C=F;D=A;E=E;F=C


 # Table of per-residue scores based on residue-residue contacts:

ID_chain  ID_rnum ID_icode  CAD_score  F1_of_areas  target_area  model_area   TP_area   FP_area   FN_area
       A        4        .   0.000000     0.000000     5.723360    0.000000  0.000000 14.123374  5.723360
       A        6        .   0.000000     0.504191     7.178632   14.612032  7.160650 14.065207  0.017982
       A       15        .   0.000000     0.000000     1.346514    0.000000  0.000000  2.051384  1.346514
       A       17        .   0.175261     0.298250    25.529513    4.474315  4.474315  0.000000 21.055197
       A       18        .   0.378808     0.293841    26.519188   10.045678 10.045678 31.810069 16.473510
       A       19        .   0.715554     0.783253    69.951082   79.263636 63.086289 28.050569  6.864793
       A       20        .   0.228410     0.399188    97.824322   75.742435 35.484263 44.473835 62.340058
       A       21        .   0.892938     0.923627    55.431386   54.675708 52.086259  5.268696  3.345127
       A       22        .   0.000000     0.277170     0.401577    2.496120  0.401577  2.094543  0.000000
       A       23        .   0.406606     0.631095    34.355429   20.906229 17.437673  3.468556 16.917756


 # Table of scores for every residue-residue contact (CAD-score values of -1 idicate that the contact was not present in the target structure):

ID1_chain  ID1_rnum ID1_icode ID2_chain  ID2_rnum ID2_icode  CAD_score  F1_of_areas  target_area  model_area  TP_area   FP_area   FN_area
        A         4         .         D         6         .   0.000000     0.000000     0.043450    0.000000 0.000000  0.000000  0.043450
        A         4         .         D        61         .   0.000000     0.000000     5.679910    0.000000 0.000000  0.000000  5.679910
        A         4         .         D       206         .  -1.000000     0.000000     0.000000    0.000000 0.000000 14.123374  0.000000
        A         6         .         D         4         .   0.000000     0.000000     0.017982    0.000000 0.000000  0.000000  0.017982
        A         6         .         D        65         .   0.000000     0.657765     7.160650   14.612032 7.160650  7.451382  0.000000
        A         6         .         D       208         .  -1.000000     0.000000     0.000000    0.000000 0.000000  6.613825  0.000000
        A        15         .         C        31         .  -1.000000     0.000000     0.000000    0.000000 0.000000  2.051384  0.000000
        A        15         .         D       137         .   0.000000     0.000000     1.346514    0.000000 0.000000  0.000000  1.346514
        A        17         .         C        31         .   0.086301     0.158889    16.995494    1.466721 1.466721  0.000000 15.528773
        A        17         .         C        32         .   0.557286     0.715714     5.396860    3.007595 3.007595  0.000000  2.389265


 # Table of per-atom scores based on atom-atom contacts (CAD-score values of -1 idicate that the atom had no relevant contacts in the target structure):

ID_chain  ID_rnum ID_icode ID_atom_name  CAD_score  F1_of_areas  target_area  model_area  TP_area   FP_area   FN_area
       A        4        .           CD  -1.000000     0.000000     0.000000    0.000000 0.000000  0.589435  0.000000
       A        4        .           CE  -1.000000     0.000000     0.000000    0.000000 0.000000  6.218582  0.000000
       A        4        .           CG  -1.000000     0.000000     0.000000    0.000000 0.000000  0.040717  0.000000
       A        4        .           NZ   0.000000     0.000000     5.723360    0.000000 0.000000  7.274641  5.723360
       A        6        .           CB  -1.000000     0.000000     0.000000    0.000000 0.000000  0.005776  0.000000
       A        6        .          CG1  -1.000000     0.000000     0.000000    0.000000 0.000000 10.924598  0.000000
       A        6        .          CG2   0.315134     0.527725     7.178632    8.956361 4.610766  5.684717  2.567866
       A       15        .           CB  -1.000000     0.000000     0.000000    0.000000 0.000000  1.960049  0.000000
       A       15        .            O   0.000000     0.000000     1.346514    0.000000 0.000000  0.091335  1.346514
       A       17        .           CB   0.057452     0.097769    25.529513    1.466721 1.466721  3.007595 24.062792


 # Table of scores for every atom-atom contact (CAD-score values of -1 idicate that the contact was not present in the target structure):

ID1_chain  ID1_rnum ID1_icode ID1_atom_name ID2_chain  ID2_rnum ID2_icode ID2_atom_name  CAD_score  F1_of_areas  target_area  model_area  TP_area  FP_area  FN_area
        A         4         .            CD         D       206         .           OE1       -1.0          0.0     0.000000         0.0      0.0 0.589435 0.000000
        A         4         .            CE         D       206         .           NE2       -1.0          0.0     0.000000         0.0      0.0 1.398847 0.000000
        A         4         .            CE         D       206         .           OE1       -1.0          0.0     0.000000         0.0      0.0 4.819735 0.000000
        A         4         .            CG         D       206         .           OE1       -1.0          0.0     0.000000         0.0      0.0 0.040717 0.000000
        A         4         .            NZ         D         6         .           CG2        0.0          0.0     0.043450         0.0      0.0 0.000000 0.043450
        A         4         .            NZ         D        61         .            CZ        0.0          0.0     0.413695         0.0      0.0 0.000000 0.413695
        A         4         .            NZ         D        61         .            OH        0.0          0.0     5.266215         0.0      0.0 0.000000 5.266215
        A         4         .            NZ         D       206         .            CD       -1.0          0.0     0.000000         0.0      0.0 0.008074 0.000000
        A         4         .            NZ         D       206         .           NE2       -1.0          0.0     0.000000         0.0      0.0 5.372747 0.000000
        A         4         .            NZ         D       206         .           OE1       -1.0          0.0     0.000000         0.0      0.0 1.893820 0.000000

```


