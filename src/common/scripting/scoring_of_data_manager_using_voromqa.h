#ifndef COMMON_SCRIPTING_SCORING_OF_DATA_MANAGER_USING_VOROMQA_H_
#define COMMON_SCRIPTING_SCORING_OF_DATA_MANAGER_USING_VOROMQA_H_

#include "../construction_of_voromqa_score.h"
#include "../conversion_of_descriptors.h"

#include "data_manager.h"

namespace common
{

namespace scripting
{

class ScoringOfDataManagerUsingVoroMQA
{
public:
	class Configuration
	{
	public:
		std::map<InteractionName, double> potential_values;
		std::map<ChainResidueAtomDescriptor, NormalDistributionParameters> means_and_sds;

		bool valid() const
		{
			return (!potential_values.empty() && !means_and_sds.empty());
		}

		static const Configuration& get_default_configuration()
		{
			return get_default_configuration_mutable();
		}

		static bool setup_default_configuration(const std::string& potential_file, const std::string& mean_and_sds_file)
		{
			if(potential_file.empty() || mean_and_sds_file.empty())
			{
				return false;
			}

			std::map<InteractionName, double> potential_values=
					auxiliaries::IOUtilities().read_file_lines_to_map< std::map<InteractionName, double> >(potential_file);
			if(potential_values.empty())
			{
				return false;
			}

			std::map<ChainResidueAtomDescriptor, NormalDistributionParameters> means_and_sds=
					auxiliaries::IOUtilities().read_file_lines_to_map< std::map<ChainResidueAtomDescriptor, NormalDistributionParameters> >(mean_and_sds_file);
			if(means_and_sds.empty())
			{
				return false;
			}

			get_default_configuration_mutable().potential_values.swap(potential_values);
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
		std::string adjunct_inter_atom_energy_scores_raw;
		std::string adjunct_inter_atom_energy_scores_normalized;
		std::string adjunct_atom_energy_scores_raw;
		std::string adjunct_atom_energy_scores_normalized;
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
		ConstructionOfVoroMQAScore::BundleOfVoroMQAEnergyInformation bundle_of_energy;
		ConstructionOfVoroMQAScore::BundleOfVoroMQAQualityInformation bundle_of_quality;
		DataManager::ChangeIndicator data_manager_change_index;

		Result() :
			global_quality_score(0.0)
		{
		}
	};

	static void construct_result(const Parameters& params, DataManager& data_manager, Result& result)
	{
		result=Result();

		if(!Configuration::get_default_configuration().valid())
		{
			throw std::runtime_error(std::string("No default configuration."));
		}

		data_manager.assert_contacts_availability();

		std::string selection_expression=""
				"( "
				" { "
				"  --min-seq-sep 1 "
				"  --atom1 { --match R<LEU,ALA,GLY,VAL,GLU,SER,LYS,ILE,ASP,THR,ARG,PRO,ASN,PHE,GLN,TYR,HIS,MET,TRP,CYS,MSE> } "
				"  --atom2 { --match R<LEU,ALA,GLY,VAL,GLU,SER,LYS,ILE,ASP,THR,ARG,PRO,ASN,PHE,GLN,TYR,HIS,MET,TRP,CYS,MSE>|c<solvent> } "
				" } "
				" and "
				" ( "
				"  not { --atom1 { --match A<C> } --atom2 { --match A<N> } --max-dist 1.6 --max-seq-sep 1 } "
				" ) "
				")";

		const std::set<std::size_t> all_contact_ids=data_manager.selection_manager().select_contacts(selection_expression, false);

		if(all_contact_ids.empty())
		{
			throw std::runtime_error(std::string("No suitable contacts."));
		}

		std::set<std::size_t> all_atom_ids;
		std::set<ChainResidueAtomDescriptorsPair> set_of_crads;
		std::map<InteractionName, double> map_of_interactions;

		for(std::set<std::size_t>::const_iterator it=all_contact_ids.begin();it!=all_contact_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];
			const ChainResidueAtomDescriptorsPair crads=ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), contact);

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

				if(ChainResidueAtomDescriptor::match_with_sequence_separation_interval(crads.a, crads.b, 1, 1, true))
				{
					category+="sep1";
				}
				else
				{
					category+="sep2";
				}
			}

			map_of_interactions[InteractionName(crads, category)]=contact.value.area;
		}

		ConstructionOfVoroMQAScore::ParametersToConstructBundleOfVoroMQAEnergyInformation parameters_for_bundle_of_energy;
		if(!ConstructionOfVoroMQAScore::construct_bundle_of_voromqa_energy_information(
				parameters_for_bundle_of_energy, Configuration::get_default_configuration().potential_values, map_of_interactions, result.bundle_of_energy))
		{
			throw std::runtime_error("Failed to calculate energies scores.");
		}

		const std::map<ChainResidueAtomDescriptor, int> map_crad_to_depth=
				ChainResidueAtomDescriptorsGraphOperations::calculate_burial_depth_values(set_of_crads);
		if(map_crad_to_depth.empty())
		{
			throw std::runtime_error(std::string("Failed to calculate burial depths."));
		}

		ConstructionOfVoroMQAScore::ParametersToConstructBundleOfVoroMQAQualityInformation parameters_for_bundle_of_quality;
		if(!common::ConstructionOfVoroMQAScore::construct_bundle_of_voromqa_quality_information(
				parameters_for_bundle_of_quality, Configuration::get_default_configuration().means_and_sds, result.bundle_of_energy.atom_energy_descriptors, result.bundle_of_quality))
		{
			throw std::runtime_error("Failed to calculate quality scores.");
		}

