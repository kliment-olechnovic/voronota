#ifndef SCRIPTING_FIGURE_H_
#define SCRIPTING_FIGURE_H_

#include <string>
#include <vector>

namespace scripting
{

struct Figure
{
	std::string name;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<unsigned int> indices;

	bool valid() const
	{
		return (vertices.size()>=3
				&& vertices.size()==normals.size()
				&& indices.size()>=3
				&& indices.size()%3==0);
	}
};

}

#endif /* SCRIPTING_FIGURE_H_ */
