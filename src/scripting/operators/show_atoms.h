#ifndef SCRIPTING_OPERATORS_SHOW_ATOMS_H_
#define SCRIPTING_OPERATORS_SHOW_ATOMS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ShowAtoms : public OperatorBase<ShowAtoms>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::vector<std::string> representation_names;

	ShowAtoms()
	{
	}

	virtual ~ShowAtoms()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("rep", CDOD::DATATYPE_STRING_ARRAY, "representation names", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();
		data_manager.assert_atoms_representations_availability();

		std::set<std::size_t> representation_ids=data_manager.atoms_representation_descriptor().ids_by_names(representation_names);

		if(positive() && representation_ids.empty())
		{
			representation_ids.insert(0);
		}

		const std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_show(positive()).set_hide(!positive()), ids);

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}

protected:
	virtual bool positive() const
	{
		return true;
	}
};

class HideAtoms : public ShowAtoms
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

#endif /* SCRIPTING_OPERATORS_SHOW_ATOMS_H_ */
