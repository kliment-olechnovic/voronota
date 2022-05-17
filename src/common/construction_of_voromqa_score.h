#ifndef COMMON_CONSTRUCTION_OF_VOROMQA_SCORE_H_
#define COMMON_CONSTRUCTION_OF_VOROMQA_SCORE_H_

#include "contacts_scoring_utilities.h"
#include "statistics_utilities.h"

namespace voronota
{

namespace common
{

class ConstructionOfVoroMQAScore
{
public:
	typedef ChainResidueAtomDescriptor CRAD;
	typedef ChainResidueAtomDescriptorsPair CRADsPair;

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
			const std::vector< std::map<InteractionName, double> >& input_maps_of_potential_values,
			const std::vector< std::map<InteractionName, double> >& input_maps_of_contacts,
			BundleOfVoroMQAEnergyInformation& bundle)
	{
		bundle=BundleOfVoroMQAEnergyInformation();
		bundle.parameters_of_construction=parameters;

		if(input_maps_of_potential_values.empty() || input_maps_of_potential_values.size()!=input_maps_of_contacts.size())
		{
			return false;
		}

		{
			bool all_empty=true;
			for(std::size_t i=0;i<input_maps_of_contacts.size() && all_empty;i++)
			{
				if(!input_maps_of_contacts[i].empty())
				{
					if(input_maps_of_potential_values[i].empty())
					{
						return false;
					}
					all_empty=false;
				}
			}
			if(all_empty)
			{
				return false;
			}
		}

		for(std::size_t i=0;i<input_maps_of_contacts.size();i++)
		{
			for(std::map<InteractionName, double>::const_iterator it=input_maps_of_contacts[i].begin();it!=input_maps_of_contacts[i].end();++it)
			{
				const CRADsPair& crads=it->first.crads;
				EnergyDescriptor& ed=bundle.inter_atom_energy_descriptors[crads];
				if(!CRAD::match_with_sequence_separation_interval(crads.a, crads.b, 0, parameters.ignorable_max_seq_sep, false) && !check_crads_pair_for_peptide_bond(crads))
				{
					const double contact_area=(it->second);
					ed.total_area+=contact_area;
					ed.contacts_count=1;
					InteractionName potential_interaction_name(generalize_crads_pair(crads), it->first.tag);
					std::map<InteractionName, double>::const_iterator potential_value_it=input_maps_of_potential_values[i].find(potential_interaction_name);
					if(potential_value_it!=input_maps_of_potential_values[i].end())
					{
						ed.energy+=contact_area*(potential_value_it->second);
					}
					else
					{
						const std::string& initial_tag=it->first.tag;
						if(initial_tag=="central_sep1")
						{
							potential_interaction_name.tag="central_sep2";
							potential_value_it=input_maps_of_potential_values[i].find(potential_interaction_name);
						}
						else if(initial_tag=="central_sep2")
						{
							potential_interaction_name.tag="central_sep1";
							potential_value_it=input_maps_of_potential_values[i].find(potential_interaction_name);
						}
						if(potential_value_it!=input_maps_of_potential_values[i].end())
						{
							ed.energy+=contact_area*(potential_value_it->second);
						}
						else
						{
							ed.strange_area+=contact_area;
						}
					}
				}
			}
		}

		bundle.atom_energy_descriptors=ChainResidueAtomDescriptorsGraphOperations::accumulate_mapped_values_by_graph_neighbors(
				bundle.inter_atom_energy_descriptors, parameters.depth);

		for(std::map<CRADsPair, EnergyDescriptor>::const_iterator it=bundle.inter_atom_energy_descriptors.begin();it!=bundle.inter_atom_energy_descriptors.end();++it)
		{
			bundle.global_energy_descriptor.add(it->second);
		}

		return true;
	}

	struct ParametersToConstructBundleOfVoroMQAQualityInformation
	{
		double default_mean;
		double default_sd;
		double mean_shift;

		ParametersToConstructBundleOfVoroMQAQualityInformation() :
			default_mean(0.4),
			default_sd(0.3),
			mean_shift(0.0)
		{
		}
	};

	struct BundleOfVoroMQAQualityInformation
	{
		ParametersToConstructBundleOfVoroMQAQualityInformation parameters_of_construction;
		std::map<CRAD, double> atom_quality_scores;
		std::map<CRAD, double> raw_residue_quality_scores;

