#ifndef VIEWER_OPERATORS_EXPORT_SESSION_H_
#define VIEWER_OPERATORS_EXPORT_SESSION_H_

#include "../operators_common.h"

#include "../../../expansion_js/src/dependencies/lodepng/lodepng.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class ExportSession : public scripting::OperatorBase<ExportSession>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	std::string file;
	scripting::CongregationOfDataManagers::ObjectQuery query;

	ExportSession()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		scripting::assert_file_name_input(file, false);
		query=scripting::OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		scripting::OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		if(congregation_of_data_managers.get_objects(query).empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		std::string data_str;
		{
			std::ostringstream str_output;

			if(!congregation_of_data_managers.save_to_stream(query, str_output))
			{
				throw std::runtime_error(std::string("Failed to export objects."));
			}

			uv::ViewerApplication::instance().save_view_to_stream(str_output);

			data_str=str_output.str();
		}

		std::vector<unsigned char> compressed_data;
		int compression_status=lodepng::compress(compressed_data, reinterpret_cast<const unsigned char*>(data_str.c_str()), data_str.size());
		if(compression_status!=0 || compressed_data.empty())
		{
			throw std::runtime_error(std::string("Failed to compress data."));
		}

		scripting::OutputSelector foutput_selector(file);
		std::ostream& foutput=foutput_selector.stream();
		scripting::assert_io_stream(file, foutput);

		foutput.write(reinterpret_cast<const char*>(&compressed_data[0]), compressed_data.size());

		Result result;

		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_EXPORT_SESSION_H_ */
