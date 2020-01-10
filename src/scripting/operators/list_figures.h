#ifndef SCRIPTING_OPERATORS_LIST_FIGURES_H_
#define SCRIPTING_OPERATORS_LIST_FIGURES_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ListFigures : public OperatorBase<ListFigures>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<VariantObject> figures;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.objects_array("figures")=figures;
		}
	};

	std::vector<std::string> name;

	ListFigures()
	{
	}

	void initialize(CommandInput& input)
	{
		name=input.get_value_vector_or_default<std::string>("name", std::vector<std::string>());
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING_ARRAY, "figure name", ""));
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

}

#endif /* SCRIPTING_OPERATORS_LIST_FIGURES_H_ */
