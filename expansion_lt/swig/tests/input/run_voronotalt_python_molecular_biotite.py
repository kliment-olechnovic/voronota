import voronotalt
import voronotalt.biotite_interface
import biotite.structure.io

structure = biotite.structure.io.load_structure("./input/assembly_1ctf.cif")

atom_balls=voronotalt.biotite_interface.molecular_atom_balls_from_atom_array(structure, include_heteroatoms=False)

mrt = voronotalt.MolecularRadicalTessellation.from_atoms(input_atom_balls=atom_balls)

print("inter_residue_contacts:")

for contact in mrt.inter_residue_contact_summaries:
    print(f"ir_contact {contact.ID1_chain} {contact.ID1_residue_seq_number} {contact.ID1_residue_name} {contact.ID2_chain} {contact.ID2_residue_seq_number} {contact.ID2_residue_name} {contact.area:.4f}");

print("residue_cells:")

for cell in mrt.residue_cell_summaries:
    print(f"r_cell {cell.ID_chain} {cell.ID_residue_seq_number} {cell.ID_residue_name} {cell.sas_area:.4f} {cell.volume:.4f}");

