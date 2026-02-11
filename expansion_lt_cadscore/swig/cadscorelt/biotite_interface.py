from . import cadscorelt_python as _cadscorelt_backend
from biotite.structure import AtomArray
from biotite.structure.info import vdw_radius_single

def molecular_atom_balls_from_atom_array(atom_array: AtomArray, include_heteroatoms=True, include_hydrogens=False, include_waters=False):
    balls = []
    for atom in atom_array:
        if not include_waters and atom.res_name in ("HOH", "WAT", "H2O"):
            continue
        
        if not include_hydrogens and atom.element == "H":
            continue
        
        if not include_heteroatoms and atom.hetero:
            continue
        
        x, y, z = map(float, atom.coord)
        balls.append(_cadscorelt_backend.MolecularAtomBall(str(atom.chain_id), int(atom.res_id), str(atom.ins_code), str(atom.atom_name), str(atom.res_name), str(atom.element), x, y, z))
    return balls

def _add_target_structure_from_biotite(
        self,
        atom_array: AtomArray,
        structure_name,
        include_heteroatoms=True):
    return self.add_target_structure_from_atoms(molecular_atom_balls_from_atom_array(atom_array, include_heteroatoms=include_heteroatoms), structure_name)

def _add_model_structure_from_biotite(
        self,
        atom_array: AtomArray,
        structure_name,
        include_heteroatoms=True):
    return self.add_model_structure_from_atoms(molecular_atom_balls_from_atom_array(atom_array, include_heteroatoms=include_heteroatoms), structure_name)

def _add_structure_from_biotite(
        self,
        atom_array: AtomArray,
        structure_name,
        include_heteroatoms=True):
    return self.add_structure_from_atoms(molecular_atom_balls_from_atom_array(atom_array, include_heteroatoms=include_heteroatoms), structure_name)

_cadscorelt_backend.CADScore.add_target_structure_from_biotite=_add_target_structure_from_biotite
_cadscorelt_backend.CADScore.add_model_structure_from_biotite=_add_model_structure_from_biotite
_cadscorelt_backend.CADScore.add_structure_from_biotite=_add_structure_from_biotite
