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

inline bool match_set_of_tags(const std::set<std::string>& tags, const std::vector<std::string>& positive_values, const std::vector<std::string>& negative_values)
{
	for(std::size_t i=0;i<positive_values.size();i++)
	{
		if(tags.count(positive_values[i])==0)
		{
			return false;
		}
	}
	for(std::size_t i=0;i<negative_values.size();i++)
	{
		if(tags.count(negative_values[i])>0)
		{
			return false;
		}
	}
	return true;
}

}

#endif /* MODESCOMMON_HANDLE_ANNOTATIONS_H_ */
