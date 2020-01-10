#include <iostream>
#include <stdexcept>
#include <fstream>

#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/atoms_io.h"

#include "../common/ball_value.h"
#include "../common/matching_utilities.h"

#include "modescommon/sequence_utilities.h"

namespace
{

typedef voronota::common::ChainResidueAtomDescriptor CRAD;
typedef voronota::auxiliaries::AtomsIO::DSSPReader::DSSPRecord DSSPRecord;

std::map<CRAD, DSSPRecord> init_map_of_dssp_records(const std::string& dssp_file_name)
{
	std::map<CRAD, DSSPRecord> map_of_dssp_records;
	if(!dssp_file_name.empty())
	{
		std::ifstream input_file(dssp_file_name.c_str(), std::ios::in);
		const voronota::auxiliaries::AtomsIO::DSSPReader::Data dssp_file_data=voronota::auxiliaries::AtomsIO::DSSPReader::read_data_from_file_stream(input_file);
		for(std::size_t i=0;i<dssp_file_data.dssp_records.size();i++)
		{
			const DSSPRecord& record=dssp_file_data.dssp_records[i];
			CRAD crad;
			crad.chainID=record.chainID;
			crad.resSeq=record.resSeq;
			crad.iCode=record.iCode;
			map_of_dssp_records.insert(std::make_pair(crad, record));
		}
	}
	return map_of_dssp_records;
}

}

