#ifndef DUKTAPER_OPERATORS_CONGRUENCE_SCORE_H_
#define DUKTAPER_OPERATORS_CONGRUENCE_SCORE_H_

#include "../../dependencies/utilities/congruence_coefficient.h"

#include "../../../../src/scripting/operators/cad_score.h"

#include "../operators_common.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class CongruenceScore : public scripting::OperatorBase<CongruenceScore>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::string target_name;
		std::string model_name;
		double congruence_coefficient;

		Result() : congruence_coefficient(0.0)
		{
		}

		void store(scripting::VariantObject& variant_object) const
		{
			variant_object.value("target_name")=target_name;
			variant_object.value("model_name")=model_name;
			variant_object.value("congruence_coefficient")=congruence_coefficient;
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			store(heterostorage.variant_object);
		}
	};

	std::string target_name;
	std::string model_name;
	std::string target_selection_expression;
	std::string model_selection_expression;
	std::string target_global_adj_prefix;
	std::string model_global_adj_prefix;

	CongruenceScore()
	{
	}

	void initialize(scripting::CommandInput& input, const bool managed)
	{
		if(managed)
		{
			target_name=input.get_value_or_first_unused_unnamed_value_or_default("target", "");
		}
		else
		{
			target_name=input.get_value_or_first_unused_unnamed_value("target");
			model_name=input.get_value_or_first_unused_unnamed_value("model");
		}
		target_selection_expression=input.get_value_or_default<std::string>("t-sel", "[--no-solvent --min-seq-sep 1]");
		model_selection_expression=input.get_value_or_default<std::string>("m-sel", target_selection_expression);
		if(!managed)
		{
			target_global_adj_prefix=input.get_value_or_default<std::string>("t-global-adj-prefix", "");
		}
		model_global_adj_prefix=input.get_value_or_default<std::string>("m-global-adj-prefix", "");
	}

	void initialize(scripting::CommandInput& input)
	{
		initialize(input, false);
	}

	void document(scripting::CommandDocumentation& doc, const bool managed) const
	{
		if(managed)
		{
			doc.set_option_decription(CDOD("target", CDOD::DATATYPE_STRING, "name of target object", ""));
		}
		else
		{
			doc.set_option_decription(CDOD("target", CDOD::DATATYPE_STRING, "name of target object"));
			doc.set_option_decription(CDOD("model", CDOD::DATATYPE_STRING, "name of model object"));
		}
		doc.set_option_decription(CDOD("t-sel", CDOD::DATATYPE_STRING, "selection of contacts for target object", "[--no-solvent --min-seq-sep 1]"));
		doc.set_option_decription(CDOD("m-sel", CDOD::DATATYPE_STRING, "selection of contacts for model object", "[--no-solvent --min-seq-sep 1]"));
		if(!managed)
		{
			doc.set_option_decription(CDOD("t-global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts of target", ""));
		}
		doc.set_option_decription(CDOD("m-global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts of model", ""));
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		document(doc, false);
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(target_name.empty())
		{
			throw std::runtime_error(std::string("No target object name provided."));
		}

		if(model_name.empty())
		{
			throw std::runtime_error(std::string("No model object name provided."));
		}

		congregation_of_data_managers.assert_object_availability(target_name);
		congregation_of_data_managers.assert_object_availability(model_name);

		scripting::DataManager& target_dm=*congregation_of_data_managers.get_object(target_name);
		scripting::DataManager& model_dm=*congregation_of_data_managers.get_object(model_name);

		target_dm.assert_contacts_availability();
		model_dm.assert_contacts_availability();

		const std::set<std::size_t> target_contacts_ids=target_dm.selection_manager().select_contacts(scripting::SelectionManager::Query(target_selection_expression, false));
		if(target_contacts_ids.empty())
		{
			throw std::runtime_error(std::string("No target contacts selected."));
		}

		const std::set<std::size_t> model_contacts_ids=model_dm.selection_manager().select_contacts(scripting::SelectionManager::Query(model_selection_expression, false));
		if(model_contacts_ids.empty())
		{
			throw std::runtime_error(std::string("No model contacts selected."));
		}

		const std::map<common::ChainResidueAtomDescriptorsPair, double> target_map_of_inter_residue_contacts=collect_map_of_inter_residue_contacts(target_dm.atoms(), target_dm.contacts(), target_contacts_ids);
		if(target_map_of_inter_residue_contacts.empty())
		{
			throw std::runtime_error(std::string("No target inter-residue contacts selected."));
		}

		const std::map<common::ChainResidueAtomDescriptorsPair, double> model_map_of_inter_residue_contacts=collect_map_of_inter_residue_contacts(model_dm.atoms(), model_dm.contacts(), model_contacts_ids);
		if(model_map_of_inter_residue_contacts.empty())
		{
			throw std::runtime_error(std::string("No model inter-residue contacts selected."));
		}

		const std::map<common::ChainResidueAtomDescriptor, std::size_t> map_of_residue_ids_to_indices=collect_map_of_residue_ids_to_indices(target_map_of_inter_residue_contacts, model_map_of_inter_residue_contacts);

		const congruence_coefficient::AdjacencyMap target_adjacency_map=convert_map_of_inter_residue_contacts_to_adjacency_map(map_of_residue_ids_to_indices, target_map_of_inter_residue_contacts);
		if(target_adjacency_map.empty())
		{
			throw std::runtime_error(std::string("Failed to prepare target adjacency map."));
		}

		const congruence_coefficient::AdjacencyMap model_adjacency_map=convert_map_of_inter_residue_contacts_to_adjacency_map(map_of_residue_ids_to_indices, model_map_of_inter_residue_contacts);
		if(model_adjacency_map.empty())
		{
			throw std::runtime_error(std::string("Failed to prepare model adjacency map."));
		}

		const double congruence_coefficient=congruence_coefficient::calculate_congruence_coefficient_of_two_square_symmetric_matrices(target_adjacency_map, model_adjacency_map);

		if(!target_global_adj_prefix.empty())
		{
			target_dm.global_numeric_adjuncts_mutable()[target_global_adj_prefix+"_congruence_coefficient"]=congruence_coefficient;
		}

		if(!model_global_adj_prefix.empty())
		{
			model_dm.global_numeric_adjuncts_mutable()[model_global_adj_prefix+"_congruence_coefficient"]=congruence_coefficient;
		}

		Result result;
		result.target_name=target_name;
		result.model_name=model_name;
		result.congruence_coefficient=congruence_coefficient;

		return result;
	}

