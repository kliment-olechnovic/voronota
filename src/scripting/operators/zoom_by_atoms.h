#ifndef SCRIPTING_OPERATORS_ZOOM_BY_ATOMS_H_
#define SCRIPTING_OPERATORS_ZOOM_BY_ATOMS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ZoomByAtoms : public OperatorBase<ZoomByAtoms>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms summary_of_atoms;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.summaries_of_atoms["zoomed"]=summary_of_atoms;
			VariantSerialization::write(summary_of_atoms.bounding_box, heterostorage.variant_object.object("bounding_box"));
		}
	};

	SelectionManager::Query parameters_for_selecting;

	ZoomByAtoms()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		const std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		Result result;
		result.summary_of_atoms=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_ZOOM_BY_ATOMS_H_ */
