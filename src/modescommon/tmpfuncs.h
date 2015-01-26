#ifndef MODESCOMMON_TMPFUNCS_H_
#define MODESCOMMON_TMPFUNCS_H_

#include "../auxiliaries/chain_residue_atom_descriptor.h"
#include "../auxiliaries/io_utilities.h"

namespace auxiliaries
{

template<typename T>
inline std::set<T> read_set_from_string(const std::string& input_str, const char sep)
{
	std::set<T> result;
	IOUtilities(sep).read_string_lines_to_sequential_container(input_str, result);
	return result;
}

template<typename T>
inline void print_set_to_stream(const std::set<T>& set, const char sep, std::ostream& output)
{
	IOUtilities(sep).write_sequential_container(set, output);
}

template<typename A, typename B>
inline std::map<A, B> read_map_from_string(const std::string& input_str, const char sep)
{
	std::map<A, B> result;
	IOUtilities(sep, '=').read_string_lines_to_map_container(input_str, result);
	return result;
}

template<typename A, typename B>
inline void print_map_to_stream(const std::map<A, B>& map, const char sep, std::ostream& output)
{
	IOUtilities(sep, '=').write_map_container(map, output);
}

}

namespace modescommon
{

template<typename Descriptor>
inline std::map< auxiliaries::ChainResidueAtomDescriptor, std::set<auxiliaries::ChainResidueAtomDescriptor> > construct_graph_from_pair_mapping_of_descriptors(const std::map< std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor>, Descriptor >& map_of_pair_descriptors, const int depth)
{
	typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
	std::map< CRAD, std::set<CRAD> > graph;
	if(depth>0)
	{
		for(typename std::map< std::pair<CRAD, CRAD>, Descriptor >::const_iterator it=map_of_pair_descriptors.begin();it!=map_of_pair_descriptors.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			if(!(crads.first==crads.second || crads.first==CRAD::solvent() || crads.second==CRAD::solvent()))
			{
				graph[crads.first].insert(crads.second);
				graph[crads.second].insert(crads.first);
			}
		}
		for(int i=1;i<depth;i++)
		{
			std::map< CRAD, std::set<CRAD> > expanded_graph=graph;
			for(std::map< CRAD, std::set<CRAD> >::const_iterator graph_it=graph.begin();graph_it!=graph.end();++graph_it)
			{
				const CRAD& center=graph_it->first;
				const std::set<CRAD>& neighbors=graph_it->second;
				std::set<CRAD>& expandable_neighbors=expanded_graph[center];
				for(std::set<CRAD>::const_iterator neighbors_it=neighbors.begin();neighbors_it!=neighbors.end();++neighbors_it)
				{
					const std::set<CRAD>& neighbor_neighbors=graph[*neighbors_it];
					expandable_neighbors.insert(neighbor_neighbors.begin(), neighbor_neighbors.end());
				}
				expandable_neighbors.erase(center);
			}
			graph=expanded_graph;
		}
	}
	return graph;
}

template<typename Descriptor>
inline std::map<auxiliaries::ChainResidueAtomDescriptor, Descriptor> construct_single_mapping_of_descriptors_from_pair_mapping_of_descriptors(const std::map< std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor>, Descriptor >& map_of_pair_descriptors, const std::map< auxiliaries::ChainResidueAtomDescriptor, std::set<auxiliaries::ChainResidueAtomDescriptor> >& graph)
{
	typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
	std::map<CRAD, Descriptor> map_of_single_descriptors;
	for(typename std::map< std::pair<CRAD, CRAD>, Descriptor >::const_iterator it=map_of_pair_descriptors.begin();it!=map_of_pair_descriptors.end();++it)
	{
		const std::pair<CRAD, CRAD>& crads=it->first;
		std::set<CRAD> related_crads;
		if(!(crads.first==CRAD::solvent()))
		{
			related_crads.insert(crads.first);
		}
		if(!(crads.second==CRAD::solvent()))
		{
			related_crads.insert(crads.second);
		}
		{
			const std::map< CRAD, std::set<CRAD> >::const_iterator graph_it=graph.find(crads.first);
			if(graph_it!=graph.end())
			{
				const std::set<CRAD>& related_crads1=graph_it->second;
				related_crads.insert(related_crads1.begin(), related_crads1.end());
			}
		}
		{
			const std::map< CRAD, std::set<CRAD> >::const_iterator graph_it=graph.find(crads.second);
			if(graph_it!=graph.end())
			{
				const std::set<CRAD>& related_crads2=graph_it->second;
				related_crads.insert(related_crads2.begin(), related_crads2.end());
			}
		}
		for(std::set<CRAD>::const_iterator jt=related_crads.begin();jt!=related_crads.end();++jt)
		{
			map_of_single_descriptors[*jt].add(it->second);
		}
	}
	return map_of_single_descriptors;
}

}

#endif /* MODESCOMMON_TMPFUNCS_H_ */
