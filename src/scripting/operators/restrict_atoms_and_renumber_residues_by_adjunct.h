#ifndef SCRIPTING_OPERATORS_RESTRICT_ATOMS_AND_RENUMBER_RESIDUES_BY_ADJUNCT_H_
#define SCRIPTING_OPERATORS_RESTRICT_ATOMS_AND_RENUMBER_RESIDUES_BY_ADJUNCT_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class RestrictAtomsAndRenumberResiduesByAdjunct : public OperatorBase<RestrictAtomsAndRenumberResiduesByAdjunct>
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

	std::string name;

	RestrictAtomsAndRenumberResiduesByAdjunct()
	{
	}

	void initialize(CommandInput& input)
	{
		name=input.get_value<std::string>("name");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "adjunct name"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		Result result;

		result.atoms_summary_old=SummaryOfAtoms(data_manager.atoms());

		data_manager.restrict_atoms_and_renumber_residues_by_adjunct(name);

		result.atoms_summary_new=SummaryOfAtoms(data_manager.atoms());

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_RESTRICT_ATOMS_AND_RENUMBER_RESIDUES_BY_ADJUNCT_H_ */
