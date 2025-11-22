import voronotalt


input_file=voronotalt.MolecularFileInput("./input/assembly_1ctf.pdb1")
input_file.read_as_assembly=True

params=voronotalt.MolecularRadicalTessellationParameters()
params.restrict_contacts_for_output="[-a1 [-chain A] -a2 [-chain A2]]"
params.restrict_cells_for_output="[-chain A]"

mrt = voronotalt.MolecularRadicalTessellation(input_file, params)

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


params=voronotalt.MolecularRadicalTessellationParameters()
params.record_everything_possible=False
params.record_inter_residue_contact_summaries=True
params.record_inter_chain_contact_summaries=True
params.record_chain_cell_summaries=True
params.restrict_contacts_for_output="[-a1 [-chain A] -a2! [-chain A]]"

mrt = voronotalt.MolecularRadicalTessellation(voronotalt.MolecularFileInput("./input/assembly_1ctf.cif"), params)

print("inter_residue_contacts:")

for contact in mrt.inter_residue_contact_summaries:
    print(f"ir_contact {contact.ID1_chain} {contact.ID1_residue_seq_number} {contact.ID1_residue_name} {contact.ID2_chain} {contact.ID2_residue_seq_number} {contact.ID2_residue_name} {contact.area:.4f}");

print("inter_chain_contacts:")

for contact in mrt.inter_chain_contact_summaries:
    print(f"ic_contact {contact.ID1_chain} {contact.ID2_chain} {contact.area:.4f}");

print("chain_cells:")

for cell in mrt.chain_cell_summaries:
    print(f"c_cell {cell.ID_chain} {cell.sas_area:.4f} {cell.volume:.4f}");

