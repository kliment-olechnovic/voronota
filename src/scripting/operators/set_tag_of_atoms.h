#ifndef SCRIPTING_OPERATORS_SET_TAG_OF_ATOMS_H_
#define SCRIPTING_OPERATORS_SET_TAG_OF_ATOMS_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class SetTagOfAtoms
{
public:
	struct Result
	{
		SummaryOfAtoms atoms_summary;

		Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			return (*this);
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string tag;

	SetTagOfAtoms()
	{
	}

	SetTagOfAtoms& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		tag=input.get_value_or_first_unused_unnamed_value("tag");
		assert_tag_input(tag, false);
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_tag_input(tag, false);

		std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			data_manager.atom_tags_mutable(*it).insert(tag);
		}

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_SET_TAG_OF_ATOMS_H_ */
