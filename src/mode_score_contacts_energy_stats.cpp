#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

#include "modescommon/contacts_scoring_utilities.h"
#include "modescommon/statistics_utilities.h"

namespace
{

inline bool read_energy_descriptors_and_accumulate_to_map_of_value_stats(std::istream& input, std::map<CRAD, ValueStat>& map_of_value_stats)
{
	CRAD crad;
	EnergyDescriptor ed;
	input >> crad >> ed;
	if(!input.fail())
	{
		if(ed.total_area>0.0)
		{
			map_of_value_stats[generalize_crad(crad)].add(ed.energy/ed.total_area);
		}
		return true;
	}
	return false;
}

}

void score_contacts_energy_stats(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of atom energy descriptors");
	pohw.describe_io("stdout", false, true, "list of normalized energy mean and sd values per atom type");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::map<CRAD, ValueStat> map_of_value_stats;
	auxiliaries::IOUtilities().read_lines_to_container(std::cin, read_energy_descriptors_and_accumulate_to_map_of_value_stats, map_of_value_stats);

	std::map<CRAD, NormalDistributionParameters> means_and_sds;
	for(std::map<CRAD, ValueStat>::const_iterator it=map_of_value_stats.begin();it!=map_of_value_stats.end();++it)
	{
		means_and_sds[it->first]=NormalDistributionParameters(it->second.mean(), it->second.sd());
	}

	auxiliaries::IOUtilities().write_map(means_and_sds, std::cout);
}
