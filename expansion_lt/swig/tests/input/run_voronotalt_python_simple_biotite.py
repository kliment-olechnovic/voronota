import voronotalt
import voronotalt.biotite_interface
import biotite.structure.io

structure = biotite.structure.io.load_structure("./input/assembly_1ctf.cif")

balls=voronotalt.biotite_interface.simple_balls_from_atom_array(structure, include_heteroatoms=False)

rt = voronotalt.RadicalTessellation(balls, probe=1.4)

print("contacts:")

for contact in rt.contacts:
    if contact.index_a<5:
        print(f"contact {contact.index_a} {contact.index_b} {contact.area:.4f} {contact.arc_length:.4f}")

cells=list(rt.cells)

print("cells:")

for i, cell in enumerate(cells[:20]):
    print(f"cell {i} {cell.sas_area:.4f} {cell.volume:.4f}");

