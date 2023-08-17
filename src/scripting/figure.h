#ifndef SCRIPTING_FIGURE_H_
#define SCRIPTING_FIGURE_H_

#include "../common/properties_value.h"

#include "long_name.h"

namespace voronota
{

namespace scripting
{

class Figure
{
public:
	struct SpecialDescriptionForLabel
	{
		std::string label_text;
		std::vector<float> label_origin;
		bool label_outline;
		float label_scale;
		bool label_centered;
		bool is_label;

		SpecialDescriptionForLabel() : label_outline(false), label_scale(1.0f), label_centered(false), is_label(false)
		{
		}

		friend std::ostream& operator<<(std::ostream& output, const SpecialDescriptionForLabel& sd)
		{
			output << sd.is_label;
			if(sd.is_label)
			{
				std::string safe_label_text=sd.label_text;
				for(std::size_t i=0;i<sd.label_text.size();i++)
				{
					if(sd.label_text[i]<33)
					{
						safe_label_text[i]='_';
					}
				}
				output << " " << safe_label_text;
				output << " " << sd.label_origin.size();
				for(std::size_t i=0;i<sd.label_origin.size();i++)
				{
					output << " " << sd.label_origin[i];
				}
				output << " " << sd.label_outline << " " << sd.label_scale << " " << sd.label_centered;
			}
			output << "\n";
			return output;
		}

		friend std::istream& operator>>(std::istream& input, SpecialDescriptionForLabel& sd)
		{
			sd=SpecialDescriptionForLabel();
			input >> sd.is_label;
			if(sd.is_label)
			{
				input >> sd.label_text;
				for(std::size_t i=0;i<sd.label_text.size();i++)
				{
					if(sd.label_text[i]=='_')
					{
						sd.label_text[i]=' ';
					}
				}
				{
					int n=0;
					input >> n;
					if(n>0)
					{
						sd.label_origin.resize(n);
						for(std::size_t i=0;i<sd.label_origin.size();i++)
						{
							input >> sd.label_origin[i];
						}
					}
				}
				input >> sd.label_outline >> sd.label_scale >> sd.label_centered;
			}
			return input;
		}
	};

	LongName name;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<unsigned int> indices;
	common::PropertiesValue props;
	bool normals_store_origin;
	double z_shift;
	SpecialDescriptionForLabel special_description_for_label;

	Figure() : normals_store_origin(false), z_shift(0.0)
	{
	}

	bool valid() const
	{
		if(!name.valid())
		{
			return false;
		}
		if(vertices.size()<3 || vertices.size()!=normals.size() || indices.size()<3 || indices.size()%3!=0)
		{
			return false;
		}
		for(std::size_t i=0;i<indices.size();i++)
		{
			if(indices[i]>=vertices.size())
			{
				return false;
			}
		}
		return true;
	}

	bool match(const LongName& longname) const
	{
		return name.match(longname);
	}

	template<class Vertex, class Normal>
	void add_triangle(const Vertex& a, const Vertex& b, const Vertex& c, const Normal& na, const Normal& nb, const Normal& nc)
	{
		indices.push_back(vertices.size()/3);
		indices.push_back(vertices.size()/3+1);
		indices.push_back(vertices.size()/3+2);
		vertices.push_back(a.x);
		vertices.push_back(a.y);
		vertices.push_back(a.z);
		vertices.push_back(b.x);
		vertices.push_back(b.y);
		vertices.push_back(b.z);
		vertices.push_back(c.x);
		vertices.push_back(c.y);
		vertices.push_back(c.z);
		normals.push_back(na.x);
		normals.push_back(na.y);
		normals.push_back(na.z);
		normals.push_back(nb.x);
		normals.push_back(nb.y);
		normals.push_back(nb.z);
		normals.push_back(nc.x);
		normals.push_back(nc.y);
		normals.push_back(nc.z);
	}

	template<class Vertex, class Normal>
	void add_triangle(const Vertex& a, const Vertex& b, const Vertex& c, const Normal& n)
	{
		add_triangle(a, b, c, n, n, n);
	}

	template<class SubdividedIcosahedron, class Point>
	void add_sphere(const SubdividedIcosahedron& sih, const Point& center, const double radius)
	{
		for(std::size_t i=0;i<sih.triples().size();i++)
		{
			add_triangle(
					center+(sih.vertices()[sih.triples()[i].get(0)].unit()*radius),
					center+(sih.vertices()[sih.triples()[i].get(1)].unit()*radius),
					center+(sih.vertices()[sih.triples()[i].get(2)].unit()*radius),
					sih.vertices()[sih.triples()[i].get(0)].unit(),
					sih.vertices()[sih.triples()[i].get(1)].unit(),
					sih.vertices()[sih.triples()[i].get(2)].unit());
		}
	}

