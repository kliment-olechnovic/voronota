from . import voronotalt_python as voronotalt
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
        
        balls.append(voronotalt.Ball(x, y, z, float(r)))
    return balls

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
        balls.append(voronotalt.MolecularAtomBall(str(atom.chain_id), int(atom.res_id), str(atom.ins_code), str(atom.res_name), str(atom.atom_name), x, y, z))
    return balls

