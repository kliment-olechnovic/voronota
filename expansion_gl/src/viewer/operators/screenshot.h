#ifndef VIEWER_OPERATORS_SCREENSHOT_H_
#define VIEWER_OPERATORS_SCREENSHOT_H_

#include "../operators_common.h"
#include "../gui_configuration.h"

#include "../dependencies/lodepng/lodepng.h"

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
		static std::vector<std::string> formats;
		if(formats.empty())
		{
			formats.push_back(".ppm");
			formats.push_back(".png");
		}

		std::string format_to_use;
		for(std::size_t i=0;i<formats.size() && format_to_use.empty();i++)
		{
			if(filename.size()>formats[i].size() && filename.substr(filename.size()-formats[i].size())==formats[i])
			{
				format_to_use=formats[i];
			}
		}

		if(format_to_use.empty())
		{
			throw std::runtime_error(std::string("Invalid file extension, must be '.ppm' or '.png'."));
		}

		int W=0;
		int H=0;
		std::vector<char> image_data;

		if(uv::ViewerApplication::instance().num_of_refresh_calls_since_last_render()>0)
		{
			uv::ViewerApplication::instance_refresh_frame(true);
		}

		if(!uv::ViewerApplication::instance().read_pixels(W, H, image_data))
		{
			throw std::runtime_error(std::string("Failed to read pixels."));
		}

		if(format_to_use==".ppm")
		{
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
		}
		else if(format_to_use==".png")
		{
			std::vector<unsigned char> png_image_data;
			png_image_data.resize(W*H*4);
			for(int y=0;y<H;y++)
			{
				for(int x=0;x<W;x++)
				{
					const int pos=3*((H-1-y)*W+x);
					const int png_pos=4*(y*W+x);
					png_image_data[png_pos]=static_cast<unsigned char>(image_data[pos]);
					png_image_data[png_pos+1]=static_cast<unsigned char>(image_data[pos+1]);
					png_image_data[png_pos+2]=static_cast<unsigned char>(image_data[pos+2]);
					png_image_data[png_pos+3]=255;
				}
			}
			unsigned int error = lodepng::encode(filename, png_image_data, static_cast<unsigned int>(W), static_cast<unsigned int>(H));
			if(error>0)
			{
				throw std::runtime_error(std::string("PNG encoding failed with error '")+lodepng_error_text(error)+"'.");
			}
		}
		else
		{
			throw std::runtime_error(std::string("File extension '")+format_to_use+"' not supported.");
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_SCREENSHOT_H_ */
