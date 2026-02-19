import cadscorelt

# init a CAD-score computation object
csc = cadscorelt.CADScoreComputer.init(subselect_contacts="[-inter-chain]")

# add a target structure
csc.add_target_structure_from_file("./input/data/protein_homodimer1/target.pdb")

# add model structures
csc.add_model_structure_from_file("./input/data/protein_homodimer1/model1.pdb")
csc.add_model_structure_from_file("./input/data/protein_homodimer1/model2.pdb")

# get a list of global scores and convert it to pandas data frame
df_global_scores_residue_residue = csc.get_all_cadscores_residue_residue_summarized_globally().to_pandas()

# print the first rows of the data frame
cadscorelt.print_head_of_pandas_data_frame(df_global_scores_residue_residue)

