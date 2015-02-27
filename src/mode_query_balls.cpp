#include <iostream>
#include <stdexcept>
#include <fstream>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/atoms_io.h"

#include "modescommon/ball_value.h"
#include "modescommon/sequence_utilities.h"
#include "modescommon/matching_utilities.h"

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
				atom_record.element=tags_it->substr(3, 1);
			}
		}
	}
	return atom_record;
}

std::map<CRAD, std::string> init_map_of_dssp_records(const std::string& dssp_file_name)
{
	std::map<CRAD, std::string> map_of_dssp_records;
	if(!dssp_file_name.empty())
	{
		std::ifstream input_file(dssp_file_name.c_str(), std::ios::in);
		const auxiliaries::AtomsIO::DSSPReader::Data dssp_file_data=auxiliaries::AtomsIO::DSSPReader::read_data_from_file_stream(input_file);
		for(std::size_t i=0;i<dssp_file_data.dssp_records.size();i++)
		{
			const auxiliaries::AtomsIO::DSSPReader::DSSPRecord& record=dssp_file_data.dssp_records[i];
			CRAD crad;
			crad.chainID=record.chainID;
			crad.resSeq=record.resSeq;
			crad.iCode=record.iCode;
			map_of_dssp_records.insert(std::make_pair(crad, record.resSSE));
		}
	}
	return map_of_dssp_records;
}

}

