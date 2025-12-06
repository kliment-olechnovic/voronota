from . import voronotalt_python as _voronotalt_backend
import pandas as _pd

def _simple_balls_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "x",
            "y",
            "z",
            "r",
        ])

    return _pd.DataFrame({
        "x":                     [c.x                     for c in vec],
        "y":                     [c.y                     for c in vec],
        "z":                     [c.z                     for c in vec],
        "r":                     [c.r                     for c in vec],
    })


def _simple_contacts_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "index_a",
            "index_b",
            "area",
            "arc_length",
        ])

    return _pd.DataFrame({
        "index_a":                [c.index_a              for c in vec],
        "index_b":                [c.index_b              for c in vec],
        "area":                   [c.area                   for c in vec],
        "arc_length":             [c.arc_length             for c in vec],
    })


def _simple_cells_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "sas_area",
            "volume",
            "included",
        ])

    return _pd.DataFrame({
        "sas_area":              [c.sas_area              for c in vec],
        "volume":                [c.volume                for c in vec],
        "included":              [c.included              for c in vec],
    })


def _molecular_atom_balls_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "ID_chain",
            "ID_residue_seq_number",
            "ID_residue_icode",
            "ID_residue_name",
            "ID_atom_name",
            "x",
            "y",
            "z",
            "r",
        ])

    return _pd.DataFrame({
        "ID_chain":              [c.ID_chain              for c in vec],
        "ID_residue_seq_number": [c.ID_residue_seq_number for c in vec],
        "ID_residue_icode":      [c.ID_residue_icode      for c in vec],
        "ID_residue_name":       [c.ID_residue_name       for c in vec],
        "ID_atom_name":          [c.ID_atom_name          for c in vec],
        "x":                     [c.x                     for c in vec],
        "y":                     [c.y                     for c in vec],
        "z":                     [c.z                     for c in vec],
        "r":                     [c.r                     for c in vec],
    })


def _molecular_inter_atom_contact_summaries_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "ID1_chain",
            "ID1_residue_seq_number",
            "ID1_residue_icode",
            "ID1_residue_name",
            "ID1_atom_name",
            "ID2_chain",
            "ID2_residue_seq_number",
            "ID2_residue_icode",
            "ID2_residue_name", "ID2_atom_name",
            "area",
            "arc_length",
        ])

    return _pd.DataFrame({
        "ID1_chain":              [c.ID1_chain              for c in vec],
        "ID1_residue_seq_number": [c.ID1_residue_seq_number for c in vec],
        "ID1_residue_icode":      [c.ID1_residue_icode      for c in vec],
        "ID1_residue_name":       [c.ID1_residue_name       for c in vec],
        "ID1_atom_name":          [c.ID1_atom_name          for c in vec],
        "ID2_chain":              [c.ID2_chain              for c in vec],
        "ID2_residue_seq_number": [c.ID2_residue_seq_number for c in vec],
        "ID2_residue_icode":      [c.ID2_residue_icode      for c in vec],
        "ID2_residue_name":       [c.ID2_residue_name       for c in vec],
        "ID2_atom_name":          [c.ID2_atom_name          for c in vec],
        "area":                   [c.area                   for c in vec],
        "arc_length":             [c.arc_length             for c in vec],
    })


def _molecular_inter_residue_contact_summaries_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "ID1_chain",
            "ID1_residue_seq_number",
            "ID1_residue_icode",
            "ID1_residue_name",
            "ID2_chain",
            "ID2_residue_seq_number",
            "ID2_residue_icode",
            "ID2_residue_name",
            "area",
            "arc_length",
        ])

    return _pd.DataFrame({
        "ID1_chain":              [c.ID1_chain              for c in vec],
        "ID1_residue_seq_number": [c.ID1_residue_seq_number for c in vec],
        "ID1_residue_icode":      [c.ID1_residue_icode      for c in vec],
        "ID1_residue_name":       [c.ID1_residue_name       for c in vec],
        "ID2_chain":              [c.ID2_chain              for c in vec],
        "ID2_residue_seq_number": [c.ID2_residue_seq_number for c in vec],
        "ID2_residue_icode":      [c.ID2_residue_icode      for c in vec],
        "ID2_residue_name":       [c.ID2_residue_name       for c in vec],
        "area":                   [c.area                   for c in vec],
        "arc_length":             [c.arc_length             for c in vec],
    })


