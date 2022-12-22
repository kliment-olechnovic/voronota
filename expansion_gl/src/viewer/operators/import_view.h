#ifndef VIEWER_OPERATORS_IMPORT_VIEW_H_
#define VIEWER_OPERATORS_IMPORT_VIEW_H_

#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class ImportView : public scripting::OperatorBase<ImportView>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	std::string file;

	ImportView()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
	}

	Result run(void*) const
	{
		if(file.empty())
		{
			throw std::runtime_error(std::string("Empty input file name."));
		}

		scripting::InputSelector finput_selector(file);
		std::istream& finput=finput_selector.stream();

		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
		}

		if(!uv::ViewerApplication::instance().load_view_from_stream(finput))
		{
			throw std::runtime_error(std::string("No view loaded."));
		}

		Result result;
		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_IMPORT_VIEW_H_ */
