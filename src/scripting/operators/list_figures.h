#ifndef SCRIPTING_OPERATORS_LIST_FIGURES_H_
#define SCRIPTING_OPERATORS_LIST_FIGURES_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class ListFigures
{
public:
	struct Result
	{
		std::vector<VariantObject> figures;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.objects_array("figures")=figures;
			return (*this);
		}
	};

	std::vector<std::string> name;

	ListFigures()
	{
	}

	ListFigures& init(CommandInput& input)
	{
		name=input.get_value_vector_or_default<std::string>("name", std::vector<std::string>());
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_figures_availability();

		const std::set<std::size_t> ids=LongName::match(data_manager.figures(), LongName(name));

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No figures selected."));
		}

		Result result;

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const Figure& figure=data_manager.figures().at(*it);
			VariantObject info;
			for(std::size_t i=0;i<figure.name.subnames().size();i++)
			{
				info.values_array("name").push_back(VariantValue(figure.name.subnames()[i]));
			}
			result.figures.push_back(info);
		}

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_LIST_FIGURES_H_ */
