# Voronota-LT Python bindings

The Voronota-LT Python interface PyPI package is hosted at [https://pypi.org/project/voronotalt/](https://pypi.org/project/voronotalt/).

## Installation

Install with pip using this command:

```bash
pip install voronotalt
```

## Basic usage examples

### Basic usage example. generic

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

for i, ball in enumerate(balls):
    print(f"ball {i} {ball.x:.4f} {ball.y:.4f} {ball.z:.4f} {ball.r:.4f}");

rt = voronotalt.RadicalTessellation(balls, probe=1.0)

contacts=list(rt.contacts)

print("contacts:")

for contact in contacts:
    print(f"contact {contact.index_a} {contact.index_b} {contact.area:.4f} {contact.arc_length:.4f}")

cells=list(rt.cells)

print("cells:")

for i, cell in enumerate(cells):
    print(f"cell {i} {cell.sas_area:.4f} {cell.volume:.4f}");

```

### Basic usage example with converting output to pandas data frames

If the [pandas](https://pandas.pydata.org/) library for data analysis is available in the Python environment,
then the tessellation computation results can also be converted to [pandas data frames](https://pandas.pydata.org/docs/reference/frame.html):

```py
rt = voronotalt.RadicalTessellation(balls, probe=1.0)

df_balls = rt.balls.to_pandas()
df_contacts = rt.contacts.to_pandas()
df_cells = rt.cells.to_pandas()

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_balls)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_contacts)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_cells)

```

To run this example, make sure you have installed pandas:

```bash
pip install pandas
```

### Basic usage example with Biotite to provide input

If [Biotite](https://www.biotite-python.org/) is available in the Python environment,
the Voronota-LT can be used in Python code with Biotite as in the following example:

```py
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

```

To run this example, make sure you have installed Biotite:

```bash
pip install biotite
```

## Biomolecules-focused usage examples

### Biomolecules-focused usage example, generic

Since version 1.0.1, the Voronota-LT Python bindings contain special classes and functions for processing biological macromolecules.
They folow the interface of the Voronota-LT command line software interface.
The main class is `MolecularRadicalTessellation`, a more biomolecules-focused counterpart of the basic `RadicalTessellation` class.
Below is an example of using `MolecularRadicalTessellation`:

```py
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

```

### Biomolecules-focused usage example with converting output to pandas data frames

Similarly to `RadicalTessellation`, the `MolecularRadicalTessellation` allows
converting the tessellation computation results to [pandas data frames](https://pandas.pydata.org/docs/reference/frame.html)
if the [pandas](https://pandas.pydata.org/) library for data analysis is available in the Python environment.
Below is an example that prints heads of different output data frames that came from the `MolecularRadicalTessellation` object:

```py
import voronotalt

mrt = voronotalt.MolecularRadicalTessellation.from_file(
    input_file="./input/assembly_1ctf.pdb1",
    read_as_assembly=True,
    restrict_contacts_for_output="[-a1 [-chain A] -a2 [-chain A2]]",
    restrict_cells_for_output="[-chain A]"
)

df_atoms = mrt.atom_balls.to_pandas()
df_inter_atom_contacts = mrt.inter_atom_contact_summaries.to_pandas()
df_inter_residue_contacts = mrt.inter_residue_contact_summaries.to_pandas()
df_inter_chain_contacts = mrt.inter_chain_contact_summaries.to_pandas()
df_atom_cells = mrt.atom_cell_summaries.to_pandas()
df_residue_cells = mrt.residue_cell_summaries.to_pandas()
df_chain_cells = mrt.chain_cell_summaries.to_pandas()

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_atoms)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_inter_atom_contacts)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_inter_residue_contacts)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_inter_chain_contacts)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_atom_cells)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_residue_cells)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_chain_cells)

```

To run this example, make sure you have installed pandas:

```bash
pip install pandas
```

### Biomolecules-focused usage example with Biotite to provide input

If [Biotite](https://www.biotite-python.org/) is available in the Python environment,
`MolecularRadicalTessellation` can use [biotite.structure](https://www.biotite-python.org/latest/apidoc/biotite.structure.html) for input:

```py
import voronotalt
import biotite.structure.io

structure = biotite.structure.io.load_structure("./input/assembly_1ctf.cif")

mrt = voronotalt.MolecularRadicalTessellation.from_biotite_atoms(structure, include_heteroatoms=False)

print("inter_residue_contacts:")

for contact in mrt.inter_residue_contact_summaries:
    print(f"ir_contact {contact.ID1_chain} {contact.ID1_residue_seq_number} {contact.ID1_residue_name} {contact.ID2_chain} {contact.ID2_residue_seq_number} {contact.ID2_residue_name} {contact.area:.4f}");

