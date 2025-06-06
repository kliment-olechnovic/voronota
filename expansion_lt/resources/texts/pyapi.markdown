# Voronota-LT Python bindings

Voronota-LT Python interface PyPI package is hosted at [https://pypi.org/project/voronotalt/](https://pypi.org/project/voronotalt/).

## Installation

Install with pip using this command:

```bash
pip install voronotalt
```

## Usage example, basic

Voronota-LT can be used in Python code as in the following example:

```py
import voronotalt

balls = []
balls.append(voronotalt.Ball(0, 0, 2, 1))
balls.append(voronotalt.Ball(0, 1, 0, 0.5))
balls.append(voronotalt.Ball(0.38268343236509, 0.923879532511287, 0, 0.5))
balls.append(voronotalt.Ball(0.707106781186547, 0.707106781186548, 0, 0.5))
balls.append(voronotalt.Ball(0.923879532511287, 0.38268343236509, 0, 0.5))
balls.append(voronotalt.Ball(1, 0, 0, 0.5))
balls.append(voronotalt.Ball(0.923879532511287, -0.38268343236509, 0, 0.5))
balls.append(voronotalt.Ball(0.707106781186548, -0.707106781186547, 0, 0.5))
balls.append(voronotalt.Ball(0.38268343236509, -0.923879532511287, 0, 0.5))
balls.append(voronotalt.Ball(0, -1, 0, 0.5))
balls.append(voronotalt.Ball(-0.38268343236509, -0.923879532511287, 0, 0.5))
balls.append(voronotalt.Ball(-0.707106781186547, -0.707106781186548, 0, 0.5))
balls.append(voronotalt.Ball(-0.923879532511287, -0.38268343236509, 0, 0.5))
balls.append(voronotalt.Ball(-1, 0, 0, 0.5))
balls.append(voronotalt.Ball(-0.923879532511287, 0.38268343236509, 0, 0.5))
balls.append(voronotalt.Ball(-0.707106781186548, 0.707106781186547, 0, 0.5))
balls.append(voronotalt.Ball(-0.38268343236509, 0.923879532511287, 0, 0.5))

rt = voronotalt.RadicalTessellation(balls, probe=1.0)

contacts=list(rt.contacts)

print("contacts:")

for contact in contacts:
    print("contact", contact.index_a, contact.index_b, contact.area, contact.arc_length);

cells=list(rt.cells)

print("cells:")

for i, cell in enumerate(cells):
    print("cell", i, cell.sas_area, cell.volume);

```


## Usage example, using Biotite to provide input

Voronota-LT can be used in Python code with Biotite as in the following example:

```py
import argparse
from voronotalt.biotite_interface import radical_tessellation_from_atom_array
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
```

To run this example, make sure you have intalled Biotite:

```bash
pip install biotite
```

Then run by providing either PDB or mmCIF file path:

```bash
python3 example_script.py 2zsk.pdb
python3 example_script.py 2zsk.cif
```


# Voronota-LT Rust bindings

Thanks to Mikael Lund, there is also Rust interface for Voronota-LT at [https://github.com/mlund/voronota-rs](https://github.com/mlund/voronota-rs).

