#ifndef SCRIPTING_OPERATORS_ADD_FIGURE_H_
#define SCRIPTING_OPERATORS_ADD_FIGURE_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class AddFigure : public OperatorBase<AddFigure>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		VariantObject added_figure;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.object("added_figure")=added_figure;
			return (*this);
		}
	};

	std::vector<std::string> name;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<unsigned int> indices;

	AddFigure()
	{
	}

	AddFigure& init(CommandInput& input)
	{
		name=input.get_value_vector<std::string>("name");
		vertices=input.get_value_vector<float>("vertices");
		normals=input.get_value_vector<float>("normals");
		indices=input.get_value_vector<unsigned int>("indices");
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		{
			Figure figure;
			figure.name=LongName(name);
			figure.vertices=vertices;
			figure.normals=normals;
			figure.indices=indices;

			data_manager.add_figure(figure);
		}

		Result result;

		{
			const Figure& figure=data_manager.figures().back();
			for(std::size_t i=0;i<figure.name.subnames().size();i++)
			{
				result.added_figure.values_array("name").push_back(VariantValue(figure.name.subnames()[i]));
			}
		}

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_ADD_FIGURE_H_ */
