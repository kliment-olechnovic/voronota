import cadscorelt

cs = cadscorelt.CADScore.init(subselect_contacts="[-inter-chain]", remap_chains=True)

cs.add_structure_from_file("./input/data/protein_homodimer1/target.pdb", "target")
cs.add_structure_from_file("./input/data/protein_homodimer1/model1.pdb", "model1")
cs.add_structure_from_file("./input/data/protein_homodimer1/model2.pdb", "model2")

df_global_scores = cs.get_all_cadscores_residue_residue_summarized_globally().to_pandas()

print("--------------------------------------------------------------------------------")
cadscorelt.print_head_of_pandas_data_frame(df_global_scores)

