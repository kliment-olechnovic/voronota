#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"
#include "auxiliaries/atoms_io.h"

#include "modescommon/ball_value.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

inline auxiliaries::AtomsIO::AtomRecord convert_ball_record_to_single_atom_record(const CRAD& crad, const BallValue& value, const std::string& temperature_factor_adjunct_name)
{
	auxiliaries::AtomsIO::AtomRecord atom_record=auxiliaries::AtomsIO::AtomRecord();
	atom_record.record_name=(value.props.tags.count("het")>0 ? std::string("HETATM") : std::string("ATOM"));
	if(crad.serial!=CRAD::null_num())
	{
		atom_record.serial=crad.serial;
		atom_record.serial_valid=true;
	}
	atom_record.name=crad.name;
	atom_record.altLoc=crad.altLoc;
	atom_record.resName=crad.resName;
	atom_record.chainID=crad.chainID;
	if(crad.resSeq!=CRAD::null_num())
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

inline int decode_model_number_from_chain_id(const std::string& chainID)
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

}

void write_balls_to_atoms_file(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'annotation x y z r tags adjuncts')");
	pohw.describe_io("stdout", false, true, "list of balls (line format: 'annotation x y z r tags adjuncts')");

	const std::string pdb_output=poh.argument<std::string>(pohw.describe_option("--pdb-output", "string", "file path to output query result in PDB format"), "");
	const std::string pdb_output_b_factor=poh.argument<std::string>(pohw.describe_option("--pdb-output-b-factor", "string", "name of adjunct to output as B-factor in PDB format"), "tf");
	const std::string pdb_output_template=poh.argument<std::string>(pohw.describe_option("--pdb-output-template", "string", "file path to input template for B-factor insertions"), "");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::vector< std::pair<CRAD, BallValue> > list_of_balls;
	auxiliaries::IOUtilities().read_lines_to_map(std::cin, list_of_balls);
	if(list_of_balls.empty())
	{
		throw std::runtime_error("No input.");
	}

	if(!pdb_output.empty())
	{
		if(pdb_output_template.empty())
		{
			std::ofstream foutput(pdb_output.c_str(), std::ios::out);
			if(foutput.good())
			{
				std::map< int, std::vector<std::size_t> > models;
				for(std::size_t i=0;i<list_of_balls.size();i++)
				{
					models[decode_model_number_from_chain_id(list_of_balls[i].first.chainID)].push_back(i);
				}
				for(std::map< int, std::vector<std::size_t> >::const_iterator models_it=models.begin();models_it!=models.end();++models_it)
				{
					if(models.size()>1)
					{
						std::ostringstream line_output;
						line_output << "MODEL" << std::right << std::setw(9) << models_it->first;
						foutput << line_output.str() << "\n";
					}
					for(std::size_t j=0;j<models_it->second.size();j++)
					{
						const std::size_t i=models_it->second[j];
						foutput << auxiliaries::AtomsIO::PDBWriter::write_atom_record_in_line(convert_ball_record_to_single_atom_record(list_of_balls[i].first, list_of_balls[i].second, pdb_output_b_factor)) << "\n";
					}
					if(models.size()>1)
					{
						foutput << "ENDMDL\n";
					}
				}
			}
		}
		else
		{
			auxiliaries::AtomsIO::PDBReader::Data pdb_file_data;
			{
				std::ifstream finput(pdb_output_template.c_str(), std::ios::in);
				pdb_file_data=auxiliaries::AtomsIO::PDBReader::read_data_from_file_stream(finput, true, true, true, true);
			}
			if(!pdb_file_data.valid())
			{
				throw std::runtime_error("Invalid PDB file output template.");
			}
			else
			{
				std::ofstream foutput(pdb_output.c_str(), std::ios::out);
				if(foutput.good())
				{
					std::map<CRAD, std::size_t> output_map_of_ball_ids;
					for(std::size_t i=0;i<list_of_balls.size();i++)
					{
						output_map_of_ball_ids[list_of_balls[i].first]=i;
					}
					int icount=0;
					for(std::size_t i=0;i<pdb_file_data.atom_records.size();i++)
					{
						const auxiliaries::AtomsIO::AtomRecord& atom_record=pdb_file_data.atom_records[i];
						const std::map<CRAD, std::size_t>::const_iterator ball_id_it=output_map_of_ball_ids.find(CRAD(atom_record.serial, atom_record.chainID, atom_record.resSeq, atom_record.resName, atom_record.name, atom_record.altLoc, atom_record.iCode));
						if(ball_id_it!=output_map_of_ball_ids.end())
						{
							const std::map<std::string, double>& ball_adjuncts=list_of_balls[ball_id_it->second].second.props.adjuncts;
							const std::map<std::string, double>::const_iterator temperature_factor_it=ball_adjuncts.find(pdb_output_b_factor);
							std::string& ball_line=pdb_file_data.all_lines[pdb_file_data.map_of_atom_records_to_all_lines.at(i)];
							ball_line=(temperature_factor_it!=ball_adjuncts.end()) ?
									auxiliaries::AtomsIO::PDBWriter::write_temperature_factor_to_line(ball_line, true, temperature_factor_it->second) :
									auxiliaries::AtomsIO::PDBWriter::write_temperature_factor_to_line(ball_line, false, 0);
							icount++;
						}
					}
					for(std::size_t i=0;i<pdb_file_data.all_lines.size();i++)
					{
						if((i+1<pdb_file_data.all_lines.size()) || !pdb_file_data.all_lines[i].empty())
						{
							foutput << pdb_file_data.all_lines[i] << "\n";
						}
					}
				}
			}
		}
	}

	auxiliaries::IOUtilities().write_map(list_of_balls, std::cout);
}
