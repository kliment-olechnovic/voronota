#ifndef SCRIPTING_OPERATORS_RESTRICT_ATOMS_H_
#define SCRIPTING_OPERATORS_RESTRICT_ATOMS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class RestrictAtoms : public OperatorBase<RestrictAtoms>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary_old;
		SummaryOfAtoms atoms_summary_new;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary_old, heterostorage.variant_object.object("atoms_summary_old"));
			VariantSerialization::write(atoms_summary_new, heterostorage.variant_object.object("atoms_summary_new"));
			return (*this);
		}
	};

	SelectionManager::Query parameters_for_selecting;

	RestrictAtoms()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
	}

	void document(CommandDocumentation& doc) const
	{
		Utilities::document_read_generic_selecting_query(doc);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		const std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.size()<4)
		{
			throw std::runtime_error(std::string("Less than 4 atoms selected."));
		}

		Result result;

		result.atoms_summary_old=SummaryOfAtoms(data_manager.atoms());

		if(ids.size()<data_manager.atoms().size())
		{
			data_manager.restrict_atoms(ids);
			result.atoms_summary_new=SummaryOfAtoms(data_manager.atoms());
		}
		else
		{
			result.atoms_summary_new=result.atoms_summary_old;
		}

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_RESTRICT_ATOMS_H_ */
