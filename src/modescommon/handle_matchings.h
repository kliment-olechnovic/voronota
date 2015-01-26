#ifndef MODESCOMMON_HANDLE_MATCHINGS_H_
#define MODESCOMMON_HANDLE_MATCHINGS_H_

#include <set>
#include <map>

#include "../auxiliaries/chain_residue_atom_descriptor.h"

#include "tmpfuncs.h"

namespace modescommon
{

inline void update_set_of_tags(std::set<std::string>& tags, const std::string& str)
{
	if(!str.empty() && str[0]!='.')
	{
		const std::set<std::string> input_tags=auxiliaries::read_set_from_string<std::string>(str, ';');
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
		const std::map<std::string, double> input_adjuncts=auxiliaries::read_map_from_string<std::string, double>(str, ';');
		for(std::map<std::string, double>::const_iterator it=input_adjuncts.begin();it!=input_adjuncts.end();++it)
		{
			adjuncts[it->first]=it->second;
		}
	}
}

template<typename T, typename F>
inline bool match_container_with_multiple_values(const T& container, const F& matcher, const std::string& values)
{
	const std::set<std::string> or_set=auxiliaries::read_set_from_string<std::string>(values, '|');
	for(std::set<std::string>::const_iterator it=or_set.begin();it!=or_set.end();++it)
	{
		const std::set<std::string> and_set=auxiliaries::read_set_from_string<std::string>(*it, '&');
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
	inline bool operator()(const auxiliaries::ChainResidueAtomDescriptor& descriptor, const std::string& value) const
	{
		return auxiliaries::ChainResidueAtomDescriptor::match_with_member_selection_string(descriptor, value);
	}
};

inline bool match_chain_residue_atom_descriptor(const auxiliaries::ChainResidueAtomDescriptor& descriptor, const std::string& positive_values, const std::string& negative_values)
{
	return ((positive_values.empty() || match_container_with_multiple_values(descriptor, functor_match_chain_residue_atom_descriptor_with_single_value(), positive_values))
			&& (negative_values.empty() || !match_container_with_multiple_values(descriptor, functor_match_chain_residue_atom_descriptor_with_single_value(), negative_values)));
}

struct functor_match_set_of_tags_with_single_value
{
	inline bool operator()(const std::set<std::string>& tags, const std::string& value) const
	{
		return (tags.count(value)>0);
	}
};

inline bool match_set_of_tags(const std::set<std::string>& tags, const std::string& positive_values, const std::string& negative_values)
{
	return ((positive_values.empty() || match_container_with_multiple_values(tags, functor_match_set_of_tags_with_single_value(), positive_values))
			&& (negative_values.empty() || !match_container_with_multiple_values(tags, functor_match_set_of_tags_with_single_value(), negative_values)));
}

struct functor_match_map_of_adjuncts_with_single_value
{
	inline bool operator()(const std::map<std::string, double>& adjuncts, const std::string& value) const
	{
		const std::size_t eq_pos=value.find('=');
		if(eq_pos!=std::string::npos)
		{
			const std::size_t sep_pos=value.find(':', eq_pos);
			if(sep_pos!=std::string::npos)
			{
				std::string spaced_value=value;
				spaced_value[eq_pos]=' ';
				spaced_value[sep_pos]=' ';
				std::istringstream input(spaced_value);
				if(input.good())
				{
					std::string name;
					double a=0.0;
					double b=0.0;
					input >> name >> a >> b;
					if(!input.fail() && !name.empty() && a<=b)
					{
						std::map<std::string, double>::const_iterator it=adjuncts.find(name);
						return (it!=adjuncts.end() && it->second>=a && it->second<=b);
					}
				}
			}
		}
		return false;
	}
};

inline bool match_map_of_adjuncts(const std::map<std::string, double>& adjuncts, const std::string& positive_values, const std::string& negative_values)
{
	return ((positive_values.empty() || match_container_with_multiple_values(adjuncts, functor_match_map_of_adjuncts_with_single_value(), positive_values))
			&& (negative_values.empty() || !match_container_with_multiple_values(adjuncts, functor_match_map_of_adjuncts_with_single_value(), negative_values)));
}

inline bool match_chain_residue_atom_descriptor_with_set_of_descriptors(const auxiliaries::ChainResidueAtomDescriptor& descriptor, const std::set<auxiliaries::ChainResidueAtomDescriptor>& set_of_descriptors)
{
	if(set_of_descriptors.count(descriptor)>0)
	{
		return true;
	}
	for(std::set<auxiliaries::ChainResidueAtomDescriptor>::const_iterator it=set_of_descriptors.begin();it!=set_of_descriptors.end();++it)
	{
		if(descriptor.contains(*it))
		{
			return true;
		}
	}
	return false;
}

inline bool match_chain_residue_atom_descriptors_pair_with_set_of_descriptors_pairs(const std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor>& descriptors_pair, const std::set< std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor> >& set_of_descriptors_pairs)
{
	if(set_of_descriptors_pairs.count(descriptors_pair)>0 || set_of_descriptors_pairs.count(std::make_pair(descriptors_pair.second, descriptors_pair.first))>0)
	{
		return true;
	}
	for(std::set< std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor> >::const_iterator it=set_of_descriptors_pairs.begin();it!=set_of_descriptors_pairs.end();++it)
	{
		if((descriptors_pair.first.contains(it->first) && descriptors_pair.second.contains(it->second)) ||
				(descriptors_pair.first.contains(it->second) && descriptors_pair.second.contains(it->first)))
		{
			return true;
		}
	}
	return false;
}

}

#endif /* MODESCOMMON_HANDLE_MATCHINGS_H_ */
