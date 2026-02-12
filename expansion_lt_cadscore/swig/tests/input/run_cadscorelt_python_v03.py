import cadscorelt
import gemmi

structure_target = gemmi.read_structure("./input/data/protein_homodimer1/target.pdb")
structure_model1 = gemmi.read_structure("./input/data/protein_homodimer1/model1.pdb")
structure_model2 = gemmi.read_structure("./input/data/protein_homodimer1/model2.pdb")

cs = cadscorelt.CADScore.init(subselect_contacts="[-inter-chain]", remap_chains=True, record_local_scores=True, score_atom_atom_contacts=True)

cs.add_structure_from_gemmi(structure_target[0], "target")
cs.add_structure_from_gemmi(structure_model1[0], "model1")
cs.add_structure_from_gemmi(structure_model2[0], "model2")

print("--------------------------------------------------------------------------------")
print("--- residue-residue contacts scoring results                                 ---")
print("--------------------------------------------------------------------------------")

print("get_all_cadscores_residue_residue_summarized_globally:")
print("--------------------------------------------------------------------------------")
df_global_scores_residue_residue = cs.get_all_cadscores_residue_residue_summarized_globally().to_pandas()
cadscorelt.print_head_of_pandas_data_frame(df_global_scores_residue_residue)
print("--------------------------------------------------------------------------------")

print("get_local_cadscores_residue_residue:")
print("--------------------------------------------------------------------------------")
df_local_scores_residue_residue = cs.get_local_cadscores_residue_residue("target", "model1").to_pandas()
cadscorelt.print_head_of_pandas_data_frame(df_local_scores_residue_residue)
print("--------------------------------------------------------------------------------")

print("get_local_cadscores_residue_residue_summarized_per_residue:")
print("--------------------------------------------------------------------------------")
df_local_scores_per_residue = cs.get_local_cadscores_residue_residue_summarized_per_residue("target", "model1").to_pandas()
cadscorelt.print_head_of_pandas_data_frame(df_local_scores_per_residue)
print("--------------------------------------------------------------------------------")

print("")

print("--------------------------------------------------------------------------------")
print("--- atom-atom contacts scoring results                                       ---")
print("--------------------------------------------------------------------------------")

print("get_all_cadscores_atom_atom_summarized_globally:")
print("--------------------------------------------------------------------------------")
df_global_scores_atom_atom = cs.get_all_cadscores_atom_atom_summarized_globally().to_pandas()
cadscorelt.print_head_of_pandas_data_frame(df_global_scores_atom_atom)
print("--------------------------------------------------------------------------------")

print("get_local_cadscores_atom_atom:")
print("--------------------------------------------------------------------------------")
df_local_scores_atom_atom = cs.get_local_cadscores_atom_atom("target", "model1").to_pandas()
cadscorelt.print_head_of_pandas_data_frame(df_local_scores_atom_atom)
print("--------------------------------------------------------------------------------")

print("get_local_cadscores_atom_atom_summarized_per_residue_residue:")
print("--------------------------------------------------------------------------------")
df_local_scores_atom_atom_summarized_per_residue_residue = cs.get_local_cadscores_atom_atom_summarized_per_residue_residue("target", "model1").to_pandas()
cadscorelt.print_head_of_pandas_data_frame(df_local_scores_atom_atom_summarized_per_residue_residue)
print("--------------------------------------------------------------------------------")

print("get_local_cadscores_atom_atom_summarized_per_atom:")
print("--------------------------------------------------------------------------------")
df_local_scores_per_atom = cs.get_local_cadscores_atom_atom_summarized_per_atom("target", "model1").to_pandas()
cadscorelt.print_head_of_pandas_data_frame(df_local_scores_per_atom)
print("--------------------------------------------------------------------------------")

print("get_local_cadscores_atom_atom_summarized_per_residue:")
print("--------------------------------------------------------------------------------")
df_local_scores_atom_atom_per_residue = cs.get_local_cadscores_atom_atom_summarized_per_residue("target", "model1").to_pandas()
cadscorelt.print_head_of_pandas_data_frame(df_local_scores_atom_atom_per_residue)
print("--------------------------------------------------------------------------------")

