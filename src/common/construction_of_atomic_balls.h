#ifndef COMMON_CONSTRUCTION_OF_ATOMIC_BALLS_H_
#define COMMON_CONSTRUCTION_OF_ATOMIC_BALLS_H_

#include <vector>

#include "../auxiliaries/atoms_io.h"
#include "../auxiliaries/atom_radius_assigner.h"
#include "../auxiliaries/io_utilities.h"

#include "chain_residue_atom_descriptor.h"
#include "ball_value.h"

namespace common
{

class ConstructionOfAtomicBalls
{
public:
	struct AtomicBall
	{
		ChainResidueAtomDescriptor crad;
		BallValue value;

		AtomicBall(const ChainResidueAtomDescriptor& crad, const BallValue& value) : crad(crad), value(value)
		{
		}
	};

	class collect_atomic_balls_from_file
	{
	public:
		bool mmcif;
		bool include_heteroatoms;
		bool include_hydrogens;
		bool multimodel_chains;
		auxiliaries::AtomRadiusAssigner atom_radius_assigner;

		collect_atomic_balls_from_file() :
			mmcif(false),
			include_heteroatoms(false),
			include_hydrogens(false),
			multimodel_chains(false),
			atom_radius_assigner(generate_atom_radius_assigner(default_default_radius(), false, ""))
		{
		}

		static double default_default_radius()
		{
			return 1.7;
		}

		void set_atom_radius_assigner(const double default_radius, const bool only_default_radius, const std::string& radii_file)
		{
			atom_radius_assigner=generate_atom_radius_assigner(default_radius, only_default_radius, radii_file);
		}

		bool operator()(std::istream& input_stream, std::vector<AtomicBall>& atomic_balls) const
		{
			atomic_balls.clear();

			const std::vector<auxiliaries::AtomsIO::AtomRecord> atoms=(mmcif ?
					auxiliaries::AtomsIO::MMCIFReader::read_data_from_file_stream(input_stream, include_heteroatoms, include_hydrogens).atom_records :
					auxiliaries::AtomsIO::PDBReader::read_data_from_file_stream(input_stream, include_heteroatoms, include_hydrogens, multimodel_chains, false).atom_records);

			if(atoms.empty())
			{
				return false;
			}

			atomic_balls.reserve(atoms.size());

			for(std::size_t i=0;i<atoms.size();i++)
			{
				const auxiliaries::AtomsIO::AtomRecord& atom=atoms[i];
				const ChainResidueAtomDescriptor crad(atom.serial, atom.chainID, atom.resSeq, atom.resName, atom.name, atom.altLoc, atom.iCode);
				if(crad.valid())
				{
					BallValue value;
					value.x=atom.x;
					value.y=atom.y;
					value.z=atom.z;
					value.r=atom_radius_assigner.get_atom_radius(atom.resName, atom.name);
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
					atomic_balls.push_back(AtomicBall(crad, value));
				}
			}

			return true;
		}

		bool operator()(std::string& input_file, std::vector<AtomicBall>& atomic_balls) const
		{
			std::ifstream input(input_file.c_str(), std::ios::in);
			return (*this)(input, atomic_balls);
		}

	private:
		static auxiliaries::AtomRadiusAssigner generate_atom_radius_assigner(const double default_radius, const bool only_default_radius, const std::string& radii_file)
		{
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
					if(radii_file_stream.good())
					{
						auxiliaries::IOUtilities().read_lines_to_container(
								radii_file_stream,
								auxiliaries::AtomRadiusAssigner::add_descriptor_and_radius_from_stream_to_atom_radius_assigner,
								atom_radius_assigner);
					}
				}
			}
			return atom_radius_assigner;
		}
	};

	template<typename PlainBall>
	static std::vector<PlainBall> collect_plain_balls_from_atomic_balls(const std::vector<AtomicBall>& atomic_balls)
	{
		std::vector<PlainBall> plain_balls;
		plain_balls.reserve(atomic_balls.size());
		for(std::size_t i=0;i<atomic_balls.size();i++)
		{
			plain_balls.push_back(PlainBall(atomic_balls[i].value));
		}
		return plain_balls;
	}
};

inline std::ostream& operator<<(std::ostream& output, const ConstructionOfAtomicBalls::AtomicBall& ball)
{
	output << ball.crad << " " << ball.value;
	return output;
}

inline std::istream& operator>>(std::istream& input, ConstructionOfAtomicBalls::AtomicBall& ball)
{
	input >> ball.crad >> ball.value;
	return input;
}

}

#endif /* COMMON_CONSTRUCTION_OF_ATOMIC_BALLS_H_ */
