from . import voronotalt_python as _voronotalt_backend
from Bio.PDB.Atom import Atom
from typing import Iterable

def _get_element(atom: Atom) -> str:
    element = getattr(atom, "element", None)
    if element is None or not str(element).strip():
        name = atom.get_name().strip()
        if not name:
            return ""
        element = name[0]
    return str(element).strip().upper()

def _is_water(residue) -> bool:
    name = residue.get_resname().upper()
    return name in ("HOH", "WAT", "H2O")

def _is_hetero(residue) -> bool:
    het_flag = residue.id[0]
    return bool(str(het_flag).strip())

def molecular_atom_balls_from_atom_iterable(atom_iterable: Iterable[Atom], include_heteroatoms=True, include_hydrogens=False, include_waters=False):
    balls = []
    for atom in atom_iterable:
        residue = atom.get_parent()
        if residue is None:
            continue

        chain = residue.get_parent()
        if chain is None:
            continue

        if not include_waters and _is_water(residue):
            continue

        element = _get_element(atom)

        if not include_hydrogens and element == "H":
            continue

        if not include_heteroatoms and _is_hetero(residue):
            continue

        x, y, z = map(float, atom.coord)

        chain_id = str(chain.id)
        heteroflag, resseq, icode = residue.id
        res_seq_number = int(resseq)
        ins_code = str(icode).strip() if icode is not None else ""
        res_name = str(residue.get_resname())
        atom_name = str(atom.get_name())

        balls.append(
            _voronotalt_backend.MolecularAtomBall(
                chain_id,
                res_seq_number,
                ins_code,
                res_name,
                atom_name,
                x,
                y,
                z,
            )
        )
    return balls

def _from_biopython_atoms(
        cls,
        atom_iterable,
        include_heteroatoms=True,
        include_hydrogens=False,
        include_waters=False,
        **kwargs):
    return cls.from_atoms(
        molecular_atom_balls_from_atom_iterable(
            atom_iterable,
            include_heteroatoms=include_heteroatoms,
            include_hydrogens=include_hydrogens,
            include_waters=include_waters),
        **kwargs)

_voronotalt_backend.MolecularRadicalTessellation.from_biopython_atoms = classmethod(_from_biopython_atoms)


