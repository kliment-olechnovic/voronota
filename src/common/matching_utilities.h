#ifndef COMMON_MATCHING_UTILITIES_H_
#define COMMON_MATCHING_UTILITIES_H_

#include "../auxiliaries/io_utilities.h"

#include "chain_residue_atom_descriptor.h"

namespace voronota
{

namespace common
{

class MatchingUtilities
{
public:
	static bool match_crad(const ChainResidueAtomDescriptor& crad, const std::string& positive_values, const std::string& negative_values)
	{
		return ((positive_values.empty() || match_container_with_multiple_values(crad, functor_match_crad_with_single_value(), positive_values))
				&& (negative_values.empty() || !match_container_with_multiple_values(crad, functor_match_crad_with_single_value(), negative_values)));
	}

	static bool match_set_of_tags(const std::set<std::string>& tags, const std::string& positive_values, const std::string& negative_values)
	{
		return ((positive_values.empty() || match_container_with_multiple_values(tags, functor_match_set_of_tags_with_single_value(), positive_values))
				&& (negative_values.empty() || !match_container_with_multiple_values(tags, functor_match_set_of_tags_with_single_value(), negative_values)));
	}

	static bool match_map_of_adjuncts(const std::map<std::string, double>& adjuncts, const std::string& positive_values, const std::string& negative_values)
	{
		return ((positive_values.empty() || match_container_with_multiple_values(adjuncts, functor_match_map_of_adjuncts_with_single_value(), positive_values))
				&& (negative_values.empty() || !match_container_with_multiple_values(adjuncts, functor_match_map_of_adjuncts_with_single_value(), negative_values)));
	}

	static bool match_crad_with_set_of_crads(const bool simplified, const ChainResidueAtomDescriptor& crad, const std::set<ChainResidueAtomDescriptor>& set_of_crads)
	{
		if(set_of_crads.count(crad)>0)
		{
			return true;
		}
		if(simplified)
		{
			if(
					set_of_crads.count(crad.without_some_info(true, true, false, false))>0 ||
					set_of_crads.count(crad.without_some_info(true, false, false, false))>0 ||
					set_of_crads.count(crad.without_some_info(true, false, false, true))>0 ||
					set_of_crads.count(crad.without_some_info(true, false, true, false))>0 ||
					set_of_crads.count(crad.without_some_info(true, false, true, true))>0 ||
					set_of_crads.count(crad.without_some_info(true, true, false, true))>0 ||
					set_of_crads.count(crad.without_some_info(true, true, true, false))>0 ||
					set_of_crads.count(crad.without_some_info(true, true, true, true))>0)
			{
				return true;
			}
		}
		else
		{
			for(std::set<ChainResidueAtomDescriptor>::const_iterator it=set_of_crads.begin();it!=set_of_crads.end();++it)
			{
				if(crad.contains(*it))
				{
					return true;
				}
			}
		}
		return false;
	}

	static bool match_crads_pair_with_set_of_crads_pairs(const bool simplified, const ChainResidueAtomDescriptorsPair& crads_pair, const std::set<ChainResidueAtomDescriptorsPair>& crads_pairs)
	{
		if(crads_pairs.count(crads_pair)>0)
		{
			return true;
		}
		if(simplified)
		{
			if(
					crads_pairs.count(crads_pair.without_some_info(true, true, false, false))>0 ||
					crads_pairs.count(crads_pair.without_some_info(true, false, false, false))>0 ||
					crads_pairs.count(crads_pair.without_some_info(true, false, false, true))>0 ||
					crads_pairs.count(crads_pair.without_some_info(true, false, true, false))>0 ||
					crads_pairs.count(crads_pair.without_some_info(true, false, true, true))>0 ||
					crads_pairs.count(crads_pair.without_some_info(true, true, false, true))>0 ||
					crads_pairs.count(crads_pair.without_some_info(true, true, true, false))>0 ||
					crads_pairs.count(crads_pair.without_some_info(true, true, true, true))>0)
			{
				return true;
			}
		}
		else
		{
			for(std::set<ChainResidueAtomDescriptorsPair>::const_iterator it=crads_pairs.begin();it!=crads_pairs.end();++it)
			{
				if((crads_pair.a.contains(it->a) && crads_pair.b.contains(it->b)) ||
						(crads_pair.a.contains(it->b) && crads_pair.b.contains(it->a)))
				{
					return true;
				}
			}
		}
		return false;
	}

	template<typename MapType>
	static std::pair<bool, typename MapType::mapped_type>  match_crad_with_map_of_crads(const bool simplified, const ChainResidueAtomDescriptor& crad, const MapType& map_of_crads)
	{
		typename MapType::const_iterator result_it=map_of_crads.find(crad);
		if(result_it==map_of_crads.end())
		{
			result_it=map_of_crads.find(crad.without_some_info(true, true, false, false));
		}
		if(simplified)
		{
			if(result_it==map_of_crads.end())
			{
				result_it=map_of_crads.find(crad.without_some_info(true, false, false, false));
			}
			if(result_it==map_of_crads.end())
			{
				result_it=map_of_crads.find(crad.without_some_info(true, false, false, true));
			}
			if(result_it==map_of_crads.end())
			{
				result_it=map_of_crads.find(crad.without_some_info(true, false, true, false));
			}
			if(result_it==map_of_crads.end())
			{
				result_it=map_of_crads.find(crad.without_some_info(true, false, true, true));
			}
			if(result_it==map_of_crads.end())
			{
				result_it=map_of_crads.find(crad.without_some_info(true, true, false, true));
			}
			if(result_it==map_of_crads.end())
			{
				result_it=map_of_crads.find(crad.without_some_info(true, true, true, false));
			}
			if(result_it==map_of_crads.end())
			{
				result_it=map_of_crads.find(crad.without_some_info(true, true, true, true));
			}
		}
		else
		{
			if(result_it==map_of_crads.end())
			{
				for(typename MapType::const_iterator it=map_of_crads.begin();result_it==map_of_crads.end() && it!=map_of_crads.end();++it)
				{
					if(crad.contains(it->first))
					{
						result_it=it;
					}
				}
			}
		}
		if(result_it!=map_of_crads.end())
		{
			return std::pair<bool, typename MapType::mapped_type>(true, result_it->second);
		}
		return std::pair<bool, typename MapType::mapped_type>(false, typename MapType::mapped_type());
	}

