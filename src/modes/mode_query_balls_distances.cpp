#include "../apollota/basic_operations_on_spheres.h"

#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"

#include "../common/ball_value.h"
#include "../common/matching_utilities.h"

#include "../common/chain_residue_atom_descriptor.h"

namespace
{

typedef voronota::common::ChainResidueAtomDescriptor CRAD;
typedef voronota::common::ChainResidueAtomDescriptorsPair CRADsPair;

}

void query_balls_distances(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'annotation x y z r tags adjuncts')");
	pohw.describe_io("stdout", false, true, "list of distances (line format: 'annotation1 annotation2 distance min-distance-between-balls')");

	const std::string match_first=poh.argument<std::string>(pohw.describe_option("--match-first", "string", "selection for first interacting group"), "");
	const std::string match_first_not=poh.argument<std::string>(pohw.describe_option("--match-first-not", "string", "negative selection for first interacting group"), "");
	const std::string match_second=poh.argument<std::string>(pohw.describe_option("--match-second", "string", "selection for second interacting group"), "");
	const std::string match_second_not=poh.argument<std::string>(pohw.describe_option("--match-second-not", "string", "negative selection for second interacting group"), "");
	const std::string match_external_first=poh.argument<std::string>(pohw.describe_option("--match-external-first", "string", "file path to input matchable annotations"), "");
	const std::string match_external_second=poh.argument<std::string>(pohw.describe_option("--match-external-second", "string", "file path to input matchable annotations"), "");

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

	if(match_first.empty() && match_first_not.empty() &&
			match_second.empty() && match_second_not.empty() &&
			match_external_first.empty() && match_external_second.empty())
	{
		throw std::runtime_error("No matching parameters.");
	}

	const std::set<CRAD> matchable_external_crads_first=voronota::auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRAD> >(match_external_first);
	const std::set<CRAD> matchable_external_crads_second=voronota::auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRAD> >(match_external_second);

	std::set<std::size_t> ids_first;
	std::set<std::size_t> ids_second;

	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		const CRAD& crad=list_of_balls[i].first;
		if(voronota::common::MatchingUtilities::match_crad(crad, match_first, match_first_not) &&
				(match_external_first.empty() || voronota::common::MatchingUtilities::match_crad_with_set_of_crads(false, crad, matchable_external_crads_first)))
		{
			ids_first.insert(ids_first.end(), i);
		}
		if(voronota::common::MatchingUtilities::match_crad(crad, match_second, match_second_not) &&
				(match_external_second.empty() || voronota::common::MatchingUtilities::match_crad_with_set_of_crads(false, crad, matchable_external_crads_second)))
		{
			ids_second.insert(ids_second.end(), i);
		}
	}

	if(ids_first.empty())
	{
		throw std::runtime_error("Empty first selection.");
	}

	if(ids_second.empty())
	{
		throw std::runtime_error("Empty second selection.");
	}

	typedef std::set< std::pair< std::pair<double, double>, CRADsPair > > Result;
	Result result;

	for(std::set<std::size_t>::const_iterator it_first=ids_first.begin();it_first!=ids_first.end();++it_first)
	{
		for(std::set<std::size_t>::const_iterator it_second=ids_second.begin();it_second!=ids_second.end();++it_second)
		{
			const double dpp=voronota::apollota::distance_from_point_to_point(list_of_balls[*it_first].second, list_of_balls[*it_second].second);
			const double dss=voronota::apollota::minimal_distance_from_sphere_to_sphere(list_of_balls[*it_first].second, list_of_balls[*it_second].second);
			result.insert(std::make_pair(std::make_pair(dpp, dss), CRADsPair(list_of_balls[*it_first].first, list_of_balls[*it_second].first)));
		}
	}

	for(Result::const_iterator it=result.begin();it!=result.end();++it)
	{
		std::cout << it->second << " " << it->first.first << " " << it->first.second << "\n";
	}
}
