#include <iostream>
#include <stdexcept>
#include <fstream>

#include "modescommon/assert_options.h"
#include "modescommon/handle_ball.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef modescommon::BallValue BallValue;

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
}
