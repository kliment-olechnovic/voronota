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
	explicit GraphicsWriter(const bool enabled) noexcept : enabled_(enabled), color_randomization_state_(static_cast<uint64_t>(42)), last_color_part_index_(false, 0), last_alpha_part_index_(false, 0)
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
		parts_.push_back(PrimitivePart(BundlingID(category_name, group_name), PrimitivePart::TriangleFan(outer_points, center, normal)));
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
		parts_.push_back(PrimitivePart(BundlingID(category_name, group_name), PrimitivePart::TriangleStrip(points, normals)));
	}

	void add_triangle_on_sphere(const std::string& category_name, const std::string& group_name, const SimplePoint& sp, const SimplePoint& p1, const SimplePoint& p2, const SimplePoint& p3) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		std::vector<voronotalt::SimplePoint> tvertices(3);
		std::vector<voronotalt::SimplePoint> tnormals(3);
		tvertices[0]=p1;
		tvertices[1]=p2;
		tvertices[2]=p3;
		tnormals[0]=voronotalt::unit_point(voronotalt::sub_of_points(tvertices[0], sp));
		tnormals[1]=voronotalt::unit_point(voronotalt::sub_of_points(tvertices[1], sp));
		tnormals[2]=voronotalt::unit_point(voronotalt::sub_of_points(tvertices[2], sp));
		const voronotalt::SimplePoint refnormal=voronotalt::unit_point(voronotalt::cross_product(voronotalt::sub_of_points(tvertices[1], tvertices[0]), voronotalt::sub_of_points(tvertices[2], tvertices[0])));
		if(voronotalt::dot_product(refnormal, tnormals[0])<0.0)
		{
			std::swap(tvertices[1], tvertices[2]);
			std::swap(tnormals[1], tnormals[2]);
		}
		add_triangle_strip(category_name, group_name, tvertices, tnormals);
	}

	void add_line_loop(const std::string& category_name, const std::string& group_name, const std::vector<SimplePoint>& outer_points) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		parts_.push_back(PrimitivePart(BundlingID(category_name, group_name), PrimitivePart::LineLoop(outer_points)));
	}

	void add_line_strip(const std::string& category_name, const std::string& group_name, const std::vector<SimplePoint>& points) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		parts_.push_back(PrimitivePart(BundlingID(category_name, group_name), PrimitivePart::LineStrip(points)));
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
		parts_.push_back(PrimitivePart(BundlingID(category_name, group_name), PrimitivePart::BasicSphere(sphere, radius_change)));
	}

	void add_color(const std::string& category_name, const std::string& group_name, const double r, const double g, const double b) noexcept
	{
		if(!enabled_)
		{
			return;
		}
		bool need_to_add=true;
		if(last_color_part_index_.first)
		{
			const PrimitivePart& lcp=parts_[last_color_part_index_.second];
			if(lcp.obj_color_.r==r && lcp.obj_color_.g==g && lcp.obj_color_.b==b && lcp.bundling_id.category_name==category_name && (lcp.bundling_id.group_name.empty() || lcp.bundling_id.group_name==group_name))
			{
				need_to_add=false;
			}
		}
		if(need_to_add)
		{
			parts_.push_back(PrimitivePart(BundlingID(category_name, group_name), PrimitivePart::Color(r, g, b)));
			last_color_part_index_.first=true;
			last_color_part_index_.second=(parts_.size()-1);
		}
	}

	void add_color(const std::string& category_name, const std::string& group_name, const unsigned int rgb) noexcept
	{
		if(!enabled_)
		{
			return;
		}

		if(rgb==0 && !group_name.empty())
		{
			add_random_color(category_name, group_name);
		}
		else
		{
			add_color(category_name, group_name, static_cast<double>((rgb&0xFF0000) >> 16)/static_cast<double>(0xFF), static_cast<double>((rgb&0x00FF00) >> 8)/static_cast<double>(0xFF), static_cast<double>(rgb&0x0000FF)/static_cast<double>(0xFF));
			if(rgb>0xFFFFFF)
			{
				add_alpha(category_name, group_name, static_cast<double>((rgb&0xFF000000) >> 24)/static_cast<double>(0xFF));
			}
		}
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
		const unsigned int rgb=static_cast<unsigned int>(x%static_cast<uint64_t>(0xFFFFFF));
		add_color(category_name, group_name, static_cast<double>((rgb&0xFF0000) >> 16)/static_cast<double>(0xFF), static_cast<double>((rgb&0x00FF00) >> 8)/static_cast<double>(0xFF), static_cast<double>(rgb&0x0000FF)/static_cast<double>(0xFF));
	}

	void add_alpha(const std::string& category_name, const std::string& group_name, const double a) noexcept
	{
		if(!enabled_)
		{
			return;
		}


		bool need_to_add=true;
		if(last_alpha_part_index_.first)
		{
			const PrimitivePart& lcp=parts_[last_alpha_part_index_.second];
			if(lcp.obj_alpha_.a==a && lcp.bundling_id.category_name==category_name && (lcp.bundling_id.group_name.empty() || lcp.bundling_id.group_name==group_name))
			{
				need_to_add=false;
			}
		}
		if(need_to_add)
		{
			parts_.push_back(PrimitivePart(BundlingID(category_name, group_name), PrimitivePart::Alpha(a)));
			last_alpha_part_index_.first=true;
			last_alpha_part_index_.second=(parts_.size()-1);
		}
	}

	bool write_to_file_for_pymol(const std::string& title, const std::string& filename) const noexcept
	{
		return write_to_file(title, filename, OutputFormat::pymol_cgo);
	}

	bool write_to_file_for_chimera(const std::string& title, const std::string& filename) const noexcept
	{
		return write_to_file(title, filename, OutputFormat::chimera_bild);
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

		return write_to_file(title, filename, detect_output_format_from_name(filename));
	}

