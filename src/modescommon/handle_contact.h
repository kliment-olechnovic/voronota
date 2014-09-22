#ifndef MODESCOMMON_HANDLE_CONTACT_H_
#define MODESCOMMON_HANDLE_CONTACT_H_

#include "handle_annotations.h"

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
			adjuncts[it->first]=it->second;
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

template<typename T>
inline T refine_pair(const T& p, const bool reverse)
{
	if(reverse)
	{
		return T(p.second, p.first);
	}
	else
	{
		return p;
	}
}

template<typename T>
inline T refine_pair_by_ordering(const T& p)
{
	return refine_pair(p, p.second<p.first);
}

inline void print_contact_record(const std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor>& names, const ContactValue& value, const bool preserve_graphics, std::ostream& output)
{
	output << names.first.str() << " " << names.second.str() << " " << value.area << " " << value.dist;
	output << " " << (value.tags.empty() ? std::string(".") : auxiliaries::print_set_to_string(value.tags, ";"));
	output << " " << (value.adjuncts.empty() ? std::string(".") : auxiliaries::print_map_to_string(value.adjuncts, ";"));
	if(preserve_graphics && !value.graphics.empty())
	{
		output << " \"";
		output << value.graphics;
		output << "\"";
	}
	output << "\n";
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
