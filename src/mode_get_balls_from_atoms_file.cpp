#include <iostream>
#include <fstream>

#include "auxiliaries/atoms_reader.h"
#include "auxiliaries/atom_radius_assigner.h"

#include "modes_commons.h"

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

void get_balls_from_atoms_file(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--include-heteroatoms"].init("", "flag to include heteroatoms");
		basic_map_of_option_descriptions["--radii-file"].init("string", "path to radii configuration file");
		basic_map_of_option_descriptions["--output-comments"].init("", "flag to output additional information about atoms");
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions full_map_of_option_descriptions=basic_map_of_option_descriptions;
		full_map_of_option_descriptions["--default-radius"].init("number", "default atomic radius");
		full_map_of_option_descriptions["--only-default-radius"].init("", "flag to make all radii equal to the default radius");
		if(poh.contains_option("--help") || poh.contains_option("--help-full"))
		{
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions(poh.contains_option("--help-full") ? full_map_of_option_descriptions : basic_map_of_option_descriptions, std::cerr);
			std::cerr << "\n";
			std::cerr << "  stdin   <-  file in PDB format\n";
			std::cerr << "  stdout  ->  list of balls (line format: 'x y z r')\n";
			return;
		}
		else
		{
			poh.compare_with_map_of_option_descriptions(full_map_of_option_descriptions);
		}
	}

	const bool include_heteroatoms=poh.contains_option("--include-heteroatoms");

	const std::string radii_file=poh.argument<std::string>("--radii-file", "");

	const bool output_comments=poh.contains_option("--output-comments");

	const double default_radius=poh.argument<double>("--default-radius", 1.70);

	const bool only_default_radius=poh.contains_option("--only-default-radius");

	const std::vector<auxiliaries::AtomsReader::AtomRecord> atoms=auxiliaries::AtomsReader::read_atom_records_from_pdb_file_stream(std::cin, include_heteroatoms);
	if(atoms.empty())
	{
		throw std::runtime_error("No atoms provided to stdin.");
	}

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
		const auxiliaries::AtomsReader::AtomRecord& atom=atoms[i];
		std::cout << atom.x << " " << atom.y << " " << atom.z << " " << atom_radius_assigner.get_atom_radius(atom.resName, atom.name);
		if(output_comments)
		{
			std::cout << " # " << atom.serial << " " << atom.chainID << " " << atom.resSeq << " " << atom.resName << " " << atom.name;
		}
		std::cout << "\n";
	}
}