void query_balls(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--match", "string", "selection"));
		ods.push_back(OD("--match-not", "string", "negative selection"));
		ods.push_back(OD("--match-tags", "string", "tags to match"));
		ods.push_back(OD("--match-tags-not", "string", "tags to not match"));
		ods.push_back(OD("--match-adjuncts", "string", "adjuncts intervals to match"));
		ods.push_back(OD("--match-adjuncts-not", "string", "adjuncts intervals to not match"));
		ods.push_back(OD("--match-external-annotations", "string", "file path to input matchable annotations"));
		ods.push_back(OD("--invert", "", "flag to invert selection"));
		ods.push_back(OD("--whole-residues", "", "flag to select whole residues"));
		ods.push_back(OD("--drop-atom-serials", "", "flag to drop atom serial numbers from input"));
		ods.push_back(OD("--drop-altloc-indicators", "", "flag to drop alternate location indicators from input"));
		ods.push_back(OD("--drop-tags", "", "flag to drop all tags from input"));
		ods.push_back(OD("--drop-adjuncts", "", "flag to drop all adjuncts from input"));
		ods.push_back(OD("--set-tags", "string", "set tags instead of filtering"));
		ods.push_back(OD("--set-dssp-tags", "string", "file path to input DSSP file"));
		ods.push_back(OD("--set-adjuncts", "string", "set adjuncts instead of filtering"));
		ods.push_back(OD("--set-external-adjuncts", "string", "file path to input external adjuncts"));
		ods.push_back(OD("--set-external-adjuncts-name", "string", "name for external adjuncts"));
		ods.push_back(OD("--rename-chains", "", "flag to rename input chains to be in interval from 'A' to 'Z'"));
		ods.push_back(OD("--renumber-from-adjunct", "string", "adjunct name to use for input residue renumbering"));
		ods.push_back(OD("--set-ref-seq-num-adjunct", "string", "file path to input reference sequence"));
		ods.push_back(OD("--ref-seq-alignment", "string", "file path to output alignment with reference"));
		ods.push_back(OD("--seq-output", "string", "file path to output query result sequence string"));
		ods.push_back(OD("--chains-summary-output", "string", "file path to output chains summary"));
		ods.push_back(OD("--chains-seq-identity", "number", "sequence identity threshold for chains summary"));
		ods.push_back(OD("--pdb-output", "string", "file path to output query result in PDB format"));
		ods.push_back(OD("--pdb-output-b-factor", "string", "name of adjunct to output as B-factor in PDB format"));
		ods.push_back(OD("--pdb-output-template", "string", "file path to input template for B-factor insertions"));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false, "list of balls (line format: 'annotation x y z r tags adjuncts')");
			poh.print_io_description("stdout", false, true, "list of balls (line format: 'annotation x y z r tags adjuncts')");
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
	const bool drop_atom_serial=poh.contains_option("--drop-atom-serials");
	const bool drop_altloc_indicators=poh.contains_option("--drop-altloc-indicators");
	const bool drop_tags=poh.contains_option("--drop-tags");
	const bool drop_adjuncts=poh.contains_option("--drop-adjuncts");
	const std::string set_tags=poh.argument<std::string>("--set-tags", "");
	const std::string set_dssp_tags=poh.argument<std::string>("--set-dssp-tags", "");
	const std::string set_adjuncts=poh.argument<std::string>("--set-adjuncts", "");
	const std::string set_external_adjuncts=poh.argument<std::string>("--set-external-adjuncts", "");
	const std::string set_external_adjuncts_name=poh.argument<std::string>("--set-external-adjuncts-name", "ex");
	const bool rename_chains=poh.contains_option("--rename-chains");
	const std::string renumber_from_adjunct=poh.argument<std::string>("--renumber-from-adjunct", "");
	const std::string set_ref_seq_num_adjunct=poh.argument<std::string>("--set-ref-seq-num-adjunct", "");
	const std::string ref_seq_alignment=poh.argument<std::string>("--ref-seq-alignment", "");
	const std::string seq_output=poh.argument<std::string>("--seq-output", "");
	const std::string chains_summary_output=poh.argument<std::string>("--chains-summary-output", "");
	const double chains_seq_identity=poh.argument<double>("--chains-seq-identity", 0.9);
	const std::string pdb_output=poh.argument<std::string>("--pdb-output", "");
	const std::string pdb_output_b_factor=poh.argument<std::string>("--pdb-output-b-factor", "tf");
	const std::string pdb_output_template=poh.argument<std::string>("--pdb-output-template", "");

	std::vector< std::pair<CRAD, BallValue> > list_of_balls;
	auxiliaries::IOUtilities().read_lines_to_map(std::cin, list_of_balls);
	if(list_of_balls.empty())
	{
		throw std::runtime_error("No input.");
	}

	if(rename_chains)
	{
		std::set<std::string> current_chain_names_set;
		std::vector<std::string> current_chain_names;
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const std::string& chainID=list_of_balls[i].first.chainID;
			if(current_chain_names_set.count(chainID)==0)
			{
				current_chain_names_set.insert(chainID);
				current_chain_names.push_back(chainID);
			}
		}
		if(current_chain_names.size()>(static_cast<std::size_t>('Z'-'A')+1))
		{
			throw std::runtime_error("Too many chains to rename to be in interval from 'A' to 'Z'.");
		}
		else
		{
			std::map<std::string, std::string> renaming_map;
			for(std::vector<std::string>::const_iterator it=current_chain_names.begin();it!=current_chain_names.end();++it)
			{
				char index_symbol='A'+static_cast<char>(renaming_map.size());
				renaming_map[*it]=std::string(1, index_symbol);
			}
			for(std::size_t i=0;i<list_of_balls.size();i++)
			{
				list_of_balls[i].first.chainID=renaming_map[list_of_balls[i].first.chainID];
			}
		}
	}

	if(!renumber_from_adjunct.empty())
	{
		std::vector< std::pair<CRAD, BallValue> > refined_list_of_balls;
		refined_list_of_balls.reserve(list_of_balls.size());
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const BallValue& value=list_of_balls[i].second;
			if(value.props.adjuncts.count(renumber_from_adjunct)>0)
			{
				refined_list_of_balls.push_back(list_of_balls[i]);
				refined_list_of_balls.back().first.resSeq=static_cast<int>(value.props.adjuncts.find(renumber_from_adjunct)->second);
				refined_list_of_balls.back().second.props.adjuncts.erase(renumber_from_adjunct);
			}
		}
		list_of_balls=refined_list_of_balls;
	}

	std::set<std::size_t> selected_set_of_ball_ids;
	{
		const std::set<CRAD> matchable_external_set_of_crads=auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRAD> >(match_external_annotations);

		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const CRAD& crad=list_of_balls[i].first;
			const BallValue& value=list_of_balls[i].second;
			const bool passed=(MatchingUtilities::match_crad(crad, match, match_not) &&
					MatchingUtilities::match_set_of_tags(value.props.tags, match_tags, match_tags_not) &&
					MatchingUtilities::match_map_of_adjuncts(value.props.adjuncts, match_adjuncts, match_adjuncts_not) &&
					(match_external_annotations.empty() || MatchingUtilities::match_crad_with_set_of_crads(crad, matchable_external_set_of_crads)));
			if((passed && !invert) || (!passed && invert))
			{
				selected_set_of_ball_ids.insert(selected_set_of_ball_ids.end(), i);
			}
		}

		if(whole_residues)
		{
			std::set<CRAD> residues_crads;
			for(std::set<std::size_t>::const_iterator it=selected_set_of_ball_ids.begin();it!=selected_set_of_ball_ids.end();++it)
			{
				residues_crads.insert(list_of_balls[*it].first.without_atom());
			}
			for(std::size_t i=0;i<list_of_balls.size();i++)
			{
				if(MatchingUtilities::match_crad_with_set_of_crads(list_of_balls[i].first, residues_crads))
				{
					selected_set_of_ball_ids.insert(i);
				}
			}
		}
	}

	std::vector<CRAD> residue_sequence_vector;
	{
		std::set<CRAD> residues_crads;
		for(std::set<std::size_t>::const_iterator it=selected_set_of_ball_ids.begin();it!=selected_set_of_ball_ids.end();++it)
		{
			const CRAD residue_crad=list_of_balls[*it].first.without_atom();
			if(residues_crads.count(residue_crad)==0)
			{
				residue_sequence_vector.push_back(residue_crad);
				residues_crads.insert(residue_crad);
			}
		}
	}

	const bool update_mode=(drop_atom_serial || drop_altloc_indicators || drop_tags || drop_adjuncts || !set_tags.empty() || !set_adjuncts.empty() || !set_external_adjuncts.empty() || !set_dssp_tags.empty() || !set_ref_seq_num_adjunct.empty());
	if(update_mode && !selected_set_of_ball_ids.empty())
	{
		const std::map<CRAD, double> map_of_external_adjunct_values=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRAD, double> >(set_external_adjuncts);
		const std::map<CRAD, std::string> map_of_dssp_records=init_map_of_dssp_records(set_dssp_tags);
		const std::string reference_sequence=SequenceUtilities::read_sequence_from_file(set_ref_seq_num_adjunct);
		const std::map<CRAD, double> sequence_mapping=SequenceUtilities::construct_sequence_mapping(residue_sequence_vector, reference_sequence, ref_seq_alignment);

		for(std::set<std::size_t>::const_iterator it=selected_set_of_ball_ids.begin();it!=selected_set_of_ball_ids.end();++it)
		{
			CRAD& crad=list_of_balls[*it].first;
			BallValue& value=list_of_balls[*it].second;
			if(drop_atom_serial)
			{
				crad.serial=CRAD::null_num();
			}
			if(drop_altloc_indicators)
			{
				crad.altLoc.clear();
			}
			if(drop_tags)
			{
				value.props.tags.clear();
			}
			if(drop_adjuncts)
			{
				value.props.adjuncts.clear();
			}
			if(!set_tags.empty())
			{
				value.props.update_tags(set_tags);
			}
			if(!set_adjuncts.empty())
			{
				value.props.update_adjuncts(set_adjuncts);
			}
			if(!map_of_external_adjunct_values.empty())
			{
				std::map<CRAD, double>::const_iterator adjunct_value_it=map_of_external_adjunct_values.find(crad);
				if(adjunct_value_it==map_of_external_adjunct_values.end())
				{
					adjunct_value_it=map_of_external_adjunct_values.find(crad.without_atom());
				}
				if(adjunct_value_it!=map_of_external_adjunct_values.end())
				{
					value.props.adjuncts[set_external_adjuncts_name]=adjunct_value_it->second;
				}
			}
			if(!map_of_dssp_records.empty())
			{
				CRAD simplified_crad;
				simplified_crad.chainID=crad.chainID;
				simplified_crad.resSeq=crad.resSeq;
				simplified_crad.iCode=crad.iCode;
				const std::map<CRAD, std::string>::const_iterator map_of_dssp_records_it=map_of_dssp_records.find(simplified_crad);
				if(map_of_dssp_records_it!=map_of_dssp_records.end() && !map_of_dssp_records_it->second.empty())
				{
					value.props.tags.insert(std::string("dssp=")+map_of_dssp_records_it->second);
				}
			}
			if(!sequence_mapping.empty())
			{
				const std::map<CRAD, double>::const_iterator sm_it=sequence_mapping.find(crad.without_atom());
				if(sm_it!=sequence_mapping.end())
				{
					value.props.adjuncts["refseq"]=sm_it->second;
				}
			}
		}
	}

	if(update_mode)
	{
		auxiliaries::IOUtilities().write_map(list_of_balls, std::cout);
	}
	else
	{
		for(std::set<std::size_t>::const_iterator it=selected_set_of_ball_ids.begin();it!=selected_set_of_ball_ids.end();++it)
		{
			std::cout << list_of_balls[*it].first << " " << list_of_balls[*it].second << "\n";
		}
	}

	if(!residue_sequence_vector.empty() && !seq_output.empty())
	{
		std::ofstream foutput(seq_output.c_str(), std::ios::out);
		if(foutput.good())
		{
			foutput << SequenceUtilities::convert_residue_sequence_container_to_string(residue_sequence_vector) << "\n";
		}
	}

	if(!residue_sequence_vector.empty() && !chains_summary_output.empty())
	{
		std::ofstream foutput(chains_summary_output.c_str(), std::ios::out);
		if(foutput.good())
		{
			std::map< std::string, std::vector<CRAD> > map_of_chains;
			for(std::size_t i=0;i<residue_sequence_vector.size();i++)
			{
				map_of_chains[residue_sequence_vector[i].chainID].push_back(residue_sequence_vector[i]);
			}
			std::map<std::string, std::string> map_of_chains_sequences;
			for(std::map< std::string, std::vector<CRAD> >::const_iterator it=map_of_chains.begin();it!=map_of_chains.end();++it)
			{
				map_of_chains_sequences[it->first]=SequenceUtilities::convert_residue_sequence_container_to_string(it->second);
			}
			std::set<std::string> repeated_chains;
			for(std::map<std::string, std::string>::const_iterator it1=map_of_chains_sequences.begin();it1!=map_of_chains_sequences.end();++it1)
			{
				if(repeated_chains.count(it1->first)==0)
				{
					std::map<std::string, std::string>::const_iterator it2=it1;
					++it2;
					for(;it2!=map_of_chains_sequences.end();++it2)
					{
						if(SequenceUtilities::calculate_sequence_identity(it1->second, it2->second)>=chains_seq_identity)
						{
							repeated_chains.insert(it2->first);
						}
					}
				}
			}
			foutput << map_of_chains_sequences.size() << " " << (map_of_chains_sequences.size()-repeated_chains.size()) << "\n";
		}
	}

	if(!selected_set_of_ball_ids.empty() && !pdb_output.empty())
	{
		if(pdb_output_template.empty())
		{
			std::ofstream foutput(pdb_output.c_str(), std::ios::out);
			if(foutput.good())
			{
				for(std::set<std::size_t>::const_iterator it=selected_set_of_ball_ids.begin();it!=selected_set_of_ball_ids.end();++it)
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
					for(std::set<std::size_t>::const_iterator it=selected_set_of_ball_ids.begin();it!=selected_set_of_ball_ids.end();++it)
					{
						output_map_of_ball_ids[list_of_balls[*it].first]=(*it);
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
						foutput << pdb_file_data.all_lines[i] << "\n";
					}
				}
			}
		}
	}
}
