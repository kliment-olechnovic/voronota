#ifndef VIEWER_OPERATORS_IMPORT_VIEW_H_
#define VIEWER_OPERATORS_IMPORT_VIEW_H_

#include "../gui_configuration.h"
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
	bool try_to_resize;

	ImportView() : try_to_resize(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		try_to_resize=input.get_flag("try-to-resize");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		doc.set_option_decription(CDOD("try-to-resize", CDOD::DATATYPE_BOOL, "flag to try to restore viewing area size if possible"));
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

		std::vector<int> recommended_effective_rendering_size;

		if(!uv::ViewerApplication::instance().load_view_from_stream(finput, recommended_effective_rendering_size))
		{
			throw std::runtime_error(std::string("No view loaded."));
		}

		if(try_to_resize && recommended_effective_rendering_size.size()==2)
		{
			GUIConfiguration::instance().hint_render_area_width=recommended_effective_rendering_size[0];
			GUIConfiguration::instance().hint_render_area_height=recommended_effective_rendering_size[1];
		}

		Result result;
		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_IMPORT_VIEW_H_ */
