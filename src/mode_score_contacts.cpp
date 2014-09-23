#include <iostream>
#include <stdexcept>
#include <fstream>

#include "modescommon/assert_options.h"
#include "modescommon/handle_contact.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

inline bool add_contact_value_from_stream_to_map_of_total_values(std::istream& input, std::map< std::pair<CRAD, CRAD>, double >& map_of_total_values)
{
	std::pair<std::string, std::string> name_strings;
	double value;
	input >> name_strings.first >> name_strings.second >> value;
	if(!input.fail() && !name_strings.first.empty() && !name_strings.second.empty())
	{
		const std::pair<CRAD, CRAD> names(CRAD::from_str(name_strings.first).without_numbering(), CRAD::from_str(name_strings.second).without_numbering());
		if(names.first.valid() && names.second.valid())
		{
			map_of_total_values[modescommon::refine_pair_by_ordering(names)]+=value;
			return true;
		}
	}
	return false;
}

}

void score_contacts_potential(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 area')\n";
			std::cerr << "stdout  ->  list of potential values (line format: 'annotation1 annotation2 value')\n";
			return;
		}
	}

	std::map< std::pair<CRAD, CRAD>, double > map_of_total_areas;
	auxiliaries::read_lines_to_container(std::cin, add_contact_value_from_stream_to_map_of_total_values, map_of_total_areas);
	if(map_of_total_areas.empty())
	{
		throw std::runtime_error("No input.");
	}

	std::map<CRAD, double> map_of_generalized_total_areas;
	double sum_of_all_areas=0.0;
	for(std::map< std::pair<CRAD, CRAD>, double >::iterator it=map_of_total_areas.begin();it!=map_of_total_areas.end();++it)
	{
		const std::pair<CRAD, CRAD>& crads=it->first;
		map_of_generalized_total_areas[crads.first]+=(it->second);
		if(!(crads.first==crads.second))
		{
			map_of_generalized_total_areas[crads.second]+=(it->second);
		}
		sum_of_all_areas+=(it->second);
	}

	for(std::map< std::pair<CRAD, CRAD>, double >::iterator it=map_of_total_areas.begin();it!=map_of_total_areas.end();++it)
	{
		const std::pair<CRAD, CRAD>& crads=it->first;
		const double ab=it->second;
		const double ax=map_of_generalized_total_areas[crads.first];
		const double bx=map_of_generalized_total_areas[crads.second];
		if(ab>0.0 && ax>0.0 && bx>0.0)
		{
			const double potential_value=(ab*sum_of_all_areas)/(ax*bx);
			std::cout << crads.first.str() << " " << crads.second.str() << potential_value << "\n";
		}
	}
}
