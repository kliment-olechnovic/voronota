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

	void add_label(const std::string& label)
	{
		string_stream_ << "label " << label << " ";
	}

	std::string str() const
	{
		return string_stream_.str();
	}

	void print_pymol_script(const std::string& obj_name, const bool two_sided_lighting, std::ostream& output)
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
			const bool type_alpha=(type_str=="alpha");
			const bool type_color=(type_str=="color");
			const bool type_label=(type_str=="label");
			const bool type_tstrip=(type_str=="tstrip");
			const bool type_tfan=(type_str=="tfan");
			const bool type_tfanc=(type_str=="tfanc");
			if(type_alpha)
			{
				double alpha=1.0;
				input >> alpha;
				output << "ALPHA, " << alpha << sep;
			}
			else if(type_color)
			{
				write_color_to_stream(read_color_from_stream(input), true, "COLOR, ", sep, sep, output);
			}
			else if(type_label)
			{
				std::string label;
				input >> label;
			}
			else if(type_tstrip || type_tfan || type_tfanc)
			{
				std::vector<PlainPoint> vertices;
				std::vector<PlainPoint> normals;
				if(read_strip_or_fan_from_stream(type_tstrip, type_tfan, type_tfanc, input, vertices, normals))
				{
					output << (type_tstrip ? "BEGIN, TRIANGLE_STRIP, " : "BEGIN, TRIANGLE_FAN, ");
					for(std::size_t i=0;i<vertices.size();i++)
					{
						write_point_to_stream(normals[i], "NORMAL, ", sep, sep, output);
						write_point_to_stream(vertices[i], "VERTEX, ", sep, sep, output);
					}
					output << "END, ";
				}
			}
		}
		output << "]\n";
		output << "cmd.load_cgo(" << obj_name << ", '" << obj_name << "')\n";
		output << "cmd.set('two_sided_lighting', '" << (two_sided_lighting ? "on" : "off") << "')\n";
	}

	void print_jmol_script(const std::string& obj_name, std::ostream& output)
	{
		std::istringstream input(str());
		double alpha=1.0;
		Color color(0xFFFFFF);
		std::string label;
		std::vector<PlainPoint> global_vertices;
		std::vector<PlainTriple> global_triples;
		while(input.good())
		{
			std::string type_str;
			input >> type_str;
			const bool type_alpha=(type_str=="alpha");
			const bool type_color=(type_str=="color");
			const bool type_label=(type_str=="label");
			const bool type_tstrip=(type_str=="tstrip");
			const bool type_tfan=(type_str=="tfan");
			const bool type_tfanc=(type_str=="tfanc");
			if(type_alpha || type_color || type_label)
			{
				print_jmol_polygon(global_vertices, global_triples, label, color, alpha, obj_name, output);
				if(type_alpha)
				{
					input >> alpha;
					alpha=(1.0-alpha);
				}
				else if(type_color)
				{
					color=read_color_from_stream(input);
				}
				else if(type_label)
				{
					input >> label;
				}
			}
			else if(type_tstrip || type_tfan || type_tfanc)
			{
				std::vector<PlainPoint> vertices;
				std::vector<PlainPoint> normals;
				if(read_strip_or_fan_from_stream(type_tstrip, type_tfan, type_tfanc, input, vertices, normals))
				{
					const std::size_t offset=global_vertices.size();
					global_vertices.insert(global_vertices.end(), vertices.begin(), vertices.end());
					for(std::size_t i=0;(i+2)<vertices.size();i++)
					{
						global_triples.push_back(PlainTriple(offset+(type_tstrip ? i : 0), offset+(i+1), offset+(i+2)));
					}
				}
			}
		}
		print_jmol_polygon(global_vertices, global_triples, label, color, alpha, obj_name, output);
	}

private:
	struct PlainPoint
	{
		double x;
		double y;
		double z;
	};

	struct PlainTriple
	{
		std::size_t a;
		std::size_t b;
		std::size_t c;

		PlainTriple() : a(0), b(0), c(0)
		{
		}

		PlainTriple(const std::size_t a, const std::size_t b, const std::size_t c) : a(a), b(b), c(c)
		{
		}
	};

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

	static PlainPoint read_point_from_stream(std::istream& input)
	{
		PlainPoint p;
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

	static std::vector<PlainPoint> read_points_vector_from_stream(std::istream& input)
	{
		std::size_t n=0;
		input >> n;
		std::vector<PlainPoint> v(n);
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

	static bool read_strip_or_fan_from_stream(
			const bool tstrip,
			const bool tfan,
			const bool tfanc,
			std::istream& input,
			std::vector<PlainPoint>& vertices,
			std::vector<PlainPoint>& normals)
	{
		vertices.clear();
		normals.clear();
		if(tstrip || tfan)
		{
			vertices=read_points_vector_from_stream(input);
			normals=read_points_vector_from_stream(input);
		}
		else if(tfanc)
		{
			const PlainPoint center=read_point_from_stream(input);
			const PlainPoint normal=read_point_from_stream(input);
			const std::vector<PlainPoint> outer_vertices=read_points_vector_from_stream(input);
			if(!outer_vertices.empty())
			{
				vertices.push_back(center);
				vertices.insert(vertices.end(), outer_vertices.begin(), outer_vertices.end());
				vertices.push_back(outer_vertices.front());
				normals.resize(vertices.size(), normal);
			}
		}
		return (vertices.size()>=3 && vertices.size()==normals.size());
	}

	static void print_jmol_polygon(
			std::vector<PlainPoint>& vertices,
			std::vector<PlainTriple>& triples,
			const std::string& label,
			const Color& color,
			const double alpha,
			const std::string& id,
			std::ostream& output)
	{
		static int use_num=0;
		if(!(vertices.empty() || triples.empty()))
		{
			output << "draw " << id << use_num << " ";
			if(!label.empty())
			{
				output << "\"" << label << "\" ";
			}
			output << "POLYGON " << vertices.size() << " ";
			for(std::size_t i=0;i<vertices.size();i++)
			{
				write_point_to_stream(vertices[i], "{", " ", "} ", output);
			}
			output  << triples.size();
			for(std::size_t i=0;i<triples.size();i++)
			{
				const PlainTriple& t=triples[i];
				output << " [" << t.a << " " << t.b << " " << t.c << " 0]";
			}
			output << "\n";
			output << "color $" << id << use_num;
			if(alpha>0.0)
			{
				output << " TRANSLUCENT " << alpha;
			}
			write_color_to_stream(color, false, " [", ",", "]\n", output);
			use_num++;
		}
		vertices.clear();
		triples.clear();
	}

	std::ostringstream string_stream_;
};

}

#endif /* APOLLOTA_OPENGL_PRINTER_H_ */
