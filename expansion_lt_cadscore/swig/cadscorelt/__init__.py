from .cadscorelt_python import *
from pathlib import Path

def _initialize(
        cls,
        probe=1.4,
        conflate_atom_names=True,
        remap_chains=False,
        score_atom_atom_contacts=False,
        score_residue_residue_contacts=True,
        score_chain_chain_contacts=False,
        score_atom_sas=False,
        score_residue_sas=False,
        score_chain_sas=False,
        score_atom_sites=False,
        score_residue_sites=False,
        score_chain_sites=False,
        calculate_identities=False,
        record_local_scores=False,
        include_self_to_self_scores=False,
        restrict_input_atoms="",
        subselect_contacts="[-min-sep 1]",
        subselect_atoms=""):
    params=CADScoreComputerParameters()
    params.probe=probe
    params.conflate_atom_names=conflate_atom_names
    params.remap_chains=remap_chains
    params.score_atom_atom_contacts=score_atom_atom_contacts
    params.score_residue_residue_contacts=score_residue_residue_contacts
    params.score_chain_chain_contacts=score_chain_chain_contacts
    params.score_atom_sas=score_atom_sas
    params.score_residue_sas=score_residue_sas
    params.score_chain_sas=score_chain_sas
    params.score_atom_sites=score_atom_sites
    params.score_residue_sites=score_residue_sites
    params.score_chain_sites=score_chain_sites
    params.calculate_identities=calculate_identities
    params.record_local_scores=record_local_scores
    params.include_self_to_self_scores=include_self_to_self_scores
    params.restrict_input_atoms=restrict_input_atoms
    params.subselect_contacts=subselect_contacts
    params.subselect_atoms=subselect_atoms
    return cls(params)

def _add_target_structure_from_file(
        self,
        input_file,
        structure_name=None,
        include_heteroatoms=True,
        read_as_assembly=False):
    if structure_name is None:
        structure_name = Path(input_file).stem
    mfi = MolecularFileInput(input_file, include_heteroatoms, read_as_assembly)
    return self.add_target_structure_from_file_descriptor(mfi, structure_name)

def _add_model_structure_from_file(
        self,
        input_file,
        structure_name=None,
        include_heteroatoms=True,
        read_as_assembly=False):
    if structure_name is None:
        structure_name = Path(input_file).stem
    mfi = MolecularFileInput(input_file, include_heteroatoms, read_as_assembly)
    return self.add_model_structure_from_file_descriptor(mfi, structure_name)
    
def _add_structure_from_file(
        self,
        input_file,
        structure_name=None,
        include_heteroatoms=True,
        read_as_assembly=False):
    if structure_name is None:
        structure_name = Path(input_file).stem
    mfi = MolecularFileInput(input_file, include_heteroatoms, read_as_assembly)
    return self.add_structure_from_file_descriptor(mfi, structure_name)

def _add_target_structure_from_string(
        self,
        input_string_data,
        structure_name,
        include_heteroatoms=True,
        read_as_assembly=False):
    mfi = MolecularFileInput("_data_blob", input_string_data, include_heteroatoms, read_as_assembly)
    return self.add_target_structure_from_file_descriptor(mfi, structure_name)

def _add_model_structure_from_string(
        self,
        input_string_data,
        structure_name,
        include_heteroatoms=True,
        read_as_assembly=False):
    mfi = MolecularFileInput("_data_blob", input_string_data, include_heteroatoms, read_as_assembly)
    return self.add_model_structure_from_file_descriptor(mfi, structure_name)

def _add_structure_from_string(
        self,
        input_string_data,
        structure_name,
        include_heteroatoms=True,
        read_as_assembly=False):
    mfi = MolecularFileInput("_data_blob", input_string_data, include_heteroatoms, read_as_assembly)
    return self.add_structure_from_file_descriptor(mfi, structure_name)

CADScoreComputer.init = classmethod(_initialize)
CADScoreComputer.add_target_structure_from_file = _add_target_structure_from_file
CADScoreComputer.add_model_structure_from_file = _add_model_structure_from_file
CADScoreComputer.add_structure_from_file = _add_structure_from_file
CADScoreComputer.add_target_structure_from_string = _add_structure_from_string
CADScoreComputer.add_model_structure_from_string = _add_structure_from_string
CADScoreComputer.add_structure_from_string = _add_structure_from_string

__all__ = []

try:
    import pandas
except ImportError:
    pass
else:
    from . import pandas_interface
    __all__.append("pandas_interface")

    def print_head_of_pandas_data_frame(df, n=10):
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

