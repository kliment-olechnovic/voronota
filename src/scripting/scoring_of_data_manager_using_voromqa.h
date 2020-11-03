#ifndef SCRIPTING_SCORING_OF_DATA_MANAGER_USING_VOROMQA_H_
#define SCRIPTING_SCORING_OF_DATA_MANAGER_USING_VOROMQA_H_

#include "../common/construction_of_voromqa_score.h"
#include "../common/conversion_of_descriptors.h"

#include "virtual_file_storage.h"
#include "data_manager.h"

namespace voronota
{

namespace scripting
{

class ScoringOfDataManagerUsingVoroMQA
{
public:
	class Configuration
	{
	public:
		std::vector< std::map<common::InteractionName, double> > potential_values;
		std::map<common::ChainResidueAtomDescriptor, common::NormalDistributionParameters> means_and_sds;

		bool valid() const
		{
			return (!potential_values.empty() && !means_and_sds.empty());
		}

		static const Configuration& get_default_configuration()
		{
			return get_default_configuration_mutable();
		}

		static bool setup_default_configuration(const std::string& potential_file, const std::string& potential_alt_file, const std::string& mean_and_sds_file)
		{
			if(potential_file.empty() || mean_and_sds_file.empty())
			{
				return false;
			}

			std::map<common::InteractionName, double> potential_values;
			{
				InputSelector potential_file_input_selector(potential_file);
				potential_values=auxiliaries::IOUtilities().read_lines_to_map< std::map<common::InteractionName, double> >(potential_file_input_selector.stream());
				if(potential_values.empty())
				{
					return false;
				}
			}

			std::map<common::InteractionName, double> potential_alt_values;
			if(!potential_alt_file.empty())
			{
				InputSelector potential_alt_file_input_selector(potential_alt_file);
				potential_alt_values=auxiliaries::IOUtilities().read_lines_to_map< std::map<common::InteractionName, double> >(potential_alt_file_input_selector.stream());
				if(potential_alt_values.empty())
				{
					return false;
				}
			}

			std::map<common::ChainResidueAtomDescriptor, common::NormalDistributionParameters> means_and_sds;
			{
				InputSelector mean_and_sds_file_input_selector(mean_and_sds_file);
				means_and_sds=auxiliaries::IOUtilities().read_lines_to_map< std::map<common::ChainResidueAtomDescriptor, common::NormalDistributionParameters> >(mean_and_sds_file_input_selector.stream());
				if(means_and_sds.empty())
				{
					return false;
				}
			}

			if(potential_alt_values.empty())
			{
				get_default_configuration_mutable().potential_values.resize(1);
				get_default_configuration_mutable().potential_values[0].swap(potential_values);
			}
			else
			{
				get_default_configuration_mutable().potential_values.resize(2);
				get_default_configuration_mutable().potential_values[0].swap(potential_values);
				get_default_configuration_mutable().potential_values[1].swap(potential_alt_values);
			}

			get_default_configuration_mutable().means_and_sds.swap(means_and_sds);

			return true;
		}

	private:
		static Configuration& get_default_configuration_mutable()
		{
			static Configuration configuration;
			return configuration;
		}
	};

	struct Parameters
	{
		unsigned int smoothing_window;
		std::string adjunct_area_scale;
		std::string adjunct_area_alt_part;
		std::string adjunct_inter_atom_energy_scores_raw;
		std::string adjunct_inter_atom_energy_scores_normalized;
		std::string adjunct_atom_depth_weights;
		std::string adjunct_atom_quality_scores;
		std::string adjunct_residue_quality_scores_raw;
		std::string adjunct_residue_quality_scores_smoothed;

		Parameters() :
			smoothing_window(5)
		{
		}
	};

	struct Result
	{
		double global_quality_score;
		double weight_of_global_quality_score;
		common::ConstructionOfVoroMQAScore::BundleOfVoroMQAEnergyInformation bundle_of_energy;
		std::map<common::ChainResidueAtomDescriptor, int> map_crad_to_depth;
		common::ConstructionOfVoroMQAScore::BundleOfVoroMQAQualityInformation bundle_of_quality;
		DataManager::ChangeIndicator data_manager_change_index;

		Result() :
			global_quality_score(0.0),
			weight_of_global_quality_score(0.0)
		{
		}
	};

