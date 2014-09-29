#include <iostream>
#include <stdexcept>
#include <fstream>

#include "auxiliaries/atoms_io.h"

#include "modescommon/assert_options.h"
#include "modescommon/handle_ball.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef modescommon::BallValue BallValue;

inline auxiliaries::AtomsIO::AtomRecord convert_ball_record_to_single_atom_record(const CRAD& crad, const BallValue& value, const std::string& temperature_factor_adjunct_name)
{
	auxiliaries::AtomsIO::AtomRecord atom_record=auxiliaries::AtomsIO::AtomRecord();
	atom_record.record_name=(value.tags.count("het")>0 ? std::string("HETATM") : std::string("ATOM"));
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
		const std::map<std::string, double>::const_iterator oc_it=value.adjuncts.find("oc");
		if(oc_it!=value.adjuncts.end())
		{
			atom_record.occupancy=oc_it->second;
			atom_record.occupancy_valid=true;
		}
	}
	{
		const std::map<std::string, double>::const_iterator tf_it=value.adjuncts.find(temperature_factor_adjunct_name);
		if(tf_it!=value.adjuncts.end())
		{
			atom_record.tempFactor=tf_it->second;
			atom_record.tempFactor_valid=true;
		}
	}
	{
		for(std::set<std::string>::const_iterator tags_it=value.tags.begin();tags_it!=value.tags.end() && atom_record.element.empty();++tags_it)
		{
			if(tags_it->find("el=")!=std::string::npos)
			{
				atom_record.element=tags_it->substr(3, 1);
			}
		}
	}
	return atom_record;
}

}

