import cadscorelt
from pathlib import Path

input_directory = Path("./input/data/protein_heteromer1/structures")

print("")
print("--------------------------------------------------------------------------------")
print("--- results 1                                                                ---")
print("--------------------------------------------------------------------------------")

csc1 = cadscorelt.CADScoreComputer.init(subselect_contacts="[-inter-chain]")

for file_path in input_directory.iterdir():
    if file_path.is_file():
        csc1.add_structure_from_file(str(file_path))

df_structure_descriptors = csc1.get_all_structure_descriptors().to_pandas()
cadscorelt.print_head_of_pandas_data_frame(df_structure_descriptors)
print("--------------------------------------------------------------------------------")
df_global_scores_residue_residue = csc1.get_all_cadscores_residue_residue_summarized_globally().to_pandas()
cadscorelt.print_head_of_pandas_data_frame(df_global_scores_residue_residue)
print("--------------------------------------------------------------------------------")

print("")
print("--------------------------------------------------------------------------------")
print("--- results 2                                                                ---")
print("--------------------------------------------------------------------------------")

csc2 = cadscorelt.CADScoreComputer.init(subselect_contacts="[-inter-chain]")
csc2.set_reference_sequences_from_file("./input/data/protein_heteromer1/sequences.fasta")
csc2.set_reference_stoichiometry([2, 2, 2])

for file_path in input_directory.iterdir():
    if file_path.is_file():
        csc2.add_structure_from_file(str(file_path))

df_structure_descriptors = csc2.get_all_structure_descriptors().to_pandas()
cadscorelt.print_head_of_pandas_data_frame(df_structure_descriptors)
print("--------------------------------------------------------------------------------")
df_global_scores_residue_residue = csc2.get_all_cadscores_residue_residue_summarized_globally().to_pandas()
cadscorelt.print_head_of_pandas_data_frame(df_global_scores_residue_residue)
print("--------------------------------------------------------------------------------")

print("")
print("--------------------------------------------------------------------------------")
print("--- results 3                                                                ---")
print("--------------------------------------------------------------------------------")

cadscorelt.enable_considering_residue_names()

csc3 = cadscorelt.CADScoreComputer.init(subselect_contacts="[-inter-chain]", remap_chains=True)
csc3.set_reference_sequences_from_file("./input/data/protein_heteromer1/sequences.fasta")
csc3.set_reference_stoichiometry([2, 2, 2])

for file_path in input_directory.iterdir():
    if file_path.is_file():
        csc3.add_structure_from_file(str(file_path))

df_structure_descriptors = csc3.get_all_structure_descriptors().to_pandas()
cadscorelt.print_head_of_pandas_data_frame(df_structure_descriptors)
print("--------------------------------------------------------------------------------")
df_global_scores_residue_residue = csc3.get_all_cadscores_residue_residue_summarized_globally().to_pandas()
cadscorelt.print_head_of_pandas_data_frame(df_global_scores_residue_residue)
print("--------------------------------------------------------------------------------")

print("")

