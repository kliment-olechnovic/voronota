#ifndef SCRIPTING_OPERATORS_PRINT_FIGURES_H_
#define SCRIPTING_OPERATORS_PRINT_FIGURES_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class PrintFigures : public OperatorBase<PrintFigures>
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
	std::vector<std::size_t> forced_ids;

	PrintFigures()
	{
	}

	void initialize(CommandInput& input)
	{
		name=input.get_value_vector_or_default<std::string>("name", std::vector<std::string>());
		forced_ids=input.get_value_vector_or_default<std::size_t>("id", std::vector<std::size_t>());
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING_ARRAY, "figure name", ""));
		doc.set_option_decription(CDOD("id", CDOD::DATATYPE_INT_ARRAY, "figure identifiers", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_figures_availability();

		const std::set<std::size_t> ids=
				LongName::match(data_manager.figures(), forced_ids.empty(), std::set<std::size_t>(forced_ids.begin(), forced_ids.end()), LongName(name));

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No figures selected."));
		}

		Result result;

		result.figures.reserve(ids.size());
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const Figure& figure=data_manager.figures()[*it];
			result.figures.push_back(VariantObject());
			for(std::size_t i=0;i<figure.name.subnames().size();i++)
			{
				result.figures.back().values_array("name").push_back(VariantValue(figure.name.subnames()[i]));
			}
			if(!figure.props.empty())
			{
				VariantSerialization::write(figure.props, result.figures.back());
			}
			result.figures.back().value("id")=(*it);
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_PRINT_FIGURES_H_ */
