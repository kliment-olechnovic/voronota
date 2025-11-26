import voronotalt
import biotite.structure.io

structure = biotite.structure.io.load_structure("./input/assembly_1ctf.cif")

rt = voronotalt.RadicalTessellation.from_biotite_atoms(structure, include_heteroatoms=False, probe=1.4)

print("contacts:")

for contact in rt.contacts:
    if contact.index_a<5:
        print(f"contact {contact.index_a} {contact.index_b} {contact.area:.4f} {contact.arc_length:.4f}")

cells=list(rt.cells)

print("cells:")

for i, cell in enumerate(cells[:20]):
    print(f"cell {i} {cell.sas_area:.4f} {cell.volume:.4f}");

