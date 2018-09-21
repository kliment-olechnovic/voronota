#ifndef COMMON_SCRIPTING_SCORING_OF_DATA_MANAGERS_USING_CAD_SCORE_H_
#define COMMON_SCRIPTING_SCORING_OF_DATA_MANAGERS_USING_CAD_SCORE_H_

#include "../construction_of_cad_score.h"
#include "../conversion_of_descriptors.h"

#include "data_manager.h"

namespace common
{

namespace scripting
{

class ScoringOfDataManagersUsingCADScore
{
public:
	struct Parameters
	{
		bool ignore_residue_names;
		unsigned int smoothing_window;
		std::string target_sel;
		std::string model_sel;
		std::string target_atom_scores;
		std::string model_atom_scores;
		std::string target_inter_atom_scores;
		std::string model_inter_atom_scores;
		std::string target_residue_scores;
		std::string model_residue_scores;
		std::string target_inter_residue_scores;
		std::string model_inter_residue_scores;

		Parameters() :
			ignore_residue_names(false),
			smoothing_window(0)
		{
		}
	};

	struct Result
	{
		ConstructionOfCADScore::BundleOfCADScoreInformation bundle;
		DataManager::ChangeIndicator target_dm_ci;
		DataManager::ChangeIndicator model_dm_ci;
	};

	static void construct_result(const Parameters& params, DataManager& target_dm, DataManager& model_dm, Result& result)
	{
		result=Result();

		target_dm.assert_contacts_availability();
		model_dm.assert_contacts_availability();

		const std::set<std::size_t> target_ids=target_dm.selection_manager().select_contacts(std::set<std::size_t>(), params.target_sel, false);
		if(target_ids.empty())
		{
			throw std::runtime_error(std::string("No target contacts selected."));
		}

		const std::set<std::size_t> model_ids=model_dm.selection_manager().select_contacts(std::set<std::size_t>(), params.model_sel, false);
		if(model_ids.empty())
		{
			throw std::runtime_error(std::string("No model contacts selected."));
		}

		ConstructionOfCADScore::ParametersToConstructBundleOfCADScoreInformation parameters_for_cad_score;
		parameters_for_cad_score.ignore_residue_names=params.ignore_residue_names;
		parameters_for_cad_score.atom_level=!(
				params.target_atom_scores.empty()
				&& params.model_atom_scores.empty()
				&& params.target_inter_atom_scores.empty()
				&& params.model_inter_atom_scores.empty());

		if(!ConstructionOfCADScore::construct_bundle_of_cadscore_information(
				parameters_for_cad_score,
				collect_map_of_contacts(target_dm.atoms(), target_dm.contacts(), target_ids),
				collect_map_of_contacts(model_dm.atoms(), model_dm.contacts(), model_ids),
				result.bundle))
		{
			throw std::runtime_error(std::string("Failed to calculate CAD-score."));
		}

		write_adjuncts(result.bundle, params.smoothing_window, target_ids,
				params.target_atom_scores, params.target_residue_scores,
				params.target_inter_atom_scores, params.target_inter_residue_scores,
				target_dm, result.target_dm_ci);
		write_adjuncts(result.bundle, params.smoothing_window, model_ids,
				params.model_atom_scores, params.model_residue_scores,
				params.model_inter_atom_scores, params.model_inter_residue_scores,
				model_dm, result.model_dm_ci);
	}

private:
	static std::map<ChainResidueAtomDescriptorsPair, double> collect_map_of_contacts(
			const std::vector<Atom>& atoms,
			const std::vector<Contact>& contacts,
			const std::set<std::size_t>& contact_ids)
	{
		std::map<ChainResidueAtomDescriptorsPair, double> map_of_contacts;
		for(std::set<std::size_t>::const_iterator it_contact_ids=contact_ids.begin();it_contact_ids!=contact_ids.end();++it_contact_ids)
		{
			const std::size_t contact_id=(*it_contact_ids);
			if(contact_id<contacts.size())
			{
				const Contact& contact=contacts[contact_id];
				const ChainResidueAtomDescriptorsPair crads=ConversionOfDescriptors::get_contact_descriptor(atoms, contact);
				if(crads.valid())
				{
					map_of_contacts[crads]=contact.value.area;
				}
			}
		}
		return map_of_contacts;
	}

