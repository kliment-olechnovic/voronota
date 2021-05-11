#ifndef SCRIPTING_OPERATORS_SHOW_FIGURES_H_
#define SCRIPTING_OPERATORS_SHOW_FIGURES_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ShowFigures : public OperatorBase<ShowFigures>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::vector<std::string> name;
	std::vector<std::string> representation_names;

	ShowFigures()
	{
	}

	virtual ~ShowFigures()
	{
	}

	void initialize(CommandInput& input)
	{
		name=input.get_value_vector_or_default<std::string>("name", std::vector<std::string>());
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING_ARRAY, "figure name", ""));
		doc.set_option_decription(CDOD("rep", CDOD::DATATYPE_STRING_ARRAY, "representation names", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_figures_availability();
		data_manager.assert_figures_representations_availability();

		const std::set<std::size_t> representation_ids=data_manager.figures_representation_descriptor().ids_by_names(representation_names);

		const std::set<std::size_t> ids=LongName::match(data_manager.figures(), LongName(name));

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No figures selected."));
		}

		data_manager.update_figures_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_show(positive()).set_hide(!positive()), ids);

		Result result;

		return result;
	}

protected:
	virtual bool positive() const
	{
		return true;
	}
};

class HideFigures : public ShowFigures
{
protected:
	bool positive() const
	{
		return false;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SHOW_FIGURES_H_ */
