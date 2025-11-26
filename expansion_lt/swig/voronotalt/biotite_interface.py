from . import voronotalt_python as _voronotalt_backend
from biotite.structure import AtomArray
from biotite.structure.info import vdw_radius_single

def simple_balls_from_atom_array(atom_array: AtomArray, include_heteroatoms=True, include_hydrogens=False, include_waters=False, default_radius=1.7):
    balls = []
    for atom in atom_array:
        if not include_waters and atom.res_name in ("HOH", "WAT", "H2O"):
            continue
        
        if not include_hydrogens and atom.element == "H":
            continue
        
        if not include_heteroatoms and atom.hetero:
            continue
        
        x, y, z = map(float, atom.coord)
        
        r = vdw_radius_single(atom.element)
        if r is None:
            r = default_radius
        
        balls.append(_voronotalt_backend.Ball(x, y, z, float(r)))
    return balls

def _from_simple_balls_from_biotite_atoms(
        cls,
        atom_array,
        include_heteroatoms=True,
        include_hydrogens=False,
        include_waters=False,
        default_radius=1.7,
        **kwargs):
    return cls(
        simple_balls_from_atom_array(
            atom_array,
            include_heteroatoms=include_heteroatoms,
            include_hydrogens=include_hydrogens,
            include_waters=include_waters),
        **kwargs)

_voronotalt_backend.RadicalTessellation.from_biotite_atoms = classmethod(_from_simple_balls_from_biotite_atoms)

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
        balls.append(_voronotalt_backend.MolecularAtomBall(str(atom.chain_id), int(atom.res_id), str(atom.ins_code), str(atom.res_name), str(atom.atom_name), x, y, z))
    return balls

def _from_molecular_atom_balls_from_biotite_atoms(
        cls,
        atom_array,
        include_heteroatoms=True,
        include_hydrogens=False,
        include_waters=False,
        **kwargs):
    return cls.from_atoms(
        molecular_atom_balls_from_atom_array(
            atom_array,
            include_heteroatoms=include_heteroatoms,
            include_hydrogens=include_hydrogens,
            include_waters=include_waters),
        **kwargs)

_voronotalt_backend.MolecularRadicalTessellation.from_biotite_atoms = classmethod(_from_molecular_atom_balls_from_biotite_atoms)

