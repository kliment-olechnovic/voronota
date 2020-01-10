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
		unsigned int smoothing_window;
		std::string target_selection_expression;
		std::string model_selection_expression;
		std::string target_adjunct_atom_scores;
		std::string target_adjunct_inter_atom_scores;
		std::string target_adjunct_residue_scores;
		std::string target_adjunct_inter_residue_scores;
		std::string model_adjunct_atom_scores;
		std::string model_adjunct_inter_atom_scores;
		std::string model_adjunct_residue_scores;
		std::string model_adjunct_inter_residue_scores;

		Parameters() :
			ignore_residue_names(false),
			binarize(false),
			smoothing_window(0)
		{
		}
	};

	struct Result
	{
		common::ConstructionOfCADScore::BundleOfCADScoreInformation bundle;
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

		common::ConstructionOfCADScore::ParametersToConstructBundleOfCADScoreInformation parameters_for_cad_score;
		parameters_for_cad_score.ignore_residue_names=params.ignore_residue_names;
		parameters_for_cad_score.binarize=params.binarize;
		parameters_for_cad_score.atom_level=!(
				params.target_adjunct_atom_scores.empty()
				&& params.model_adjunct_atom_scores.empty()
				&& params.target_adjunct_inter_atom_scores.empty()
				&& params.model_adjunct_inter_atom_scores.empty());

		if(!common::ConstructionOfCADScore::construct_bundle_of_cadscore_information(
				parameters_for_cad_score,
				collect_map_of_contacts(target_dm.atoms(), target_dm.contacts(), target_contacts_ids),
				collect_map_of_contacts(model_dm.atoms(), model_dm.contacts(), model_contact_ids),
				result.bundle))
		{
			throw std::runtime_error(std::string("Failed to calculate CAD-score."));
		}

		write_adjuncts(result.bundle, params.smoothing_window, target_contacts_ids,
				params.target_adjunct_atom_scores, params.target_adjunct_residue_scores,
				params.target_adjunct_inter_atom_scores, params.target_adjunct_inter_residue_scores,
				target_dm);
		write_adjuncts(result.bundle, params.smoothing_window, model_contact_ids,
				params.model_adjunct_atom_scores, params.model_adjunct_residue_scores,
				params.model_adjunct_inter_atom_scores, params.model_adjunct_inter_residue_scores,
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

	static void write_adjuncts(
			const common::ConstructionOfCADScore::BundleOfCADScoreInformation& bundle,
			const unsigned int smoothing_window,
			const std::set<std::size_t>& contact_ids,
			const std::string& adjunct_atom_scores,
			const std::string& adjunct_residue_scores,
			const std::string& adjunct_inter_atom_scores,
			const std::string& adjunct_inter_residue_scores,
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

		if(!adjunct_inter_atom_scores.empty() || !adjunct_inter_residue_scores.empty())
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
			}
			for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
			{
				const Contact& contact=dm.contacts()[*it];
				std::map<std::string, double>& contact_adjuncts=dm.contact_adjuncts_mutable(*it);
				const common::ConstructionOfCADScore::CRADsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(dm.atoms(), contact);
				if(crads.valid())
				{
					if(!adjunct_inter_atom_scores.empty())
					{
						std::map<common::ConstructionOfCADScore::CRADsPair, common::ConstructionOfCADScore::CADDescriptor>::const_iterator jt=
								bundle.map_of_inter_atom_cad_descriptors.find(crads);
						if(jt!=bundle.map_of_inter_atom_cad_descriptors.end() && jt->second.target_area_sum>0.0)
						{
							contact_adjuncts[adjunct_inter_atom_scores]=jt->second.score();
						}
					}
					if(!adjunct_inter_residue_scores.empty())
					{
						const common::ConstructionOfCADScore::CRADsPair ir_crads(
								crads.a.without_some_info(true, true, false, bundle.parameters_of_construction.ignore_residue_names),
								crads.b.without_some_info(true, true, false, bundle.parameters_of_construction.ignore_residue_names));
						std::map<common::ConstructionOfCADScore::CRADsPair, common::ConstructionOfCADScore::CADDescriptor>::const_iterator jt=
								bundle.map_of_inter_residue_cad_descriptors.find(ir_crads);
						if(jt!=bundle.map_of_inter_residue_cad_descriptors.end() && jt->second.target_area_sum>0.0)
						{
							contact_adjuncts[adjunct_inter_residue_scores]=jt->second.score();
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

