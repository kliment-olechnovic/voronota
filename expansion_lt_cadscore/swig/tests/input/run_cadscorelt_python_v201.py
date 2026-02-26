import cadscorelt
from pathlib import Path

cadscorelt.disable_considering_residue_names()

csc = cadscorelt.CADScoreComputer.init(subselect_contacts="[-inter-chain]")

csc.set_reference_stoichiometry([1, 1])

csc.set_reference_sequences_from_file("./input/data/protein_heterodimer_multiseq1/sequences_for_1ktz.fasta")
csc.add_structure_from_file("./input/data/protein_heterodimer_multiseq1/1ktz_A_1-1ktz_B_1.pdb")

csc.set_reference_sequences_from_file("./input/data/protein_heterodimer_multiseq1/sequences_for_2pjy.fasta")
csc.add_structure_from_file("./input/data/protein_heterodimer_multiseq1/2pjy_A_1-2pjy_B_1.pdb")

df_structure_descriptors = csc.get_all_structure_descriptors().to_pandas()
print("")
print(" # Table of structure decriptors:")
print("")
cadscorelt.print_head_of_pandas_data_frame(df_structure_descriptors)
print("")

df_identity_descriptors = csc.get_all_identity_descriptors().to_pandas()
print("")
print(" # Table of identity descriptors:")
print("")
cadscorelt.print_head_of_pandas_data_frame(df_identity_descriptors)
print("")


df_global_scores_residue_residue = csc.get_all_cadscores_residue_residue_summarized_globally().to_pandas()
print("")
print(" # Table of globals scores based on residue-residue contacts:")
print("")
cadscorelt.print_head_of_pandas_data_frame(df_global_scores_residue_residue)
print("")

print(" # Reference_alignment_info for '1ktz_A_1-1ktz_B_1':")
print(csc.get_structure_descriptor("1ktz_A_1-1ktz_B_1").reference_alignment_info)
print("")

print(" # Reference_alignment_info for '2pjy_A_1-2pjy_B_1':")
print(csc.get_structure_descriptor("2pjy_A_1-2pjy_B_1").reference_alignment_info)
print("")
