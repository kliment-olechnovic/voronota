#ifndef MODESCOMMON_HANDLE_CONTACT_H_
#define MODESCOMMON_HANDLE_CONTACT_H_

#include "handle_annotations.h"
#include "handle_matchings.h"

namespace modescommon
{

struct ContactValue
{
	double area;
	double dist;
	std::set<std::string> tags;
	std::map<std::string, double> adjuncts;
	std::string graphics;

	ContactValue() : area(0.0), dist(0.0)
	{
	}

	void add(const ContactValue& v)
	{
		area+=v.area;
		dist=(dist<=0.0 ? v.dist : std::min(dist, v.dist));
		tags.insert(v.tags.begin(), v.tags.end());
		for(std::map<std::string, double>::const_iterator it=v.adjuncts.begin();it!=v.adjuncts.end();++it)
		{
			adjuncts[it->first]+=it->second;
		}
		if(!v.graphics.empty())
		{
			if(graphics.empty())
			{
				graphics=v.graphics;
			}
			else
			{
				if(graphics[graphics.size()-1]!=' ' || v.graphics[0]!=' ')
				{
					graphics+=" ";
				}
				graphics+=v.graphics;
			}
		}
	}
};

inline void print_contact_record(const std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor>& names, const ContactValue& value, const bool preserve_graphics, std::ostream& output)
{
	output << names.first.str() << " " << names.second.str() << " " << value.area << " " << value.dist;
	output << (value.tags.empty() ? " ." : " ");
	auxiliaries::print_set_to_stream(value.tags, ";", output);
	output << (value.adjuncts.empty() ? " ." : " ");
	auxiliaries::print_map_to_stream(value.adjuncts, ";", output);
	if(preserve_graphics && !value.graphics.empty())
	{
		output << " \"";
		output << value.graphics;
		output << "\"";
	}
	output << "\n";
}

inline void print_contact_records_map(const std::map< std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor>, ContactValue >& map_of_records, const bool preserve_graphics, std::ostream& output)
{
	for(std::map< std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor>, ContactValue >::const_iterator it=map_of_records.begin();it!=map_of_records.end();++it)
	{
		modescommon::print_contact_record(it->first, it->second, preserve_graphics, output);
	}
}

inline void print_summary_of_contact_records_map(const std::map< std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor>, ContactValue >& map_of_records, const bool preserve_graphics, std::ostream& output)
{
	ContactValue summary;
	for(std::map< std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor>, ContactValue >::const_iterator it=map_of_records.begin();it!=map_of_records.end();++it)
	{
		summary.add(it->second);
		if(!preserve_graphics)
		{
			summary.graphics.clear();
		}
	}
	print_contact_record(std::make_pair(auxiliaries::ChainResidueAtomDescriptor("any"), auxiliaries::ChainResidueAtomDescriptor("any")), summary, preserve_graphics, output);
}

inline bool add_contact_record_from_stream_to_map(std::istream& input, std::map< std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor>, ContactValue >& map_of_records)
{
	std::pair<std::string, std::string> name_strings;
	ContactValue value;
	input >> name_strings.first >> name_strings.second >> value.area >> value.dist;
	{
		std::string tags;
		input >> tags;
		update_set_of_tags(value.tags, tags);
	}
	{
		std::string adjuncts;
		input >> adjuncts;
		update_map_of_adjuncts(value.adjuncts, adjuncts);
	}
	if(input.good())
	{
		std::getline(input, value.graphics, '"');
		std::getline(input, value.graphics, '"');
	}
	if(!input.fail() && !name_strings.first.empty() && !name_strings.second.empty())
	{
		const std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor> names(auxiliaries::ChainResidueAtomDescriptor::from_str(name_strings.first), auxiliaries::ChainResidueAtomDescriptor::from_str(name_strings.second));
		if(names.first.valid() && names.second.valid())
		{
			map_of_records[refine_pair_by_ordering(names)]=value;
			return true;
		}
	}
	return false;
}

}

#endif /* MODESCOMMON_HANDLE_CONTACT_H_ */
