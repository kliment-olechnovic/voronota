import cadscorelt
import Bio.PDB

parser = Bio.PDB.PDBParser(QUIET=True)

structure_target = parser.get_structure("id", "./input/data/protein_homodimer1/target.pdb").get_atoms()
structure_model1 = parser.get_structure("id", "./input/data/protein_homodimer1/model1.pdb").get_atoms()
structure_model2 = parser.get_structure("id", "./input/data/protein_homodimer1/model2.pdb").get_atoms()

cs = cadscorelt.CADScore.init(subselect_contacts="[-inter-chain]", remap_chains=True)

cs.add_structure_from_biopython(structure_target, "target")
cs.add_structure_from_biopython(structure_model1, "model1")
cs.add_structure_from_biopython(structure_model2, "model2")

df_global_scores = cs.get_all_cadscores_residue_residue_summarized_globally().to_pandas()

print("--------------------------------------------------------------------------------")
cadscorelt.print_head_of_pandas_data_frame(df_global_scores)