def _molecular_inter_chain_contact_summaries_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "ID1_chain",
            "ID2_chain",
            "area", "arc_length",
        ])

    return _pd.DataFrame({
        "ID1_chain":  [c.ID1_chain  for c in vec],
        "ID2_chain":  [c.ID2_chain  for c in vec],
        "area":       [c.area       for c in vec],
        "arc_length": [c.arc_length for c in vec],
    })


def _molecular_atom_cell_summaries_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "ID_chain",
            "ID_residue_seq_number",
            "ID_residue_icode",
            "ID_residue_name",
            "ID_atom_name",
            "sas_area",
            "volume",
        ])

    return _pd.DataFrame({
        "ID_chain":              [c.ID_chain              for c in vec],
        "ID_residue_seq_number": [c.ID_residue_seq_number for c in vec],
        "ID_residue_icode":      [c.ID_residue_icode      for c in vec],
        "ID_residue_name":       [c.ID_residue_name       for c in vec],
        "ID_atom_name":          [c.ID_atom_name          for c in vec],
        "sas_area":              [c.sas_area              for c in vec],
        "volume":                [c.volume                for c in vec],
    })


def _molecular_residue_cell_summaries_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "ID_chain",
            "ID_residue_seq_number",
            "ID_residue_icode",
            "ID_residue_name",
            "sas_area",
            "volume",
        ])

    return _pd.DataFrame({
        "ID_chain":              [c.ID_chain              for c in vec],
        "ID_residue_seq_number": [c.ID_residue_seq_number for c in vec],
        "ID_residue_icode":      [c.ID_residue_icode      for c in vec],
        "ID_residue_name":       [c.ID_residue_name       for c in vec],
        "sas_area":              [c.sas_area              for c in vec],
        "volume":                [c.volume                for c in vec],
    })


def _molecular_chain_cell_summaries_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "ID_chain",
            "sas_area", "volume",
        ])

    return _pd.DataFrame({
        "ID_chain": [c.ID_chain for c in vec],
        "sas_area": [c.sas_area for c in vec],
        "volume":   [c.volume   for c in vec],
    })

def _attach_methods():
    if hasattr(_voronotalt_backend, "VectorBall"):
        _voronotalt_backend.VectorBall.to_pandas = _simple_balls_vector_to_df

    if hasattr(_voronotalt_backend, "VectorContacts"):
        _voronotalt_backend.VectorContacts.to_pandas = _simple_contacts_vector_to_df

    if hasattr(_voronotalt_backend, "VectorMolecularAtomBall"):
        _voronotalt_backend.VectorMolecularAtomBall.to_pandas = _molecular_atom_balls_vector_to_df

    if hasattr(_voronotalt_backend, "VectorCell"):
        _voronotalt_backend.VectorCell.to_pandas = _simple_cells_vector_to_df

    if hasattr(_voronotalt_backend, "VectorMolecularInterAtomContactSummary"):
        _voronotalt_backend.VectorMolecularInterAtomContactSummary.to_pandas = _molecular_inter_atom_contact_summaries_vector_to_df

    if hasattr(_voronotalt_backend, "VectorMolecularInterResidueContactSummary"):
        _voronotalt_backend.VectorMolecularInterResidueContactSummary.to_pandas = _molecular_inter_residue_contact_summaries_vector_to_df

    if hasattr(_voronotalt_backend, "VectorMolecularInterChainContactSummary"):
        _voronotalt_backend.VectorMolecularInterChainContactSummary.to_pandas = _molecular_inter_chain_contact_summaries_vector_to_df

    if hasattr(_voronotalt_backend, "VectorMolecularAtomCellSummary"):
        _voronotalt_backend.VectorMolecularAtomCellSummary.to_pandas = _molecular_atom_cell_summaries_vector_to_df

    if hasattr(_voronotalt_backend, "VectorMolecularResidueCellSummary"):
        _voronotalt_backend.VectorMolecularResidueCellSummary.to_pandas = _molecular_residue_cell_summaries_vector_to_df

    if hasattr(_voronotalt_backend, "VectorMolecularChainCellSummary"):
        _voronotalt_backend.VectorMolecularChainCellSummary.to_pandas = _molecular_chain_cell_summaries_vector_to_df

_attach_methods()


