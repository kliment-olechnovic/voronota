#ifndef COMMON_CONSTRUCTION_OF_ATOMIC_BALLS_H_
#define COMMON_CONSTRUCTION_OF_ATOMIC_BALLS_H_

#include <vector>

#include "../auxiliaries/atoms_io.h"
#include "../auxiliaries/atom_radius_assigner.h"
#include "../auxiliaries/io_utilities.h"

#include "chain_residue_atom_descriptor.h"
#include "ball_value.h"

namespace voronota
{

namespace common
{

class ConstructionOfAtomicBalls
{
public:
	struct AtomicBall
	{
		ChainResidueAtomDescriptor crad;
		BallValue value;

		AtomicBall()
		{
		}

		AtomicBall(const ChainResidueAtomDescriptor& crad, const BallValue& value) : crad(crad), value(value)
		{
		}
	};

	struct ParametersToCollectAtomicBallsFromFile
	{
		bool mmcif;
		bool include_heteroatoms;
		bool include_hydrogens;
		bool multimodel_chains;

		ParametersToCollectAtomicBallsFromFile() :
			mmcif(false),
			include_heteroatoms(false),
			include_hydrogens(false),
			multimodel_chains(false)
		{
		}
	};

	static bool collect_atomic_balls_from_file(const auxiliaries::AtomRadiusAssigner& atom_radius_assigner, const ParametersToCollectAtomicBallsFromFile& parameters, std::istream& input_stream, std::vector<AtomicBall>& atomic_balls)
	{
		atomic_balls.clear();

		const std::vector<auxiliaries::AtomsIO::AtomRecord> atoms=(parameters.mmcif ?
				auxiliaries::AtomsIO::MMCIFReader::read_data_from_file_stream(input_stream, parameters.include_heteroatoms, parameters.include_hydrogens, parameters.multimodel_chains).atom_records :
				auxiliaries::AtomsIO::PDBReader::read_data_from_file_stream(input_stream, parameters.include_heteroatoms, parameters.include_hydrogens, parameters.multimodel_chains, false).atom_records);

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

	static bool collect_atomic_balls_from_file(const auxiliaries::AtomRadiusAssigner& atom_radius_assigner, const ParametersToCollectAtomicBallsFromFile& parameters, const std::string& input_file, std::vector<AtomicBall>& atomic_balls)
	{
		std::ifstream input(input_file.c_str(), std::ios::in);
		return collect_atomic_balls_from_file(atom_radius_assigner, parameters, input, atomic_balls);
	}

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

}

#endif /* COMMON_CONSTRUCTION_OF_ATOMIC_BALLS_H_ */
