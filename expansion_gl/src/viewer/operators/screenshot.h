#ifndef VIEWER_OPERATORS_SCREENSHOT_H_
#define VIEWER_OPERATORS_SCREENSHOT_H_

#include "../operators_common.h"
#include "../gui_configuration.h"

#include "../../../expansion_js/src/dependencies/lodepng/lodepng.h"

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
	bool opaque;
	std::vector<int> size;
	double scale;
	bool autocrop;

	Screenshot() : opaque(false), scale(1.0), autocrop(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		filename=input.get_value_or_first_unused_unnamed_value("file");
		opaque=input.get_flag("opaque");
		size=input.get_value_vector_or_default<int>("size", std::vector<int>());
		scale=input.get_value_or_default<double>("scale", 1.0);
		autocrop=input.get_flag("autocrop");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		doc.set_option_decription(CDOD("opaque", CDOD::DATATYPE_BOOL, "flag to disable transparency of PNG background"));
		doc.set_option_decription(CDOD("size", CDOD::DATATYPE_INT_ARRAY, "width and height", ""));
		doc.set_option_decription(CDOD("scale", CDOD::DATATYPE_FLOAT, "scaling coefficient for width and height", 1.0));
		doc.set_option_decription(CDOD("autocrop", CDOD::DATATYPE_BOOL, "flag to automatically crop empty space around image"));
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

		if(!size.empty() && size.size()!=2)
		{
			throw std::runtime_error(std::string("Invalid size, must be two numbers, width and height."));
		}

		if(scale<=0.0)
		{
			throw std::runtime_error(std::string("Invalid scale value, must be positive."));
		}

		int W=static_cast<int>(static_cast<double>(size.empty() ? uv::ViewerApplication::instance().effective_rendering_window_width() : size[0])*scale);
		int H=static_cast<int>(static_cast<double>(size.empty() ? uv::ViewerApplication::instance().effective_rendering_window_height() : size[1])*scale);

		if(!(W>10 && H>10))
		{
			throw std::runtime_error(std::string("Invalid scaled size, too small, both width and height must be greater than 10."));
		}

		if(!(W<10000 && H<10000))
		{
			throw std::runtime_error(std::string("Invalid scaled size, too big, both width and height must be less than 10000."));
		}

		const int supersampling_levels=((W*H<(2001*2001)) ? ((W*H<(801*801)) ? 2 : 1) : 0);

		for(int l=0;l<supersampling_levels;l++)
		{
			W*=2;
			H*=2;
		}

		std::vector<unsigned char> image_data_rgba(W*H*4);

		{
			std::vector<unsigned char> image_data_rgb;

			if(!uv::ViewerApplication::instance().render_in_screenshot_mode_and_read_pixels(W, H, image_data_rgb))
			{
				throw std::runtime_error(std::string("Failed to read pixels."));
			}

			for(int y=0;y<H;y++)
			{
				for(int x=0;x<W;x++)
				{
					const int pos_a=4*((H-1-y)*W+x);
					const int pos_b=4*(y*W+x);
					image_data_rgba[pos_b]=image_data_rgb[pos_a];
					image_data_rgba[pos_b+1]=image_data_rgb[pos_a+1];
					image_data_rgba[pos_b+2]=image_data_rgb[pos_a+2];
					image_data_rgba[pos_b+3]=(image_data_rgb[pos_a+3]==0 ? 0 : 255);
				}
			}
		}

		for(int l=0;l<supersampling_levels;l++)
		{
			W/=2;
			H/=2;
			std::vector<unsigned char> sample(W*H*4);
			for(int y=0;y<H;y++)
			{
				for(int x=0;x<W;x++)
				{
					const int pos=4*(y*W+x);
					const int pos00=4*((y*2+0)*W*2+(x*2+0));
					const int pos01=4*((y*2+0)*W*2+(x*2+1));
					const int pos10=4*((y*2+1)*W*2+(x*2+0));
					const int pos11=4*((y*2+1)*W*2+(x*2+1));
					for(int i=0;i<4;i++)
					{
						sample[pos+i]=static_cast<unsigned char>((static_cast<int>(image_data_rgba[pos00+i])+static_cast<int>(image_data_rgba[pos01+i])+static_cast<int>(image_data_rgba[pos10+i])+static_cast<int>(image_data_rgba[pos11+i]))/4);
					}
				}
			}
			image_data_rgba.swap(sample);
		}

		if(autocrop)
		{
			int min_y=H;
			int min_x=W;
			int max_y=-1;
			int max_x=-1;
			for(int y=0;y<H;y++)
			{
				for(int x=0;x<W;x++)
				{
					const int pos=4*(y*W+x);
					if(image_data_rgba[pos+3]!=0)
					{
						min_y=std::min(min_y, y);
						min_x=std::min(min_x, x);
						max_y=std::max(max_y, y);
						max_x=std::max(max_x, x);
					}
				}
			}
			if(min_y>=0 && max_y<H && min_y<=max_y && min_x>=0 && max_x<W && min_x<=max_x)
			{
				const int oW=W;
				W=(max_x-min_x)+1;
				H=(max_y-min_y)+1;
				std::vector<unsigned char> sample(W*H*4);
				for(int y=0;y<H;y++)
				{
					for(int x=0;x<W;x++)
					{
						const int pos_a=4*((y+min_y)*oW+(x+min_x));
						const int pos_b=4*(y*W+x);
						for(int i=0;i<4;i++)
						{
							sample[pos_b+i]=image_data_rgba[pos_a+i];
						}
					}
				}
				image_data_rgba.swap(sample);
			}
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
					const int pos=4*(y*W+x);
					output.write(reinterpret_cast<char*>(&(image_data_rgba[pos])), 3);
				}
			}
		}
		else if(format_to_use==".png")
		{
			if(opaque)
			{
				for(std::size_t i=3;i<image_data_rgba.size();i+=4)
				{
					image_data_rgba[i]=static_cast<unsigned char>(255);
				}
			}
			unsigned int error=lodepng::encode(filename, image_data_rgba, static_cast<unsigned int>(W), static_cast<unsigned int>(H));
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
