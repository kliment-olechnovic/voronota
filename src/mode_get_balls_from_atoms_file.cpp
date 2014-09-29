#include <iostream>
#include <fstream>

#include "apollota/spheres_boundary_construction.h"

#include "auxiliaries/atoms_io.h"
#include "auxiliaries/atom_radius_assigner.h"

#include "modescommon/assert_options.h"
#include "modescommon/handle_ball.h"

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

std::string refine_empty_string(const std::string& x)
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
		list_of_option_descriptions.push_back(OD("--hull-offset", "number", "positive offset distance enables adding artificial hull balls"));
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
	const double hull_offset=poh.argument<double>("--hull-offset", -1.0);

	const std::vector<auxiliaries::AtomsIO::AtomRecord> atoms=(mmcif ?
			auxiliaries::AtomsIO::MMCIFReader::read_data_from_file_stream(std::cin, include_heteroatoms, include_hydrogens).atom_records :
			auxiliaries::AtomsIO::PDBReader::read_data_from_file_stream(std::cin, include_heteroatoms, include_hydrogens, false).atom_records);
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

	std::vector<apollota::SimpleSphere> all_spheres;
	all_spheres.reserve(atoms.size());

	for(std::size_t i=0;i<atoms.size();i++)
	{
		const auxiliaries::AtomsIO::AtomRecord& atom=atoms[i];
		const double radius=atom_radius_assigner.get_atom_radius(atom.resName, atom.name);
		if(annotated)
		{
			const auxiliaries::ChainResidueAtomDescriptor crad(atom, atom.chainID);
			if(crad.valid())
			{
				modescommon::BallValue value;
				value.x=atom.x;
				value.y=atom.y;
				value.z=atom.z;
				value.r=radius;
				if(atom.record_name=="HETATM")
				{
					modescommon::update_set_of_tags(value.tags, "het");
				}
				if(!atom.element.empty())
				{
					modescommon::update_set_of_tags(value.tags, std::string("el=")+atom.element);
				}
				if(atom.occupancy_valid)
				{
					value.adjuncts["oc"]=atom.occupancy;
				}
				if(atom.tempFactor_valid)
				{
					value.adjuncts["tf"]=atom.tempFactor;
				}
				modescommon::print_ball_record(crad, value, std::cout);
				all_spheres.push_back(apollota::SimpleSphere(value));
			}
		}
		else
		{
			std::cout << atom.x << " " << atom.y << " " << atom.z << " " << radius;
			std::cout << " # " << atom.serial << " " << refine_empty_string(atom.chainID) << " " << atom.resSeq << " " << refine_empty_string(atom.resName) << " " << refine_empty_string(atom.name) << " " << refine_empty_string(atom.altLoc) << " " << refine_empty_string(atom.iCode) << "\n";
			all_spheres.push_back(apollota::SimpleSphere(atom, radius));
		}
	}

	if(hull_offset>0.0 && !all_spheres.empty())
	{
		const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(all_spheres, hull_offset);
		for(std::size_t i=0;i<artificial_boundary.size();i++)
		{
			const apollota::SimpleSphere& s=artificial_boundary[i];
			if(annotated)
			{
				auxiliaries::ChainResidueAtomDescriptor crad;
				crad.chainID="hull";
				modescommon::BallValue value;
				value.x=s.x;
				value.y=s.y;
				value.z=s.z;
				value.r=s.r;
				modescommon::print_ball_record(crad, value, std::cout);
			}
			else
			{
				std::cout << s.x << " " << s.y << " " << s.z << " " << s.r << " # artificial hull ball\n";
			}
		}
	}
}
