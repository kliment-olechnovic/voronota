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
		list_of_option_descriptions.push_back(OD("--invert", "", "flag to invert selection"));
		list_of_option_descriptions.push_back(OD("--drop-tags", "", "flag to drop all tags from input"));
		list_of_option_descriptions.push_back(OD("--set-tags", "string", "set tags instead of filtering"));
		list_of_option_descriptions.push_back(OD("--drop-adjuncts", "", "flag to drop all adjuncts from input"));
		list_of_option_descriptions.push_back(OD("--set-adjuncts", "string", "set adjuncts instead of filtering"));
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
	const bool invert=poh.contains_option("--invert");
	const bool drop_tags=poh.contains_option("--drop-tags");
	const std::string set_tags=poh.argument<std::string>("--set-tags", "");
	const bool drop_adjuncts=poh.contains_option("--drop-adjuncts");
	const std::string set_adjuncts=poh.argument<std::string>("--set-adjuncts", "");

	std::vector< std::pair<CRAD, BallValue> > list_of_balls;
	auxiliaries::read_lines_to_container(std::cin, modescommon::add_ball_record_from_stream_to_vector<CRAD>, list_of_balls);
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

	std::set<std::size_t> output_set_of_ball_ids;

	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		const CRAD& crad=list_of_balls[i].first;
		const BallValue& value=list_of_balls[i].second;
		const bool passed=(modescommon::match_chain_residue_atom_descriptor(crad, match, match_not) &&
				modescommon::match_set_of_tags(value.tags, match_tags, match_tags_not) &&
				modescommon::match_map_of_adjuncts(value.adjuncts, match_adjuncts, match_adjuncts_not));
		if((passed && !invert) || (!passed && invert))
		{
			output_set_of_ball_ids.insert(i);
		}
	}

	if(!set_tags.empty() || !set_adjuncts.empty())
	{
		for(std::set<std::size_t>::const_iterator it=output_set_of_ball_ids.begin();it!=output_set_of_ball_ids.end();++it)
		{
			BallValue& value=list_of_balls[*it].second;
			modescommon::update_set_of_tags(value.tags, set_tags);
			modescommon::update_map_of_adjuncts(value.adjuncts, set_adjuncts);
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