print("residue_cells:")

for cell in mrt.residue_cell_summaries:
    print(f"r_cell {cell.ID_chain} {cell.ID_residue_seq_number} {cell.ID_residue_name} {cell.sas_area:.4f} {cell.volume:.4f}");

```

To run this example, make sure you have installed Biotite:

```bash
pip install biotite
```

### Biomolecules-focused usage example with Gemmi to provide input

If [Gemmi](https://gemmi.readthedocs.io/) is available in the Python environment,
`MolecularRadicalTessellation` can use [gemmi.Model](https://project-gemmi.github.io/python-api/gemmi.Model.html) for input:

```py
import voronotalt
import gemmi

structure=gemmi.read_structure("./input/assembly_1ctf.cif")
model=structure[0]

mrt = voronotalt.MolecularRadicalTessellation.from_gemmi_model_atoms(model, include_heteroatoms=False)

print("inter_residue_contacts:")

for contact in mrt.inter_residue_contact_summaries:
    print(f"ir_contact {contact.ID1_chain} {contact.ID1_residue_seq_number} {contact.ID1_residue_name} {contact.ID2_chain} {contact.ID2_residue_seq_number} {contact.ID2_residue_name} {contact.area:.4f}");

print("residue_cells:")

for cell in mrt.residue_cell_summaries:
    print(f"r_cell {cell.ID_chain} {cell.ID_residue_seq_number} {cell.ID_residue_name} {cell.sas_area:.4f} {cell.volume:.4f}");

```

To run this example, make sure you have installed Gemmi:

```bash
pip install gemmi
```

### Biomolecules-focused usage example with Biopython to provide input

If [Biopython](https://biopython.org/) is available in the Python environment,
`MolecularRadicalTessellation` can use Biopython parsing results for input:

```py
import voronotalt
import Bio.PDB

parser = Bio.PDB.MMCIFParser(QUIET=True)
structure = parser.get_structure("id", "./input/assembly_1ctf.cif")
atoms=structure.get_atoms()

mrt = voronotalt.MolecularRadicalTessellation.from_biopython_atoms(atoms, include_heteroatoms=False)

print("inter_residue_contacts:")

for contact in mrt.inter_residue_contact_summaries:
    print(f"ir_contact {contact.ID1_chain} {contact.ID1_residue_seq_number} {contact.ID1_residue_name} {contact.ID2_chain} {contact.ID2_residue_seq_number} {contact.ID2_residue_name} {contact.area:.4f}");

print("residue_cells:")

for cell in mrt.residue_cell_summaries:
    print(f"r_cell {cell.ID_chain} {cell.ID_residue_seq_number} {cell.ID_residue_name} {cell.sas_area:.4f} {cell.volume:.4f}");

```

To run this example, make sure you have installed Biopython:

```bash
pip install biopython
```

### Biomolecules-focused usage example with custom radii

`MolecularRadicalTessellation` can use a configuration file to specify what van der Waals radii
to assign to different atoms based on their names and their residue names:

```py
import voronotalt

voronotalt.configure_molecular_radii_assignment_rules("./input/custom_radii.txt");

mrt = voronotalt.MolecularRadicalTessellation.from_file(
    input_file="./input/assembly_1ctf.pdb1",
    read_as_assembly=True,
    restrict_contacts_for_output="[-a1 [-chain A] -a2 [-chain A2]]",
    restrict_cells_for_output="[-chain A]"
)

df_atoms = mrt.atom_balls.to_pandas()
df_inter_atom_contacts = mrt.inter_atom_contact_summaries.to_pandas()
df_inter_residue_contacts = mrt.inter_residue_contact_summaries.to_pandas()
df_inter_chain_contacts = mrt.inter_chain_contact_summaries.to_pandas()
df_atom_cells = mrt.atom_cell_summaries.to_pandas()
df_residue_cells = mrt.residue_cell_summaries.to_pandas()
df_chain_cells = mrt.chain_cell_summaries.to_pandas()

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_atoms, n=20)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_inter_chain_contacts)

print("--------------------------------------------------------------------------------")
voronotalt.print_head_of_pandas_data_frame(df_chain_cells)

```

A custom radii configuration file format is the same as the one used by the standalone Voronota and Voronota-LT software.
An example of a full radii configuration file is [here](https://github.com/kliment-olechnovic/voronota/blob/master/resources/radii).


# Voronota-LT Rust bindings

Thanks to Mikael Lund, there is also Rust interface for Voronota-LT at [https://github.com/mlund/voronota-rs](https://github.com/mlund/voronota-rs).