		std::map<CRAD, double> residue_quality_scores(const unsigned int smoothing_window) const
		{
			return ChainResidueAtomDescriptorsSequenceOperations::smooth_residue_scores_along_sequence(
					raw_residue_quality_scores, smoothing_window);
		}

		template<typename MapOfWeights>
		std::pair<double, double> global_quality_score_with_weight(const MapOfWeights& external_weights, bool only_with_weights) const
		{
			double sum_of_weighted_scores=0.0;
			double sum_of_weights=0.0;

			for(std::map<CRAD, double>::const_iterator it=atom_quality_scores.begin();it!=atom_quality_scores.end();++it)
			{
				const CRAD& crad=it->first;
				const double unweighted_quality_score=it->second;
				typename MapOfWeights::const_iterator external_weights_it=external_weights.find(crad);
				if(external_weights_it!=external_weights.end())
				{
					const double external_weight=external_weights_it->second;
					sum_of_weighted_scores+=(unweighted_quality_score*external_weight);
					sum_of_weights+=external_weight;
				}
				else if(!only_with_weights)
				{
					sum_of_weighted_scores+=unweighted_quality_score;
					sum_of_weights+=1.0;
				}
			}

			return std::pair<double, double>((sum_of_weights>0.0 ? sum_of_weighted_scores/sum_of_weights : 0.0), sum_of_weights);
		}

		template<typename MapOfWeights>
		double global_quality_score(const MapOfWeights& external_weights, bool only_with_weights) const
		{
			return global_quality_score_with_weight(external_weights, only_with_weights).first;
		}
	};

	static bool construct_bundle_of_voromqa_quality_information(
			const ParametersToConstructBundleOfVoroMQAQualityInformation& parameters,
			const std::map<CRAD, NormalDistributionParameters>& means_and_sds,
			const std::map<CRAD, EnergyDescriptor>& atom_energy_descriptors,
			BundleOfVoroMQAQualityInformation& bundle)
	{
		bundle=BundleOfVoroMQAQualityInformation();
		bundle.parameters_of_construction=parameters;

		if(atom_energy_descriptors.empty())
		{
			return false;
		}

		for(std::map<CRAD, EnergyDescriptor>::const_iterator it=atom_energy_descriptors.begin();it!=atom_energy_descriptors.end();++it)
		{
			const CRAD& crad=it->first;
			const EnergyDescriptor& ed=it->second;
			if(ed.total_area>0.0)
			{
				const double actuality_score=(1.0-(ed.strange_area/ed.total_area));
				const double normalized_energy=(ed.energy/ed.total_area);
				std::map<CRAD, NormalDistributionParameters>::const_iterator mean_and_sd_it=means_and_sds.find(generalize_crad(crad));
				const double mean=(mean_and_sd_it!=means_and_sds.end() ? mean_and_sd_it->second.mean : parameters.default_mean);
				const double sd=(mean_and_sd_it!=means_and_sds.end() ? mean_and_sd_it->second.sd : parameters.default_sd);
				const double adjusted_normalized_energy=((normalized_energy-mean)/sd);
				const double energy_score=(1.0-(0.5*(1.0+erf((adjusted_normalized_energy-parameters.mean_shift)/sqrt(2.0)))));
				const double unweighted_quality_score=(energy_score*actuality_score);
				bundle.atom_quality_scores[crad]=unweighted_quality_score;
			}
			else
			{
				bundle.atom_quality_scores[crad]=0.0;
			}
		}

		bundle.raw_residue_quality_scores=average_atom_scores_by_residue(bundle.atom_quality_scores);

		return true;
	}

private:
	static std::map<CRAD, double> average_atom_scores_by_residue(const std::map<CRAD, double>& atom_scores)
	{
		std::map<CRAD, std::pair<int, double> > summed_scores;
		for(std::map<CRAD, double>::const_iterator it=atom_scores.begin();it!=atom_scores.end();++it)
		{
			std::pair<int, double>& value=summed_scores[it->first.without_atom()];
			value.first++;
			value.second+=it->second;
		}
		std::map<CRAD, double> average_scores;
		for(std::map<CRAD, std::pair<int, double> >::const_iterator it=summed_scores.begin();it!=summed_scores.end();++it)
		{
			const std::pair<int, double>& value=it->second;
			average_scores[it->first]=((value.first>0) ? (value.second/static_cast<double>(value.first)) : 0.0);
		}
		return average_scores;
	}
};


}

}

#endif /* COMMON_CONSTRUCTION_OF_VOROMQA_SCORE_H_ */

