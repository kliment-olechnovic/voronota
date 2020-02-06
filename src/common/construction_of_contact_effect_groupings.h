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

		void init(const std::size_t number_of_layers_of_crads, const CRAD& related_crad, const CRADsPair& related_crads_pair)
		{
			if(number_of_layers_of_crads<1)
			{
				return;
			}
			if(related_crads.size()!=number_of_layers_of_crads)
			{
				related_crads.resize(number_of_layers_of_crads);
			}
			if(related_crads_pairs.size()!=(number_of_layers_of_crads*2))
			{
				related_crads_pairs.resize(number_of_layers_of_crads*2);
			}
			related_crads[0].insert(related_crad);
			collapsed_related_crads.insert(related_crad);
			related_crads_pairs[0].insert(related_crads_pair);
			collapsed_related_crads_pairs.insert(related_crads_pair);
		}

		void update_related_crads(const std::size_t layer_id, const CRAD& related_crad)
		{
			if(layer_id<related_crads.size() && collapsed_related_crads.count(related_crad)<1)
			{
				related_crads[layer_id].insert(related_crad);
				collapsed_related_crads.insert(related_crad);
			}
		}

		void update_related_crads_pairs(const std::set<CRADsPair>& valid_crads_pairs, const std::size_t layer_id, const CRADsPair& related_crads_pair)
		{
			if(layer_id<related_crads_pairs.size() && collapsed_related_crads_pairs.count(related_crads_pair)<1 && valid_crads_pairs.count(related_crads_pair)>0)
			{
				related_crads_pairs[layer_id].insert(related_crads_pair);
				collapsed_related_crads_pairs.insert(related_crads_pair);
			}
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
			const CRADsPair crads_pair(it->a.without_atom(), it->b.without_atom());
			if(crads_pair.a!=CRAD::solvent() && crads_pair.b!=CRAD::solvent() && CRAD::match_with_sequence_separation_interval(crads_pair.a, crads_pair.b, 1, CRAD::null_num(), true))
			{
				all_crads_pairs.insert(crads_pair);
				crad_effect_groupings[crads_pair.a].init(number_of_layers_of_crads, crads_pair.b, crads_pair);
				crad_effect_groupings[crads_pair.b].init(number_of_layers_of_crads, crads_pair.a, crads_pair);
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
						ceg.update_related_crads(top_layer_id, *jt);
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
							ceg.update_related_crads_pairs(all_crads_pairs, (layer_id*2+1), CRADsPair(*it, *jt));
						}
					}
					if((layer_id+1)<number_of_layers_of_crads)
					{
						for(std::set<CRAD>::const_iterator jt=ceg.related_crads[layer_id+1].begin();jt!=ceg.related_crads[layer_id+1].end();++jt)
						{
							ceg.update_related_crads_pairs(all_crads_pairs, (layer_id*2+2), CRADsPair(*it, *jt));
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
