import cadscorelt
import gemmi

structure_target = gemmi.read_structure("./input/data/protein_homodimer1/target.pdb")
#structure_model1 = gemmi.read_structure("./input/data/protein_homodimer1/model1.pdb")
structure_model2 = gemmi.read_structure("./input/data/protein_homodimer1/model2.pdb")

cs = cadscorelt.CADScore.init(subselect_contacts="[-inter-chain]", remap_chains=True)

cs.add_structure_from_gemmi(structure_target[0], "target")
#cs.add_structure_from_gemmi(structure_model1[0], "model1")
cs.add_structure_from_gemmi(structure_model2[0], "model2")

df_global_scores = cs.get_all_cadscores_residue_residue_summarized_globally().to_pandas()

print("--------------------------------------------------------------------------------")
cadscorelt.print_head_of_pandas_data_frame(df_global_scores)

