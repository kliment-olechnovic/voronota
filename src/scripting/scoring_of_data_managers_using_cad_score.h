#ifndef SCRIPTING_SCORING_OF_DATA_MANAGERS_USING_CAD_SCORE_H_
#define SCRIPTING_SCORING_OF_DATA_MANAGERS_USING_CAD_SCORE_H_

#include "../common/construction_of_cad_score.h"
#include "../common/conversion_of_descriptors.h"

#include "data_manager.h"

namespace voronota
{

namespace scripting
{

class ScoringOfDataManagersUsingCADScore
{
public:
	struct Parameters
	{
		bool ignore_residue_names;
		bool binarize;
		int depth;
		int max_chains_to_fully_permute;
		unsigned int smoothing_window;
		bool also_site_based;
		bool remap_chains;
		bool remap_chains_logging;
		std::string target_selection_expression;
		std::string model_selection_expression;
		std::string site_selection_expression;
		std::vector<std::string> chain_renaming_pairs;
		std::string target_adjunct_atom_scores;
		std::string target_adjunct_inter_atom_scores;
		std::string target_adjunct_inter_atom_relevant_areas;
		std::string target_adjunct_residue_scores;
		std::string target_adjunct_inter_residue_scores;
		std::string target_adjunct_inter_residue_relevant_areas;
		std::string model_adjunct_atom_scores;
		std::string model_adjunct_inter_atom_scores;
		std::string model_adjunct_inter_atom_relevant_areas;
		std::string model_adjunct_residue_scores;
		std::string model_adjunct_inter_residue_scores;
		std::string model_adjunct_inter_residue_relevant_areas;

		Parameters() :
			ignore_residue_names(false),
			binarize(false),
			depth(0),
			max_chains_to_fully_permute(6),
			smoothing_window(0),
			also_site_based(false),
			remap_chains(false),
			remap_chains_logging(false)
		{
		}
	};

	struct Result
	{
		common::ConstructionOfCADScore::BundleOfCADScoreInformation bundle;
		common::ConstructionOfCADScore::BundleOfCADScoreInformation site_bundle;
	};

