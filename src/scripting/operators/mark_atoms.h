#ifndef SCRIPTING_OPERATORS_MARK_ATOMS_H_
#define SCRIPTING_OPERATORS_MARK_ATOMS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class MarkAtoms : public OperatorBase<MarkAtoms>
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
	std::string motif;

	MarkAtoms()
	{
	}

	virtual ~MarkAtoms()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		motif=input.get_value_or_default<std::string>("motif", "");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("motif", CDOD::DATATYPE_STRING, "sequence motif", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		std::set<std::size_t> ids;

		if(motif.empty())
		{
			ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		}
		else
		{
			SelectionManager::Query restricted_parameters_for_selecting=parameters_for_selecting;
			if(restricted_parameters_for_selecting.restrict_from_ids(common::ConstructionOfPrimaryStructure::collect_atom_ids_from_residue_ids(data_manager.primary_structure_info(), common::ConstructionOfPrimaryStructure::find_residue_ids_of_motif(data_manager.primary_structure_info(), motif))))
			{
				ids=data_manager.selection_manager().select_atoms(restricted_parameters_for_selecting);
			}
		}

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_mark(positive()).set_unmark(!positive()), ids);

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

class UnmarkAtoms : public MarkAtoms
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

#endif /* SCRIPTING_OPERATORS_MARK_ATOMS_H_ */

