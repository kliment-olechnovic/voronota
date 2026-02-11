import cadscorelt
import biotite.structure.io

structure_target = biotite.structure.io.load_structure("./input/data/protein_homodimer1/target.pdb")
structure_model1 = biotite.structure.io.load_structure("./input/data/protein_homodimer1/model1.pdb")
structure_model2 = biotite.structure.io.load_structure("./input/data/protein_homodimer1/model2.pdb")

cs = cadscorelt.CADScore.init(subselect_contacts="[-inter-chain]", remap_chains=True)

cs.add_structure_from_biotite(structure_target, "target")
cs.add_structure_from_biotite(structure_model1, "model1")
cs.add_structure_from_biotite(structure_model2, "model2")

df_global_scores = cs.get_all_cadscores_residue_residue_summarized_globally().to_pandas()

print("--------------------------------------------------------------------------------")
cadscorelt.print_head_of_pandas_data_frame(df_global_scores)

