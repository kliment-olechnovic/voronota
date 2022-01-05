#ifndef SCRIPTING_OPERATORS_MERGE_OBJECTS_H_
#define SCRIPTING_OPERATORS_MERGE_OBJECTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class MergeObjects : public OperatorBase<MergeObjects>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string object_name;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("object_name")=object_name;
		}
	};

	CongregationOfDataManagers::ObjectQuery query;
	std::string title;

	MergeObjects()
	{
	}

	void initialize(CommandInput& input)
	{
		query=OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		title=input.get_value<std::string>("title");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		doc.set_option_decription(CDOD("title", CDOD::DATATYPE_STRING, "new object title"));
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		OperatorsUtilities::assert_new_object_name_input(title);

		const std::vector<DataManager*> objects=congregation_of_data_managers.get_objects(query);
		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		DataManager* object_new=congregation_of_data_managers.add_object(DataManager(), title);

		{
			DataManager& data_manager=*object_new;
			std::vector<const DataManager*> other_data_managers(objects.size(), 0);
			for(std::size_t i=0;i<objects.size();i++)
			{
				other_data_managers[i]=objects[i];
			}
			data_manager.append_atoms_from_other_data_managers(other_data_managers);
		}

		Result result;
		result.object_name=congregation_of_data_managers.get_object_attributes(object_new).name;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_MERGE_OBJECTS_H_ */
