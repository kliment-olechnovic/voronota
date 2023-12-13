#ifndef SCRIPTING_OPERATORS_DELETE_OBJECTS_IF_H_
#define SCRIPTING_OPERATORS_DELETE_OBJECTS_IF_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class DeleteObjectsIf : public OperatorBase<DeleteObjectsIf>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<std::string> deleted_objects;

		void store(HeterogeneousStorage& heterostorage) const
		{
			for(std::size_t i=0;i<deleted_objects.size();i++)
			{
				heterostorage.variant_object.values_array("deleted_objects").push_back(VariantValue(deleted_objects[i]));
			}
		}
	};

	unsigned int atoms_fewer_than;
	unsigned int residues_fewer_than;
	unsigned int chains_fewer_than;
	unsigned int objects_fewer_than;
	double atoms_to_residues_ratio_less_than;
	unsigned int min_atoms_in_selection;
	unsigned int min_contacts_in_selection;
	std::string not_enough_atoms_in_selection;
	std::string not_enough_contacts_in_selection;
	CongregationOfDataManagers::ObjectQuery query;

	DeleteObjectsIf() : atoms_fewer_than(1), residues_fewer_than(1), chains_fewer_than(1), objects_fewer_than(1), atoms_to_residues_ratio_less_than(0.0), min_atoms_in_selection(1), min_contacts_in_selection(1)
	{
	}

	void initialize(CommandInput& input)
	{
		query=OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		atoms_fewer_than=input.get_value_or_default<unsigned int>("atoms-fewer-than", 1);
		residues_fewer_than=input.get_value_or_default<unsigned int>("residues-fewer-than", 1);
		chains_fewer_than=input.get_value_or_default<unsigned int>("chains-fewer-than", 1);
		objects_fewer_than=input.get_value_or_default<unsigned int>("objects-fewer-than", 1);
		atoms_to_residues_ratio_less_than=input.get_value_or_default<double>("atoms-to-residues-ratio-less-than", 0.0);
		min_atoms_in_selection=input.get_value_or_default<unsigned int>("min-atoms-in-selection", 1);
		min_contacts_in_selection=input.get_value_or_default<unsigned int>("min-contacts-in-selection", 1);
		not_enough_atoms_in_selection=input.get_value_or_default<std::string>("not-enough-atoms-in-selection", "");
		not_enough_contacts_in_selection=input.get_value_or_default<std::string>("not-enough-contacts-in-selection", "");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		doc.set_option_decription(CDOD("atoms-fewer-than", CDOD::DATATYPE_INT, "minimal allowed number of atoms", 1));
		doc.set_option_decription(CDOD("residues-fewer-than", CDOD::DATATYPE_INT, "minimal allowed number of residues", 1));
		doc.set_option_decription(CDOD("chains-fewer-than", CDOD::DATATYPE_INT, "minimal allowed number of chains", 1));
		doc.set_option_decription(CDOD("objects-fewer-than", CDOD::DATATYPE_INT, "minimal allowed number of objects", 1));
		doc.set_option_decription(CDOD("atoms-to-residues-ratio-less-than", CDOD::DATATYPE_FLOAT, "minimal allowed atoms to residues ratio", 0.0));
		doc.set_option_decription(CDOD("min-atoms-in-selection", CDOD::DATATYPE_INT, "minimal allowed number of atoms in selection", 1));
		doc.set_option_decription(CDOD("min-contacts-in-selection", CDOD::DATATYPE_INT, "minimal allowed number of contacts in selection", 1));
		doc.set_option_decription(CDOD("not-enough-atoms-in-selection", CDOD::DATATYPE_STRING, "atoms selection expression", ""));
		doc.set_option_decription(CDOD("not-enough-contacts-in-selection", CDOD::DATATYPE_STRING, "contacts selection expression", ""));
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		const std::vector<DataManager*> objects=congregation_of_data_managers.get_objects(query);
		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects to check selected."));
		}

		Result result;

		for(std::size_t i=0;i<objects.size();i++)
		{
			DataManager& data_manager=*(objects[i]);
			bool need_to_delete=objects.size()<objects_fewer_than;
			need_to_delete=need_to_delete || (data_manager.atoms().size()<atoms_fewer_than);
			need_to_delete=need_to_delete || (data_manager.primary_structure_info().residues.size()<residues_fewer_than);
			need_to_delete=need_to_delete || (data_manager.primary_structure_info().chains.size()<chains_fewer_than);
			need_to_delete=need_to_delete || (static_cast<double>(data_manager.atoms().size())/static_cast<double>(data_manager.primary_structure_info().residues.size())<atoms_to_residues_ratio_less_than);
			need_to_delete=need_to_delete || (!not_enough_atoms_in_selection.empty() && data_manager.selection_manager().select_atoms(SelectionManager::Query(not_enough_atoms_in_selection, false)).size()<min_atoms_in_selection);
			need_to_delete=need_to_delete || (!not_enough_contacts_in_selection.empty() && (data_manager.contacts().empty() || data_manager.selection_manager().select_contacts(SelectionManager::Query(not_enough_contacts_in_selection, false)).size()<min_contacts_in_selection));
			if(need_to_delete)
			{
				result.deleted_objects.push_back(congregation_of_data_managers.get_object_attributes(objects[i]).name);
				congregation_of_data_managers.delete_object(objects[i]);
			}
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_DELETE_OBJECTS_IF_H_ */