private:
	struct BundlingID
	{
		BundlingID() noexcept
		{
		}

		BundlingID(const std::string& category_name, const std::string& group_name) noexcept : category_name(category_name), group_name(group_name)
		{
		}

		bool operator<(const BundlingID& v) const
		{
			return ((category_name<v.category_name) || (category_name==v.category_name && group_name<v.group_name));
		}

		std::string category_name;
		std::string group_name;
	};

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

		explicit PrimitivePart(const BundlingID& bundling_id) noexcept : bundling_id(bundling_id)
		{
		}

		PrimitivePart(const BundlingID& bundling_id, const TriangleFan& obj) noexcept : bundling_id(bundling_id), obj_triangle_fan(obj)
		{
		}

		PrimitivePart(const BundlingID& bundling_id, const TriangleStrip& obj) noexcept : bundling_id(bundling_id), obj_triangle_strip(obj)
		{
		}

		PrimitivePart(const BundlingID& bundling_id, const LineLoop& obj) noexcept : bundling_id(bundling_id), obj_line_loop(obj)
		{
		}

		PrimitivePart(const BundlingID& bundling_id, const LineStrip& obj) noexcept : bundling_id(bundling_id), obj_line_strip(obj)
		{
		}

		PrimitivePart(const BundlingID& bundling_id, const BasicSphere& obj) noexcept : bundling_id(bundling_id), obj_basic_sphere(obj)
		{
		}

		PrimitivePart(const BundlingID& bundling_id, const Color& obj) noexcept : bundling_id(bundling_id), obj_color_(obj)
		{
		}

		PrimitivePart(const BundlingID& bundling_id, const Alpha& obj) noexcept : bundling_id(bundling_id), obj_alpha_(obj)
		{
		}

		BundlingID bundling_id;
		TriangleFan obj_triangle_fan;
		TriangleStrip obj_triangle_strip;
		LineLoop obj_line_loop;
		LineStrip obj_line_strip;
		BasicSphere obj_basic_sphere;
		Color obj_color_;
		Alpha obj_alpha_;
	};

	class PrinterForPymolCGO
	{
	public:
		PrinterForPymolCGO() : prefix(""), postfix(", \n")
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
			output << prefix << "ALPHA, " << (1.0-obj.a) << postfix;
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

			std::set<BundlingID> main_bundling_ids;
			for(std::size_t i=0;i<parts.size();i++)
			{
				if(!parts[i].bundling_id.group_name.empty())
				{
					main_bundling_ids.insert(parts[i].bundling_id);
				}
			}

			if(main_bundling_ids.empty())
			{
				return false;
			}

			output << "from pymol.cgo import *\n";
			output << "from pymol import cmd\n";

			for(std::set<BundlingID>::const_iterator it=main_bundling_ids.begin();it!=main_bundling_ids.end();++it)
			{
				const BundlingID& current_bundling_id=(*it);
				output << "cgo_graphics_list_" << current_bundling_id.category_name << "_" << current_bundling_id.group_name << " = [";
				for(std::size_t i=0;i<parts.size();i++)
				{
					const PrimitivePart& part=parts[i];
					if(part.bundling_id.category_name==current_bundling_id.category_name && (part.bundling_id.group_name.empty() || part.bundling_id.group_name==current_bundling_id.group_name))
					{
						print(part, output);
					}
				}
				output << "]\n";
				output << "cmd.load_cgo(cgo_graphics_list_" << current_bundling_id.category_name << "_" << current_bundling_id.group_name;
				output << ", '" << (title.empty() ? std::string("cgo") : title) << "_" << current_bundling_id.category_name << "_" << current_bundling_id.group_name << "')\n";
			}

			output << "cmd.set('two_sided_lighting', 1)\n";
			output << "cmd.set('cgo_line_width', 1)\n";

			return true;
		}

		std::string prefix;
		std::string postfix;
	};

	class PrinterForChimeraBILD
	{
	public:
		PrinterForChimeraBILD() : sep(" "), bigsep("\n")
		{
		}

		void print(const PrimitivePart::TriangleFan& obj, std::ostream& output) const noexcept
		{
			if(!obj.active)
			{
				return;
			}
			if(!obj.outer_points.empty())
			{
				for(std::size_t j=0;j<obj.outer_points.size();j++)
				{
					output << ".polygon";
					output << sep << obj.center.x << sep << obj.center.y << sep << obj.center.z;
					output << sep << obj.outer_points[j].x << sep << obj.outer_points[j].y << sep << obj.outer_points[j].z;
					if((j+1)<obj.outer_points.size())
					{
						output << sep << obj.outer_points[j+1].x << sep << obj.outer_points[j+1].y << sep << obj.outer_points[j+1].z;
					}
					else
					{
						output << sep << obj.outer_points[0].x << sep << obj.outer_points[0].y << sep << obj.outer_points[0].z;
					}
					output << bigsep;
				}
			}
		}

		void print(const PrimitivePart::TriangleStrip& obj, std::ostream& output) const noexcept
		{
			if(!obj.active)
			{
				return;
			}
			if(!obj.points.empty())
			{
				for(std::size_t j=0;(j+2)<obj.points.size();j++)
				{
					output << ".polygon";
					output << sep << obj.points[j].x << sep << obj.points[j].y << sep << obj.points[j].z;
					output << sep << obj.points[j+1].x << sep << obj.points[j+1].y << sep << obj.points[j+1].z;
					output << sep << obj.points[j+2].x << sep << obj.points[j+2].y << sep << obj.points[j+2].z;
					output << bigsep;
				}
			}
		}

		void print(const PrimitivePart::LineLoop& obj, std::ostream& output) const noexcept
		{
			if(!obj.active)
			{
				return;
			}
			if(!obj.outer_points.empty())
			{
				for(std::size_t j=0;j<obj.outer_points.size();j++)
				{
					output << (j==0 ? ".move" : ".draw") << sep << obj.outer_points[j].x << sep << obj.outer_points[j].y << sep << obj.outer_points[j].z << bigsep;
				}
				output << ".draw" << sep << obj.outer_points[0].x << sep << obj.outer_points[0].y << sep << obj.outer_points[0].z << bigsep;
			}
		}

		void print(const PrimitivePart::LineStrip& obj, std::ostream& output) const noexcept
		{
			if(!obj.active)
			{
				return;
			}
			if(!obj.points.empty())
			{
				for(std::size_t j=0;j<obj.points.size();j++)
				{
					output << (j==0 ? ".move" : ".draw") << sep << obj.points[j].x << sep << obj.points[j].y << sep << obj.points[j].z << bigsep;
				}
			}
		}

		void print(const PrimitivePart::BasicSphere& obj, std::ostream& output) const noexcept
		{
			if(!obj.active)
			{
				return;
			}
			output << ".sphere" << sep << obj.sphere.p.x << sep << obj.sphere.p.y << sep << obj.sphere.p.z << sep << obj.sphere.r << bigsep;
		}

		void print(const PrimitivePart::Color& obj, std::ostream& output) const noexcept
		{
			if(!obj.active)
			{
				return;
			}
			output << ".color" << sep << obj.r << sep << obj.g << sep << obj.b << bigsep;
		}

		void print(const PrimitivePart::Alpha& obj, std::ostream& output) const noexcept
		{
			if(!obj.active)
			{
				return;
			}
			output << ".transparency" << sep << obj.a << bigsep;
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

		bool print(const std::vector<PrimitivePart>& parts, std::ostream& output) const noexcept
		{
			if(!output.good())
			{
				return false;
			}

			std::set<BundlingID> main_bundling_ids;
			for(std::size_t i=0;i<parts.size();i++)
			{
				if(!parts[i].bundling_id.group_name.empty())
				{
					main_bundling_ids.insert(parts[i].bundling_id);
				}
			}

			if(main_bundling_ids.empty())
			{
				return false;
			}

			for(std::set<BundlingID>::const_iterator it=main_bundling_ids.begin();it!=main_bundling_ids.end();++it)
			{
				const BundlingID& current_bundling_id=(*it);
				for(std::size_t i=0;i<parts.size();i++)
				{
					const PrimitivePart& part=parts[i];
					if(part.bundling_id.category_name==current_bundling_id.category_name && (part.bundling_id.group_name.empty() || part.bundling_id.group_name==current_bundling_id.group_name))
					{
						print(part, output);
					}
				}
			}

			return true;
		}

		std::string sep;
		std::string bigsep;
	};

	struct OutputFormat
	{
		enum ID
		{
			pymol_cgo,
			chimera_bild,
			undefined
		};
	};

	static bool match_name_extension(const std::string& str, const std::string& ext) noexcept
	{
	    if(ext.size()>str.size())
		{
			return false;
		}
		std::string::const_iterator it_str=str.begin()+(str.size()-ext.size());
		std::string::const_iterator it_ext=ext.begin();
		while(it_ext!=ext.end())
		{
			if(std::tolower(static_cast<unsigned char>(*it_str))!=std::tolower(static_cast<unsigned char>(*it_ext)))
			{
				return false;
			}
			++it_ext;
			++it_str;
		}
	    return true;
	}

	static OutputFormat::ID detect_output_format_from_name(const std::string& str) noexcept
	{
		if(match_name_extension(str, ".py"))
		{
			return OutputFormat::pymol_cgo;
		}
		else if(match_name_extension(str, ".bild") || match_name_extension(str, ".bld"))
		{
			return OutputFormat::chimera_bild;
		}
		return OutputFormat::undefined;
	}

	bool write_to_file(const std::string& title, const std::string& filename, const OutputFormat::ID output_format) const noexcept
	{
		if(!enabled_)
		{
			return false;
		}

		if(filename.empty())
		{
			return false;
		}

		if(output_format!=OutputFormat::chimera_bild && output_format!=OutputFormat::pymol_cgo)
		{
			return false;
		}

		std::ofstream output(filename, std::ios::out);
		if(!output.good())
		{
			return false;
		}

		if(output_format==OutputFormat::chimera_bild)
		{
			PrinterForChimeraBILD printer;
			return printer.print(parts_, output);
		}
		else if(output_format==OutputFormat::pymol_cgo)
		{
			PrinterForPymolCGO printer;
			return printer.print(title, parts_, output);
		}

		return false;
	}

	bool enabled_;
	uint64_t color_randomization_state_;
	std::pair<bool, std::size_t> last_color_part_index_;
	std::pair<bool, std::size_t> last_alpha_part_index_;
	std::vector<PrimitivePart> parts_;
};


}

#endif /* VORONOTALT_GRAPHICS_WRITER_H_ */
