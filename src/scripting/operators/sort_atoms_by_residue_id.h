#ifndef SCRIPTING_OPERATORS_SORT_ATOMS_BY_RESIDUE_ID_H_
#define SCRIPTING_OPERATORS_SORT_ATOMS_BY_RESIDUE_ID_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SortAtomsByResidueID : public OperatorBase<SortAtomsByResidueID>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary_old;
		SummaryOfAtoms atoms_summary_new;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary_old, heterostorage.variant_object.object("atoms_summary_old"));
			VariantSerialization::write(atoms_summary_new, heterostorage.variant_object.object("atoms_summary_new"));
		}
	};

	SortAtomsByResidueID()
	{
	}

	void initialize(CommandInput&)
	{
	}

	void document(CommandDocumentation&) const
	{
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		Result result;

		result.atoms_summary_old=SummaryOfAtoms(data_manager.atoms());

		data_manager.sort_atoms_by_residue_id();

		result.atoms_summary_new=SummaryOfAtoms(data_manager.atoms());

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SORT_ATOMS_BY_RESIDUE_ID_H_ */
