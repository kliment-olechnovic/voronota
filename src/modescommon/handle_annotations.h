#ifndef MODESCOMMON_HANDLE_ANNOTATIONS_H_
#define MODESCOMMON_HANDLE_ANNOTATIONS_H_

#include <set>
#include <vector>

#include "../auxiliaries/chain_residue_atom_descriptor.h"

namespace modescommon
{

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
