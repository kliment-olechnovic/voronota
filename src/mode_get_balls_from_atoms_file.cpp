#include <iostream>
#include <fstream>

#include "apollota/spheres_boundary_construction.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/atoms_io.h"
#include "auxiliaries/atom_radius_assigner.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

#include "modescommon/ball_value.h"

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
		std::vector<OD> ods;
		ods.push_back(OD("--annotated", "", "flag to enable annotated mode"));
		ods.push_back(OD("--include-heteroatoms", "", "flag to include heteroatoms"));
		ods.push_back(OD("--include-hydrogens", "", "flag to include hydrogen atoms"));
		ods.push_back(OD("--multimodel-chains", "", "flag to read multiple models in PDB format and rename chains accordingly"));
		ods.push_back(OD("--mmcif", "", "flag to input in mmCIF format"));
		ods.push_back(OD("--radii-file", "string", "path to radii configuration file"));
		ods.push_back(OD("--default-radius", "number", "default atomic radius"));
		ods.push_back(OD("--only-default-radius", "", "flag to make all radii equal to the default radius"));
		ods.push_back(OD("--hull-offset", "number", "positive offset distance enables adding artificial hull balls"));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false, "file in PDB or mmCIF format");
			poh.print_io_description("stdout", false, true,
					"list of balls\n(default mode line format: 'x y z r # atomSerial chainID resSeq resName atomName altLoc iCode')\n(annotated mode line format: 'annotation x y z r tags adjuncts')");
			return;
		}
	}

	const bool annotated=poh.contains_option("--annotated");
	const bool mmcif=poh.contains_option("--mmcif");
	const bool include_heteroatoms=poh.contains_option("--include-heteroatoms");
	const bool include_hydrogens=poh.contains_option("--include-hydrogens");
	const bool multimodel_chains=poh.contains_option("--multimodel-chains");
	const std::string radii_file=poh.argument<std::string>("--radii-file", "");
	const double default_radius=poh.argument<double>("--default-radius", 1.70);
	const bool only_default_radius=poh.contains_option("--only-default-radius");
	const double hull_offset=poh.argument<double>("--hull-offset", -1.0);

	const std::vector<auxiliaries::AtomsIO::AtomRecord> atoms=(mmcif ?
			auxiliaries::AtomsIO::MMCIFReader::read_data_from_file_stream(std::cin, include_heteroatoms, include_hydrogens).atom_records :
			auxiliaries::AtomsIO::PDBReader::read_data_from_file_stream(std::cin, include_heteroatoms, include_hydrogens, multimodel_chains, false).atom_records);
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
			auxiliaries::IOUtilities().read_lines_to_container(radii_file_stream, add_descriptor_and_radius_from_stream_to_atom_radius_assigner, atom_radius_assigner);
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
			const auxiliaries::ChainResidueAtomDescriptor crad(atom.serial, atom.chainID, atom.resSeq, atom.resName, atom.name, atom.altLoc, atom.iCode);
			if(crad.valid())
			{
				BallValue value;
				value.x=atom.x;
				value.y=atom.y;
				value.z=atom.z;
				value.r=radius;
				if(atom.record_name=="HETATM")
				{
					value.props.tags.insert("het");
				}
				if(!atom.element.empty())
				{
					value.props.tags.insert(std::string("el=")+atom.element);
				}
				if(atom.occupancy_valid)
				{
					value.props.adjuncts["oc"]=atom.occupancy;
				}
				if(atom.tempFactor_valid)
				{
					value.props.adjuncts["tf"]=atom.tempFactor;
				}
				std::cout << crad << " " << value << "\n";
				all_spheres.push_back(apollota::SimpleSphere(value));
			}
		}
		else
		{
			std::cout << apollota::SimpleSphere(atom, radius);
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
				BallValue value;
				value.x=s.x;
				value.y=s.y;
				value.z=s.z;
				value.r=s.r;
				std::cout << crad << " " << value << "\n";
			}
			else
			{
				std::cout << s << " # artificial hull ball\n";
			}
		}
	}
}