	static void construct_result(const Configuration& configuration, const Parameters& params, DataManager& data_manager, Result& result)
	{
		result=Result();

		if(!configuration.valid())
		{
			throw std::runtime_error(std::string("Invalid configuration."));
		}

		data_manager.assert_contacts_availability();

		std::string selection_expression=""
				"( "
				" [ "
				"  --min-seq-sep 1 "
				"  --atom1 [ --match R<LEU,ALA,GLY,VAL,GLU,SER,LYS,ILE,ASP,THR,ARG,PRO,ASN,PHE,GLN,TYR,HIS,MET,TRP,CYS,MSE> ] "
				"  --atom2 [ --match R<LEU,ALA,GLY,VAL,GLU,SER,LYS,ILE,ASP,THR,ARG,PRO,ASN,PHE,GLN,TYR,HIS,MET,TRP,CYS,MSE>|c<solvent> ] "
				" ] "
				" and "
				" ( "
				"  not [ --atom1 [ --match A<C> ] --atom2 [ --match A<N> ] --max-dist 1.6 --max-seq-sep 1 ] "
				" ) "
				")";

		const std::set<std::size_t> all_contact_ids=data_manager.selection_manager().select_contacts(SelectionManager::Query(selection_expression, false));

		if(all_contact_ids.empty())
		{
			throw std::runtime_error(std::string("No suitable contacts."));
		}

		std::set<std::size_t> all_atom_ids;
		std::set<common::ChainResidueAtomDescriptorsPair> set_of_crads;
		std::vector< std::map<common::InteractionName, double> > maps_of_interactions(configuration.potential_values.size());

		for(std::set<std::size_t>::const_iterator it=all_contact_ids.begin();it!=all_contact_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];
			const common::ChainResidueAtomDescriptorsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), contact);

			all_atom_ids.insert(contact.ids[0]);
			all_atom_ids.insert(contact.ids[1]);

			set_of_crads.insert(crads);

			std::string category;

			if(contact.solvent())
			{
				category=".";
			}
			else
			{
				if(contact.value.props.tags.count("central")>0)
				{
					category="central_";
				}

				if(common::ChainResidueAtomDescriptor::match_with_sequence_separation_interval(crads.a, crads.b, 1, 1, true))
				{
					category+="sep1";
				}
				else
				{
					category+="sep2";
				}
			}

			double contact_area_total=contact.value.area;

			if(!params.adjunct_area_scale.empty())
			{
				std::map<std::string, double>::const_iterator as_it=contact.value.props.adjuncts.find(params.adjunct_area_scale);
				if(as_it!=contact.value.props.adjuncts.end())
				{
					const double scale_value=std::max(0.0, std::min(1.0, as_it->second));
					contact_area_total*=scale_value;
				}
			}

			if(maps_of_interactions.size()==1)
			{
				maps_of_interactions[0][common::InteractionName(crads, category)]=contact_area_total;
			}
			else
			{
				double contact_area_standard=contact_area_total;
				double contact_area_alt=0.0;

				if(!params.adjunct_area_alt_part.empty())
				{
					std::map<std::string, double>::const_iterator ap_it=contact.value.props.adjuncts.find(params.adjunct_area_alt_part);
					if(ap_it!=contact.value.props.adjuncts.end())
					{
						const double part_value=std::max(0.0, std::min(1.0, ap_it->second));
						contact_area_alt=contact_area_total*part_value;
						contact_area_standard=(contact_area_total-contact_area_alt);
					}
				}

				if(contact_area_standard>0.0)
				{
					maps_of_interactions[0][common::InteractionName(crads, category)]=contact_area_standard;
				}

				if(contact_area_alt>0.0)
				{
					maps_of_interactions[1][common::InteractionName(crads, category)]=contact_area_alt;
				}
			}
		}

		common::ConstructionOfVoroMQAScore::ParametersToConstructBundleOfVoroMQAEnergyInformation parameters_for_bundle_of_energy;
		if(!common::ConstructionOfVoroMQAScore::construct_bundle_of_voromqa_energy_information(
				parameters_for_bundle_of_energy, configuration.potential_values, maps_of_interactions, result.bundle_of_energy))
		{
			throw std::runtime_error("Failed to calculate energies scores.");
		}

		result.map_crad_to_depth=
				common::ChainResidueAtomDescriptorsGraphOperations::calculate_burial_depth_values(set_of_crads);
		if(result.map_crad_to_depth.empty())
		{
			throw std::runtime_error(std::string("Failed to calculate burial depths."));
		}

		common::ConstructionOfVoroMQAScore::ParametersToConstructBundleOfVoroMQAQualityInformation parameters_for_bundle_of_quality;
		if(!common::ConstructionOfVoroMQAScore::construct_bundle_of_voromqa_quality_information(
				parameters_for_bundle_of_quality, configuration.means_and_sds, result.bundle_of_energy.atom_energy_descriptors, result.bundle_of_quality))
		{
			throw std::runtime_error("Failed to calculate quality scores.");
		}

		{
			std::pair<double, double> global_quality_score_with_weight=result.bundle_of_quality.global_quality_score_with_weight(result.map_crad_to_depth, false);
			result.global_quality_score=global_quality_score_with_weight.first;
			result.weight_of_global_quality_score=global_quality_score_with_weight.second;
		}

