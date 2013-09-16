#ifndef APOLLOTA_OPENGL_PRINTER_H_
#define APOLLOTA_OPENGL_PRINTER_H_

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>

namespace apollota
{

class OpenGLPrinter
{
public:
	OpenGLPrinter(std::ostream& output_stream, const std::string& obj_name, const std::string& cgo_name) : output_stream_(output_stream), obj_name_(obj_name), cgo_name_(cgo_name)
	{
		string_stream_ << obj_name_ << " = [\n";
	}

	~OpenGLPrinter()
	{
		string_stream_ << "]\ncmd.load_cgo(" << obj_name_ << ", '" << cgo_name_ << "')\n";
		output_stream_ << string_stream_.str() << "\n";
	}

	static void print_setup(std::ostream& output_stream)
	{
		output_stream << "from pymol.cgo import *\n";
		output_stream << "from pymol import cmd\n\n";
	}

	void print_alpha(const double alpha)
	{
		string_stream_ << "    ALPHA, " << alpha << ",\n";
	}

	void print_color(const double r, const double g, const double b)
	{
		string_stream_ << "    COLOR, " << rgb_to_string(r, g, b) << ",\n";
	}

	void print_color(const unsigned int rgb)
	{
		string_stream_ << "    COLOR, " << rgb_to_string(rgb) << ",\n";
	}

	template<typename SphereType>
	void print_sphere(const SphereType& sphere)
	{
		string_stream_ << "    SPHERE, " << point_to_string(sphere) << ", " << sphere.r << ",\n";
	}

	template<typename PointType>
	void print_line_strip(const std::vector<PointType>& vertices, const bool loop=false)
	{
		if(!vertices.empty())
		{
			if(loop)
			{
				string_stream_ << "    BEGIN, LINE_LOOP,\n";
			}
			else
			{
				string_stream_ << "    BEGIN, LINE_STRIP,\n";
			}
			for(std::size_t i=0;i<vertices.size();i++)
			{
				string_stream_ << "    VERTEX, " << point_to_string(vertices[i]) << ",\n";
			}
			string_stream_ << "    END,\n";
		}
	}

	template<typename PointType>
	void print_triangle_strip(const std::vector<PointType>& vertices, const std::vector<PointType>& normals, const bool fan=false)
	{
		if(!vertices.empty() && vertices.size()==normals.size())
		{
			if(fan)
			{
				string_stream_ << "    BEGIN, TRIANGLE_FAN,\n";
			}
			else
			{
				string_stream_ << "    BEGIN, TRIANGLE_STRIP,\n";
			}
			for(std::size_t i=0;i<vertices.size();i++)
			{
				string_stream_ << "    NORMAL, " << point_to_string(normals[i]) << ",\n";
				string_stream_ << "    VERTEX, " << point_to_string(vertices[i]) << ",\n";
			}
			string_stream_ << "    END,\n";
		}
	}

	template<typename PointType>
	void print_cylinder(const PointType& p1, const PointType& p2, const double radius, const unsigned int rgb1, const unsigned int rgb2)
	{
		string_stream_ << "    CYLINDER, " << point_to_string(p1) << ", " << point_to_string(p2) << ", " << radius << ", " << rgb_to_string(rgb1) << ", " << rgb_to_string(rgb2) << ",\n";
	}

private:
	OpenGLPrinter(const OpenGLPrinter& /*opengl_printer*/);
	OpenGLPrinter& operator=(const OpenGLPrinter& /*opengl_printer*/);

	template<typename PointType>
	static std::string point_to_string(const PointType& a)
	{
		std::ostringstream output;
		output.precision(std::numeric_limits<double>::digits10);
		output << std::fixed << a.x << ", " << a.y << ", " << a.z;
		return output.str();
	}

	static std::string rgb_to_string(const double r, const double g, const double b)
	{
		std::ostringstream output;
		output << r << ", " << g << ", " << b;
		return output.str();
	}

	static std::string rgb_to_string(const unsigned int rgb)
	{
		return rgb_to_string(static_cast<double>((rgb&0xFF0000) >> 16)/255.0, static_cast<double>((rgb&0x00FF00) >> 8)/255.0, static_cast<double>(rgb&0x0000FF)/255.0);
	}

	std::ostream& output_stream_;
	const std::string obj_name_;
	const std::string cgo_name_;
	std::ostringstream string_stream_;
};

}

#endif /* APOLLOTA_OPENGL_PRINTER_H_ */
