from . import cadscorelt_python as _cadscorelt_backend
import gemmi

def molecular_atom_balls_from_gemmi_model(model: "gemmi.Model", include_heteroatoms: bool = True, include_hydrogens: bool = False, include_waters: bool = False):
    balls = []
    for chain in model:
        chain_id = chain.name

        for residue in chain:
            if not include_waters and residue.is_water():
                continue

            is_hetero = (residue.het_flag == "H")
            if not include_heteroatoms and is_hetero:
                continue

            res_name = residue.name
            seqid = residue.seqid
            res_id = int(seqid.num) if seqid.num is not None else 0
            ins_code = seqid.icode
            if ins_code == " ":
                ins_code = ""

            for atom in residue:
                if not include_hydrogens and atom.is_hydrogen():
                    continue

                pos = atom.pos
                x, y, z = float(pos.x), float(pos.y), float(pos.z)
                balls.append(_cadscorelt_backend.MolecularAtomBall(str(chain_id), int(res_id), str(ins_code), str(atom.name), str(res_name), atom.element.name, x, y, z,))
    return balls

def _add_target_structure_from_gemmi(
        self,
        model: "gemmi.Model",
        structure_name,
        include_heteroatoms=True):
    return self.add_target_structure_from_atoms(molecular_atom_balls_from_gemmi_model(model, include_heteroatoms=include_heteroatoms), structure_name)

def _add_model_structure_from_gemmi(
        self,
        model: "gemmi.Model",
        structure_name,
        include_heteroatoms=True):
    return self.add_model_structure_from_atoms(molecular_atom_balls_from_gemmi_model(model, include_heteroatoms=include_heteroatoms), structure_name)

def _add_structure_from_gemmi(
        self,
        model: "gemmi.Model",
        structure_name,
        include_heteroatoms=True):
    return self.add_structure_from_atoms(molecular_atom_balls_from_gemmi_model(model, include_heteroatoms=include_heteroatoms), structure_name)

_cadscorelt_backend.CADScoreComputer.add_target_structure_from_gemmi=_add_target_structure_from_gemmi
_cadscorelt_backend.CADScoreComputer.add_model_structure_from_gemmi=_add_model_structure_from_gemmi
_cadscorelt_backend.CADScoreComputer.add_structure_from_gemmi=_add_structure_from_gemmi
