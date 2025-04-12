# Using Voronota-LT Python bindings

## Compiling Python bindings

Python bindings of Voronota-LT can be built using SWIG, in the "expansion_lt/swig" directory:

```bash
swig -python -c++ voronotalt_python.i

g++ -fPIC -shared -O3 -fopenmp voronotalt_python_wrap.cxx -o _voronotalt_python.so $(python3-config --includes)
```

This produces "_voronotalt_python.so" and "voronotalt_python.py" that are needed to call Voronota-LT from Python code.

## Using Python bindings

When "_voronotalt_python.so" and "voronotalt_python.py" are generated, the "voronotalt_python" module can be made findable by python by adding its directory to the PYTHONPATH environmental variable:

```bash
export PYTHONPATH="${PYTHONPATH}:/path/to/voronota/expansion_lt/swig"
```

Then Voronota-LT can be used in Python code as in the following example:

```py
import voronotalt_python as voronotalt

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

### Using Python bindings with Biotite input

When, in addition to the requirements specified in the previous section, an extra module "voronotalt_python_biotite.py" is available, Voronota-LT can be used in Python code with Biotite as in the following example:

```py
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
```

