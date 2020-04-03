#ifndef DUKTAPER_OPERATORS_TMALIGN_MANY_H_
#define DUKTAPER_OPERATORS_TMALIGN_MANY_H_

#include "tmalign.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class TMalignMany : public scripting::operators::OperatorBase<TMalign>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		std::vector<TMalign::Result> tmalign_results;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			for(std::size_t i=0;i<tmalign_results.size();i++)
			{
				std::vector<scripting::VariantObject>& voa=heterostorage.variant_object.objects_array("tmalign_results");
				voa.push_back(scripting::VariantObject());
				tmalign_results[i].store(voa.back());
			}
		}
	};

	std::string target_name;
	scripting::CongregationOfDataManagers::ObjectQuery query;
	std::string target_selection;
	std::string model_selection;

	TMalignMany()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		target_name=input.get_value_or_first_unused_unnamed_value_or_default("target", "");
		query=scripting::operators::Utilities::read_congregation_of_data_managers_object_query(input);
		target_selection=input.get_value_or_default<std::string>("target-sel", "");
		model_selection=input.get_value_or_default<std::string>("model-sel", "");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(scripting::CDOD("target", scripting::CDOD::DATATYPE_STRING, "name of target object", ""));
		scripting::operators::Utilities::document_read_congregation_of_data_managers_object_query(doc);
		doc.set_option_decription(scripting::CDOD("target-sel", scripting::CDOD::DATATYPE_STRING, "selection of atoms for target object", ""));
		doc.set_option_decription(scripting::CDOD("model-sel", scripting::CDOD::DATATYPE_STRING, "selection of atoms for model object", ""));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		const std::vector<scripting::DataManager*> all_objects=congregation_of_data_managers.get_objects();

		if(all_objects.size()<2)
		{
			throw std::runtime_error(std::string("Less than two objects overall available."));
		}

		const std::vector<scripting::DataManager*> objects=congregation_of_data_managers.get_objects(query);

		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		const std::string target_name_to_use=(target_name.empty() ? congregation_of_data_managers.get_object_attributes(all_objects[0]).name : target_name);

		congregation_of_data_managers.assert_object_availability(target_name_to_use);

		Result result;

		for(std::size_t i=0;i<objects.size();i++)
		{
			const scripting::CongregationOfDataManagers::ObjectAttributes attributes=congregation_of_data_managers.get_object_attributes(objects[i]);
			if(attributes.name!=target_name_to_use)
			{
				std::ostringstream args;
				args << "-target " << target_name_to_use << " -model " << attributes.name;
				if(!target_selection.empty())
				{
					args << " -target-sel " << target_selection;
				}
				if(!model_selection.empty())
				{
					args << " -model-sel " << model_selection;
				}
				result.tmalign_results.push_back(TMalign().init(args.str()).run(congregation_of_data_managers));
			}
		}

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_TMALIGN_MANY_H_ */
