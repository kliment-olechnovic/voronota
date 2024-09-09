#ifndef VORONOTALT_GRAPHICS_WRITER_H_
#define VORONOTALT_GRAPHICS_WRITER_H_

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <sstream>
#include <fstream>
#include <cstdint>

#include "../voronotalt/basic_types_and_functions.h"

namespace voronotalt
{

class GraphicsWriter
{
public:
	explicit GraphicsWriter(const bool enabled) noexcept : enabled_(enabled), color_randomization_state_(static_cast<uint64_t>(42))
	{
	}

	bool enabled() const noexcept
	{
		return enabled_;
	}

	void add_triangle_fan(const std::string& category_name, const std::string& group_name, const std::vector<SimplePoint>& outer_points, const SimplePoint& center, const SimplePoint& normal) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		add_part(category_name, group_name, print_triangle_fan_for_pymol(outer_points, center, normal, "", ", \n"));
	}

	void add_triangle_fan(const std::string& category_name, const std::string& group_name, const std::vector<SimplePoint>& outer_points, const SimplePoint& center, const SimplePoint& normal_direction_start, const SimplePoint& normal_direction_end) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		add_triangle_fan(category_name, group_name, outer_points, center, unit_point(sub_of_points(normal_direction_end, normal_direction_start)));
	}

	void add_line_loop(const std::string& category_name, const std::string& group_name, const std::vector<SimplePoint>& outer_points) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		add_part(category_name, group_name, print_line_loop_for_pymol(outer_points, "", ", \n"));
	}

	void add_line_strip(const std::string& category_name, const std::string& group_name, const std::vector<SimplePoint>& points) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		add_part(category_name, group_name, print_line_strip_for_pymol(points, "", ", \n"));
	}

	void add_line(const std::string& category_name, const std::string& group_name, const SimplePoint& point_a, const SimplePoint& point_b) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		std::vector<SimplePoint> points(2);
		points[0]=point_a;
		points[1]=point_b;
		add_line_strip(category_name, group_name, points);
	}

	void add_sphere(const std::string& category_name, const std::string& group_name, const SimpleSphere& sphere, const Float& radius_change) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		add_part(category_name, group_name, print_sphere(sphere, radius_change, "", ",\n"));
	}

	void add_color(const std::string& category_name, const std::string& group_name, const double r, const double g, const double b) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		std::ostringstream output;
		output << "COLOR, " << r << ", " << g << ", " << b << ",\n";
		add_part(category_name, group_name, output.str());
	}

	void add_color(const std::string& category_name, const std::string& group_name, const unsigned int rgb) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		add_color(category_name, group_name, static_cast<double>((rgb&0xFF0000) >> 16)/static_cast<double>(0xFF), static_cast<double>((rgb&0x00FF00) >> 8)/static_cast<double>(0xFF), static_cast<double>(rgb&0x0000FF)/static_cast<double>(0xFF));
	}

	void add_random_color(const std::string& category_name, const std::string& group_name) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		uint32_t x=color_randomization_state_;
		x ^= x << 13;
		x ^= x >> 17;
		x ^= x << 5;
		color_randomization_state_=x;
		add_color(category_name, group_name, static_cast<unsigned int>(x%static_cast<uint64_t>(0xFFFFFF)));
	}

	void add_alpha(const std::string& category_name, const std::string& group_name, const double a) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		std::ostringstream output;
		output << "ALPHA, " << a << ",\n";
		add_part(category_name, group_name, output.str());
	}

	bool write_to_file(const std::string& title, const std::string& filename) const noexcept
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

		std::set< std::pair<std::string, std::string> > category_group_pairs;
		for(std::size_t i=0;i<parts_.size();i++)
		{
			const std::pair<std::string, std::string>& category_group_pair=parts_[i].first;
			if(!category_group_pair.second.empty())
			{
				category_group_pairs.insert(category_group_pair);
			}
		}

		if(category_group_pairs.empty())
		{
			return false;
		}

		output << "from pymol.cgo import *\n";
		output << "from pymol import cmd\n";

		for(std::set< std::pair<std::string, std::string> >::const_iterator it=category_group_pairs.begin();it!=category_group_pairs.end();++it)
		{
			const std::pair<std::string, std::string>& current_category_group_pair=(*it);
			output << "cgo_graphics_list_" << current_category_group_pair.first << "_" << current_category_group_pair.second << " = [";
			for(std::size_t i=0;i<parts_.size();i++)
			{
				const std::pair<std::string, std::string>& category_group_pair=parts_[i].first;
				if(category_group_pair.first==current_category_group_pair.first && (category_group_pair.second.empty() || category_group_pair.second==current_category_group_pair.second))
				{
					output << parts_[i].second;
				}
			}
			output << "]\n";
			output << "cmd.load_cgo(cgo_graphics_list_" << current_category_group_pair.first << "_" << current_category_group_pair.second;
			output << ", '" << (title.empty() ? std::string("cgo") : title) << "_" << current_category_group_pair.first << "_" << current_category_group_pair.second << "')\n";
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

	static std::string print_line_strip_for_pymol(const std::vector<SimplePoint>& points, const std::string& prefix, const std::string& postfix) noexcept
	{
		std::ostringstream output;
		output << prefix << "BEGIN, LINE_STRIP";
		if(!points.empty())
		{
			for(std::size_t j=0;j<points.size();j++)
			{
				output << ", \nVERTEX, " << points[j].x << ", " << points[j].y << ", " << points[j].z;
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

	void add_part(const std::string& category_name, const std::string& group_name, const std::string& content)
	{
		parts_.push_back(std::pair< std::pair<std::string, std::string>, std::string >(std::pair<std::string, std::string>(category_name, group_name), content));
	}

	bool enabled_;
	uint64_t color_randomization_state_;
	std::vector< std::pair< std::pair<std::string, std::string>, std::string > > parts_;
};


}

#endif /* VORONOTALT_GRAPHICS_WRITER_H_ */
