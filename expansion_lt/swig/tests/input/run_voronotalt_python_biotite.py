import argparse
from voronotalt_python_biotite import radical_tessellation_from_atom_array
from biotite.structure.io import load_structure

# Parse command-line arguments
parser = argparse.ArgumentParser(description="Compute radical tessellation using Voronota-LT for an input PDB file.")
parser.add_argument("input_file", help="Path to the input PDB file")
args = parser.parse_args()

# Load AtomArray from a PDB file
structure = load_structure(args.input_file)

# Compute tessellation
rt = radical_tessellation_from_atom_array(structure, probe=1.4)

# Print input balls
for i, ball in enumerate(rt.balls):
    print("ball", i, ball.x, ball.y, ball.z, ball.r)

# Print contacts
print("contacts:")
for contact in rt.contacts:
    print("contact", contact.index_a, contact.index_b, contact.area, contact.arc_length)

# Print cells
print("cells:")
for i, cell in enumerate(rt.cells):
    print("cell", i, cell.sas_area, cell.volume)
