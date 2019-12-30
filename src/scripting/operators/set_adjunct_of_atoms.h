#ifndef SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_H_
#define SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class SetAdjunctOfAtoms
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

	SelectionManager::Query parameters_for_selecting;
	std::string name;
	bool value_present;
	double value;
	bool remove;

	SetAdjunctOfAtoms() : value_present(false), value(0.0), remove(false)
	{
	}

	SetAdjunctOfAtoms& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		name=input.get_value<std::string>("name");
		value_present=input.is_option("value");
		value=input.get_value_or_default<double>("value", 0.0);
		remove=input.get_flag("remove");
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(value_present && remove)
		{
			throw std::runtime_error(std::string("Value setting and removing options used together."));
		}

		assert_adjunct_name_input(name, false);

		std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
			if(remove)
			{
				atom_adjuncts.erase(name);
			}
			else
			{
				atom_adjuncts[name]=value;
			}
		}

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_H_ */
