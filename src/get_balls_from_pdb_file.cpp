#include <iostream>
#include <fstream>
#include <sstream>

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/pdb_file_parsing.h"
#include "auxiliaries/atom_radius_assigner.h"
#include "auxiliaries/command_line_options.h"

namespace
{

void add_description_and_radius_from_stream_to_atom_radius_assigner(std::istream& input, auxiliaries::AtomRadiusAssigner& atom_radius_assigner)
{
	std::string resName;
	std::string name;
	double radius=0.0;
	input >> resName >> name >> radius;
	if(!input.fail())
	{
		atom_radius_assigner.add_radius_by_descriptor(resName, name, radius);
	}
}

}

void get_balls_from_pdb_file(const auxiliaries::CommandLineOptions& clo)
{
	clo.check_allowed_options("--include-heteroatoms --default-radius: --only-default-radius --radii-file: --output-comments");

	const bool include_heteroatoms=clo.isopt("--include-heteroatoms");
	const double default_radius=clo.isopt("--default-radius") ? clo.arg<double>("--default-radius") : 1.70;
	const bool only_default_radius=clo.isopt("--only-default-radius");
	const std::string radii_file=clo.isopt("--radii-file") ? clo.arg<std::string>("--radii-file") : std::string();
	const bool output_comments=clo.isopt("--output-comments");

	const std::vector<auxiliaries::PDBFileParsing::AtomRecord> atoms=auxiliaries::PDBFileParsing::read_atom_records_from_pdb_file_stream(std::cin, include_heteroatoms);

	auxiliaries::AtomRadiusAssigner atom_radius_assigner(default_radius);
	if(!only_default_radius)
	{
		if(radii_file.empty())
		{
			atom_radius_assigner.add_radius_by_descriptor("*", "C*", 1.70);
			atom_radius_assigner.add_radius_by_descriptor("*", "N*", 1.55);
			atom_radius_assigner.add_radius_by_descriptor("*", "O*", 1.52);
			atom_radius_assigner.add_radius_by_descriptor("*", "P*", 1.80);
		}
		else
		{
			std::ifstream radii_file_stream(radii_file.c_str(), std::ios::in);
			auxiliaries::read_lines_to_container(radii_file_stream, "#", add_description_and_radius_from_stream_to_atom_radius_assigner, atom_radius_assigner);
		}
	}

	for(std::size_t i=0;i<atoms.size();i++)
	{
		const auxiliaries::PDBFileParsing::AtomRecord& atom=atoms[i];
		std::cout << atom.x << " " << atom.y << " " << atom.z << " " << atom_radius_assigner.get_atom_radius(atom.resName, atom.name);
		if(output_comments)
		{
			std::cout << " # " << atom.serial << " " << atom.resName << " " << atom.name;
		}
		std::cout << "\n";
	}
}
