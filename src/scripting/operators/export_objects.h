#ifndef SCRIPTING_OPERATORS_EXPORT_OBJECTS_H_
#define SCRIPTING_OPERATORS_EXPORT_OBJECTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportObjects : public OperatorBase<ExportObjects>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string file;
	CongregationOfDataManagers::ObjectQuery query;

	ExportObjects()
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
		query=OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		if(congregation_of_data_managers.get_objects(query).empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		if(!congregation_of_data_managers.check_if_saveable_to_stream(query))
		{
			throw std::runtime_error(std::string("Selected objects cannot be exported."));
		}

		OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		assert_io_stream(file, output);

		if(!congregation_of_data_managers.save_to_stream(query, output))
		{
			throw std::runtime_error(std::string("Failed to export objects."));
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_OBJECTS_H_ */
