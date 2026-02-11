from . import cadscorelt_python as _cadscorelt_backend
import pandas as _pd

def _molecular_atom_balls_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "ID_chain",
            "ID_rnum",
            "ID_icode",
            "ID_atom_name",
            "residue_name",
            "element",
            "x",
            "y",
            "z",
            "r",
        ])

    return _pd.DataFrame({
        "ID_chain":              [c.ID_chain or "."              for c in vec],
        "ID_rnum":               [c.ID_residue_seq_number        for c in vec],
        "ID_icode":              [c.ID_residue_icode or "."      for c in vec],
        "ID_atom_name":          [c.ID_atom_name or "."          for c in vec],
        "residue_name":          [c.residue_name or "."          for c in vec],
        "element":               [c.element or "."               for c in vec],
        "x":                     [c.x                            for c in vec],
        "y":                     [c.y                            for c in vec],
        "z":                     [c.z                            for c in vec],
        "r":                     [c.r                            for c in vec],
    })

def _global_scores_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "target_name",
            "model_name",
            "CAD_score",
            "F1_of_areas",
            "target_area",
            "model_area",
            "TP_area",
            "FP_area",
            "FN_area",
            "renaming_of_model_chains",
        ])
    return _pd.DataFrame({
        "target_name":              [c.target_name or "."                for c in vec],
        "model_name":               [c.model_name or "."                 for c in vec],
        "CAD_score":                [c.cadscore                          for c in vec],
        "F1_of_areas":              [c.F1_of_areas                       for c in vec],
        "target_area":              [c.target_area                       for c in vec],
        "model_area":               [c.model_area                        for c in vec],
        "TP_area":                  [c.TP_area                           for c in vec],
        "FP_area":                  [c.FP_area                           for c in vec],
        "FN_area":                  [c.FN_area                           for c in vec],
        "renaming_of_model_chains": [c.renaming_of_model_chains or "."   for c in vec],
    })

def _atom_scores_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "ID_chain",
            "ID_rnum",
            "ID_icode",
            "ID_atom_name",
            "CAD_score",
            "F1_of_areas",
            "target_area",
            "model_area",
            "TP_area",
            "FP_area",
            "FN_area",
        ])
    return _pd.DataFrame({
        "ID_chain":              [c.ID_chain or "."              for c in vec],
        "ID_rnum":               [c.ID_residue_seq_number        for c in vec],
        "ID_icode":              [c.ID_residue_icode or "."      for c in vec],
        "ID_atom_name":          [c.ID_atom_name or "."          for c in vec],
        "CAD_score":             [c.cadscore                     for c in vec],
        "F1_of_areas":           [c.F1_of_areas                  for c in vec],
        "target_area":           [c.target_area                  for c in vec],
        "model_area":            [c.model_area                   for c in vec],
        "TP_area":               [c.TP_area                      for c in vec],
        "FP_area":               [c.FP_area                      for c in vec],
        "FN_area":               [c.FN_area                      for c in vec],
    })

def _residue_scores_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "ID_chain",
            "ID_rnum",
            "ID_icode",
            "CAD_score",
            "F1_of_areas",
            "target_area",
            "model_area",
            "TP_area",
            "FP_area",
            "FN_area",
        ])
    return _pd.DataFrame({
        "ID_chain":              [c.ID_chain or "."              for c in vec],
        "ID_rnum":               [c.ID_residue_seq_number        for c in vec],
        "ID_icode":              [c.ID_residue_icode or "."      for c in vec],
        "CAD_score":             [c.cadscore                     for c in vec],
        "F1_of_areas":           [c.F1_of_areas                  for c in vec],
        "target_area":           [c.target_area                  for c in vec],
        "model_area":            [c.model_area                   for c in vec],
        "TP_area":               [c.TP_area                      for c in vec],
        "FP_area":               [c.FP_area                      for c in vec],
        "FN_area":               [c.FN_area                      for c in vec],
    })

def _chain_scores_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "ID_chain",
            "CAD_score",
            "F1_of_areas",
            "target_area",
            "model_area",
            "TP_area",
            "FP_area",
            "FN_area",
        ])
    return _pd.DataFrame({
        "ID_chain":    [c.ID_chain or "."    for c in vec],
        "CAD_score":   [c.cadscore           for c in vec],
        "F1_of_areas": [c.F1_of_areas        for c in vec],
        "target_area": [c.target_area        for c in vec],
        "model_area":  [c.model_area         for c in vec],
        "TP_area":     [c.TP_area            for c in vec],
        "FP_area":     [c.FP_area            for c in vec],
        "FN_area":     [c.FN_area            for c in vec],
    })

