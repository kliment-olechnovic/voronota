select site_residues, none
select site_residues, site_residues or (chain A and resi 32)
select site_residues, site_residues or (chain A and resi 54)
select site_residues, site_residues or (chain A and resi 55)
select site_residues, site_residues or (chain A and resi 57)
select site_residues, site_residues or (chain A and resi 161)
select site_atoms, none
select site_atoms, site_atoms or (chain A and resi 32 and name "CD")
select site_atoms, site_atoms or (chain A and resi 32 and name "CE")
select site_atoms, site_atoms or (chain A and resi 32 and name "NZ")
select site_atoms, site_atoms or (chain A and resi 54 and name "CD2")
select site_atoms, site_atoms or (chain A and resi 55 and name "CG")
select site_atoms, site_atoms or (chain A and resi 55 and name "CD")
select site_atoms, site_atoms or (chain A and resi 57 and name "NH2")
select site_atoms, site_atoms or (chain A and resi 161 and name "CA")
select site_atoms, site_atoms or (chain A and resi 161 and name "CB")
select site_atoms, site_atoms or (chain A and resi 161 and name "CG")
select site_atoms, site_atoms or (chain A and resi 161 and name "CT")
select site_atoms, site_atoms or (chain A and resi 161 and name "O2")
show sticks, site_residues
color cyan, site_residues
color red, site_atoms
