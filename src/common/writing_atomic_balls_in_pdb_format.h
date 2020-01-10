#ifndef COMMON_WRITING_ATOMIC_BALLS_IN_PDB_FORMAT_H_
#define COMMON_WRITING_ATOMIC_BALLS_IN_PDB_FORMAT_H_

#include "../auxiliaries/atoms_io.h"

#include "construction_of_atomic_balls.h"

namespace voronota
{

namespace common
{

class WritingAtomicBallsInPDBFormat
{
public:
	static void write_atomic_balls(
			const std::vector<ConstructionOfAtomicBalls::AtomicBall>& atomic_balls,
			const std::string& b_factor_name,
			const bool add_chain_terminators,
			std::ostream& output)
	{
		std::map< int, std::vector<std::size_t> > models;
		for(std::size_t i=0;i<atomic_balls.size();i++)
		{
			models[WritingAtomicBallsInPDBFormat::decode_model_number_from_chain_id(atomic_balls[i].crad.chainID)].push_back(i);
		}
		for(std::map< int, std::vector<std::size_t> >::const_iterator models_it=models.begin();models_it!=models.end();++models_it)
		{
			if(models.size()>1)
			{
				std::ostringstream line_output;
				line_output << "MODEL" << std::right << std::setw(9) << models_it->first;
				output << line_output.str() << "\n";
			}
			for(std::size_t j=0;j<models_it->second.size();j++)
			{
				const std::size_t i=models_it->second[j];
				output << auxiliaries::AtomsIO::PDBWriter::write_atom_record_in_line(
						WritingAtomicBallsInPDBFormat::convert_ball_record_to_single_atom_record(
								atomic_balls[i].crad, atomic_balls[i].value, b_factor_name)) << "\n";
				if(add_chain_terminators &&
						((j+1)>=models_it->second.size() ||
								atomic_balls[i].crad.chainID!=atomic_balls[models_it->second[j+1]].crad.chainID))
				{
					output << "TER\n";
				}
			}
			if(models.size()>1)
			{
				output << "ENDMDL\n";
			}
		}
	}

private:
	static auxiliaries::AtomsIO::AtomRecord convert_ball_record_to_single_atom_record(
			const ChainResidueAtomDescriptor& crad,
			const BallValue& value,
			const std::string& temperature_factor_adjunct_name)
	{
		auxiliaries::AtomsIO::AtomRecord atom_record=auxiliaries::AtomsIO::AtomRecord();
		atom_record.record_name=(value.props.tags.count("het")>0 ? std::string("HETATM") : std::string("ATOM"));
		if(crad.serial!=ChainResidueAtomDescriptor::null_num())
		{
			atom_record.serial=crad.serial;
			atom_record.serial_valid=true;
		}
		atom_record.name=crad.name;
		atom_record.altLoc=crad.altLoc;
		atom_record.resName=crad.resName;
		atom_record.chainID=crad.chainID;
		if(crad.resSeq!=ChainResidueAtomDescriptor::null_num())
		{
			atom_record.resSeq=crad.resSeq;
			atom_record.resSeq_valid=true;
		}
		atom_record.iCode=crad.iCode;
		atom_record.x=value.x;
		atom_record.x_valid=true;
		atom_record.y=value.y;
		atom_record.y_valid=true;
		atom_record.z=value.z;
		atom_record.z_valid=true;
		{
			const std::map<std::string, double>::const_iterator oc_it=value.props.adjuncts.find("oc");
			if(oc_it!=value.props.adjuncts.end())
			{
				atom_record.occupancy=oc_it->second;
				atom_record.occupancy_valid=true;
			}
			else
			{
				atom_record.occupancy=1.0;
				atom_record.occupancy_valid=true;
			}
		}
		{
			const std::map<std::string, double>::const_iterator tf_it=value.props.adjuncts.find(temperature_factor_adjunct_name);
			if(tf_it!=value.props.adjuncts.end())
			{
				atom_record.tempFactor=tf_it->second;
				atom_record.tempFactor_valid=true;
			}
		}
		{
			for(std::set<std::string>::const_iterator tags_it=value.props.tags.begin();tags_it!=value.props.tags.end() && atom_record.element.empty();++tags_it)
			{
				if(tags_it->find("el=")!=std::string::npos)
				{
					atom_record.element=tags_it->substr(3, 2);
				}
			}
		}
		return atom_record;
	}

	static int decode_model_number_from_chain_id(const std::string& chainID)
	{
		int model_number=1;
		if(chainID.size()>1)
		{
			std::istringstream input(chainID.substr(1));
			int value=0;
			input >> value;
			if(!input.fail() && value>0)
			{
				model_number=value;
			}
		}
		return model_number;
	}
};

}

}

#endif /* COMMON_WRITING_ATOMIC_BALLS_IN_PDB_FORMAT_H_ */
