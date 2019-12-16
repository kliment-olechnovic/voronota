% VORONOTA(1)
% Kliment Olechnovic
% December, 2019

# NAME

Voronota - software for Voronoi tessellation-based analysis of macromolecular structures

# SYNOPSIS

voronota *command* [*options*]...

voronota-voromqa [*options*]...

voronota-cadscore [*options*]...

voronota-contacts [*options*]...

voronota-volumes [*options*]...

voronota-pocket [*options*]...

voronota-membrane [*options*]...

# DESCRIPTION

The analysis of macromolecular structures often requires
a comprehensive definition of atomic neighborhoods.
Such a definition can be based on the Voronoi diagram of balls,
where each ball represents an atom of some van der Waals radius.
Voronota is a software tool for finding all the vertices
of the Voronoi diagram of balls. Such vertices correspond to
the centers of the empty tangent spheres defined by quadruples of balls.
Voronota is especially suitable for processing three-dimensional
structures of biological macromolecules such as proteins and RNA.

Since version 1.2 Voronota also uses the Voronoi vertices to construct
inter-atom contact surfaces and solvent accessible surfaces.
Voronota provides tools to query contacts, generate contacts graphics,
compare contacts and evaluate quality of protein structural models using contacts.

The list of all available Voronota commands is displayed when executing Voronota without any parameters.

Command help is shown when `--help` command line option is present, for example:

    voronota calculate-vertices --help

Using `--help` option without specific command results in printing help for all commands:

    voronota --help

There are several Voronota wrapper scripts, their interfaces can displayed similarly:

    voronota-voromqa --help
    voronota-cadscore --help
    voronota-contacts --help
    voronota-volumes --help
    voronota-pocket --help
    voronota-membrane --help

# SEE ALSO

The *README* file distributed with Voronota contains more detailed documentation.

The Voronota source code and all the supporting files may be downloaded from
<https://github.com/kliment-olechnovic/voronota/releases>.

