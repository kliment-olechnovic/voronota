#ifndef SCRIPTING_OPERATORS_DELETE_ADJUNCTS_OF_ATOMS_H_
#define SCRIPTING_OPERATORS_DELETE_ADJUNCTS_OF_ATOMS_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class DeleteAdjunctsOfAtoms
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

	const bool all;
	const SelectionManager::Query parameters_for_selecting;
	const std::vector<std::string> adjuncts;

	DeleteAdjunctsOfAtoms() : all(false)
	{
	}

	DeleteAdjunctsOfAtoms& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		all=input.get_flag("all");
		adjuncts=input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(!all && adjuncts.empty())
		{
			throw std::runtime_error(std::string("No adjuncts specified."));
		}

		if(all && !adjuncts.empty())
		{
			throw std::runtime_error(std::string("Conflicting specification of adjuncts."));
		}

		std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
			if(all)
			{
				atom_adjuncts.clear();
			}
			else
			{
				for(std::size_t i=0;i<adjuncts.size();i++)
				{
					atom_adjuncts.erase(adjuncts[i]);
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

#endif /* SCRIPTING_OPERATORS_DELETE_ADJUNCTS_OF_ATOMS_H_ */