private:
	static std::map<common::ChainResidueAtomDescriptorsPair, double> collect_map_of_inter_residue_contacts(
			const std::vector<scripting::Atom>& atoms,
			const std::vector<scripting::Contact>& contacts,
			const std::set<std::size_t>& contact_ids)
	{
		std::map<common::ChainResidueAtomDescriptorsPair, double> map_of_contacts;
		for(std::set<std::size_t>::const_iterator it_contact_ids=contact_ids.begin();it_contact_ids!=contact_ids.end();++it_contact_ids)
		{
			const std::size_t contact_id=(*it_contact_ids);
			if(contact_id<contacts.size())
			{
				const scripting::Contact& contact=contacts[contact_id];
				const common::ChainResidueAtomDescriptorsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(atoms, contact);
				if(crads.valid())
				{
					map_of_contacts[crads.without_some_info(true, true, false, true)]+=contact.value.area;
				}
			}
		}
		return map_of_contacts;
	}

	static std::map<common::ChainResidueAtomDescriptor, std::size_t> collect_map_of_residue_ids_to_indices(
			const std::map<common::ChainResidueAtomDescriptorsPair, double> contacts1,
			const std::map<common::ChainResidueAtomDescriptorsPair, double> contacts2)
	{
		std::map<common::ChainResidueAtomDescriptor, std::size_t> result;
		for(std::map<common::ChainResidueAtomDescriptorsPair, double>::const_iterator it=contacts1.begin();it!=contacts1.end();++it)
		{
			result[it->first.a]=0;
			result[it->first.b]=0;
		}
		for(std::map<common::ChainResidueAtomDescriptorsPair, double>::const_iterator it=contacts2.begin();it!=contacts2.end();++it)
		{
			result[it->first.a]=0;
			result[it->first.b]=0;
		}
		std::size_t index=0;
		for(std::map<common::ChainResidueAtomDescriptor, std::size_t>::iterator it=result.begin();it!=result.end();++it)
		{
			it->second=index;
			index++;
		}
		return result;
	}

	static congruence_coefficient::AdjacencyMap convert_map_of_inter_residue_contacts_to_adjacency_map(
			const std::map<common::ChainResidueAtomDescriptor, std::size_t>& map_of_residue_ids_to_indices,
			const std::map<common::ChainResidueAtomDescriptorsPair, double> map_of_inter_residue_contacts)
	{
		congruence_coefficient::AdjacencyMap result;
		for(std::map<common::ChainResidueAtomDescriptorsPair, double>::const_iterator it=map_of_inter_residue_contacts.begin();it!=map_of_inter_residue_contacts.end();++it)
		{
			std::map<common::ChainResidueAtomDescriptor, std::size_t>::const_iterator it1=map_of_residue_ids_to_indices.find(it->first.a);
			if(it1==map_of_residue_ids_to_indices.end())
			{
				throw std::runtime_error(std::string("Incomplete mapping of residue ids to indices."));
			}
			std::map<common::ChainResidueAtomDescriptor, std::size_t>::const_iterator it2=map_of_residue_ids_to_indices.find(it->first.b);
			if(it2==map_of_residue_ids_to_indices.end())
			{
				throw std::runtime_error(std::string("Incomplete mapping of residue ids to indices."));
			}
			result[congruence_coefficient::Adjacency(it1->second, it2->second)]=it->second;
		}
		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_CONGRUENCE_SCORE_H_ */
