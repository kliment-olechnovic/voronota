#ifndef MODESCOMMON_HANDLE_ANNOTATIONS_H_
#define MODESCOMMON_HANDLE_ANNOTATIONS_H_

#include <set>
#include <vector>

#include "../auxiliaries/io_utilities.h"
#include "../auxiliaries/chain_residue_atom_descriptor.h"

namespace modescommon
{

inline void update_set_of_tags(std::set<std::string>& tags, const std::string& str)
{
	if(!str.empty() && str[0]!='.')
	{
		const std::set<std::string> input_tags=auxiliaries::read_set_from_string<std::string>(str, ";,");
		if(!input_tags.empty())
		{
			tags.insert(input_tags.begin(), input_tags.end());
		}
	}
}

inline void update_map_of_adjuncts(std::map<std::string, double>& adjuncts, const std::string& str)
{
	if(!str.empty() && str[0]!='.')
	{
		const std::map<std::string, double> input_adjuncts=auxiliaries::read_map_from_string<std::string, double>(str, ";,");
		for(std::map<std::string, double>::const_iterator it=input_adjuncts.begin();it!=input_adjuncts.end();++it)
		{
			adjuncts[it->first]=it->second;
		}
	}
}

inline bool match_chain_residue_atom_descriptor(const auxiliaries::ChainResidueAtomDescriptor& full_descriptor, const std::vector<std::string>& positive_descriptors, const std::vector<std::string>& negative_descriptors)
{
	for(std::size_t i=0;i<positive_descriptors.size();i++)
	{
		if(!auxiliaries::ChainResidueAtomDescriptor::match_with_member_descriptor(full_descriptor, positive_descriptors[i]))
		{
			return false;
		}
	}
	for(std::size_t i=0;i<negative_descriptors.size();i++)
	{
		if(auxiliaries::ChainResidueAtomDescriptor::match_with_member_descriptor(full_descriptor, negative_descriptors[i]))
		{
			return false;
		}
	}
	return true;
}

inline bool match_set_of_tags(const std::set<std::string>& tags, const std::string& values)
{
	const std::set<std::string> or_set=auxiliaries::read_set_from_string<std::string>(values, "|");
	for(std::set<std::string>::const_iterator it=or_set.begin();it!=or_set.end();++it)
	{
		const std::set<std::string> and_set=auxiliaries::read_set_from_string<std::string>(*it, "&");
		bool and_result=true;
		for(std::set<std::string>::const_iterator jt=and_set.begin();jt!=and_set.end();++jt)
		{
			and_result=(and_result && (tags.count(*jt)>0));
		}
		if(and_result)
		{
			return true;
		}
	}
	return false;
}

inline bool match_set_of_tags(const std::set<std::string>& tags, const std::string& positive_values, const std::string& negative_values)
{
	return ((positive_values.empty() || match_set_of_tags(tags, positive_values)) && (negative_values.empty() || !match_set_of_tags(tags, negative_values)));
}

}

#endif /* MODESCOMMON_HANDLE_ANNOTATIONS_H_ */