void query_balls(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'annotation x y z r tags adjuncts')");
	pohw.describe_io("stdout", false, true, "list of balls (line format: 'annotation x y z r tags adjuncts')");

	const std::string match=poh.argument<std::string>(pohw.describe_option("--match", "string", "selection"), "");
	const std::string match_not=poh.argument<std::string>(pohw.describe_option("--match-not", "string", "negative selection"), "");
	const std::string match_tags=poh.argument<std::string>(pohw.describe_option("--match-tags", "string", "tags to match"), "");
	const std::string match_tags_not=poh.argument<std::string>(pohw.describe_option("--match-tags-not", "string", "tags to not match"), "");
	const std::string match_adjuncts=poh.argument<std::string>(pohw.describe_option("--match-adjuncts", "string", "adjuncts intervals to match"), "");
	const std::string match_adjuncts_not=poh.argument<std::string>(pohw.describe_option("--match-adjuncts-not", "string", "adjuncts intervals to not match"), "");
	const std::string match_external_annotations=poh.argument<std::string>(pohw.describe_option("--match-external-annotations", "string", "file path to input matchable annotations"), "");
	const bool invert=poh.contains_option(pohw.describe_option("--invert", "", "flag to invert selection"));
	const bool whole_residues=poh.contains_option(pohw.describe_option("--whole-residues", "", "flag to select whole residues"));
	const bool drop_atom_serial=poh.contains_option(pohw.describe_option("--drop-atom-serials", "", "flag to drop atom serial numbers from input"));
	const bool drop_altloc_indicators=poh.contains_option(pohw.describe_option("--drop-altloc-indicators", "", "flag to drop alternate location indicators from input"));
	const bool drop_tags=poh.contains_option(pohw.describe_option("--drop-tags", "", "flag to drop all tags from input"));
	const bool drop_adjuncts=poh.contains_option(pohw.describe_option("--drop-adjuncts", "", "flag to drop all adjuncts from input"));
	const std::string set_tags=poh.argument<std::string>(pohw.describe_option("--set-tags", "string", "set tags instead of filtering"), "");
	const std::string set_dssp_info=poh.argument<std::string>(pohw.describe_option("--set-dssp-info", "string", "file path to input DSSP file"), "");
	const std::string set_adjuncts=poh.argument<std::string>(pohw.describe_option("--set-adjuncts", "string", "set adjuncts instead of filtering"), "");
	const std::string set_external_adjuncts=poh.argument<std::string>(pohw.describe_option("--set-external-adjuncts", "string", "file path to input external adjuncts"), "");
	const std::string set_external_adjuncts_name=poh.argument<std::string>(pohw.describe_option("--set-external-adjuncts-name", "string", "name for external adjuncts"), "ex");
	const bool rename_chains=poh.contains_option(pohw.describe_option("--rename-chains", "", "flag to rename input chains to be in interval from 'A' to 'Z'"));
	const bool guess_chain_names=poh.contains_option(pohw.describe_option("--guess-chain-names", "", "flag to assign input chain names based on residue numbering"));
	const std::string renumber_from_adjunct=poh.argument<std::string>(pohw.describe_option("--renumber-from-adjunct", "string", "adjunct name to use for input residue renumbering"), "");
	const bool renumber_positively=poh.contains_option(pohw.describe_option("--renumber-positively", "", "flag to increment residue numbers to make them positive"));
	const bool reset_serials=poh.contains_option(pohw.describe_option("--reset-serials", "", "flag to reset atom serial numbers"));
	const bool set_seq_pos_adjunct=poh.contains_option(pohw.describe_option("--set-seq-pos-adjunct", "", "flag to set normalized sequence position adjunct"));
	const std::string set_ref_seq_num_adjunct=poh.argument<std::string>(pohw.describe_option("--set-ref-seq-num-adjunct", "string", "file path to input reference sequence"), "");
	const std::string ref_seq_alignment=poh.argument<std::string>(pohw.describe_option("--ref-seq-alignment", "string", "file path to output alignment with reference"), "");
	const std::string seq_output=poh.argument<std::string>(pohw.describe_option("--seq-output", "string", "file path to output query result sequence string"), "");
	const std::string chains_summary_output=poh.argument<std::string>(pohw.describe_option("--chains-summary-output", "string", "file path to output chains summary"), "");
	const double chains_seq_identity=poh.argument<double>(pohw.describe_option("--chains-seq-identity", "number", "sequence identity threshold for chains summary, default is 0.9"), 0.9);

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::vector< std::pair<CRAD, voronota::common::BallValue> > list_of_balls;
	voronota::auxiliaries::IOUtilities().read_lines_to_map(std::cin, list_of_balls);
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
		if(!current_chain_names.empty() && current_chain_names.size()>(static_cast<std::size_t>('Z'-'A')+1))
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

	if(guess_chain_names)
	{
		char current_chain_name='A';
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			if(i>0 && list_of_balls[i].first.resSeq<list_of_balls[i-1].first.resSeq)
			{
				if(current_chain_name=='Z')
				{
					current_chain_name='a';
				}
				else if(current_chain_name=='z')
				{
					throw std::runtime_error("Too many chains to guess their names.");
				}
				else
				{
					current_chain_name++;
				}
			}
			list_of_balls[i].first.chainID=std::string(1, current_chain_name);
		}
	}

	if(!renumber_from_adjunct.empty())
	{
		std::vector< std::pair<CRAD, voronota::common::BallValue> > refined_list_of_balls;
		refined_list_of_balls.reserve(list_of_balls.size());
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const voronota::common::BallValue& value=list_of_balls[i].second;
			if(value.props.adjuncts.count(renumber_from_adjunct)>0)
			{
				refined_list_of_balls.push_back(list_of_balls[i]);
				refined_list_of_balls.back().first.resSeq=static_cast<int>(value.props.adjuncts.find(renumber_from_adjunct)->second);
				refined_list_of_balls.back().second.props.adjuncts.erase(renumber_from_adjunct);
			}
		}
		list_of_balls=refined_list_of_balls;
	}

	if(renumber_positively)
	{
		std::map<std::string, int> min_resSeq;
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const CRAD& crad=list_of_balls[i].first;
			if(crad.resSeq!=CRAD::null_num() && (min_resSeq.count(crad.chainID)==0 || crad.resSeq<min_resSeq[crad.chainID]))
			{
				min_resSeq[crad.chainID]=crad.resSeq;
			}
		}
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			CRAD& crad=list_of_balls[i].first;
			if(crad.resSeq!=CRAD::null_num() && min_resSeq.count(crad.chainID)>0 && min_resSeq[crad.chainID]<=0)
			{
				crad.resSeq=(crad.resSeq-min_resSeq[crad.chainID]+1);
			}
		}
	}

	if(reset_serials)
	{
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			list_of_balls[i].first.serial=static_cast<int>(i+1);
		}
	}

	if(set_seq_pos_adjunct)
	{
		typedef std::map<std::string, std::pair<double, double> > MapOfRanges;
		MapOfRanges chain_seq_num_ranges;
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const CRAD& crad=list_of_balls[i].first;
			MapOfRanges::iterator chain_seq_num_ranges_it=chain_seq_num_ranges.find(crad.chainID);
			if(chain_seq_num_ranges_it==chain_seq_num_ranges.end())
			{
				std::pair<double, double>& range=chain_seq_num_ranges[crad.chainID];
				range.first=crad.resSeq;
				range.second=crad.resSeq;
			}
			else
			{
				std::pair<double, double>& range=chain_seq_num_ranges_it->second;
				range.first=std::min(range.first, static_cast<double>(crad.resSeq));
				range.second=std::max(range.second, static_cast<double>(crad.resSeq));
			}
		}
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const CRAD& crad=list_of_balls[i].first;
			const std::pair<double, double>& range=chain_seq_num_ranges[crad.chainID];
			list_of_balls[i].second.props.adjuncts["seqpos"]=((range.first<range.second) ? ((crad.resSeq-range.first)/(range.second-range.first)) : 0.0);
		}
	}

	std::set<std::size_t> selected_set_of_ball_ids;
	{
		const std::set<CRAD> matchable_external_set_of_crads=voronota::auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRAD> >(match_external_annotations);

		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const CRAD& crad=list_of_balls[i].first;
			const voronota::common::BallValue& value=list_of_balls[i].second;
			const bool passed=(voronota::common::MatchingUtilities::match_crad(crad, match, match_not) &&
					voronota::common::MatchingUtilities::match_set_of_tags(value.props.tags, match_tags, match_tags_not) &&
					voronota::common::MatchingUtilities::match_map_of_adjuncts(value.props.adjuncts, match_adjuncts, match_adjuncts_not) &&
					(match_external_annotations.empty() || voronota::common::MatchingUtilities::match_crad_with_set_of_crads(false, crad, matchable_external_set_of_crads)));
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
				if(voronota::common::MatchingUtilities::match_crad_with_set_of_crads(false, list_of_balls[i].first, residues_crads))
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

	const bool update_mode=(drop_atom_serial || drop_altloc_indicators || drop_tags || drop_adjuncts || !set_tags.empty() || !set_adjuncts.empty() || !set_external_adjuncts.empty() || !set_dssp_info.empty() || !set_ref_seq_num_adjunct.empty());
	if(update_mode && !selected_set_of_ball_ids.empty())
	{
		const std::map<CRAD, double> map_of_external_adjunct_values=voronota::auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRAD, double> >(set_external_adjuncts);
		const std::map<CRAD, DSSPRecord> map_of_dssp_records=init_map_of_dssp_records(set_dssp_info);
		const std::string reference_sequence=voronota::modescommon::SequenceUtilities::read_sequence_from_file(set_ref_seq_num_adjunct);
		const std::map<CRAD, int> sequence_mapping=voronota::modescommon::SequenceUtilities::construct_sequence_mapping(residue_sequence_vector, reference_sequence, ref_seq_alignment);

		for(std::set<std::size_t>::const_iterator it=selected_set_of_ball_ids.begin();it!=selected_set_of_ball_ids.end();++it)
		{
			CRAD& crad=list_of_balls[*it].first;
			voronota::common::BallValue& value=list_of_balls[*it].second;
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
				const std::pair<bool, double> adjunct_value=voronota::common::MatchingUtilities::match_crad_with_map_of_crads(false, crad, map_of_external_adjunct_values);
				if(adjunct_value.first)
				{
					value.props.adjuncts[set_external_adjuncts_name]=adjunct_value.second;
				}
			}
			if(!map_of_dssp_records.empty())
			{
				CRAD simplified_crad;
				simplified_crad.chainID=crad.chainID;
				simplified_crad.resSeq=crad.resSeq;
				simplified_crad.iCode=crad.iCode;
				const std::map<CRAD, DSSPRecord>::const_iterator map_of_dssp_records_it=map_of_dssp_records.find(simplified_crad);
				if(map_of_dssp_records_it!=map_of_dssp_records.end())
				{
					const DSSPRecord& dssp_record=map_of_dssp_records_it->second;
					if(!dssp_record.resSSE.empty())
					{
						value.props.tags.insert(std::string("dssp=")+dssp_record.resSSE);
					}
					value.props.adjuncts["phi"]=dssp_record.anglePhi;
					value.props.adjuncts["psi"]=dssp_record.anglePsi;
				}
			}
			if(!sequence_mapping.empty())
			{
				const std::map<CRAD, int>::const_iterator sm_it=sequence_mapping.find(crad.without_atom());
				if(sm_it!=sequence_mapping.end())
				{
					value.props.adjuncts["refseq"]=static_cast<double>(sm_it->second);
				}
			}
		}
	}

	if(update_mode)
	{
		voronota::auxiliaries::IOUtilities().write_map(list_of_balls, std::cout);
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
			foutput << voronota::modescommon::SequenceUtilities::convert_residue_sequence_container_to_string(residue_sequence_vector) << "\n";
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
				map_of_chains_sequences[it->first]=voronota::modescommon::SequenceUtilities::convert_residue_sequence_container_to_string(it->second);
			}
			std::set<std::string> representative_chains;
			std::set<std::string> repeated_chains;
			for(std::map<std::string, std::string>::const_iterator it1=map_of_chains_sequences.begin();it1!=map_of_chains_sequences.end();++it1)
			{
				if(repeated_chains.count(it1->first)==0)
				{
					representative_chains.insert(it1->first);
					std::map<std::string, std::string>::const_iterator it2=it1;
					++it2;
					for(;it2!=map_of_chains_sequences.end();++it2)
					{
						if(voronota::modescommon::SequenceUtilities::calculate_sequence_identity(it1->second, it2->second)>=chains_seq_identity)
						{
							repeated_chains.insert(it2->first);
						}
					}
				}
			}
			foutput << map_of_chains_sequences.size() << " " << representative_chains.size() << "\n";
			voronota::auxiliaries::IOUtilities().write_set(representative_chains, foutput);
		}
	}
}