void query_balls(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--match", "string", "selection"));
		list_of_option_descriptions.push_back(OD("--match-not", "string", "negative selection"));
		list_of_option_descriptions.push_back(OD("--match-tags", "string", "tags to match"));
		list_of_option_descriptions.push_back(OD("--match-tags-not", "string", "tags to not match"));
		list_of_option_descriptions.push_back(OD("--match-adjuncts", "string", "adjuncts intervals to match"));
		list_of_option_descriptions.push_back(OD("--match-adjuncts-not", "string", "adjuncts intervals to not match"));
		list_of_option_descriptions.push_back(OD("--match-external-annotations", "string", "file path to input matchable annotations"));
		list_of_option_descriptions.push_back(OD("--invert", "", "flag to invert selection"));
		list_of_option_descriptions.push_back(OD("--whole-residues", "", "flag to select whole residues"));
		list_of_option_descriptions.push_back(OD("--drop-tags", "", "flag to drop all tags from input"));
		list_of_option_descriptions.push_back(OD("--set-tags", "string", "set tags instead of filtering"));
		list_of_option_descriptions.push_back(OD("--drop-adjuncts", "", "flag to drop all adjuncts from input"));
		list_of_option_descriptions.push_back(OD("--set-adjuncts", "string", "set adjuncts instead of filtering"));
		list_of_option_descriptions.push_back(OD("--set-external-adjuncts", "string", "file path to input external adjuncts"));
		list_of_option_descriptions.push_back(OD("--set-external-adjuncts-name", "string", "name for external adjuncts"));
		list_of_option_descriptions.push_back(OD("--pdb-output", "string", "file path to output query result in PDB format"));
		list_of_option_descriptions.push_back(OD("--pdb-output-b-factor", "string", "name of adjunct to output as B-factor in PDB format"));
		list_of_option_descriptions.push_back(OD("--pdb-output-template", "string", "file path to input template for B-factor insertions"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of balls (line format: 'annotation x y z r tags adjuncts')\n";
			std::cerr << "stdout  ->  list of balls (line format: 'annotation x y z r tags adjuncts')\n";
			return;
		}
	}

	const std::string match=poh.argument<std::string>("--match", "");
	const std::string match_not=poh.argument<std::string>("--match-not", "");
	const std::string match_tags=poh.argument<std::string>("--match-tags", "");
	const std::string match_tags_not=poh.argument<std::string>("--match-tags-not", "");
	const std::string match_adjuncts=poh.argument<std::string>("--match-adjuncts", "");
	const std::string match_adjuncts_not=poh.argument<std::string>("--match-adjuncts-not", "");
	const std::string match_external_annotations=poh.argument<std::string>("--match-external-annotations", "");
	const bool invert=poh.contains_option("--invert");
	const bool whole_residues=poh.contains_option("--whole-residues");
	const bool drop_tags=poh.contains_option("--drop-tags");
	const std::string set_tags=poh.argument<std::string>("--set-tags", "");
	const bool drop_adjuncts=poh.contains_option("--drop-adjuncts");
	const std::string set_adjuncts=poh.argument<std::string>("--set-adjuncts", "");
	const std::string set_external_adjuncts=poh.argument<std::string>("--set-external-adjuncts", "");
	const std::string set_external_adjuncts_name=poh.argument<std::string>("--set-external-adjuncts-name", "ex");
	const std::string pdb_output=poh.argument<std::string>("--pdb-output", "");
	const std::string pdb_output_b_factor=poh.argument<std::string>("--pdb-output-b-factor", "tf");
	const std::string pdb_output_template=poh.argument<std::string>("--pdb-output-template", "");

	std::vector< std::pair<CRAD, BallValue> > list_of_balls;
	auxiliaries::read_lines_to_container(std::cin, modescommon::add_ball_record_from_stream_to_vector, list_of_balls);
	if(list_of_balls.empty())
	{
		throw std::runtime_error("No input.");
	}

	if(drop_tags || drop_adjuncts)
	{
		for(std::vector< std::pair<CRAD, BallValue> >::iterator it=list_of_balls.begin();it!=list_of_balls.end();++it)
		{
			if(drop_tags)
			{
				it->second.tags.clear();
			}
			if(drop_adjuncts)
			{
				it->second.adjuncts.clear();
			}
		}
	}

	std::set<CRAD> matchable_external_set_of_crads;
	if(!match_external_annotations.empty())
	{
		std::ifstream input_file(match_external_annotations.c_str(), std::ios::in);
		auxiliaries::read_lines_to_container(input_file, modescommon::add_chain_residue_atom_descriptors_from_stream_to_set, matchable_external_set_of_crads);
	}

	std::map<CRAD, double> map_of_external_adjunct_values;
	if(!set_external_adjuncts.empty())
	{
		std::ifstream input_file(set_external_adjuncts.c_str(), std::ios::in);
		auxiliaries::read_lines_to_container(input_file, modescommon::add_chain_residue_atom_descriptor_value_from_stream_to_map<false>, map_of_external_adjunct_values);
	}

	std::set<std::size_t> output_set_of_ball_ids;

	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		const CRAD& crad=list_of_balls[i].first;
		const BallValue& value=list_of_balls[i].second;
		const bool passed=(modescommon::match_chain_residue_atom_descriptor(crad, match, match_not) &&
				modescommon::match_set_of_tags(value.tags, match_tags, match_tags_not) &&
				modescommon::match_map_of_adjuncts(value.adjuncts, match_adjuncts, match_adjuncts_not) &&
				(matchable_external_set_of_crads.empty() || modescommon::match_chain_residue_atom_descriptor_with_set_of_descriptors(crad, matchable_external_set_of_crads)));
		if((passed && !invert) || (!passed && invert))
		{
			output_set_of_ball_ids.insert(i);
		}
	}

	if(whole_residues)
	{
		std::set<CRAD> residues_crads;
		for(std::set<std::size_t>::const_iterator it=output_set_of_ball_ids.begin();it!=output_set_of_ball_ids.end();++it)
		{
			residues_crads.insert(list_of_balls[*it].first.without_atom());
		}
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			if(modescommon::match_chain_residue_atom_descriptor_with_set_of_descriptors(list_of_balls[i].first, residues_crads))
			{
				output_set_of_ball_ids.insert(i);
			}
		}
	}

	if(!set_tags.empty() || !set_adjuncts.empty() || !map_of_external_adjunct_values.empty())
	{
		for(std::set<std::size_t>::const_iterator it=output_set_of_ball_ids.begin();it!=output_set_of_ball_ids.end();++it)
		{
			const CRAD& crad=list_of_balls[*it].first;
			BallValue& value=list_of_balls[*it].second;
			modescommon::update_set_of_tags(value.tags, set_tags);
			modescommon::update_map_of_adjuncts(value.adjuncts, set_adjuncts);
			if(!map_of_external_adjunct_values.empty())
			{
				std::map<CRAD, double>::const_iterator adjunct_value_it=map_of_external_adjunct_values.find(crad);
				if(adjunct_value_it==map_of_external_adjunct_values.end())
				{
					adjunct_value_it=map_of_external_adjunct_values.find(crad.without_atom());
				}
				if(adjunct_value_it!=map_of_external_adjunct_values.end())
				{
					value.adjuncts[set_external_adjuncts_name]=adjunct_value_it->second;
				}
			}
		}
		for(std::vector< std::pair<CRAD, BallValue> >::iterator it=list_of_balls.begin();it!=list_of_balls.end();++it)
		{
			modescommon::print_ball_record(it->first, it->second, std::cout);
		}
	}
	else
	{
		for(std::set<std::size_t>::const_iterator it=output_set_of_ball_ids.begin();it!=output_set_of_ball_ids.end();++it)
		{
			modescommon::print_ball_record(list_of_balls[*it].first, list_of_balls[*it].second, std::cout);
		}
	}

	if(!output_set_of_ball_ids.empty() && !pdb_output.empty())
	{
		if(pdb_output_template.empty())
		{
			std::ofstream foutput(pdb_output.c_str(), std::ios::out);
			if(foutput.good())
			{
				for(std::set<std::size_t>::const_iterator it=output_set_of_ball_ids.begin();it!=output_set_of_ball_ids.end();++it)
				{
					foutput << auxiliaries::AtomsIO::PDBWriter::write_atom_record_in_line(convert_ball_record_to_single_atom_record(list_of_balls[*it].first, list_of_balls[*it].second, pdb_output_b_factor)) << "\n";
				}
			}
		}
		else
		{
			auxiliaries::AtomsIO::PDBReader::Data pdb_file_data;
			{
				std::ifstream finput(pdb_output_template.c_str(), std::ios::in);
				pdb_file_data=auxiliaries::AtomsIO::PDBReader::read_data_from_file_stream(finput, true, true, true);
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
					for(std::set<std::size_t>::const_iterator it=output_set_of_ball_ids.begin();it!=output_set_of_ball_ids.end();++it)
					{
						output_map_of_ball_ids[list_of_balls[*it].first]=(*it);
					}
					int icount=0;
					for(std::size_t i=0;i<pdb_file_data.atom_records.size();i++)
					{
						const std::map<CRAD, std::size_t>::const_iterator ball_id_it=output_map_of_ball_ids.find(CRAD(pdb_file_data.atom_records[i], pdb_file_data.atom_records[i].chainID));
						if(ball_id_it!=output_map_of_ball_ids.end())
						{
							const std::map<std::string, double>& ball_adjuncts=list_of_balls[ball_id_it->second].second.adjuncts;
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
						foutput << pdb_file_data.all_lines[i] << "\n";
					}
				}
			}
		}
	}
}
