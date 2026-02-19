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

