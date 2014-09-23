#include <iostream>
#include <stdexcept>
#include <fstream>

#include "modescommon/assert_options.h"
#include "modescommon/handle_contact.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef modescommon::ContactValue ContactValue;

}

void score_contacts_potential(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contact files (separated by whitespace characters)\n";
			std::cerr << "stdout  ->  list of potential values (line format: 'annotation1 annotation2 value')\n";
			return;
		}
	}

	std::set<std::string> filenames;
	while(std::cin.good())
	{
		std::string token;
		std::cin >> token;
		if(!token.empty())
		{
			filenames.insert(token);
		}
	}
	if(filenames.empty())
	{
		throw std::runtime_error("No input.");
	}

	std::map< std::pair<CRAD, CRAD>, double > map_of_total_areas;
	for(std::set<std::string>::const_iterator filename=filenames.begin();filename!=filenames.end();++filename)
	{
		std::map< std::pair<CRAD, CRAD>, ContactValue > map_of_contacts;
		{
			std::ifstream finput(filename->c_str(), std::ios::in);
			auxiliaries::read_lines_to_container(finput, modescommon::add_contact_record_from_stream_to_map, map_of_contacts);
		}
		if(!map_of_contacts.empty())
		{
			for(std::map< std::pair<CRAD, CRAD>, ContactValue >::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
			{
				map_of_total_areas[modescommon::refine_pair_by_ordering(std::make_pair(it->first.first.without_numbering(), it->first.second.without_numbering()))]+=(it->second.area);
			}
		}
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
