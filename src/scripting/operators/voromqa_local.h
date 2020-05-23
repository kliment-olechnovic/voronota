#ifndef SCRIPTING_OPERATORS_VOROMQA_LOCAL_H_
#define SCRIPTING_OPERATORS_VOROMQA_LOCAL_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class VoroMQALocal : public OperatorBase<VoroMQALocal>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		VariantObject atoms_result;
		VariantObject contacts_result;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.object("atoms_result")=atoms_result;
			if(!contacts_result.empty())
			{
				heterostorage.variant_object.object("contacts_result")=contacts_result;
			}
		}
	};

	std::string selection_expresion_for_atoms;
	bool provided_selection_expresion_for_contacts;
	std::string selection_expresion_for_contacts;
	std::string adjunct_inter_atom_energy_scores_raw;
	std::string adjunct_atom_depth_weights;
	std::string adjunct_atom_quality_scores;
	std::string global_adj_prefix;

	VoroMQALocal() : provided_selection_expresion_for_contacts(false)
	{
	}

	void initialize(CommandInput& input)
	{
		selection_expresion_for_atoms=input.get_value_or_default<std::string>("atoms", "[]");
		provided_selection_expresion_for_contacts=input.is_option("contacts");
		selection_expresion_for_contacts=input.get_value_or_default<std::string>("contacts", "[]");
		adjunct_inter_atom_energy_scores_raw=input.get_value_or_default<std::string>("adj-contact-energy", "voromqa_energy");
		adjunct_atom_depth_weights=input.get_value_or_default<std::string>("adj-atom-depth", "voromqa_depth");
		adjunct_atom_quality_scores=input.get_value_or_default<std::string>("adj-atom-quality", "voromqa_score_a");
		global_adj_prefix=input.get_value_or_default<std::string>("global-adj-prefix", "voromqa_local");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("atoms", CDOD::DATATYPE_STRING, "selection expression for atoms", "[]"));
		doc.set_option_decription(CDOD("contacts", CDOD::DATATYPE_STRING, "selection expression for contacts", ""));
		doc.set_option_decription(CDOD("adj-contact-energy", CDOD::DATATYPE_STRING, "name of input adjunct with contact energy values", "voromqa_energy"));
		doc.set_option_decription(CDOD("adj-atom-depth", CDOD::DATATYPE_STRING, "name of input adjunct with atom values", "voromqa_depth"));
		doc.set_option_decription(CDOD("adj-atom-quality", CDOD::DATATYPE_STRING, "name of input adjunct with atom quality scores", "voromqa_score_a"));
		doc.set_option_decription(CDOD("global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts", "voromqa_local"));
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

		data_manager.assert_contacts_adjunct_availability(adjunct_inter_atom_energy_scores_raw);
		data_manager.assert_atoms_adjunct_availability(adjunct_atom_depth_weights);
		data_manager.assert_atoms_adjunct_availability(adjunct_atom_quality_scores);

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

			if(!global_adj_prefix.empty())
			{
				data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_atoms_count"]=atom_ids_with_adjuncts.size();
				data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_residues_count"]=data_manager.count_residues_by_atom_ids(atom_ids_with_adjuncts);
				data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_atoms_quality_score"]=quality_score;
			}

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

			if(!global_adj_prefix.empty())
			{
				data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_contacts_count"]=contact_ids_with_adjuncts.size();
				data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_contacts_area"]=sum_of_areas;
				data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_contacts_pseudo_energy"]=sum_of_energies;
				if(sum_of_areas>0.0)
				{
					data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_contacts_pseudo_energy_norm"]=sum_of_energies/sum_of_areas;
				}
				else
				{
					data_manager.global_numeric_adjuncts_mutable().erase(global_adj_prefix+"_contacts_pseudo_energy_norm");
				}
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

}

#endif /* SCRIPTING_OPERATORS_VOROMQA_LOCAL_H_ */
