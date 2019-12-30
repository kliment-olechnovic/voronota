#ifndef SCRIPTING_OPERATORS_DELETE_TAGS_OF_ATOMS_H_
#define SCRIPTING_OPERATORS_DELETE_TAGS_OF_ATOMS_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class DeleteTagsOfAtoms
{
public:
	struct Result
	{
		SummaryOfAtoms atoms_summary;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			return (*this);
		}
	};

	bool all;
	SelectionManager::Query parameters_for_selecting;
	std::vector<std::string> tags;

	DeleteTagsOfAtoms() : all(false)
	{
	}

	DeleteTagsOfAtoms& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		all=input.get_flag("all");
		tags=input.get_value_vector_or_default<std::string>("tags", std::vector<std::string>());
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(!all && tags.empty())
		{
			throw std::runtime_error(std::string("No tags specified."));
		}

		if(all && !tags.empty())
		{
			throw std::runtime_error(std::string("Conflicting specification of tags."));
		}

		std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			std::set<std::string>& atom_tags=data_manager.atom_tags_mutable(*it);
			if(all)
			{
				atom_tags.clear();
			}
			else
			{
				for(std::size_t i=0;i<tags.size();i++)
				{
					atom_tags.erase(tags[i]);
				}
			}
		}

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_DELETE_TAGS_OF_ATOMS_H_ */
