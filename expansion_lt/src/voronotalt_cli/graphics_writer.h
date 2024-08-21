#ifndef VORONOTALT_GRAPHICS_WRITER_H_
#define VORONOTALT_GRAPHICS_WRITER_H_

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <sstream>
#include <fstream>

#include "../voronotalt/basic_types_and_functions.h"

namespace voronotalt
{

class GraphicsWriter
{
public:
	explicit GraphicsWriter(const bool enabled) noexcept : enabled_(enabled)
	{
	}

	bool enabled() const noexcept
	{
		return enabled_;
	}

	void add_triangle_fan(const std::string& group_name, const std::vector<SimplePoint>& outer_points, const SimplePoint& center, const SimplePoint& normal) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		parts_.push_back(std::pair<std::string, std::string>(group_name, print_triangle_fan_for_pymol(outer_points, center, normal, "", ", \n")));
	}

	void add_triangle_fan(const std::string& group_name, const std::vector<SimplePoint>& outer_points, const SimplePoint& center, const SimplePoint& normal_direction_start, const SimplePoint& normal_direction_end) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		add_triangle_fan(group_name, outer_points, center, unit_point(sub_of_points(normal_direction_end, normal_direction_start)));
	}

	void add_line_loop(const std::string& group_name, const std::vector<SimplePoint>& outer_points) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		parts_.push_back(std::pair<std::string, std::string>(group_name, print_line_loop_for_pymol(outer_points, "", ", \n")));
	}

	void add_sphere(const std::string& group_name, const SimpleSphere& sphere, const Float& radius_change) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		parts_.push_back(std::pair<std::string, std::string>(group_name, print_sphere(sphere, radius_change, "", ",\n")));
	}

	void add_color(const double r, const double g, const double b) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		std::ostringstream output;
		output << "COLOR, " << r << ", " << g << ", " << b << ",\n";
		parts_.push_back(std::pair<std::string, std::string>("", output.str()));
	}

	void add_alpha(const double a) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		std::ostringstream output;
		output << "ALPHA, " << a << ",\n";
		parts_.push_back(std::pair<std::string, std::string>("", output.str()));
	}

	bool write_to_file(const std::string& title, const std::string& filename) noexcept
	{
		if(!enabled_)
		{
			return false;
		}

		if(filename.empty())
		{
			return false;
		}

		std::ofstream output(filename, std::ios::out);

		if(!output.good())
		{
			return false;
		}

		std::set<std::string> category_names;
		for(std::size_t i=0;i<parts_.size();i++)
		{
			const std::string& category_name=parts_[i].first;
			if(!category_name.empty())
			{
				category_names.insert(category_name);
			}
		}

		if(category_names.empty())
		{
			return false;
		}

		output << "from pymol.cgo import *\n";
		output << "from pymol import cmd\n";

		for(std::set<std::string>::const_iterator it=category_names.begin();it!=category_names.end();++it)
		{
			const std::string& current_category_name=(*it);
			output << "cgo_graphics_list_" << current_category_name << " = [";
			for(std::size_t i=0;i<parts_.size();i++)
			{
				const std::string& category_name=parts_[i].first;
				if(category_name.empty() || category_name==current_category_name)
				{
					output << parts_[i].second;
				}
			}
			output << "]\n";
			output << "cmd.load_cgo(cgo_graphics_list_" <<  current_category_name << ", '" << (title.empty() ? std::string("cgo") : title) << "_" << current_category_name << "')\n";
		}

		output << "cmd.set('two_sided_lighting', 1)\n";
		output << "cmd.set('cgo_line_width', 3)\n";

		return true;
	}

private:
	static std::string print_triangle_fan_for_pymol(const std::vector<SimplePoint>& outer_points, const SimplePoint& center, const SimplePoint& normal, const std::string& prefix, const std::string& postfix) noexcept
	{
		std::ostringstream output;
		output << prefix << "BEGIN, TRIANGLE_FAN";
		if(!outer_points.empty())
		{
			output << ", \nNORMAL, " << normal.x << ", " << normal.y << ", " << normal.z;
			output << ", VERTEX, " << center.x << ", " << center.y << ", " << center.z;
			for(std::size_t j=0;j<outer_points.size();j++)
			{
				output << ", \nNORMAL, " << normal.x << ", " << normal.y << ", " << normal.z;
				output << ", VERTEX, " << outer_points[j].x << ", " << outer_points[j].y << ", " << outer_points[j].z;
			}
			output << ", \nNORMAL, " << normal.x << ", " << normal.y << ", " << normal.z;
			output << ", VERTEX, " << outer_points[0].x << ", " << outer_points[0].y << ", " << outer_points[0].z;
		}
		output << ", \nEND" << postfix;
		return output.str();
	}

	static std::string print_line_loop_for_pymol(const std::vector<SimplePoint>& outer_points, const std::string& prefix, const std::string& postfix) noexcept
	{
		std::ostringstream output;
		output << prefix << "BEGIN, LINE_LOOP";
		if(!outer_points.empty())
		{
			for(std::size_t j=0;j<outer_points.size();j++)
			{
				output << ", \nVERTEX, " << outer_points[j].x << ", " << outer_points[j].y << ", " << outer_points[j].z;
			}
		}
		output << ", \nEND" << postfix;
		return output.str();
	}

	static std::string print_sphere(const SimpleSphere& sphere, const Float& radius_change, const std::string& prefix, const std::string& postfix) noexcept
	{
		std::ostringstream output;
		output << prefix << "SPHERE" << ", " << sphere.p.x << ", " << sphere.p.y << ", " << sphere.p.z << ", " << (sphere.r-radius_change) << postfix;
		return output.str();
	}

	bool enabled_;
	std::vector< std::pair<std::string, std::string> > parts_;
};


}

#endif /* VORONOTALT_GRAPHICS_WRITER_H_ */
