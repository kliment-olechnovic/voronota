#ifndef MODESCOMMON_HANDLE_ANNOTATIONS_H_
#define MODESCOMMON_HANDLE_ANNOTATIONS_H_

#include "../auxiliaries/io_utilities.h"
#include "../auxiliaries/chain_residue_atom_descriptor.h"

namespace modescommon
{

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

inline bool add_chain_residue_atom_descriptors_from_stream_to_set(std::istream& input, std::set<auxiliaries::ChainResidueAtomDescriptor>& set_of_descriptors)
{
	bool filled=false;
	while(input.good())
	{
		std::string crad_string;
		input >> crad_string;
		if(!input.fail() && !crad_string.empty())
		{
			auxiliaries::ChainResidueAtomDescriptor crad;
			if(auxiliaries::ChainResidueAtomDescriptor::from_str(crad_string, crad).empty())
			{
				set_of_descriptors.insert(crad);
				filled=true;
			}
		}
	}
	return filled;
}

inline bool add_chain_residue_atom_descriptors_pair_from_stream_to_set(std::istream& input, std::set< std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor> >& set_of_descriptors_pairs)
{
	std::pair<std::string, std::string> crad_strings;
	input >> crad_strings.first >> crad_strings.second;
	if(!input.fail() && !crad_strings.first.empty() && !crad_strings.second.empty())
	{
		std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor> crads;
		if(auxiliaries::ChainResidueAtomDescriptor::from_str(crad_strings.first, crads.first).empty()
				&& auxiliaries::ChainResidueAtomDescriptor::from_str(crad_strings.second, crads.second).empty())
		{
			set_of_descriptors_pairs.insert(refine_pair_by_ordering(crads));
			return true;
		}
	}
	return false;
}

template<bool Additive>
inline bool add_chain_residue_atom_descriptor_value_from_stream_to_map(std::istream& input, std::map<auxiliaries::ChainResidueAtomDescriptor, double>& map_of_values)
{
	std::string name_string;
	double value;
	input >> name_string >> value;
	if(!input.fail() && !name_string.empty())
	{
		auxiliaries::ChainResidueAtomDescriptor name=auxiliaries::ChainResidueAtomDescriptor::from_str(name_string);
		if(Additive)
		{
			name=name.without_numbering();
		}
		if(name.valid())
		{
			double& left_value=map_of_values[name];
			left_value=(Additive ? (left_value+value) : value);
			return true;
		}
	}
	return false;
}

template<bool Additive>
inline bool add_chain_residue_atom_descriptors_pair_value_from_stream_to_map(std::istream& input, std::map< std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor>, double >& map_of_values)
{
	std::pair<std::string, std::string> name_strings;
	double value;
	input >> name_strings.first >> name_strings.second >> value;
	if(!input.fail() && !name_strings.first.empty() && !name_strings.second.empty())
	{
		std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor> names(auxiliaries::ChainResidueAtomDescriptor::from_str(name_strings.first), auxiliaries::ChainResidueAtomDescriptor::from_str(name_strings.second));
		if(Additive)
		{
			names.first=names.first.without_numbering();
			names.second=names.second.without_numbering();
		}
		if(names.first.valid() && names.second.valid())
		{
			double& left_value=map_of_values[refine_pair_by_ordering(names)];
			left_value=(Additive ? (left_value+value) : value);
			return true;
		}
	}
	return false;
}

inline bool add_chain_residue_atom_descriptors_pair_value_from_stream_to_list(std::istream& input, std::list< std::pair<std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor>, double> >& list_of_values)
{
	std::pair<std::string, std::string> name_strings;
	double value;
	input >> name_strings.first >> name_strings.second >> value;
	if(!input.fail() && !name_strings.first.empty() && !name_strings.second.empty())
	{
		std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor> names(auxiliaries::ChainResidueAtomDescriptor::from_str(name_strings.first), auxiliaries::ChainResidueAtomDescriptor::from_str(name_strings.second));
		if(names.first.valid() && names.second.valid())
		{
			list_of_values.push_back(std::make_pair(refine_pair_by_ordering(names), value));
			return true;
		}
	}
	return false;
}

}

#endif /* MODESCOMMON_HANDLE_ANNOTATIONS_H_ */
