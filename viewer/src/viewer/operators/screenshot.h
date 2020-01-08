#ifndef VIEWER_OPERATORS_SCREENSHOT_H_
#define VIEWER_OPERATORS_SCREENSHOT_H_

#include "../operators_common.h"

namespace viewer
{

namespace operators
{

class Screenshot : public scripting::operators::OperatorBase<Screenshot>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		const Result& write(scripting::HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	std::string filename;

	explicit Screenshot(uv::ViewerApplication& app) : app_ptr_(&app)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		filename=input.get_value_or_first_unused_unnamed_value("file");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(scripting::CDOD("file", scripting::CDOD::DATATYPE_STRING, "path to file"));
	}

	Result run(void*&) const
	{
		int W=0;
		int H=0;
		std::vector<char> image_data;

		if(!app_ptr_->read_pixels(W, H, image_data))
		{
			throw std::runtime_error(std::string("Failed to read pixels."));
		}

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

private:
	uv::ViewerApplication* app_ptr_;
};

}

}

#endif /* VIEWER_OPERATORS_SCREENSHOT_H_ */
