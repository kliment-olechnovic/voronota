#ifndef VIEWER_OPERATORS_SCREENSHOT_H_
#define VIEWER_OPERATORS_SCREENSHOT_H_

#include "../operators_common.h"
#include "../gui_configuration.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Screenshot : public scripting::OperatorBase<Screenshot>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	std::string filename;

	Screenshot()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		filename=input.get_value_or_first_unused_unnamed_value("file");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
	}

	Result run(void*) const
	{
		int W=0;
		int H=0;
		std::vector<char> image_data;

		GUIConfiguration::push();

		if(GUIConfiguration::instance().enabled_widgets())
		{
			GUIConfiguration::instance().set_enabled_widgets(false);
			uv::ViewerApplication::instance_refresh_frame();
			uv::ViewerApplication::instance_refresh_frame();
		}

		if(!uv::ViewerApplication::instance().read_pixels(W, H, image_data))
		{
			throw std::runtime_error(std::string("Failed to read pixels."));
		}

		GUIConfiguration::pop();

		std::ofstream output(filename.c_str(), std::ios::out);

		if(!output.good())
		{
			throw std::runtime_error(std::string("Failed to open file '")+filename+"' for writing.");
		}

		output << "P6\n" << W << " " << H << "\n255\n";
		for(int y=0;y<H;y++)
		{
			for(int x=0;x<W;x++)
			{
				const int pos=3*((H-1-y)*W+x);
				output.write(&(image_data[pos]), 3);
			}
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_SCREENSHOT_H_ */
