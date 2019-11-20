#ifndef SCRIPTING_FIGURE_H_
#define SCRIPTING_FIGURE_H_

#include "../common/properties_value.h"

#include <string>
#include <vector>
#include <set>

namespace scripting
{

class Figure
{
public:
	static std::set<std::size_t> match_name(const std::vector<Figure>& figures, const std::vector<std::string>& refname)
	{
		std::set<std::size_t> ids;
		for(std::size_t i=0;i<figures.size();i++)
		{
			if(figures[i].match_name(refname))
			{
				ids.insert(ids.end(), i);
			}
		}
		return ids;
	}

	static std::set<std::size_t> match_name(const std::vector<Figure>& figures, const bool from_all, const std::set<std::size_t>& from_ids, const std::vector<std::string>& refname)
	{
		std::set<std::size_t> ids;
		for(std::size_t i=0;i<figures.size();i++)
		{
			if(figures[i].match_name(refname) && (from_all || from_ids.count(i)>0))
			{
				ids.insert(ids.end(), i);
			}
		}
		return ids;
	}

	std::vector<std::string> name;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<unsigned int> indices;
	common::PropertiesValue props;

	bool valid() const
	{
		if(name.empty() || vertices.size()<3 || vertices.size()!=normals.size() || indices.size()<3 || indices.size()%3!=0)
		{
			return false;
		}
		for(std::size_t i=0;i<name.size();i++)
		{
			if(name[i].empty())
			{
				return false;
			}
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

	bool match_name(const std::vector<std::string>& refname) const
	{
		if(refname.size()>name.size())
		{
			return false;
		}
		for(std::size_t i=0;i<refname.size();i++)
		{
			if(name[i]!=refname[i])
			{
				return false;
			}
		}
		return true;
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
};

}

#endif /* SCRIPTING_FIGURE_H_ */
