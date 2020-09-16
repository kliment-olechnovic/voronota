#ifndef SCRIPTING_OPERATORS_PRINT_ATOMS_H_
#define SCRIPTING_OPERATORS_PRINT_ATOMS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class PrintAtoms : public OperatorBase<PrintAtoms>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<VariantObject> atoms;
		SummaryOfAtoms atoms_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.objects_array("atoms")=atoms;
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
		}
	};

	SelectionManager::Query parameters_for_selecting;

	PrintAtoms()
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
		result.atoms.reserve(ids.size());
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			result.atoms.push_back(VariantObject());
			VariantSerialization::write(data_manager.atoms()[*it], result.atoms.back());
			result.atoms.back().value("id")=(*it);
		}
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_PRINT_ATOMS_H_ */
