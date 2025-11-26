import voronotalt

mrt = voronotalt.MolecularRadicalTessellation.from_file(
    input_file="./input/assembly_1ctf.pdb1",
    read_as_assembly=True,
    restrict_contacts_for_output="[-a1 [-chain A] -a2 [-chain A2]]",
    restrict_cells_for_output="[-chain A]"
)

df_atoms = mrt.atom_balls.to_pandas()
df_inter_atom_contacts = mrt.inter_atom_contact_summaries.to_pandas()
df_inter_residue_contacts = mrt.inter_residue_contact_summaries.to_pandas()
df_inter_chain_contacts = mrt.inter_chain_contact_summaries.to_pandas()
df_atom_cells = mrt.atom_cell_summaries.to_pandas()
df_residue_cells = mrt.residue_cell_summaries.to_pandas()
df_chain_cells = mrt.chain_cell_summaries.to_pandas()

print("--------------------------------------------------------------------------------")
voronotalt.pandas_interface.print_head_of_table(df_atoms.head())

print("--------------------------------------------------------------------------------")
voronotalt.pandas_interface.print_head_of_table(df_inter_atom_contacts.head())

print("--------------------------------------------------------------------------------")
voronotalt.pandas_interface.print_head_of_table(df_inter_residue_contacts.head())

print("--------------------------------------------------------------------------------")
voronotalt.pandas_interface.print_head_of_table(df_inter_chain_contacts.head())

print("--------------------------------------------------------------------------------")
voronotalt.pandas_interface.print_head_of_table(df_atom_cells.head())

print("--------------------------------------------------------------------------------")
voronotalt.pandas_interface.print_head_of_table(df_residue_cells.head())

print("--------------------------------------------------------------------------------")
voronotalt.pandas_interface.print_head_of_table(df_chain_cells.head())

