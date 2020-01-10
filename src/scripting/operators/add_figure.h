#ifndef SCRIPTING_OPERATORS_ADD_FIGURE_H_
#define SCRIPTING_OPERATORS_ADD_FIGURE_H_

#include "../operators_common.h"

namespace voronota
{

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

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.object("added_figure")=added_figure;
		}
	};

	std::vector<std::string> name;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<unsigned int> indices;

	AddFigure()
	{
	}

	void initialize(CommandInput& input)
	{
		name=input.get_value_vector<std::string>("name");
		vertices=input.get_value_vector<float>("vertices");
		normals=input.get_value_vector<float>("normals");
		indices=input.get_value_vector<unsigned int>("indices");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING_ARRAY, "figure name"));
		doc.set_option_decription(CDOD("vertices", CDOD::DATATYPE_FLOAT_ARRAY, "figure vertix coordinated"));
		doc.set_option_decription(CDOD("normals", CDOD::DATATYPE_FLOAT_ARRAY, "figure normals coordinated"));
		doc.set_option_decription(CDOD("indices", CDOD::DATATYPE_INT_ARRAY, "figure triangle indices"));
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

}

#endif /* SCRIPTING_OPERATORS_ADD_FIGURE_H_ */