		if(!params.adjunct_inter_atom_energy_scores_raw.empty() || params.adjunct_inter_atom_energy_scores_normalized.empty())
		{
			for(std::size_t i=0;i<data_manager.contacts().size();i++)
			{
				std::map<std::string, double>& contact_adjuncts=data_manager.contact_adjuncts_mutable(i);
				if(!params.adjunct_inter_atom_energy_scores_raw.empty())
				{
					contact_adjuncts.erase(params.adjunct_inter_atom_energy_scores_raw);
				}
				if(!params.adjunct_inter_atom_energy_scores_normalized.empty())
				{
					contact_adjuncts.erase(params.adjunct_inter_atom_energy_scores_normalized);
				}
			}

			for(std::set<std::size_t>::const_iterator jt=all_contact_ids.begin();jt!=all_contact_ids.end();++jt)
			{
				const Contact& contact=data_manager.contacts()[*jt];
				std::map<std::string, double>& contact_adjuncts=data_manager.contact_adjuncts_mutable(*jt);
				const common::ChainResidueAtomDescriptorsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), contact);
				std::map<common::ChainResidueAtomDescriptorsPair, common::EnergyDescriptor>::const_iterator it=result.bundle_of_energy.inter_atom_energy_descriptors.find(crads);

				if(!params.adjunct_inter_atom_energy_scores_raw.empty())
				{
					if(it!=result.bundle_of_energy.inter_atom_energy_descriptors.end())
					{
						const common::EnergyDescriptor& ed=it->second;
						if(ed.total_area>0.0 && ed.strange_area==0.0)
						{
							contact_adjuncts[params.adjunct_inter_atom_energy_scores_raw]=ed.energy;
						}
					}
				}

				if(!params.adjunct_inter_atom_energy_scores_normalized.empty())
				{
					if(it!=result.bundle_of_energy.inter_atom_energy_descriptors.end())
					{
						const common::EnergyDescriptor& ed=it->second;
						if(ed.total_area>0.0 && ed.strange_area==0.0)
						{
							contact_adjuncts[params.adjunct_inter_atom_energy_scores_normalized]=ed.energy/ed.total_area;
						}
					}
				}
			}
		}

		if(!params.adjunct_atom_depth_weights.empty()
				|| !params.adjunct_atom_quality_scores.empty()
				|| !params.adjunct_residue_quality_scores_raw.empty()
				|| !params.adjunct_residue_quality_scores_smoothed.empty())
		{
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(i);
				if(!params.adjunct_atom_depth_weights.empty())
				{
					atom_adjuncts.erase(params.adjunct_atom_depth_weights);
				}
				if(!params.adjunct_atom_quality_scores.empty())
				{
					atom_adjuncts.erase(params.adjunct_atom_quality_scores);
				}
				if(!params.adjunct_residue_quality_scores_raw.empty())
				{
					atom_adjuncts.erase(params.adjunct_residue_quality_scores_raw);
				}
				if(!params.adjunct_residue_quality_scores_smoothed.empty())
				{
					atom_adjuncts.erase(params.adjunct_residue_quality_scores_smoothed);
				}
			}

			std::map<common::ChainResidueAtomDescriptor, double> smoothed_scores;
			if(!params.adjunct_residue_quality_scores_smoothed.empty())
			{
				smoothed_scores=result.bundle_of_quality.residue_quality_scores(params.smoothing_window);
			}

			for(std::set<std::size_t>::const_iterator jt=all_atom_ids.begin();jt!=all_atom_ids.end();++jt)
			{
				const Atom& atom=data_manager.atoms()[*jt];
				std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*jt);
				if(!params.adjunct_atom_depth_weights.empty())
				{
					std::map<common::ChainResidueAtomDescriptor, int>::const_iterator it=result.map_crad_to_depth.find(atom.crad);
					if(it!=result.map_crad_to_depth.end())
					{
						atom_adjuncts[params.adjunct_atom_depth_weights]=it->second;
					}
				}
				if(!params.adjunct_atom_quality_scores.empty())
				{
					std::map<common::ChainResidueAtomDescriptor, double>::const_iterator it=result.bundle_of_quality.atom_quality_scores.find(atom.crad);
					if(it!=result.bundle_of_quality.atom_quality_scores.end())
					{
						atom_adjuncts[params.adjunct_atom_quality_scores]=it->second;
					}
				}
				if(!params.adjunct_residue_quality_scores_raw.empty())
				{
					std::map<common::ChainResidueAtomDescriptor, double>::const_iterator it=result.bundle_of_quality.raw_residue_quality_scores.find(atom.crad.without_atom());
					if(it!=result.bundle_of_quality.raw_residue_quality_scores.end())
					{
						atom_adjuncts[params.adjunct_residue_quality_scores_raw]=it->second;
					}
				}
				if(!params.adjunct_residue_quality_scores_smoothed.empty())
				{
					std::map<common::ChainResidueAtomDescriptor, double>::const_iterator it=smoothed_scores.find(atom.crad.without_atom());
					if(it!=smoothed_scores.end())
					{
						atom_adjuncts[params.adjunct_residue_quality_scores_smoothed]=it->second;
					}
				}
			}
		}
	}

	static void construct_result(const Parameters& params, DataManager& data_manager, Result& result)
	{
		construct_result(Configuration::get_default_configuration(), params, data_manager, result);
	}
};

}

}

#endif /* SCRIPTING_SCORING_OF_DATA_MANAGER_USING_VOROMQA_H_ */