	static void construct_result(const Parameters& params, DataManager& target_dm, DataManager& model_dm, Result& result)
	{
		result=Result();

		target_dm.assert_contacts_availability();
		model_dm.assert_contacts_availability();

		const std::set<std::size_t> target_contacts_ids=target_dm.selection_manager().select_contacts(SelectionManager::Query(params.target_selection_expression, false));
		if(target_contacts_ids.empty())
		{
			throw std::runtime_error(std::string("No target contacts selected."));
		}

		const std::set<std::size_t> model_contact_ids=model_dm.selection_manager().select_contacts(SelectionManager::Query(params.model_selection_expression, false));
		if(model_contact_ids.empty())
		{
			throw std::runtime_error(std::string("No model contacts selected."));
		}

		const bool need_nonempty_site_selections=(params.also_site_based && !params.site_selection_expression.empty() && params.site_selection_expression!="[]");

		const std::set<std::size_t> target_site_atom_ids=(need_nonempty_site_selections ? target_dm.selection_manager().select_atoms(SelectionManager::Query(params.site_selection_expression, false)) : std::set<std::size_t>());

		if(need_nonempty_site_selections && target_site_atom_ids.empty())
		{
			throw std::runtime_error(std::string("No target site atoms selected."));
		}

		const std::set<std::size_t> model_site_atom_ids=(need_nonempty_site_selections ? model_dm.selection_manager().select_atoms(SelectionManager::Query(params.site_selection_expression, false)) : std::set<std::size_t>());

		if(need_nonempty_site_selections && model_site_atom_ids.empty())
		{
			throw std::runtime_error(std::string("No model site atoms selected."));
		}

		common::ConstructionOfCADScore::ParametersToConstructBundleOfCADScoreInformation parameters_for_cad_score;

		parameters_for_cad_score.ignore_residue_names=params.ignore_residue_names;
		parameters_for_cad_score.binarize=params.binarize;
		parameters_for_cad_score.depth=params.depth;
		parameters_for_cad_score.max_chains_to_fully_permute=params.max_chains_to_fully_permute;
		parameters_for_cad_score.atom_level=!(
				params.target_adjunct_atom_scores.empty()
				&& params.model_adjunct_atom_scores.empty()
				&& params.target_adjunct_inter_atom_scores.empty()
				&& params.model_adjunct_inter_atom_scores.empty()
				&& params.model_adjunct_inter_atom_relevant_areas.empty());

		if(!params.chain_renaming_pairs.empty())
		{
			if(params.chain_renaming_pairs.size()%2!=0)
			{
				throw std::runtime_error(std::string("Invalid chain renaming pairs vector size."));
			}
			for(std::size_t i=0;i+1<params.chain_renaming_pairs.size();i++)
			{
				parameters_for_cad_score.map_of_renamings[params.chain_renaming_pairs[i]]=params.chain_renaming_pairs[i+1];
			}
		}

		parameters_for_cad_score.remap_chains=params.remap_chains;
		parameters_for_cad_score.remap_chains_logging=params.remap_chains_logging;

		if(!common::ConstructionOfCADScore::construct_bundle_of_cadscore_information(
				parameters_for_cad_score,
				collect_map_of_contacts(target_dm.atoms(), target_dm.contacts(), target_contacts_ids),
				collect_map_of_contacts(model_dm.atoms(), model_dm.contacts(), model_contact_ids),
				result.bundle))
		{
			throw std::runtime_error(std::string("Failed to calculate CAD-score."));
		}

		if(params.also_site_based)
		{
			if(params.remap_chains && !result.bundle.map_of_renamings_from_remapping.empty())
			{
				parameters_for_cad_score.map_of_renamings=result.bundle.map_of_renamings_from_remapping;
				parameters_for_cad_score.remap_chains=false;
			}

			if(!common::ConstructionOfCADScore::construct_bundle_of_cadscore_information(
					parameters_for_cad_score,
					collect_map_of_contacts_summarized_by_first(target_dm.atoms(), target_dm.contacts(), target_contacts_ids, target_site_atom_ids),
					collect_map_of_contacts_summarized_by_first(model_dm.atoms(), model_dm.contacts(), model_contact_ids, model_site_atom_ids),
					result.site_bundle))
			{
				throw std::runtime_error(std::string("Failed to calculate site-based CAD-score."));
			}
		}

		write_adjuncts(result.bundle, params.smoothing_window, target_contacts_ids,
				params.target_adjunct_atom_scores, params.target_adjunct_residue_scores,
				params.target_adjunct_inter_atom_scores, params.target_adjunct_inter_residue_scores,
				params.target_adjunct_inter_atom_relevant_areas, params.target_adjunct_inter_residue_relevant_areas,
				target_dm);
		write_adjuncts(result.bundle, params.smoothing_window, model_contact_ids,
				params.model_adjunct_atom_scores, params.model_adjunct_residue_scores,
				params.model_adjunct_inter_atom_scores, params.model_adjunct_inter_residue_scores,
				params.model_adjunct_inter_atom_relevant_areas, params.model_adjunct_inter_residue_relevant_areas,
				model_dm);
	}

private:
	static std::map<common::ChainResidueAtomDescriptorsPair, double> collect_map_of_contacts(
			const std::vector<Atom>& atoms,
			const std::vector<Contact>& contacts,
			const std::set<std::size_t>& contact_ids)
	{
		std::map<common::ChainResidueAtomDescriptorsPair, double> map_of_contacts;
		for(std::set<std::size_t>::const_iterator it_contact_ids=contact_ids.begin();it_contact_ids!=contact_ids.end();++it_contact_ids)
		{
			const std::size_t contact_id=(*it_contact_ids);
			if(contact_id<contacts.size())
			{
				const Contact& contact=contacts[contact_id];
				const common::ChainResidueAtomDescriptorsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(atoms, contact);
				if(crads.valid())
				{
					map_of_contacts[crads]=contact.value.area;
				}
			}
		}
		return map_of_contacts;
	}

