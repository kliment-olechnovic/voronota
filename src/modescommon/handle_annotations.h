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

template<typename T, typename F>
inline bool match_container_with_multiple_values(const T& container, const F& matcher, const std::string& values)
{
	const std::set<std::string> or_set=auxiliaries::read_set_from_string<std::string>(values, "|");
	for(std::set<std::string>::const_iterator it=or_set.begin();it!=or_set.end();++it)
	{
		const std::set<std::string> and_set=auxiliaries::read_set_from_string<std::string>(*it, "&");
		bool and_result=true;
		for(std::set<std::string>::const_iterator jt=and_set.begin();and_result && jt!=and_set.end();++jt)
		{
			and_result=(and_result && matcher(container, *jt));
		}
		if(and_result)
		{
			return true;
		}
	}
	return false;
}

struct functor_match_chain_residue_atom_descriptor_with_single_value
{
	bool operator()(const auxiliaries::ChainResidueAtomDescriptor& descriptor, const std::string& value) const
	{
		return auxiliaries::ChainResidueAtomDescriptor::match_with_member_descriptor(descriptor, value);
	}
};

inline bool match_chain_residue_atom_descriptor(const auxiliaries::ChainResidueAtomDescriptor& full_descriptor, const std::string& positive_values, const std::string& negative_values)
{
	return ((positive_values.empty() || match_container_with_multiple_values(full_descriptor, functor_match_chain_residue_atom_descriptor_with_single_value(), positive_values))
			&& (negative_values.empty() || !match_container_with_multiple_values(full_descriptor, functor_match_chain_residue_atom_descriptor_with_single_value(), negative_values)));
}

struct functor_match_set_of_tags_with_single_value
{
	bool operator()(const std::set<std::string>& tags, const std::string& value) const
	{
		return (tags.count(value)>0);
	}
};

inline bool match_set_of_tags(const std::set<std::string>& tags, const std::string& positive_values, const std::string& negative_values)
{
	return ((positive_values.empty() || match_container_with_multiple_values(tags, functor_match_set_of_tags_with_single_value(), positive_values))
			&& (negative_values.empty() || !match_container_with_multiple_values(tags, functor_match_set_of_tags_with_single_value(), negative_values)));
}

}

#endif /* MODESCOMMON_HANDLE_ANNOTATIONS_H_ */
