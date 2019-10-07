#ifndef SCRIPTING_FIGURE_H_
#define SCRIPTING_FIGURE_H_

#include <string>
#include <vector>
#include <set>

namespace scripting
{

struct Figure
{
	std::vector<std::string> name;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<unsigned int> indices;

	bool valid() const
	{
		return (!name.empty()
				&& !name[0].empty()
				&& vertices.size()>=3
				&& vertices.size()==normals.size()
				&& indices.size()>=3
				&& indices.size()%3==0);
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
};

}

#endif /* SCRIPTING_FIGURE_H_ */
