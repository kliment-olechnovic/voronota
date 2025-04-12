import voronotalt_python as voronotalt
from biotite.structure import AtomArray
from biotite.structure.info import vdw_radius_single

def balls_from_atom_array(atom_array: AtomArray, include_hydrogens=False, default_radius=1.7):
    """
    Convert a Biotite AtomArray to a list of Voronota-LT Ball objects.

    Parameters
    ----------
    atom_array : AtomArray
        Input structure from Biotite.
    include_hydrogens : bool
        If False, skip atoms with element "H".
    default_radius : float
        Radius to assign when van der Waals radius is unknown.

    Returns
    -------
    list of voronotalt.Ball
        List of balls for RadicalTessellation.
    """
    balls = []
    for atom in atom_array:
        if not include_hydrogens and atom.element == "H":
            continue
        x, y, z = map(float, atom.coord)
        r = vdw_radius_single(atom.element)
        if r is None:
            r = default_radius
        balls.append(voronotalt.Ball(x, y, z, float(r)))
    return balls

def radical_tessellation_from_atom_array(atom_array: AtomArray, probe=1.4, **kwargs):
    """
    Create a RadicalTessellation from a Biotite AtomArray.

    Parameters
    ----------
    atom_array : AtomArray
        The Biotite AtomArray to tessellate.
    probe : float
        Probe radius for SAS definition.
    kwargs : dict
        Additional arguments passed to balls_from_atom_array.

    Returns
    -------
    voronotalt.RadicalTessellation
        The resulting tessellation object.
    """
    balls = balls_from_atom_array(atom_array, **kwargs)
    return voronotalt.RadicalTessellation(balls, probe)


