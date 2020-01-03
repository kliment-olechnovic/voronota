#ifndef SCRIPTING_OPERATORS_VOROMQA_LOCAL_H_
#define SCRIPTING_OPERATORS_VOROMQA_LOCAL_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class VoroMQALocal
{
public:
	struct Result
	{
		VariantObject atoms_result;
		VariantObject contacts_result;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.object("atoms_result")=atoms_result;
			if(!contacts_result.empty())
			{
				heterostorage.variant_object.object("contacts_result")=contacts_result;
			}
			return (*this);
		}
	};

	std::string selection_expresion_for_atoms;
	bool provided_selection_expresion_for_contacts;
	std::string selection_expresion_for_contacts;
	std::string adjunct_inter_atom_energy_scores_raw;
	std::string adjunct_atom_depth_weights;
	std::string adjunct_atom_quality_scores;

	VoroMQALocal() : provided_selection_expresion_for_contacts(false)
	{
	}

	VoroMQALocal& init(CommandInput& input)
	{
		selection_expresion_for_atoms=input.get_value_or_default<std::string>("atoms", "[]");
		provided_selection_expresion_for_contacts=input.is_option("contacts");
		selection_expresion_for_contacts=input.get_value_or_default<std::string>("contacts", "[]");
		adjunct_inter_atom_energy_scores_raw=input.get_value_or_default<std::string>("adj-contact-energy", "voromqa_energy");
		adjunct_atom_depth_weights=input.get_value_or_default<std::string>("adj-atom-depth", "voromqa_depth");
		adjunct_atom_quality_scores=input.get_value_or_default<std::string>("adj-atom-quality", "voromqa_score_a");
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(provided_selection_expresion_for_contacts)
		{
			data_manager.assert_contacts_availability();
		}

		assert_adjunct_name_input(adjunct_inter_atom_energy_scores_raw, false);
		assert_adjunct_name_input(adjunct_atom_depth_weights, false);
		assert_adjunct_name_input(adjunct_atom_quality_scores, false);

		std::set<std::size_t> atom_ids;
		std::set<std::size_t> contact_ids;

		atom_ids=data_manager.selection_manager().select_atoms(SelectionManager::Query(selection_expresion_for_atoms, false));

		if(!data_manager.contacts().empty())
		{
			contact_ids=data_manager.selection_manager().select_contacts(SelectionManager::Query(selection_expresion_for_contacts, false));
			atom_ids=data_manager.selection_manager().select_atoms_by_contacts(atom_ids, contact_ids, false);
			contact_ids=data_manager.selection_manager().select_contacts_by_atoms(contact_ids, atom_ids, false);
		}

		Result result;

		{
			if(atom_ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			const std::set<std::size_t> atom_ids_with_adjuncts=data_manager.selection_manager().select_atoms(
					SelectionManager::Query(atom_ids, (std::string("[")+"--adjuncts "+adjunct_atom_depth_weights+"&"+adjunct_atom_quality_scores+"]"), false));

			double sum_of_atom_weights=0.0;
			double sum_of_atom_weighted_scores=0.0;

			for(std::set<std::size_t>::const_iterator it=atom_ids_with_adjuncts.begin();it!=atom_ids_with_adjuncts.end();++it)
			{
				const std::size_t id=(*it);
				const double weight=data_manager.atoms()[id].value.props.adjuncts.find(adjunct_atom_depth_weights)->second;
				const double score=data_manager.atoms()[id].value.props.adjuncts.find(adjunct_atom_quality_scores)->second;
				sum_of_atom_weights+=weight;
				sum_of_atom_weighted_scores+=(weight*score);
			}

			const double quality_score=(sum_of_atom_weights>0.0 ? (sum_of_atom_weighted_scores/sum_of_atom_weights) : 0.0);

			result.atoms_result.value("atoms_selected").set_value_int(atom_ids.size());
			result.atoms_result.value("atoms_relevant").set_value_int(atom_ids_with_adjuncts.size());
			result.atoms_result.value("quality_score")=quality_score;
		}

		if(!data_manager.contacts().empty())
		{
			if(contact_ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			const std::set<std::size_t> contact_ids_with_adjuncts=data_manager.selection_manager().select_contacts(
					SelectionManager::Query(contact_ids, (std::string("[")+"--adjuncts "+adjunct_inter_atom_energy_scores_raw+"]"), false));

			double sum_of_areas=0.0;
			double sum_of_energies=0.0;

			for(std::set<std::size_t>::const_iterator it=contact_ids_with_adjuncts.begin();it!=contact_ids_with_adjuncts.end();++it)
			{
				const std::size_t id=(*it);
				const double area=data_manager.contacts()[id].value.area;
				const double energy=data_manager.contacts()[id].value.props.adjuncts.find(adjunct_inter_atom_energy_scores_raw)->second;
				sum_of_areas+=area;
				sum_of_energies+=energy;
			}

			result.contacts_result.value("contacts_selected").set_value_int(contact_ids.size());
			result.contacts_result.value("contacts_relevant").set_value_int(contact_ids_with_adjuncts.size());
			result.contacts_result.value("area")=sum_of_areas;
			result.contacts_result.value("pseudo_energy")=sum_of_energies;
		}

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_VOROMQA_LOCAL_H_ */