def _atom_atom_scores_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "ID1_chain",
            "ID1_rnum",
            "ID1_icode",
            "ID1_atom_name",
            "ID2_chain",
            "ID2_rnum",
            "ID2_icode",
            "ID2_atom_name",
            "CAD_score",
            "F1_of_areas",
            "target_area",
            "model_area",
            "TP_area",
            "FP_area",
            "FN_area",
        ])
    return _pd.DataFrame({
        "ID1_chain":              [c.ID1_chain or "."              for c in vec],
        "ID1_rnum":               [c.ID1_residue_seq_number        for c in vec],
        "ID1_icode":              [c.ID1_residue_icode or "."      for c in vec],
        "ID1_atom_name":          [c.ID1_atom_name or "."          for c in vec],
        "ID2_chain":              [c.ID2_chain or "."              for c in vec],
        "ID2_rnum":               [c.ID2_residue_seq_number        for c in vec],
        "ID2_icode":              [c.ID2_residue_icode or "."      for c in vec],
        "ID2_atom_name":          [c.ID2_atom_name or "."          for c in vec],
        "CAD_score":              [c.cadscore                      for c in vec],
        "F1_of_areas":            [c.F1_of_areas                   for c in vec],
        "target_area":            [c.target_area                   for c in vec],
        "model_area":             [c.model_area                    for c in vec],
        "TP_area":                [c.TP_area                       for c in vec],
        "FP_area":                [c.FP_area                       for c in vec],
        "FN_area":                [c.FN_area                       for c in vec],
    })

def _residue_residue_scores_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "ID1_chain",
            "ID1_rnum",
            "ID1_icode",
            "ID2_chain",
            "ID2_rnum",
            "ID2_icode",
            "CAD_score",
            "F1_of_areas",
            "target_area",
            "model_area",
            "TP_area",
            "FP_area",
            "FN_area",
        ])
    return _pd.DataFrame({
        "ID1_chain":              [c.ID1_chain or "."              for c in vec],
        "ID1_rnum":               [c.ID1_residue_seq_number        for c in vec],
        "ID1_icode":              [c.ID1_residue_icode or "."      for c in vec],
        "ID2_chain":              [c.ID2_chain or "."              for c in vec],
        "ID2_rnum":               [c.ID2_residue_seq_number        for c in vec],
        "ID2_icode":              [c.ID2_residue_icode or "."      for c in vec],
        "CAD_score":              [c.cadscore                      for c in vec],
        "F1_of_areas":            [c.F1_of_areas                   for c in vec],
        "target_area":            [c.target_area                   for c in vec],
        "model_area":             [c.model_area                    for c in vec],
        "TP_area":                [c.TP_area                       for c in vec],
        "FP_area":                [c.FP_area                       for c in vec],
        "FN_area":                [c.FN_area                       for c in vec],
    })

def _chain_chain_scores_vector_to_df(vec):
    if len(vec) == 0:
        return _pd.DataFrame(columns=[
            "ID1_chain",
            "ID2_chain",
            "CAD_score",
            "F1_of_areas",
            "target_area",
            "model_area",
            "TP_area",
            "FP_area",
            "FN_area",
        ])
    return _pd.DataFrame({
        "ID1_chain":   [c.ID1_chain or "."   for c in vec],
        "ID2_chain":   [c.ID2_chain or "."   for c in vec],
        "CAD_score":   [c.cadscore           for c in vec],
        "F1_of_areas": [c.F1_of_areas        for c in vec],
        "target_area": [c.target_area        for c in vec],
        "model_area":  [c.model_area         for c in vec],
        "TP_area":     [c.TP_area            for c in vec],
        "FP_area":     [c.FP_area            for c in vec],
        "FN_area":     [c.FN_area            for c in vec],
    })

def _attach_methods():
    if hasattr(_cadscorelt_backend, "VectorMolecularAtomBall"):
        _cadscorelt_backend.VectorMolecularAtomBall.to_pandas = _molecular_atom_balls_vector_to_df

    if hasattr(_cadscorelt_backend, "VectorGlobalScore"):
        _cadscorelt_backend.VectorGlobalScore.to_pandas = _global_scores_vector_to_df

    if hasattr(_cadscorelt_backend, "VectorAtomScore"):
        _cadscorelt_backend.VectorAtomScore.to_pandas = _atom_scores_vector_to_df

    if hasattr(_cadscorelt_backend, "VectorResidueScore"):
        _cadscorelt_backend.VectorResidueScore.to_pandas = _residue_scores_vector_to_df

    if hasattr(_cadscorelt_backend, "VectorChainScore"):
        _cadscorelt_backend.VectorChainScore.to_pandas = _chain_scores_vector_to_df

    if hasattr(_cadscorelt_backend, "VectorAtomAtomScore"):
        _cadscorelt_backend.VectorAtomAtomScore.to_pandas = _atom_atom_scores_vector_to_df

    if hasattr(_cadscorelt_backend, "VectorResidueResidueScore"):
        _cadscorelt_backend.VectorResidueResidueScore.to_pandas = _residue_residue_scores_vector_to_df

    if hasattr(_cadscorelt_backend, "VectorChainChainScore"):
        _cadscorelt_backend.VectorChainChainScore.to_pandas = _chain_chain_scores_vector_to_df

_attach_methods()
