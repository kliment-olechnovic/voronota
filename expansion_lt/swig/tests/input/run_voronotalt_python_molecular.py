import voronotalt

mrt = voronotalt.MolecularRadicalTessellation.from_file(
    input_file="./input/assembly_1ctf.pdb1",
    read_as_assembly=True,
    restrict_contacts_for_output="[-a1 [-chain A] -a2 [-chain A2]]",
    restrict_cells_for_output="[-chain A]"
)

print("inter_residue_contacts:")

for contact in mrt.inter_residue_contact_summaries:
    print(f"ir_contact {contact.ID1_chain} {contact.ID1_residue_seq_number} {contact.ID1_residue_name} {contact.ID2_chain} {contact.ID2_residue_seq_number} {contact.ID2_residue_name} {contact.area:.4f}");

print("residue_cells:")

for cell in mrt.residue_cell_summaries:
    print(f"r_cell {cell.ID_chain} {cell.ID_residue_seq_number} {cell.ID_residue_name} {cell.sas_area:.4f} {cell.volume:.4f}");

print("inter_chain_contacts:")

for contact in mrt.inter_chain_contact_summaries:
    print(f"ic_contact {contact.ID1_chain} {contact.ID2_chain} {contact.area:.4f}");

print("chain_cells:")

for cell in mrt.chain_cell_summaries:
    print(f"c_cell {cell.ID_chain} {cell.sas_area:.4f} {cell.volume:.4f}");

mrt = voronotalt.MolecularRadicalTessellation.from_file(
    input_file="./input/assembly_1ctf.cif",
    record_everything_possible=False,
    record_inter_residue_contact_summaries=True,
    record_inter_chain_contact_summaries=True,
    record_chain_cell_summaries=True,
    restrict_contacts_for_output="[-a1 [-chain A] -a2! [-chain A]]"
)

print("inter_residue_contacts:")

for contact in mrt.inter_residue_contact_summaries:
    print(f"ir_contact {contact.ID1_chain} {contact.ID1_residue_seq_number} {contact.ID1_residue_name} {contact.ID2_chain} {contact.ID2_residue_seq_number} {contact.ID2_residue_name} {contact.area:.4f}");

print("inter_chain_contacts:")

for contact in mrt.inter_chain_contact_summaries:
    print(f"ic_contact {contact.ID1_chain} {contact.ID2_chain} {contact.area:.4f}");

print("chain_cells:")

for cell in mrt.chain_cell_summaries:
    print(f"c_cell {cell.ID_chain} {cell.sas_area:.4f} {cell.volume:.4f}");

