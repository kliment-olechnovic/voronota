#ifndef COMMON_CONSTRUCTION_OF_VOROMQA_SCORE_H_
#define COMMON_CONSTRUCTION_OF_VOROMQA_SCORE_H_

#include "contacts_scoring_utilities.h"
#include "conversion_of_descriptors.h"

namespace common
{

class ConstructionOfVoroMQAScore
{
public:
	typedef ChainResidueAtomDescriptor CRAD;
	typedef ChainResidueAtomDescriptorsPair CRADsPair;
	typedef ConstructionOfAtomicBalls::AtomicBall Atom;
	typedef ConstructionOfContacts::Contact Contact;

	struct ParametersToConstructBundleOfVoroMQAEnergyInformation
	{
		int ignorable_max_seq_sep;
		int depth;

		ParametersToConstructBundleOfVoroMQAEnergyInformation() :
			ignorable_max_seq_sep(1),
			depth(2)
		{
		}
	};

	struct BundleOfVoroMQAEnergyInformation
	{
		ParametersToConstructBundleOfVoroMQAEnergyInformation parameters_of_construction;
		std::map<CRADsPair, EnergyDescriptor> inter_atom_energy_descriptors;
		std::map<CRAD, EnergyDescriptor> atom_energy_descriptors;
		EnergyDescriptor global_energy_descriptor;
	};

	static bool construct_bundle_of_voromqa_energy_information(
			const ParametersToConstructBundleOfVoroMQAEnergyInformation& parameters,
			const std::map<InteractionName, double>& input_map_of_potential_values,
			const std::map<InteractionName, double>& input_map_of_contacts,
			BundleOfVoroMQAEnergyInformation& bundle)
	{
		bundle=BundleOfVoroMQAEnergyInformation();
		bundle.parameters_of_construction=parameters;

		if(input_map_of_potential_values.empty() || input_map_of_contacts.empty())
		{
			return false;
		}

		for(std::map<InteractionName, double>::const_iterator it=input_map_of_contacts.begin();it!=input_map_of_contacts.end();++it)
		{
			const CRADsPair& crads=it->first.crads;
			EnergyDescriptor& ed=bundle.inter_atom_energy_descriptors[crads];
			if(!CRAD::match_with_sequence_separation_interval(crads.a, crads.b, 0, parameters.ignorable_max_seq_sep, false) && !common::check_crads_pair_for_peptide_bond(crads))
			{
				ed.total_area=(it->second);
				ed.contacts_count=1;
				std::map<InteractionName, double>::const_iterator potential_value_it=
						input_map_of_potential_values.find(InteractionName(common::generalize_crads_pair(crads), it->first.tag));
				if(potential_value_it!=input_map_of_potential_values.end())
				{
					ed.energy=ed.total_area*(potential_value_it->second);
				}
				else
				{
					ed.strange_area=ed.total_area;
				}
			}
		}

		bundle.atom_energy_descriptors=common::ChainResidueAtomDescriptorsGraphOperations::accumulate_mapped_values_by_graph_neighbors(
				bundle.inter_atom_energy_descriptors, parameters.depth);

		for(std::map<CRADsPair, EnergyDescriptor>::const_iterator it=bundle.inter_atom_energy_descriptors.begin();it!=bundle.inter_atom_energy_descriptors.end();++it)
		{
			bundle.global_energy_descriptor.add(it->second);
		}

		return true;
	}
};


}

#endif /* COMMON_CONSTRUCTION_OF_VOROMQA_SCORE_H_ */

