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
	LongName name;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<unsigned int> indices;
	common::PropertiesValue props;
	bool normals_store_origin;
	double z_shift;

	Figure() : normals_store_origin(true), z_shift(0.0)
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
};

}

}

#endif /* SCRIPTING_FIGURE_H_ */
