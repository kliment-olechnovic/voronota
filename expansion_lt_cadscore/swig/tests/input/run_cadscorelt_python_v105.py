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

# get the table of identity decriptors and print its top rows
df_identity_descriptors = csc.get_all_identity_descriptors().to_pandas()
print("")
print(" # Table of identity descriptors:")
print("")
cadscorelt.print_head_of_pandas_data_frame(df_identity_descriptors)
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
print(" # Table of scores for every residue-residue contact (CAD-score values of -1 indicate that the contact was not present in the target structure):")
print("")
cadscorelt.print_head_of_pandas_data_frame(df_local_scores_residue_residue)
print("")

# get the table of per-atom scores based on atom-atom contacts, print top rows
df_local_scores_per_atom = csc.get_local_cadscores_atom_atom_summarized_per_atom(target_name, model_name).to_pandas()
print("")
print(" # Table of per-atom scores based on atom-atom contacts (CAD-score values of -1 indicate that the atom had no relevant contacts in the target structure):")
print("")
cadscorelt.print_head_of_pandas_data_frame(df_local_scores_per_atom)
print("")

# get the table of scores for every atom-atom contact, print top rows
df_local_scores_atom_atom = csc.get_local_cadscores_atom_atom(target_name, model_name).to_pandas()
print("")
print(" # Table of scores for every atom-atom contact (CAD-score values of -1 indicate that the contact was not present in the target structure):")
print("")
cadscorelt.print_head_of_pandas_data_frame(df_local_scores_atom_atom)
print("")