	static void write_adjuncts(
			const ConstructionOfCADScore::BundleOfCADScoreInformation& bundle,
			const unsigned int smoothing_window,
			const std::set<std::size_t>& contact_ids,
			const std::string& adjunct_atom_scores,
			const std::string& adjunct_residue_scores,
			const std::string& adjunct_inter_atom_scores,
			const std::string& adjunct_inter_residue_scores,
			DataManager& dm,
			DataManager::ChangeIndicator& dm_ci)
	{
		if(!adjunct_atom_scores.empty() || !adjunct_residue_scores.empty())
		{
			for(std::size_t i=0;i<dm.atoms_mutable().size();i++)
			{
				Atom& atom=dm.atoms_mutable()[i];
				if(!adjunct_atom_scores.empty())
				{
					std::map<ConstructionOfCADScore::CRAD, ConstructionOfCADScore::CADDescriptor>::const_iterator jt=
							bundle.map_of_atom_cad_descriptors.find(atom.crad);
					if(jt!=bundle.map_of_atom_cad_descriptors.end())
					{
						atom.value.props.adjuncts[adjunct_atom_scores]=jt->second.score();
						dm_ci.changed_atoms_adjuncts=true;
					}
				}
				if(!adjunct_residue_scores.empty())
				{
					std::map<ConstructionOfCADScore::CRAD, double> smoothed_residue_scores=bundle.residue_scores(smoothing_window);
					std::map<ConstructionOfCADScore::CRAD, double>::const_iterator jt=
							smoothed_residue_scores.find(atom.crad.without_some_info(true, true, false, bundle.parameters_of_construction.ignore_residue_names));
					if(jt!=smoothed_residue_scores.end())
					{
						atom.value.props.adjuncts[adjunct_residue_scores]=jt->second;
						dm_ci.changed_atoms_adjuncts=true;
					}
				}
			}
		}

		if(!adjunct_inter_atom_scores.empty() || !adjunct_inter_residue_scores.empty())
		{
			for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
			{
				Contact& contact=dm.contacts_mutable()[*it];
				const ConstructionOfCADScore::CRADsPair crads=ConversionOfDescriptors::get_contact_descriptor(dm.atoms(), contact);
				if(crads.valid())
				{
					if(!adjunct_inter_atom_scores.empty())
					{
						std::map<ConstructionOfCADScore::CRADsPair, ConstructionOfCADScore::CADDescriptor>::const_iterator jt=
								bundle.map_of_inter_atom_cad_descriptors.find(crads);
						if(jt!=bundle.map_of_inter_atom_cad_descriptors.end())
						{
							if(jt->second.target_area_sum>0.0)
							{
								contact.value.props.adjuncts[adjunct_inter_atom_scores]=jt->second.score();
								dm_ci.changed_contacts_adjuncts=true;
							}
						}
					}
					if(!adjunct_inter_residue_scores.empty())
					{
						const ConstructionOfCADScore::CRADsPair ir_crads(
								crads.a.without_some_info(true, true, false, bundle.parameters_of_construction.ignore_residue_names),
								crads.b.without_some_info(true, true, false, bundle.parameters_of_construction.ignore_residue_names));
						std::map<ConstructionOfCADScore::CRADsPair, ConstructionOfCADScore::CADDescriptor>::const_iterator jt=
								bundle.map_of_inter_residue_cad_descriptors.find(ir_crads);
						if(jt!=bundle.map_of_inter_residue_cad_descriptors.end())
						{
							if(jt->second.target_area_sum>0.0)
							{
								contact.value.props.adjuncts[adjunct_inter_residue_scores]=jt->second.score();
								dm_ci.changed_contacts_adjuncts=true;
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

#endif /* COMMON_SCRIPTING_SCORING_OF_DATA_MANAGERS_USING_CAD_SCORE_H_ */

