#include <iostream>
#include <fstream>

#include "../apollota/spheres_boundary_construction.h"

#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/atoms_io.h"
#include "../auxiliaries/atom_radius_assigner.h"

#include "../common/chain_residue_atom_descriptor.h"
#include "../common/ball_value.h"

namespace
{

std::string refine_empty_string(const std::string& x)
{
	return (x.empty() ? std::string(".") : x);
}

}

void get_balls_from_atoms_file(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false,
			"file in PDB or mmCIF format");
	pohw.describe_io("stdout", false, true,
			"list of balls\n(default mode line format: 'x y z r # atomSerial chainID resSeq resName atomName altLoc iCode')\n(annotated mode line format: 'annotation x y z r tags adjuncts')");

	const bool annotated=poh.contains_option(pohw.describe_option("--annotated", "", "flag to enable annotated mode"));
	const bool include_heteroatoms=poh.contains_option(pohw.describe_option("--include-heteroatoms", "", "flag to include heteroatoms"));
	const bool include_hydrogens=poh.contains_option(pohw.describe_option("--include-hydrogens", "", "flag to include hydrogen atoms"));
	const bool multimodel_chains=poh.contains_option(pohw.describe_option("--multimodel-chains", "", "flag to read multiple models in PDB format and rename chains accordingly"));
	const bool mmcif=poh.contains_option(pohw.describe_option("--mmcif", "", "flag to input in mmCIF format"));
	const std::string radii_file=poh.argument<std::string>(pohw.describe_option("--radii-file", "string", "path to radii configuration file"), "");
	const double default_radius=poh.argument<double>(pohw.describe_option("--default-radius", "number", "default atomic radius"), 1.70);
	const bool only_default_radius=poh.contains_option(pohw.describe_option("--only-default-radius", "", "flag to make all radii equal to the default radius"));
	const double hull_offset=poh.argument<double>(pohw.describe_option("--hull-offset", "number", "positive offset distance enables adding artificial hull balls"), -1.0);

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const std::vector<auxiliaries::AtomsIO::AtomRecord> atoms=(mmcif ?
			auxiliaries::AtomsIO::MMCIFReader::read_data_from_file_stream(std::cin, include_heteroatoms, include_hydrogens, multimodel_chains).atom_records :
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
			auxiliaries::IOUtilities().read_lines_to_container(radii_file_stream, auxiliaries::AtomRadiusAssigner::add_descriptor_and_radius_from_stream_to_atom_radius_assigner, atom_radius_assigner);
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
			const common::ChainResidueAtomDescriptor crad(atom.serial, atom.chainID, atom.resSeq, atom.resName, atom.name, atom.altLoc, atom.iCode);
			if(crad.valid())
			{
				common::BallValue value;
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
				common::ChainResidueAtomDescriptor crad;
				crad.chainID="hull";
				common::BallValue value;
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
