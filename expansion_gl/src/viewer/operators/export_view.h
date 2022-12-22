#ifndef VIEWER_OPERATORS_EXPORT_VIEW_H_
#define VIEWER_OPERATORS_EXPORT_VIEW_H_

#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class ExportView : public scripting::OperatorBase<ExportView>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	std::string file;

	ExportView()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		scripting::assert_file_name_input(file, false);
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
	}

	Result run(void*) const
	{
		scripting::OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		scripting::assert_io_stream(file, output);

		uv::ViewerApplication::instance().save_view_to_stream(output);

		Result result;
		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_EXPORT_VIEW_H_ */
