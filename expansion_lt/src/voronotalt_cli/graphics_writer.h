#ifndef VORONOTALT_GRAPHICS_WRITER_H_
#define VORONOTALT_GRAPHICS_WRITER_H_

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "../voronotalt/basic_types_and_functions.h"

namespace voronotalt
{

class GraphicsWriter
{
public:
	explicit GraphicsWriter(const bool enabled) : enabled_(enabled)
	{
	}

	bool enabled() const
	{
		return enabled_;
	}

	void add_triangle_fan(const std::vector<SimplePoint>& outer_points, const SimplePoint& center, const SimplePoint& normal_direction_start, const SimplePoint& normal_direction_end)
	{
		if(!enabled_)
		{
			return;
		}
		print_triangle_fan_for_pymol(outer_points, center, unit_point(sub_of_points(normal_direction_end, normal_direction_start)), "", ", \n", graphics_output_stream_);
	}

	void add_spheres(const std::vector<SimpleSphere>& spheres, const Float& radius_change)
	{
		if(!enabled_)
		{
			return;
		}
		for(std::size_t i=0;i<spheres.size();i++)
		{
			print_sphere(spheres[i], radius_change, "", ",\n", graphics_output_stream_);
		}
	}

	void add_color(const double r, const double g, const double b)
	{
		if(!enabled_)
		{
			return;
		}
		graphics_output_stream_ << "COLOR, " << r << ", " << g << ", " << b << ",\n";
	}

	bool write_to_file(const std::string& title, const std::string& filename)
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
		if(output.good())
		{
			output << "from pymol.cgo import *\n";
			output << "from pymol import cmd\n";
			output << "cgo_graphics_list = [";
			output << graphics_output_stream_.str();
			output << "]\n";
			output << "cmd.load_cgo(cgo_graphics_list, '" << (title.empty() ? std::string("cgo") : title) << "')\n";
			output << "cmd.set('two_sided_lighting', 1)\n";
			return true;
		}
		return false;
	}

private:
	static void print_triangle_fan_for_pymol(const std::vector<SimplePoint>& outer_points, const SimplePoint& center, const SimplePoint& normal, const std::string& prefix, const std::string& postfix, std::ostream& output)
	{
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
	}

	static void print_sphere(const SimpleSphere& sphere, const Float& radius_change, const std::string& prefix, const std::string& postfix, std::ostream& output)
	{
		output << prefix << "SPHERE" << ", " << sphere.p.x << ", " << sphere.p.y << ", " << sphere.p.z << ", " << (sphere.r-radius_change) << postfix;
	}

	bool enabled_;
	std::ostringstream graphics_output_stream_;
};


}

#endif /* VORONOTALT_GRAPHICS_WRITER_H_ */
