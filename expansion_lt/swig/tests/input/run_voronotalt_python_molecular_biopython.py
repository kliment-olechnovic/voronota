import voronotalt
import voronotalt.biopython_interface
import Bio.PDB

parser = Bio.PDB.MMCIFParser(QUIET=True)
structure = parser.get_structure("id", "./input/assembly_1ctf.cif")
atoms=structure.get_atoms()

atom_balls=voronotalt.biopython_interface.molecular_atom_balls_from_atom_iterable(atoms, include_heteroatoms=False)

mrt = voronotalt.MolecularRadicalTessellation(atom_balls)

print("inter_residue_contacts:")

for contact in mrt.inter_residue_contact_summaries:
    print(f"ir_contact {contact.ID1_chain} {contact.ID1_residue_seq_number} {contact.ID1_residue_name} {contact.ID2_chain} {contact.ID2_residue_seq_number} {contact.ID2_residue_name} {contact.area:.4f}");

print("residue_cells:")

for cell in mrt.residue_cell_summaries:
    print(f"r_cell {cell.ID_chain} {cell.ID_residue_seq_number} {cell.ID_residue_name} {cell.sas_area:.4f} {cell.volume:.4f}");