	static std::map<common::ChainResidueAtomDescriptorsPair, double> collect_map_of_contacts_summarized_by_first(
			const std::vector<Atom>& atoms,
			const std::vector<Contact>& contacts,
			const std::set<std::size_t>& contact_ids,
			const std::set<std::size_t>& site_atom_ids)
	{
		std::map<common::ChainResidueAtomDescriptorsPair, double> map_of_contacts;
		for(std::set<std::size_t>::const_iterator it_contact_ids=contact_ids.begin();it_contact_ids!=contact_ids.end();++it_contact_ids)
		{
			const std::size_t contact_id=(*it_contact_ids);
			if(contact_id<contacts.size())
			{
				const Contact& contact=contacts[contact_id];
				if(site_atom_ids.empty())
				{
					const common::ChainResidueAtomDescriptorsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(atoms, contact);
					if(crads.valid())
					{
						map_of_contacts[common::ChainResidueAtomDescriptorsPair(crads.a, common::ChainResidueAtomDescriptor::any())]+=contact.value.area;
						map_of_contacts[common::ChainResidueAtomDescriptorsPair(crads.b, common::ChainResidueAtomDescriptor::any())]+=contact.value.area;
					}
				}
				else
				{
					if(site_atom_ids.count(contact.ids[0])>0)
					{
						map_of_contacts[common::ChainResidueAtomDescriptorsPair(atoms[contact.ids[0]].crad, common::ChainResidueAtomDescriptor::any())]+=contact.value.area;
					}
					if(site_atom_ids.count(contact.ids[1])>0)
					{
						map_of_contacts[common::ChainResidueAtomDescriptorsPair(atoms[contact.ids[1]].crad, common::ChainResidueAtomDescriptor::any())]+=contact.value.area;
					}
				}
			}
		}
		return map_of_contacts;
	}

