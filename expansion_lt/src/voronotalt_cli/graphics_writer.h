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
		parts_.push_back(PrimitivePart(category_name, group_name, PrimitivePart::TriangleFan(outer_points, center, normal)));
	}

	void add_triangle_fan(const std::string& category_name, const std::string& group_name, const std::vector<SimplePoint>& outer_points, const SimplePoint& center, const SimplePoint& normal_direction_start, const SimplePoint& normal_direction_end) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		add_triangle_fan(category_name, group_name, outer_points, center, unit_point(sub_of_points(normal_direction_end, normal_direction_start)));
	}

	void add_triangle_strip(const std::string& category_name, const std::string& group_name, const std::vector<SimplePoint>& points, const std::vector<SimplePoint>& normals) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		parts_.push_back(PrimitivePart(category_name, group_name, PrimitivePart::TriangleStrip(points, normals)));
	}

	void add_line_loop(const std::string& category_name, const std::string& group_name, const std::vector<SimplePoint>& outer_points) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		parts_.push_back(PrimitivePart(category_name, group_name, PrimitivePart::LineLoop(outer_points)));
	}

	void add_line_strip(const std::string& category_name, const std::string& group_name, const std::vector<SimplePoint>& points) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		parts_.push_back(PrimitivePart(category_name, group_name, PrimitivePart::LineStrip(points)));
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
		parts_.push_back(PrimitivePart(category_name, group_name, PrimitivePart::BasicSphere(sphere, radius_change)));
	}

	void add_color(const std::string& category_name, const std::string& group_name, const double r, const double g, const double b) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		parts_.push_back(PrimitivePart(category_name, group_name, PrimitivePart::Color(r, g, b)));
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
		parts_.push_back(PrimitivePart(category_name, group_name, PrimitivePart::Alpha(a)));
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

		PrinterForPymol printer;

		return printer.print(title, parts_, output);
	}

