#ifndef VIEWER_CUSTOM_COMMANDS_H_
#define VIEWER_CUSTOM_COMMANDS_H_

#include "../../../src/scripting/generic_command.h"
#include "../../../src/auxiliaries/color_utilities.h"

#include "../uv/viewer_application.h"

namespace viewer
{

class CustomCommands
{
public:
	class resize_window : public scripting::GenericCommand
	{
	public:
		explicit resize_window(uv::ViewerApplication& app) :
			app_(app)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			const int width=cargs.input.get_value_or_default<int>("width", app_.window_width());
			const int height=cargs.input.get_value_or_default<int>("height", app_.window_height());

			cargs.input.assert_nothing_unusable();

			if(width<1 || width>9999)
			{
				throw std::runtime_error(std::string("Invalid width."));
			}

			if(height<1 || height>9999)
			{
				throw std::runtime_error(std::string("Invalid height."));
			}

			app_.set_window_size(width, height);
		}

	private:
		uv::ViewerApplication& app_;
	};

	class background : public scripting::GenericCommand
	{
	public:
		explicit background(uv::ViewerApplication& app) :
			app_(app)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			auxiliaries::ColorUtilities::ColorInteger color=auxiliaries::ColorUtilities::color_from_name(cargs.input.get_value_or_first_unused_unnamed_value("col"));

			cargs.input.assert_nothing_unusable();

			if(!auxiliaries::ColorUtilities::color_valid(color))
			{
				throw std::runtime_error(std::string("Invalid color."));
			}

			app_.set_background_color(color);
		}

	private:
		uv::ViewerApplication& app_;
	};

	class mono : public scripting::GenericCommand
	{
	public:
		explicit mono(uv::ViewerApplication& app) :
			app_(app)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.input.assert_nothing_unusable();
			app_.set_rendering_mode_to_simple();
		}

	private:
		uv::ViewerApplication& app_;
	};

	class stereo : public scripting::GenericCommand
	{
	public:
		explicit stereo(uv::ViewerApplication& app) :
			app_(app)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			const float angle=cargs.input.get_value_or_default<float>("angle", app_.stereo_angle());
			const float offset=cargs.input.get_value_or_default<float>("offset", app_.stereo_offset());

			cargs.input.assert_nothing_unusable();

			app_.set_rendering_mode_to_stereo(angle, offset);

			cargs.heterostorage.variant_object.value("angle")=angle;
			cargs.heterostorage.variant_object.value("offset")=offset;
		}

	private:
		uv::ViewerApplication& app_;
	};

	template<int grid_variant_value>
	class grid : public scripting::GenericCommand
	{
	public:
		explicit grid(uv::ViewerApplication& app, int& grid_variant) :
			app_(app),
			grid_variant_(grid_variant)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.input.assert_nothing_unusable();
			app_.set_rendering_mode_to_grid();
			grid_variant_=grid_variant_value;
		}

	private:
		uv::ViewerApplication& app_;
		int& grid_variant_;
	};

	class fog : public scripting::GenericCommand
	{
	public:
		explicit fog(uv::ViewerApplication& app) :
			app_(app)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			const bool off=cargs.input.get_flag("off");
			cargs.input.assert_nothing_unusable();
			app_.set_fog_enabled(!off);
		}

	private:
		uv::ViewerApplication& app_;
	};

	class rotate : public scripting::GenericCommand
	{
	public:
		explicit rotate(uv::ViewerApplication& app) :
			app_(app)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			std::vector<double> default_axis(3, 0.0);
			default_axis[1]=1.0;

			const double angle_in_degrees=cargs.input.get_value_or_default<double>("angle", 10.0f);
			const std::vector<double> axis=cargs.input.get_value_vector_or_default<double>("axis", default_axis);

			cargs.input.assert_nothing_unusable();

			if(angle_in_degrees>360.0 || angle_in_degrees<(-360.0))
			{
				throw std::runtime_error(std::string("Invalid angle."));
			}

			if(axis.size()!=3 || (axis[0]==0.0 && axis[1]==0.0 && axis[2]==0.0))
			{
				throw std::runtime_error(std::string("Invalid axis."));
			}

			const double angle_in_radians=(angle_in_degrees*glm::pi<double>()/180.0);

			app_.rotate(glm::vec3(axis[0], axis[1], axis[2]), angle_in_radians);
		}

	private:
		uv::ViewerApplication& app_;
	};

	class screenshot : public scripting::GenericCommand
	{
	public:
		explicit screenshot(uv::ViewerApplication& app) :
			app_(app)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			std::string filename=cargs.input.get_value_or_first_unused_unnamed_value("file");

			cargs.input.assert_nothing_unusable();

			int W=0;
			int H=0;
			std::vector<char> image_data;

			if(!app_.read_pixels(W, H, image_data))
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
		}

	private:
		uv::ViewerApplication& app_;
	};

	class setup_rendering : public scripting::GenericCommand
	{
	protected:
		void run(CommandArguments& cargs)
		{
			DrawerForDataManager::RenderingParameters p=DrawerForDataManager::RenderingParameters::default_rendering_parameters();

			const bool reset=cargs.input.get_flag("reset");

			if(reset)
			{
				p=DrawerForDataManager::RenderingParameters();
			}

			p.ball_sphere_quality=cargs.input.get_value_or_default<unsigned int>("ball-sphere-quality", p.ball_sphere_quality);
			p.stick_sphere_quality=cargs.input.get_value_or_default<unsigned int>("stick-sphere-quality", p.stick_sphere_quality);
			p.stick_cylinder_quality=cargs.input.get_value_or_default<unsigned int>("stick-cylinder-quality", p.stick_cylinder_quality);
			p.trace_sphere_quality=cargs.input.get_value_or_default<unsigned int>("trace-sphere-quality", p.trace_sphere_quality);
			p.trace_cylinder_quality=cargs.input.get_value_or_default<unsigned int>("trace-cylinder-quality", p.trace_cylinder_quality);
			p.cartoon_style=cargs.input.get_value_or_default<int>("cartoon-style", p.cartoon_style);

			cargs.input.assert_nothing_unusable();

			if(p.ball_sphere_quality>4)
			{
				throw std::runtime_error(std::string("Too high 'ball-sphere-quality'."));
			}

			if(p.stick_sphere_quality>4)
			{
				throw std::runtime_error(std::string("Too high 'stick-sphere-quality'."));
			}

			if(p.stick_cylinder_quality>360)
			{
				throw std::runtime_error(std::string("Too high 'stick-cylinder-quality'."));
			}

			if(p.trace_sphere_quality>4)
			{
				throw std::runtime_error(std::string("Too high 'trace-sphere-quality'."));
			}

			if(p.trace_cylinder_quality>360)
			{
				throw std::runtime_error(std::string("Too high 'trace-cylinder-quality'."));
			}

			if(p.cartoon_style<0 || p.cartoon_style>2)
			{
				throw std::runtime_error(std::string("Invalid cartoon style, must be in {0, 1, 2}."));
			}

			DrawerForDataManager::RenderingParameters::default_rendering_parameters()=p;
		}
	};
};

}

#endif /* VIEWER_CUSTOM_COMMANDS_H_ */