	static void write_adjuncts(
			const common::ConstructionOfCADScore::BundleOfCADScoreInformation& bundle,
			const unsigned int smoothing_window,
			const std::set<std::size_t>& contact_ids,
			const std::string& adjunct_atom_scores,
			const std::string& adjunct_residue_scores,
			const std::string& adjunct_inter_atom_scores,
			const std::string& adjunct_inter_residue_scores,
			const std::string& adjunct_inter_atom_relevant_areas,
			const std::string& adjunct_inter_residue_relevant_areas,
			DataManager& dm)
	{
		if(!adjunct_atom_scores.empty())
		{
			for(std::size_t i=0;i<dm.atoms().size();i++)
			{
				const Atom& atom=dm.atoms()[i];
				std::map<std::string, double>& atom_adjuncts=dm.atom_adjuncts_mutable(i);
				if(!adjunct_atom_scores.empty())
				{
					std::map<common::ConstructionOfCADScore::CRAD, common::ConstructionOfCADScore::CADDescriptor>::const_iterator jt=
							bundle.map_of_atom_cad_descriptors.find(atom.crad);
					if(jt!=bundle.map_of_atom_cad_descriptors.end())
					{
						atom_adjuncts[adjunct_atom_scores]=jt->second.score();
					}
					else
					{
						atom_adjuncts.erase(adjunct_atom_scores);
					}
				}
			}
		}

		if(!adjunct_residue_scores.empty())
		{
			const std::map<common::ConstructionOfCADScore::CRAD, double> smoothed_residue_scores=bundle.residue_scores(smoothing_window);
			for(std::size_t i=0;i<dm.atoms().size();i++)
			{
				const Atom& atom=dm.atoms()[i];
				std::map<std::string, double>& atom_adjuncts=dm.atom_adjuncts_mutable(i);
				if(!adjunct_residue_scores.empty())
				{
					std::map<common::ConstructionOfCADScore::CRAD, double>::const_iterator jt=
							smoothed_residue_scores.find(atom.crad.without_some_info(true, true, false, bundle.parameters_of_construction.ignore_residue_names));
					if(jt!=smoothed_residue_scores.end())
					{
						atom_adjuncts[adjunct_residue_scores]=jt->second;
					}
					else
					{
						atom_adjuncts.erase(adjunct_residue_scores);
					}
				}
			}
		}

		if(!adjunct_inter_atom_scores.empty() || !adjunct_inter_residue_scores.empty() || !adjunct_inter_atom_relevant_areas.empty() || !adjunct_inter_residue_relevant_areas.empty())
		{
			for(std::size_t i=0;i<dm.contacts().size();i++)
			{
				std::map<std::string, double>& contact_adjuncts=dm.contact_adjuncts_mutable(i);
				if(!adjunct_inter_atom_scores.empty())
				{
					contact_adjuncts.erase(adjunct_inter_atom_scores);
				}
				if(!adjunct_inter_residue_scores.empty())
				{
					contact_adjuncts.erase(adjunct_inter_residue_scores);
				}
				if(!adjunct_inter_atom_relevant_areas.empty())
				{
					contact_adjuncts.erase(adjunct_inter_atom_relevant_areas);
				}
				if(!adjunct_inter_residue_relevant_areas.empty())
				{
					contact_adjuncts.erase(adjunct_inter_residue_relevant_areas);
				}
			}
			for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
			{
				const Contact& contact=dm.contacts()[*it];
				std::map<std::string, double>& contact_adjuncts=dm.contact_adjuncts_mutable(*it);
				const common::ConstructionOfCADScore::CRADsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(dm.atoms(), contact);
				if(crads.valid())
				{
					if(!adjunct_inter_atom_scores.empty() || !adjunct_inter_atom_relevant_areas.empty())
					{
						std::map<common::ConstructionOfCADScore::CRADsPair, common::ConstructionOfCADScore::CADDescriptor>::const_iterator jt=
								bundle.map_of_inter_atom_cad_descriptors.find(crads);
						if(jt!=bundle.map_of_inter_atom_cad_descriptors.end())
						{
							if(!adjunct_inter_atom_scores.empty() && jt->second.target_area_sum>0.0)
							{
								contact_adjuncts[adjunct_inter_atom_scores]=jt->second.score();
							}
							if(!adjunct_inter_atom_relevant_areas.empty())
							{
								contact_adjuncts[adjunct_inter_atom_relevant_areas+"_t"]=jt->second.target_area_sum;
								contact_adjuncts[adjunct_inter_atom_relevant_areas+"_m"]=jt->second.model_area_sum;
							}
						}
					}
					if(!adjunct_inter_residue_scores.empty() || !adjunct_inter_residue_relevant_areas.empty())
					{
						const common::ConstructionOfCADScore::CRADsPair ir_crads(
								crads.a.without_some_info(true, true, false, bundle.parameters_of_construction.ignore_residue_names),
								crads.b.without_some_info(true, true, false, bundle.parameters_of_construction.ignore_residue_names));
						std::map<common::ConstructionOfCADScore::CRADsPair, common::ConstructionOfCADScore::CADDescriptor>::const_iterator jt=
								bundle.map_of_inter_residue_cad_descriptors.find(ir_crads);
						if(jt!=bundle.map_of_inter_residue_cad_descriptors.end())
						{
							if(!adjunct_inter_residue_scores.empty() && jt->second.target_area_sum>0.0)
							{
								contact_adjuncts[adjunct_inter_residue_scores]=jt->second.score();
							}
							if(!adjunct_inter_residue_relevant_areas.empty())
							{
								contact_adjuncts[adjunct_inter_residue_relevant_areas+"_t"]=jt->second.target_area_sum;
								contact_adjuncts[adjunct_inter_residue_relevant_areas+"_m"]=jt->second.model_area_sum;
							}
						}
					}
				}
			}
		}
	}
};

}

}

#endif /* SCRIPTING_SCORING_OF_DATA_MANAGERS_USING_CAD_SCORE_H_ */

