#ifndef MODESCOMMON_HANDLE_MAPPINGS_H_
#define MODESCOMMON_HANDLE_MAPPINGS_H_

#include <set>
#include <map>

#include "../auxiliaries/chain_residue_atom_descriptor.h"

namespace modescommon
{

template<typename Descriptor>
inline std::map< auxiliaries::ChainResidueAtomDescriptor, std::set<auxiliaries::ChainResidueAtomDescriptor> > construct_graph_from_mapping_of_descriptors_pairs(const std::map< auxiliaries::ChainResidueAtomDescriptorsPair, Descriptor >& map_of_pair_descriptors, const int depth)
{
	typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
	typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;
	std::map< CRAD, std::set<CRAD> > graph;
	if(depth>0)
	{
		for(typename std::map< CRADsPair, Descriptor >::const_iterator it=map_of_pair_descriptors.begin();it!=map_of_pair_descriptors.end();++it)
		{
			const CRADsPair& crads=it->first;
			if(!(crads.a==crads.b || crads.a==CRAD::solvent() || crads.b==CRAD::solvent()))
			{
				graph[crads.a].insert(crads.b);
				graph[crads.b].insert(crads.a);
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
inline std::map<auxiliaries::ChainResidueAtomDescriptor, Descriptor> construct_single_mapping_of_descriptors_from_mapping_of_descriptors_pairs(const std::map< auxiliaries::ChainResidueAtomDescriptorsPair, Descriptor >& map_of_pair_descriptors, const std::map< auxiliaries::ChainResidueAtomDescriptor, std::set<auxiliaries::ChainResidueAtomDescriptor> >& graph)
{
	typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
	typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;
	std::map<CRAD, Descriptor> map_of_single_descriptors;
	for(typename std::map< CRADsPair, Descriptor >::const_iterator it=map_of_pair_descriptors.begin();it!=map_of_pair_descriptors.end();++it)
	{
		const CRADsPair& crads=it->first;
		std::set<CRAD> related_crads;
		if(!(crads.a==CRAD::solvent()))
		{
			related_crads.insert(crads.a);
		}
		if(!(crads.b==CRAD::solvent()))
		{
			related_crads.insert(crads.b);
		}
		{
			const std::map< CRAD, std::set<CRAD> >::const_iterator graph_it=graph.find(crads.a);
			if(graph_it!=graph.end())
			{
				const std::set<CRAD>& related_crads1=graph_it->second;
				related_crads.insert(related_crads1.begin(), related_crads1.end());
			}
		}
		{
			const std::map< CRAD, std::set<CRAD> >::const_iterator graph_it=graph.find(crads.b);
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

#endif /* MODESCOMMON_HANDLE_MAPPINGS_H_ */
