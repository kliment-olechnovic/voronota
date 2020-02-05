#ifndef COMMON_CONSTRUCTION_OF_CONTACT_EFFECT_GROUPINGS_H_
#define COMMON_CONSTRUCTION_OF_CONTACT_EFFECT_GROUPINGS_H_

#include "chain_residue_atom_descriptor.h"

namespace voronota
{

namespace common
{

class ConstructionOfContactEffectGroupings
{
public:
	typedef ChainResidueAtomDescriptor CRAD;
	typedef ChainResidueAtomDescriptorsPair CRADsPair;

	struct ContactEffectGrouping
	{
		std::vector< std::set<CRAD> > related_crads;
		std::set<CRAD> collapsed_related_crads;
		std::vector< std::set<CRADsPair> > related_crads_pairs;
		std::set<CRADsPair> collapsed_related_crads_pairs;

		void init(const std::size_t number_of_levels_of_crads, const CRAD& crad, const CRADsPair& crads_pair)
		{
			if(number_of_levels_of_crads<1)
			{
				return;
			}
			if(related_crads.size()!=number_of_levels_of_crads)
			{
				related_crads.resize(number_of_levels_of_crads);
			}
			if(related_crads_pairs.size()!=(number_of_levels_of_crads*2))
			{
				related_crads_pairs.resize(number_of_levels_of_crads*2);
			}
			related_crads[0].insert(crad);
			collapsed_related_crads.insert(crad);
			related_crads_pairs[0].insert(crads_pair);
			collapsed_related_crads_pairs.insert(crads_pair);
		}
	};

	static std::map<CRAD, ContactEffectGrouping> construct_contact_effect_groupings(const std::set<CRADsPair>& raw_all_crads_pairs, const std::size_t number_of_layers_of_crads)
	{
		if(number_of_layers_of_crads<1)
		{
			return construct_contact_effect_groupings(raw_all_crads_pairs, 1);
		}

		std::map<CRAD, ContactEffectGrouping> crad_effect_groupings;
		std::set<CRADsPair> all_crads_pairs;

		for(std::set<CRADsPair>::const_iterator it=raw_all_crads_pairs.begin();it!=raw_all_crads_pairs.end();++it)
		{
			const CRAD crad_a=it->a.without_atom();
			const CRAD crad_b=it->b.without_atom();
			if(crad_a!=CRAD::solvent() && crad_b!=CRAD::solvent() && CRAD::match_with_sequence_separation_interval(crad_a, crad_b, 1, CRAD::null_num(), true))
			{
				const CRADsPair crads_pair(crad_a, crad_b);
				all_crads_pairs.insert(crads_pair);
				crad_effect_groupings[crad_a].init(number_of_layers_of_crads, crad_b, crads_pair);
				crad_effect_groupings[crad_b].init(number_of_layers_of_crads, crad_a, crads_pair);
			}
		}

		for(std::size_t top_layer_id=1;top_layer_id<number_of_layers_of_crads;top_layer_id++)
		{
			const std::size_t prev_layer_id=(top_layer_id-1);
			for(std::map<CRAD, ContactEffectGrouping>::const_iterator ceg_it=crad_effect_groupings.begin();ceg_it!=crad_effect_groupings.end();++ceg_it)
			{
				ContactEffectGrouping& ceg=ceg_it->second;
				for(std::set<CRAD>::const_iterator it=ceg.related_crads[prev_layer_id].begin();it!=ceg.related_crads[prev_layer_id].end();++it)
				{
					const std::set<CRAD>& candidate_crads=crad_effect_groupings[*it].related_crads[0];
					for(std::set<CRAD>::const_iterator jt=candidate_crads.begin();jt!=candidate_crads.end();++jt)
					{
						if(ceg.collapsed_related_crads.count(*jt)<1)
						{
							ceg.related_crads[top_layer_id].insert(*jt);
							ceg.collapsed_related_crads.insert(*jt);
						}
					}
				}
			}
		}

		for(std::map<CRAD, ContactEffectGrouping>::const_iterator ceg_it=crad_effect_groupings.begin();ceg_it!=crad_effect_groupings.end();++ceg_it)
		{
			ContactEffectGrouping& ceg=ceg_it->second;
			for(std::size_t layer_id=0;layer_id<number_of_layers_of_crads;layer_id++)
			{
				for(std::set<CRAD>::const_iterator it=ceg.related_crads[layer_id].begin();it!=ceg.related_crads[layer_id].end();++it)
				{
					{
						std::set<CRAD>::const_iterator jt=it;
						++jt;
						for(;jt!=ceg.related_crads[layer_id].end();++jt)
						{
							const CRADsPair candidate(*it, *jt);
							if(ceg.collapsed_related_crads_pairs.count(candidate)<1 && all_crads_pairs.count(candidate)>0)
							{
								ceg.related_crads_pairs[layer_id*2+1].insert(candidate);
								ceg.collapsed_related_crads_pairs.insert(candidate);
							}
						}
					}
					if((layer_id+1)<number_of_layers_of_crads)
					{
						for(std::set<CRAD>::const_iterator jt=ceg.related_crads[layer_id+1].begin();jt!=ceg.related_crads[layer_id+1].end();++jt)
						{
							const CRADsPair candidate(*it, *jt);
							if(ceg.collapsed_related_crads_pairs.count(candidate)<1 && all_crads_pairs.count(candidate)>0)
							{
								ceg.related_crads_pairs[layer_id*2+2].insert(candidate);
								ceg.collapsed_related_crads_pairs.insert(candidate);
							}
						}
					}
				}
			}
		}



		return crad_effect_groupings;
	}
};

}

}

#endif /* COMMON_CONSTRUCTION_OF_CONTACT_EFFECT_GROUPINGS_H_ */