	template<class Point>
	void add_box(const Point& bottom_left_corner, const double l_x, const double l_y, const double l_z)
	{
		add_triangle(bottom_left_corner,                      bottom_left_corner+Point(l_x, 0.0, 0.0), bottom_left_corner+Point(l_x, l_y, 0.0), Point(0.0, 0.0, 0.0-l_z));
		add_triangle(bottom_left_corner,                      bottom_left_corner+Point(0.0, l_y, 0.0), bottom_left_corner+Point(l_x, l_y, 0.0), Point(0.0, 0.0, 0.0-l_z));
		add_triangle(bottom_left_corner+Point(0.0, 0.0, l_z), bottom_left_corner+Point(l_x, 0.0, l_z), bottom_left_corner+Point(l_x, l_y, l_z), Point(0.0, 0.0,     l_z));
		add_triangle(bottom_left_corner+Point(0.0, 0.0, l_z), bottom_left_corner+Point(0.0, l_y, l_z), bottom_left_corner+Point(l_x, l_y, l_z), Point(0.0, 0.0,     l_z));

		add_triangle(bottom_left_corner,                      bottom_left_corner+Point(l_x, 0.0, 0.0), bottom_left_corner+Point(l_x, 0.0, l_z), Point(0.0, 0.0-l_y, 0.0));
		add_triangle(bottom_left_corner,                      bottom_left_corner+Point(0.0, 0.0, l_z), bottom_left_corner+Point(l_x, 0.0, l_z), Point(0.0, 0.0-l_y, 0.0));
		add_triangle(bottom_left_corner+Point(0.0, l_y, 0.0), bottom_left_corner+Point(l_x, l_y, 0.0), bottom_left_corner+Point(l_x, l_y, l_z), Point(0.0,     l_y, 0.0));
		add_triangle(bottom_left_corner+Point(0.0, l_y, 0.0), bottom_left_corner+Point(0.0, l_y, l_z), bottom_left_corner+Point(l_x, l_y, l_z), Point(0.0,     l_y, 0.0));

		add_triangle(bottom_left_corner,                      bottom_left_corner+Point(0.0, l_y, 0.0), bottom_left_corner+Point(0.0, l_y, l_z), Point(0.0-l_x, 0.0, 0.0));
		add_triangle(bottom_left_corner,                      bottom_left_corner+Point(0.0, 0.0, l_z), bottom_left_corner+Point(0.0, l_y, l_z), Point(0.0-l_x, 0.0, 0.0));
		add_triangle(bottom_left_corner+Point(l_x, 0.0, 0.0), bottom_left_corner+Point(l_x, l_y, 0.0), bottom_left_corner+Point(l_x, l_y, l_z), Point(    l_x, 0.0, 0.0));
		add_triangle(bottom_left_corner+Point(l_x, 0.0, 0.0), bottom_left_corner+Point(l_x, 0.0, l_z), bottom_left_corner+Point(l_x, l_y, l_z), Point(    l_x, 0.0, 0.0));
	}

	template<class Point>
	void add_voxel(const Point& center, const double diameter)
	{
		const double radius=diameter*0.5;
		add_box(center+Point(0.0-radius, 0.0-radius, 0.0-radius), diameter, diameter, diameter);
	}

	friend std::ostream& operator<<(std::ostream& output, const Figure& f)
	{
		output << f.name;
		output << "\n" << f.special_description_for_label;
		output << "\n" << f.props;
		output << "\n" << f.normals_store_origin << " " << f.z_shift;
		if(!f.special_description_for_label.is_label)
		{
			output << "\n" << f.vertices.size();
			for(std::size_t i=0;i<f.vertices.size();i++)
			{
				output << " " << f.vertices[i];
			}
			output << "\n" << f.normals.size();
			for(std::size_t i=0;i<f.normals.size();i++)
			{
				output << " " << f.normals[i];
			}
			output << "\n" << f.indices.size();
			for(std::size_t i=0;i<f.indices.size();i++)
			{
				output << " " << f.indices[i];
			}
		}
		output << "\n";
		return output;
	}

	friend std::istream& operator>>(std::istream& input, Figure& f)
	{
		f=Figure();
		input >> f.name;
		input >> f.special_description_for_label;
		input >> f.props;
		input >> f.normals_store_origin >> f.z_shift;
		if(!f.special_description_for_label.is_label)
		{
			{
				int n=0;
				input >> n;
				if(n>0)
				{
					f.vertices.resize(n);
					for(std::size_t i=0;i<f.vertices.size();i++)
					{
						input >> f.vertices[i];
					}
				}
			}
			{
				int n=0;
				input >> n;
				if(n>0)
				{
					f.normals.resize(n);
					for(std::size_t i=0;i<f.normals.size();i++)
					{
						input >> f.normals[i];
					}
				}
			}
			{
				int n=0;
				input >> n;
				if(n>0)
				{
					f.indices.resize(n);
					for(std::size_t i=0;i<f.indices.size();i++)
					{
						input >> f.indices[i];
					}
				}
			}
		}
		return input;
	}
};

}

}

#endif /* SCRIPTING_FIGURE_H_ */
