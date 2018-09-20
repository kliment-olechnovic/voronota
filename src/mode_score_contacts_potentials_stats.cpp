#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

#include "common/contacts_scoring_utilities.h"
#include "common/statistics_utilities.h"

namespace
{

typedef common::InteractionName InteractionName;

}

void score_contacts_potentials_stats(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of potential files");
	pohw.describe_io("stdout", false, true, "list of normalized energy mean and sd values per interaction type");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::map<InteractionName, common::ValueStat> map_of_value_stats;
	while(std::cin.good())
	{
		std::string potential_file;
		std::cin >> potential_file;
		const std::map<InteractionName, double> map_of_potential_values=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<InteractionName, double> >(potential_file);
		for(std::map<InteractionName, double>::const_iterator it=map_of_potential_values.begin();it!=map_of_potential_values.end();++it)
		{
			map_of_value_stats[it->first].add(it->second);
		}
	}

	std::map<InteractionName, common::NormalDistributionParameters> means_and_sds;
	for(std::map<InteractionName, common::ValueStat>::const_iterator it=map_of_value_stats.begin();it!=map_of_value_stats.end();++it)
	{
		means_and_sds[it->first]=common::NormalDistributionParameters(it->second.mean(), it->second.sd());
	}

	auxiliaries::IOUtilities().write_map(means_and_sds, std::cout);
}
