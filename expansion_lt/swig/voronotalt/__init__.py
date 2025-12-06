from .voronotalt_python import *

def _from_file(
        cls,
        input_file,
        include_heteroatoms=True,
        read_as_assembly=False,
        restrict_input_atoms="",
        restrict_contacts_to_construct="",
        restrict_contacts_for_output="",
        restrict_cells_for_output="",
        probe=1.4,
        compute_only_inter_chain_contacts=False,
        compute_only_inter_residue_contacts=False,
        record_atom_balls=False,
        record_inter_atom_contact_summaries=False,
        record_inter_residue_contact_summaries=False,
        record_inter_chain_contact_summaries=False,
        record_atom_cell_summaries=False,
        record_residue_cell_summaries=False,
        record_chain_cell_summaries=False,
        record_everything_possible=True):
    input_file=MolecularFileInput(input_file)
    input_file.include_heteroatoms=include_heteroatoms
    input_file.read_as_assembly=read_as_assembly
    params=MolecularRadicalTessellationParameters()
    params.restrict_input_atoms=restrict_input_atoms
    params.restrict_contacts_to_construct=restrict_contacts_to_construct
    params.restrict_contacts_for_output=restrict_contacts_for_output
    params.restrict_cells_for_output=restrict_cells_for_output
    params.probe=probe
    params.compute_only_inter_chain_contacts=compute_only_inter_chain_contacts
    params.compute_only_inter_residue_contacts=compute_only_inter_residue_contacts
    params.record_atom_balls=record_atom_balls
    params.record_inter_atom_contact_summaries=record_inter_atom_contact_summaries
    params.record_inter_residue_contact_summaries=record_inter_residue_contact_summaries
    params.record_inter_chain_contact_summaries=record_inter_chain_contact_summaries
    params.record_atom_cell_summaries=record_atom_cell_summaries
    params.record_residue_cell_summaries=record_residue_cell_summaries
    params.record_chain_cell_summaries=record_chain_cell_summaries
    params.record_everything_possible=record_everything_possible
    return cls(input_file, params)

def _from_atoms(
        cls,
        input_atom_balls,
        restrict_input_atoms="",
        restrict_contacts_to_construct="",
        restrict_contacts_for_output="",
        restrict_cells_for_output="",
        probe=1.4,
        compute_only_inter_chain_contacts=False,
        compute_only_inter_residue_contacts=False,
        record_atom_balls=False,
        record_inter_atom_contact_summaries=False,
        record_inter_residue_contact_summaries=False,
        record_inter_chain_contact_summaries=False,
        record_atom_cell_summaries=False,
        record_residue_cell_summaries=False,
        record_chain_cell_summaries=False,
        record_everything_possible=True):
    params=MolecularRadicalTessellationParameters()
    params.restrict_input_atoms=restrict_input_atoms
    params.restrict_contacts_to_construct=restrict_contacts_to_construct
    params.restrict_contacts_for_output=restrict_contacts_for_output
    params.restrict_cells_for_output=restrict_cells_for_output
    params.probe=probe
    params.compute_only_inter_chain_contacts=compute_only_inter_chain_contacts
    params.compute_only_inter_residue_contacts=compute_only_inter_residue_contacts
    params.record_atom_balls=record_atom_balls
    params.record_inter_atom_contact_summaries=record_inter_atom_contact_summaries
    params.record_inter_residue_contact_summaries=record_inter_residue_contact_summaries
    params.record_inter_chain_contact_summaries=record_inter_chain_contact_summaries
    params.record_atom_cell_summaries=record_atom_cell_summaries
    params.record_residue_cell_summaries=record_residue_cell_summaries
    params.record_chain_cell_summaries=record_chain_cell_summaries
    params.record_everything_possible=record_everything_possible
    return cls(input_atom_balls, params)

MolecularRadicalTessellation.from_file = classmethod(_from_file)
MolecularRadicalTessellation.from_atoms = classmethod(_from_atoms)

__all__ = []

try:
    import pandas
except ImportError:
    pass
else:
    from . import pandas_interface
    __all__.append("pandas_interface")

    def print_head_of_pandas_data_frame(df, n=5):
        with pandas.option_context("display.max_columns", None, "display.width", None):
            print(df.head(n).to_string(index=False))

try:
    import biotite
except ImportError:
    pass
else:
    from . import biotite_interface
    __all__.append("biotite_interface")

try:
    import gemmi
except ImportError:
    pass
else:
    from . import gemmi_interface
    __all__.append("gemmi_interface")

try:
    import Bio.PDB
except ImportError:
    pass
else:
    from . import biopython_interface
    __all__.append("biopython_interface")

