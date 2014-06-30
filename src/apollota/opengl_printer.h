#ifndef APOLLOTA_OPENGL_PRINTER_H_
#define APOLLOTA_OPENGL_PRINTER_H_

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>

#include "basic_operations_on_points.h"

namespace apollota
{

class OpenGLPrinter
{
public:
	OpenGLPrinter()
	{
	}

	void add(const std::string& str)
	{
		string_stream_ << " " << str;
	}

	void add_alpha(const double alpha)
	{
		string_stream_ << "alpha " << alpha << " ";
	}

	void add_color(const unsigned int rgb)
	{
		string_stream_ << "color ";
		write_color_to_stream(Color(rgb), string_stream_);
	}

	template<typename PointType>
	void add_triangle_strip(const std::vector<PointType>& vertices, const std::vector<PointType>& normals)
	{
		if(!vertices.empty() && vertices.size()==normals.size())
		{
			string_stream_ << "tstrip ";
			write_points_vector_to_stream(vertices, string_stream_);
			write_points_vector_to_stream(normals, string_stream_);
		}
	}

	template<typename PointType>
	void add_triangle_fan(const std::vector<PointType>& vertices, const std::vector<PointType>& normals)
	{
		if(!vertices.empty() && vertices.size()==normals.size())
		{
			string_stream_ << "tfan ";
			write_points_vector_to_stream(vertices, string_stream_);
			write_points_vector_to_stream(normals, string_stream_);
		}
	}

	template<typename PointType>
	void add_triangle_fan(const PointType& center, const std::vector<PointType>& vertices, const PointType& normal)
	{
		if(!vertices.empty())
		{
			string_stream_ << "tfanc ";
			write_point_to_stream(center, string_stream_);
			write_point_to_stream(normal, string_stream_);
			write_points_vector_to_stream(vertices, string_stream_);
		}
	}

	bool empty() const
	{
		return string_stream_.str().empty();
	}

	std::string str() const
	{
		return string_stream_.str();
	}

	void print_pymol_script(const std::string& obj_name, const std::string& cgo_name, const bool two_sided_lighting, std::ostream& output)
	{
		const std::string sep=", ";
		std::istringstream input(str());
		output << "from pymol.cgo import *\n";
		output << "from pymol import cmd\n";
		output << obj_name << " = [";
		while(input.good())
		{
			std::string type_str;
			input >> type_str;
			if(type_str=="alpha")
			{
				double alpha=1.0;
				input >> alpha;
				output << "ALPHA, " << alpha << sep;
			}
			else if(type_str=="color")
			{
				write_color_to_stream(read_color_from_stream(input), true, "COLOR, ", sep, sep, output);
			}
			else if(type_str=="tstrip" || type_str=="tfan")
			{
				const std::vector<PODPoint> vertices=read_points_vector_from_stream(input);
				const std::vector<PODPoint> normals=read_points_vector_from_stream(input);
				if(!vertices.empty() && vertices.size()==normals.size())
				{
					output << (type_str=="tstrip" ? "BEGIN, TRIANGLE_STRIP, " : "BEGIN, TRIANGLE_FAN, ");
					for(std::size_t i=0;i<vertices.size();i++)
					{
						write_point_to_stream(normals[i], "NORMAL, ", sep, sep, output);
						write_point_to_stream(vertices[i], "VERTEX, ", sep, sep, output);
					}
					output << "END, ";
				}
			}
			else if(type_str=="tfanc")
			{
				const PODPoint center=read_point_from_stream(input);
				const PODPoint normal=read_point_from_stream(input);
				const std::vector<PODPoint> vertices=read_points_vector_from_stream(input);
				output << "BEGIN, TRIANGLE_FAN, ";
				write_point_to_stream(normal, "NORMAL, ", sep, sep, output);
				write_point_to_stream(center, "VERTEX, ", sep, sep, output);
				for(std::size_t i=0;i<vertices.size();i++)
				{
					write_point_to_stream(normal, "NORMAL, ", sep, sep, output);
					write_point_to_stream(vertices[i], "VERTEX, ", sep, sep, output);
				}
				write_point_to_stream(normal, "NORMAL, ", sep, sep, output);
				write_point_to_stream(vertices.front(), "VERTEX, ", sep, sep, output);
				output << "END, ";
			}
		}
		output << "]\n";
		output << "cmd.load_cgo(" << obj_name << ", '" << cgo_name << "')\n";
		output << "cmd.set('two_sided_lighting', '" << (two_sided_lighting ? "on" : "off") << "')\n";
	}

private:
	struct Color
	{
		unsigned int r;
		unsigned int g;
		unsigned int b;

		Color() : r(0), g(0), b(0)
		{
		}

		Color(const unsigned int rgb) : r((rgb&0xFF0000) >> 16), g((rgb&0x00FF00) >> 8), b(rgb&0x0000FF)
		{
		}
	};

	template<typename PointType>
	static void write_point_to_stream(const PointType& p, const std::string& start, const std::string& sep, const std::string& end, std::ostream& output)
	{
		output << start;
		output.precision(3);
		output << std::fixed << p.x << sep << p.y << sep << p.z;
		output << end;
	}

	template<typename PointType>
	static void write_point_to_stream(const PointType& p, std::ostream& output)
	{
		write_point_to_stream(p, "", " ", " ", output);
	}

	static PODPoint read_point_from_stream(std::istream& input)
	{
		PODPoint p;
		input >> p.x >> p.y >> p.z;
		return p;
	}

	template<typename PointType>
	static void write_points_vector_to_stream(const std::vector<PointType>& v, std::ostream& output)
	{
		output << v.size() << " ";
		for(std::size_t i=0;i<v.size();i++)
		{
			write_point_to_stream(v[i], output);
		}
	}

	static std::vector<PODPoint> read_points_vector_from_stream(std::istream& input)
	{
		std::size_t n=0;
		input >> n;
		std::vector<PODPoint> v(n);
		for(std::size_t i=0;i<v.size();i++)
		{
			v[i]=read_point_from_stream(input);
		}
		return v;
	}

	static void write_color_to_stream(const Color& c, const bool normalized, const std::string& start, const std::string& sep, const std::string& end, std::ostream& output)
	{
		output << start;
		if(normalized)
		{
			output.precision(3);
			output << std::fixed << (static_cast<double>(c.r)/255.0) << sep << (static_cast<double>(c.g)/255.0) << sep << (static_cast<double>(c.b)/255.0);
		}
		else
		{
			output << c.r << sep << c.g << sep << c.b;
		}
		output << end;
	}

	static void write_color_to_stream(const Color& c, std::ostream& output)
	{
		write_color_to_stream(c, false, "", " ", " ", output);
	}

	static Color read_color_from_stream(std::istream& input)
	{
		Color c;
		input >> c.r >> c.g >> c.b;
		return c;
	}

	std::ostringstream string_stream_;
};

}

#endif /* APOLLOTA_OPENGL_PRINTER_H_ */
