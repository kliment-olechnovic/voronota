#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"

#include "../common/contacts_scoring_utilities.h"
#include "../common/statistics_utilities.h"

namespace
{

typedef voronota::common::ChainResidueAtomDescriptor CRAD;
typedef voronota::common::EnergyDescriptor EnergyDescriptor;

inline bool read_energy_descriptors_and_accumulate_to_map_of_value_stats(std::istream& input, std::map<CRAD, voronota::common::ValueStat>& map_of_value_stats)
{
	CRAD crad;
	EnergyDescriptor ed;
	input >> crad >> ed;
	if(!input.fail())
	{
		if(ed.total_area>0.0)
		{
			map_of_value_stats[voronota::common::generalize_crad(crad)].add(ed.energy/ed.total_area);
		}
		return true;
	}
	return false;
}

}

void score_contacts_energy_stats(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of atom energy descriptors");
	pohw.describe_io("stdout", false, true, "list of normalized energy mean and sd values per atom type");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::map<CRAD, voronota::common::ValueStat> map_of_value_stats;
	voronota::auxiliaries::IOUtilities().read_lines_to_container(std::cin, read_energy_descriptors_and_accumulate_to_map_of_value_stats, map_of_value_stats);

	std::map<CRAD, voronota::common::NormalDistributionParameters> means_and_sds;
	for(std::map<CRAD, voronota::common::ValueStat>::const_iterator it=map_of_value_stats.begin();it!=map_of_value_stats.end();++it)
	{
		means_and_sds[it->first]=voronota::common::NormalDistributionParameters(it->second.mean(), it->second.sd());
	}

	voronota::auxiliaries::IOUtilities().write_map(means_and_sds, std::cout);
}