private:
	struct PrimitivePart
	{
		struct TriangleFan
		{
			bool active;
			std::vector<SimplePoint> outer_points;
			SimplePoint center;
			SimplePoint normal;

			TriangleFan() noexcept : active(false)
			{
			}

			explicit TriangleFan(const std::vector<SimplePoint>& outer_points, const SimplePoint& center, const SimplePoint& normal) noexcept : active(true), outer_points(outer_points), center(center), normal(normal)
			{
			}
		};

		struct TriangleStrip
		{
			bool active;
			std::vector<SimplePoint> points;
			std::vector<SimplePoint> normals;

			TriangleStrip() noexcept : active(false)
			{
			}

			explicit TriangleStrip(const std::vector<SimplePoint>& points, const std::vector<SimplePoint>& normals) noexcept : active(true), points(points), normals(normals)
			{
			}
		};

		struct LineLoop
		{
			bool active;
			std::vector<SimplePoint> outer_points;

			LineLoop() noexcept : active(false)
			{
			}

			explicit LineLoop(const std::vector<SimplePoint>& outer_points) noexcept : active(true), outer_points(outer_points)
			{
			}
		};

		struct LineStrip
		{
			bool active;
			std::vector<SimplePoint> points;

			LineStrip() noexcept : active(false)
			{
			}

			explicit LineStrip(const std::vector<SimplePoint>& points) noexcept : active(true), points(points)
			{
			}
		};

		struct BasicSphere
		{
			bool active;
			SimpleSphere sphere;

			BasicSphere() noexcept : active(false)
			{
			}

			explicit BasicSphere(const SimpleSphere& sphere) noexcept : active(true), sphere(sphere)
			{
			}

			explicit BasicSphere(const SimpleSphere& raw_sphere, const Float radius_change) noexcept : active(true), sphere(raw_sphere.p, raw_sphere.r-radius_change)
			{
			}
		};

		struct Color
		{
			bool active;
			double r;
			double g;
			double b;

			Color() noexcept : active(false), r(0.0), g(0.0), b(0.0)
			{
			}

			explicit Color(const double r, const double g, const double b) noexcept : active(true), r(r), g(g), b(b)
			{
			}
		};

		struct Alpha
		{
			bool active;
			double a;

			Alpha() noexcept : active(false), a(0.0)
			{
			}

			explicit Alpha(const double a) noexcept : active(true), a(a)
			{
			}
		};

		PrimitivePart() noexcept
		{
		}

		PrimitivePart(const std::string& category_name, const std::string& group_name) noexcept : category_name(category_name), group_name(group_name)
		{
		}

		PrimitivePart(const std::string& category_name, const std::string& group_name, const TriangleFan& obj) noexcept : category_name(category_name), group_name(group_name), obj_triangle_fan(obj)
		{
		}

		PrimitivePart(const std::string& category_name, const std::string& group_name, const TriangleStrip& obj) noexcept : category_name(category_name), group_name(group_name), obj_triangle_strip(obj)
		{
		}

		PrimitivePart(const std::string& category_name, const std::string& group_name, const LineLoop& obj) noexcept : category_name(category_name), group_name(group_name), obj_line_loop(obj)
		{
		}

		PrimitivePart(const std::string& category_name, const std::string& group_name, const LineStrip& obj) noexcept : category_name(category_name), group_name(group_name), obj_line_strip(obj)
		{
		}

		PrimitivePart(const std::string& category_name, const std::string& group_name, const BasicSphere& obj) noexcept : category_name(category_name), group_name(group_name), obj_basic_sphere(obj)
		{
		}

		PrimitivePart(const std::string& category_name, const std::string& group_name, const Color& obj) noexcept : category_name(category_name), group_name(group_name), obj_color_(obj)
		{
		}

		PrimitivePart(const std::string& category_name, const std::string& group_name, const Alpha& obj) noexcept : category_name(category_name), group_name(group_name), obj_alpha_(obj)
		{
		}

		std::string category_name;
		std::string group_name;
		TriangleFan obj_triangle_fan;
		TriangleStrip obj_triangle_strip;
		LineLoop obj_line_loop;
		LineStrip obj_line_strip;
		BasicSphere obj_basic_sphere;
		Color obj_color_;
		Alpha obj_alpha_;
	};

	class PrinterForPymol
	{
	public:
		PrinterForPymol() : prefix(""), postfix(", \n")
		{
		}

		void print(const PrimitivePart::TriangleFan& obj, std::ostream& output) const noexcept
		{
			if(!obj.active)
			{
				return;
			}
			output << prefix << "BEGIN, TRIANGLE_FAN";
			if(!obj.outer_points.empty())
			{
				output << ", \nNORMAL, " << obj.normal.x << ", " << obj.normal.y << ", " << obj.normal.z;
				output << ", VERTEX, " << obj.center.x << ", " << obj.center.y << ", " << obj.center.z;
				for(std::size_t j=0;j<obj.outer_points.size();j++)
				{
					output << ", \nNORMAL, " << obj.normal.x << ", " << obj.normal.y << ", " << obj.normal.z;
					output << ", VERTEX, " << obj.outer_points[j].x << ", " << obj.outer_points[j].y << ", " << obj.outer_points[j].z;
				}
				output << ", \nNORMAL, " << obj.normal.x << ", " << obj.normal.y << ", " << obj.normal.z;
				output << ", VERTEX, " << obj.outer_points[0].x << ", " << obj.outer_points[0].y << ", " << obj.outer_points[0].z;
			}
			output << ", \nEND" << postfix;
		}

		void print(const PrimitivePart::TriangleStrip& obj, std::ostream& output) const noexcept
		{
			if(!obj.active)
			{
				return;
			}
			output << prefix << "BEGIN, TRIANGLE_STRIP";
			if(!obj.points.empty())
			{
				for(std::size_t j=0;j<obj.points.size() && j<obj.normals.size();j++)
				{
					output << ", \nNORMAL, " << obj.normals[j].x << ", " << obj.normals[j].y << ", " << obj.normals[j].z;
					output << ", VERTEX, " << obj.points[j].x << ", " << obj.points[j].y << ", " << obj.points[j].z;
				}
			}
			output << ", \nEND" << postfix;
		}

		void print(const PrimitivePart::LineLoop& obj, std::ostream& output) const noexcept
		{
			if(!obj.active)
			{
				return;
			}
			output << prefix << "BEGIN, LINE_LOOP";
			if(!obj.outer_points.empty())
			{
				for(std::size_t j=0;j<obj.outer_points.size();j++)
				{
					output << ", \nVERTEX, " << obj.outer_points[j].x << ", " << obj.outer_points[j].y << ", " << obj.outer_points[j].z;
				}
			}
			output << ", \nEND" << postfix;
		}

		void print(const PrimitivePart::LineStrip& obj, std::ostream& output) const noexcept
		{
			if(!obj.active)
			{
				return;
			}
			output << prefix << "BEGIN, LINE_STRIP";
			if(!obj.points.empty())
			{
				for(std::size_t j=0;j<obj.points.size();j++)
				{
					output << ", \nVERTEX, " << obj.points[j].x << ", " << obj.points[j].y << ", " << obj.points[j].z;
				}
			}
			output << ", \nEND" << postfix;
		}

		void print(const PrimitivePart::BasicSphere& obj, std::ostream& output) const noexcept
		{
			if(!obj.active)
			{
				return;
			}
			output << prefix << "SPHERE" << ", " << obj.sphere.p.x << ", " << obj.sphere.p.y << ", " << obj.sphere.p.z << ", " << obj.sphere.r << postfix;
		}

		void print(const PrimitivePart::Color& obj, std::ostream& output) const noexcept
		{
			if(!obj.active)
			{
				return;
			}
			output << prefix << "COLOR, " << obj.r << ", " << obj.g << ", " << obj.b << postfix;
		}

		void print(const PrimitivePart::Alpha& obj, std::ostream& output) const noexcept
		{
			if(!obj.active)
			{
				return;
			}
			output << prefix << "ALPHA, " << obj.a << postfix;
		}

		void print(const PrimitivePart& part, std::ostream& output) const noexcept
		{
			print(part.obj_color_, output);
			print(part.obj_alpha_, output);
			print(part.obj_triangle_fan, output);
			print(part.obj_triangle_strip, output);
			print(part.obj_line_loop, output);
			print(part.obj_line_strip, output);
			print(part.obj_basic_sphere, output);
		}

		bool print(const std::string& title, const std::vector<PrimitivePart>& parts, std::ostream& output) const noexcept
		{
			if(!output.good())
			{
				return false;
			}

			std::set< std::pair<std::string, std::string> > category_group_pairs;
			for(std::size_t i=0;i<parts.size();i++)
			{
				if(!parts[i].group_name.empty())
				{
					category_group_pairs.insert(std::pair<std::string, std::string>(parts[i].category_name, parts[i].group_name));
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
				const std::string& current_category=it->first;
				const std::string& current_group=it->second;
				output << "cgo_graphics_list_" << current_category << "_" << current_group << " = [";
				for(std::size_t i=0;i<parts.size();i++)
				{
					const PrimitivePart& part=parts[i];
					if(part.category_name==current_category && (part.group_name.empty() || part.group_name==current_group))
					{
						print(part, output);
					}
				}
				output << "]\n";
				output << "cmd.load_cgo(cgo_graphics_list_" << current_category << "_" << current_group;
				output << ", '" << (title.empty() ? std::string("cgo") : title) << "_" << current_category << "_" << current_group << "')\n";
			}

			output << "cmd.set('two_sided_lighting', 1)\n";
			output << "cmd.set('cgo_line_width', 3)\n";

			return true;
		}

		std::string prefix;
		std::string postfix;
	};

	bool enabled_;
	uint64_t color_randomization_state_;
	std::vector<PrimitivePart> parts_;
};


}

#endif /* VORONOTALT_GRAPHICS_WRITER_H_ */
