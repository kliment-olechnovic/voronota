#include <iostream>
#include <fstream>

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/pdb_file_parser.h"
#include "auxiliaries/atom_radius_assigner.h"
#include "auxiliaries/command_line_options_handler.h"

namespace
{

void add_descriptor_and_radius_from_stream_to_atom_radius_assigner(std::istream& input, auxiliaries::AtomRadiusAssigner& atom_radius_assigner)
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

void get_balls_from_pdb_file(const auxiliaries::CommandLineOptionsHandler& clo)
{
	auxiliaries::CommandLineOptionsHandler::MapOfOptionDescriptions map_of_option_descriptions;
	map_of_option_descriptions["--include-heteroatoms"]=auxiliaries::CommandLineOptionsHandler::OptionDescription(false, "flag to include heteroatoms");
	map_of_option_descriptions["--default-radius"]=auxiliaries::CommandLineOptionsHandler::OptionDescription(true, "default atomic radius");
	map_of_option_descriptions["--only-default-radius"]=auxiliaries::CommandLineOptionsHandler::OptionDescription(false, "flag to make all radii equal to the default radius");
	map_of_option_descriptions["--radii-file"]=auxiliaries::CommandLineOptionsHandler::OptionDescription(true, "path to radii configuration file");
	map_of_option_descriptions["--output-comments"]=auxiliaries::CommandLineOptionsHandler::OptionDescription(false, "flag to output additional information about atoms");
	clo.compare_with_map_of_option_descriptions(map_of_option_descriptions);

	const bool include_heteroatoms=clo.contains_option("--include-heteroatoms");
	const double default_radius=clo.argument<double>("--default-radius", 1.70);
	const bool only_default_radius=clo.contains_option("--only-default-radius");
	const std::string radii_file=clo.argument<std::string>("--radii-file", "");
	const bool output_comments=clo.contains_option("--output-comments");

	const std::vector<auxiliaries::PDBFileParser::AtomRecord> atoms=auxiliaries::PDBFileParser::read_atom_records_from_pdb_file_stream(std::cin, include_heteroatoms);

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
			auxiliaries::read_lines_to_container(radii_file_stream, "#", add_descriptor_and_radius_from_stream_to_atom_radius_assigner, atom_radius_assigner);
		}
	}

	for(std::size_t i=0;i<atoms.size();i++)
	{
		const auxiliaries::PDBFileParser::AtomRecord& atom=atoms[i];
		std::cout << atom.x << " " << atom.y << " " << atom.z << " " << atom_radius_assigner.get_atom_radius(atom.resName, atom.name);
		if(output_comments)
		{
			std::cout << " # " << atom.serial << " " << atom.resName << " " << atom.name;
		}
		std::cout << "\n";
	}
}
