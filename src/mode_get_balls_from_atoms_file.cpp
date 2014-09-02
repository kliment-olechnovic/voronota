#include <iostream>
#include <fstream>

#include "auxiliaries/atoms_reader.h"
#include "auxiliaries/atom_radius_assigner.h"

#include "modescommon_assert_options.h"
#include "modescommon_handle_ball.h"
#include "modescommon_handle_annotations.h"

namespace
{

bool add_descriptor_and_radius_from_stream_to_atom_radius_assigner(std::istream& input, auxiliaries::AtomRadiusAssigner& atom_radius_assigner)
{
	std::string resName;
	std::string name;
	double radius=0.0;
	input >> resName >> name >> radius;
	if(!input.fail())
	{
		atom_radius_assigner.add_radius_by_descriptor(resName, name, radius);
		return true;
	}
	return false;
}

std::string refine_string(const std::string& x)
{
	return (x.empty() ? std::string(".") : x);
}

}

void get_balls_from_atoms_file(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--annotated", "", "flag to enable annotated mode"));
		list_of_option_descriptions.push_back(OD("--include-heteroatoms", "", "flag to include heteroatoms"));
		list_of_option_descriptions.push_back(OD("--include-hydrogens", "", "flag to include hydrogen atoms"));
		list_of_option_descriptions.push_back(OD("--mmcif", "", "flag to input in mmCIF format"));
		list_of_option_descriptions.push_back(OD("--radii-file", "string", "path to radii configuration file"));
		list_of_option_descriptions.push_back(OD("--default-radius", "number", "default atomic radius"));
		list_of_option_descriptions.push_back(OD("--only-default-radius", "", "flag to make all radii equal to the default radius"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  file in PDB or mmCIF format\n";
			std::cerr << "stdout  ->  list of balls\n";
			std::cerr << "              (default mode line format: 'x y z r # atomSerial chainID resSeq resName atomName altLoc iCode')\n";
			std::cerr << "              (annotated mode line format: 'annotation x y z r tags adjuncts')\n";
			return;
		}
	}

	const bool annotated=poh.contains_option("--annotated");
	const bool mmcif=poh.contains_option("--mmcif");
	const bool include_heteroatoms=poh.contains_option("--include-heteroatoms");
	const bool include_hydrogens=poh.contains_option("--include-hydrogens");
	const std::string radii_file=poh.argument<std::string>("--radii-file", "");
	const double default_radius=poh.argument<double>("--default-radius", 1.70);
	const bool only_default_radius=poh.contains_option("--only-default-radius");

	const std::vector<auxiliaries::AtomsReader::AtomRecord> atoms=(mmcif ?
			auxiliaries::AtomsReader::read_atom_records_from_mmcif_file_stream(std::cin, include_heteroatoms, include_hydrogens) :
			auxiliaries::AtomsReader::read_atom_records_from_pdb_file_stream(std::cin, include_heteroatoms, include_hydrogens));
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
			atom_radius_assigner.add_radius_by_descriptor("*", "S*", 1.80);
			atom_radius_assigner.add_radius_by_descriptor("*", "P*", 1.80);
			atom_radius_assigner.add_radius_by_descriptor("*", "H*", 1.20);
		}
		else
		{
			std::ifstream radii_file_stream(radii_file.c_str(), std::ios::in);
			auxiliaries::read_lines_to_container(radii_file_stream, add_descriptor_and_radius_from_stream_to_atom_radius_assigner, atom_radius_assigner);
		}
	}

	for(std::size_t i=0;i<atoms.size();i++)
	{
		const auxiliaries::AtomsReader::AtomRecord& atom=atoms[i];
		const double radius=atom_radius_assigner.get_atom_radius(atom.resName, atom.name);
		std::ostringstream comment_output;
		comment_output << refine_string(atom.serial) << " " << refine_string(atom.chainID) << " " << refine_string(atom.resSeq) << " "
				<< refine_string(atom.resName) << " " << refine_string(atom.name) << " " << refine_string(atom.altLoc) << " " << refine_string(atom.iCode);
		if(annotated)
		{
			std::istringstream comment_input(comment_output.str());
			auxiliaries::ChainResidueAtomDescriptor comment;
			comment_input >> comment.serial >> comment.chainID >> comment.resSeq >> comment.resName >> comment.name >> comment.altLoc >> comment.iCode;
			if(!comment_input.fail())
			{
				if(comment.altLoc.find_first_of(".?")==0)
				{
					comment.altLoc.clear();
				}
				if(comment.iCode.find_first_of(".?")==0)
				{
					comment.iCode.clear();
				}
				if(comment.valid())
				{
					modescommon::BallValue value;
					value.x=atom.x;
					value.y=atom.y;
					value.z=atom.z;
					value.r=radius;
					if(atom.record_name=="HETATM")
					{
						value.set_tags("het");
					}
					if(!atom.element.empty())
					{
						value.set_tags(atom.element);
					}
					if(!atom.occupancy.empty())
					{
						value.set_adjuncts(std::string("oc=")+atom.occupancy);
					}
					if(!atom.tempFactor.empty())
					{
						value.set_adjuncts(std::string("tf=")+atom.tempFactor);
					}
					modescommon::print_ball_record(comment, value, std::cout);
				}
			}
		}
		else
		{
			std::cout << atom.x << " " << atom.y << " " << atom.z << " " << radius;
			std::cout << " # " << comment_output.str() << "\n";
		}
	}
}