		result.global_quality_score=result.bundle_of_quality.global_quality_score(map_crad_to_depth, false);

		if(!params.adjunct_inter_atom_energy_scores_raw.empty() || params.adjunct_inter_atom_energy_scores_normalized.empty())
		{
			result.data_manager_change_index.changed_contacts_adjuncts=true;

			for(std::size_t i=0;i<data_manager.contacts_mutable().size();i++)
			{
				Contact& contact=data_manager.contacts_mutable()[i];
				if(!params.adjunct_inter_atom_energy_scores_raw.empty())
				{
					contact.value.props.adjuncts.erase(params.adjunct_inter_atom_energy_scores_raw);
				}
				if(!params.adjunct_inter_atom_energy_scores_normalized.empty())
				{
					contact.value.props.adjuncts.erase(params.adjunct_inter_atom_energy_scores_normalized);
				}
			}

			for(std::set<std::size_t>::const_iterator jt=all_contact_ids.begin();jt!=all_contact_ids.end();++jt)
			{
				Contact& contact=data_manager.contacts_mutable()[*jt];
				const ChainResidueAtomDescriptorsPair crads=ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), contact);
				std::map<ChainResidueAtomDescriptorsPair, EnergyDescriptor>::const_iterator it=result.bundle_of_energy.inter_atom_energy_descriptors.find(crads);

				if(!params.adjunct_inter_atom_energy_scores_raw.empty())
				{
					if(it!=result.bundle_of_energy.inter_atom_energy_descriptors.end())
					{
						const EnergyDescriptor& ed=it->second;
						if(ed.total_area>0.0 && ed.strange_area==0.0)
						{
							contact.value.props.adjuncts[params.adjunct_inter_atom_energy_scores_raw]=ed.energy;
						}
					}
				}

				if(!params.adjunct_inter_atom_energy_scores_normalized.empty())
				{
					if(it!=result.bundle_of_energy.inter_atom_energy_descriptors.end())
					{
						const EnergyDescriptor& ed=it->second;
						if(ed.total_area>0.0 && ed.strange_area==0.0)
						{
							contact.value.props.adjuncts[params.adjunct_inter_atom_energy_scores_normalized]=ed.energy/ed.total_area;
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
			for(std::size_t i=0;i<data_manager.atoms_mutable().size();i++)
			{
				Atom& atom=data_manager.atoms_mutable()[i];
				if(!params.adjunct_atom_depth_weights.empty())
				{
					atom.value.props.adjuncts.erase(params.adjunct_atom_depth_weights);
				}
				if(!params.adjunct_atom_quality_scores.empty())
				{
					atom.value.props.adjuncts.erase(params.adjunct_atom_quality_scores);
				}
				if(!params.adjunct_residue_quality_scores_raw.empty())
				{
					atom.value.props.adjuncts.erase(params.adjunct_residue_quality_scores_raw);
				}
				if(!params.adjunct_residue_quality_scores_smoothed.empty())
				{
					atom.value.props.adjuncts.erase(params.adjunct_residue_quality_scores_smoothed);
				}
			}

			std::map<ChainResidueAtomDescriptor, double> smoothed_scores;
			if(!params.adjunct_residue_quality_scores_smoothed.empty())
			{
				smoothed_scores=result.bundle_of_quality.residue_quality_scores(params.smoothing_window);
			}

			for(std::set<std::size_t>::const_iterator jt=all_atom_ids.begin();jt!=all_atom_ids.end();++jt)
			{
				Atom& atom=data_manager.atoms_mutable()[*jt];
				if(!params.adjunct_atom_depth_weights.empty())
				{
					std::map<ChainResidueAtomDescriptor, int>::const_iterator it=map_crad_to_depth.find(atom.crad);
					if(it!=map_crad_to_depth.end())
					{
						atom.value.props.adjuncts[params.adjunct_atom_depth_weights]=it->second;
					}
				}
				if(!params.adjunct_atom_quality_scores.empty())
				{
					std::map<ChainResidueAtomDescriptor, double>::const_iterator it=result.bundle_of_quality.atom_quality_scores.find(atom.crad);
					if(it!=result.bundle_of_quality.atom_quality_scores.end())
					{
						atom.value.props.adjuncts[params.adjunct_atom_quality_scores]=it->second;
					}
				}
				if(!params.adjunct_residue_quality_scores_raw.empty())
				{
					std::map<ChainResidueAtomDescriptor, double>::const_iterator it=result.bundle_of_quality.raw_residue_quality_scores.find(atom.crad.without_atom());
					if(it!=result.bundle_of_quality.raw_residue_quality_scores.end())
					{
						atom.value.props.adjuncts[params.adjunct_residue_quality_scores_raw]=it->second;
					}
				}
				if(!params.adjunct_residue_quality_scores_smoothed.empty())
				{
					std::map<ChainResidueAtomDescriptor, double>::const_iterator it=smoothed_scores.find(atom.crad.without_atom());
					if(it!=smoothed_scores.end())
					{
						atom.value.props.adjuncts[params.adjunct_residue_quality_scores_smoothed]=it->second;
					}
				}
			}
		}
	}
};

}

}

#endif /* COMMON_SCRIPTING_SCORING_OF_DATA_MANAGER_USING_VOROMQA_H_ */