	template<typename MapType>
	static std::pair<bool, typename MapType::mapped_type>  match_crads_pair_with_map_of_crads_pairs(const bool simplified, const ChainResidueAtomDescriptorsPair& crads_pair, const MapType& map_of_crads_pairs)
	{
		typename MapType::const_iterator result_it=map_of_crads_pairs.find(crads_pair);
		if(result_it==map_of_crads_pairs.end())
		{
			result_it=map_of_crads_pairs.find(crads_pair.without_some_info(true, true, false, false));
		}
		if(simplified)
		{
			if(result_it==map_of_crads_pairs.end())
			{
				result_it=map_of_crads_pairs.find(crads_pair.without_some_info(true, false, false, false));
			}
			if(result_it==map_of_crads_pairs.end())
			{
				result_it=map_of_crads_pairs.find(crads_pair.without_some_info(true, false, false, true));
			}
			if(result_it==map_of_crads_pairs.end())
			{
				result_it=map_of_crads_pairs.find(crads_pair.without_some_info(true, false, true, false));
			}
			if(result_it==map_of_crads_pairs.end())
			{
				result_it=map_of_crads_pairs.find(crads_pair.without_some_info(true, false, true, true));
			}
			if(result_it==map_of_crads_pairs.end())
			{
				result_it=map_of_crads_pairs.find(crads_pair.without_some_info(true, true, false, true));
			}
			if(result_it==map_of_crads_pairs.end())
			{
				result_it=map_of_crads_pairs.find(crads_pair.without_some_info(true, true, true, false));
			}
			if(result_it==map_of_crads_pairs.end())
			{
				result_it=map_of_crads_pairs.find(crads_pair.without_some_info(true, true, true, true));
			}
		}
		else
		{
			if(result_it==map_of_crads_pairs.end())
			{
				for(typename MapType::const_iterator it=map_of_crads_pairs.begin();result_it==map_of_crads_pairs.end() && it!=map_of_crads_pairs.end();++it)
				{
					if((crads_pair.a.contains(it->first.a) && crads_pair.b.contains(it->first.b)) ||
							(crads_pair.a.contains(it->first.b) && crads_pair.b.contains(it->first.a)))
					{
						result_it=it;
					}
				}
			}
		}
		if(result_it!=map_of_crads_pairs.end())
		{
			return std::pair<bool, typename MapType::mapped_type>(true, result_it->second);
		}
		return std::pair<bool, typename MapType::mapped_type>(false, typename MapType::mapped_type());
	}

private:
	template<typename T, typename F>
	static bool match_container_with_multiple_values(const T& container, const F& matcher, const std::string& values)
	{
		std::set<std::string> or_set;
		auxiliaries::IOUtilities('|').read_string_lines_to_set(values, or_set);
		for(std::set<std::string>::const_iterator it=or_set.begin();it!=or_set.end();++it)
		{
			std::set<std::string> and_set;
			auxiliaries::IOUtilities('&').read_string_lines_to_set(*it, and_set);
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

	struct functor_match_crad_with_single_value
	{
		inline bool operator()(const ChainResidueAtomDescriptor& crad, const std::string& value) const
		{
			return ChainResidueAtomDescriptor::match_with_member_selection_string(crad, value);
		}
	};

	struct functor_match_set_of_tags_with_single_value
	{
		inline bool operator()(const std::set<std::string>& tags, const std::string& value) const
		{
			return (tags.count(value)>0);
		}
	};

	struct functor_match_map_of_adjuncts_with_single_value
	{
		inline bool operator()(const std::map<std::string, double>& adjuncts, const std::string& value) const
		{
			std::size_t eq_pos=value.find('=');
			if(eq_pos==std::string::npos)
			{
				eq_pos=value.find(':');
			}
			if(eq_pos==std::string::npos)
			{
				return (adjuncts.count(value)>0);
			}
			if(eq_pos!=std::string::npos && (eq_pos+1)<value.size())
			{
				const std::size_t sep_pos=value.find(':', eq_pos+1);
				if(sep_pos==std::string::npos)
				{
					std::string spaced_value=value;
					spaced_value[eq_pos]=' ';
					std::istringstream input(spaced_value);
					if(input.good())
					{
						std::string name;
						double a=0.0;
						input >> name >> a;
						if(!input.fail() && !name.empty())
						{
							std::map<std::string, double>::const_iterator it=adjuncts.find(name);
							return (it!=adjuncts.end() && it->second==a);
						}
					}
				}
				else
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
};

}

}

#endif /* COMMON_MATCHING_UTILITIES_H_ */
