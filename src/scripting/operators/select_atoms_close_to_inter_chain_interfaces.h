#ifndef SCRIPTING_OPERATORS_SELECT_ATOMS_CLOSE_TO_INTER_CHAIN_INTERFACES_H_
#define SCRIPTING_OPERATORS_SELECT_ATOMS_CLOSE_TO_INTER_CHAIN_INTERFACES_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SelectAtomsCloseToInterchainInterfaces : public OperatorBase<SelectAtomsCloseToInterchainInterfaces>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::set<std::size_t> ids;
		SummaryOfAtoms atoms_summary;
		std::string selection_name;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.variant_object.value("selection_name")=selection_name;
		}
	};

	SelectionManager::Query parameters_for_selecting;
	double probe;
	std::string name;
	bool mark;
	std::string selection_expresion_for_atoms_a;
	std::string selection_expresion_for_atoms_b;

	SelectAtomsCloseToInterchainInterfaces() : probe(1.4), mark(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		probe=input.get_value_or_default<double>("probe", 1.4);
		name=input.get_value_or_first_unused_unnamed_value_or_default("name", "");
		mark=input.get_flag("mark");
		selection_expresion_for_atoms_a=input.get_value_or_default<std::string>("atoms-first", "");
		selection_expresion_for_atoms_b=input.get_value_or_default<std::string>("atoms-second", "");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("probe", CDOD::DATATYPE_FLOAT, "probe radius", 1.4));
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "atom selection name", ""));
		doc.set_option_decription(CDOD("mark", CDOD::DATATYPE_BOOL, "flag to mark selected atoms"));
		doc.set_option_decription(CDOD("atoms-first", CDOD::DATATYPE_STRING, "selection expression for the first group of atoms", ""));
		doc.set_option_decription(CDOD("atoms-second", CDOD::DATATYPE_STRING, "selection expression for the second group of atoms", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_selection_name_input(name, true);

		const std::set<std::size_t> base_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(base_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		std::map<std::size_t, int> atom_groups;

		if(!selection_expresion_for_atoms_a.empty() || !selection_expresion_for_atoms_b.empty())
		{
			const std::set<std::size_t> atom_ids_a=data_manager.selection_manager().select_atoms(SelectionManager::Query(base_ids, (selection_expresion_for_atoms_a.empty() ? std::string("[]") : selection_expresion_for_atoms_a), false));

			if(atom_ids_a.empty())
			{
				throw std::runtime_error(std::string("No first atoms selected."));
			}

			const std::set<std::size_t> atom_ids_b=data_manager.selection_manager().select_atoms(SelectionManager::Query(base_ids, (selection_expresion_for_atoms_b.empty() ? std::string("[]") : selection_expresion_for_atoms_b), false));

			if(atom_ids_b.empty())
			{
				throw std::runtime_error(std::string("No second atoms selected."));
			}

			for(std::set<std::size_t>::const_iterator it=atom_ids_a.begin();it!=atom_ids_a.end();++it)
			{
				atom_groups[*it]+=1;
			}

			for(std::set<std::size_t>::const_iterator it=atom_ids_b.begin();it!=atom_ids_b.end();++it)
			{
				atom_groups[*it]+=2;
			}
		}

		std::set<std::size_t> ids;

		{
			std::vector<apollota::SimpleSphere> spheres(data_manager.atoms().size());
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				spheres[i]=voronota::apollota::SimpleSphere(data_manager.atoms()[i].value, data_manager.atoms()[i].value.r+probe);
			}

			const apollota::BoundingSpheresHierarchy bsh(spheres, 3.5, 1);

			if(atom_groups.empty())
			{
				for(std::set<std::size_t>::const_iterator it=base_ids.begin();it!=base_ids.end();++it)
				{
					const std::size_t i=(*it);
					const std::vector<std::size_t> collisions=voronota::apollota::SearchForSphericalCollisions::find_all_collisions(bsh, spheres[i]);
					bool passed=false;
					for(std::size_t j=0;j<collisions.size() && !passed;j++)
					{
						if(data_manager.atoms()[collisions[j]].crad.chainID!=data_manager.atoms()[i].crad.chainID)
						{
							ids.insert(i);
							passed=true;
						}
					}
				}
			}
			else
			{
				for(std::map<std::size_t, int>::const_iterator it=atom_groups.begin();it!=atom_groups.end();++it)
				{
					const std::size_t i=(it->first);
					const std::vector<std::size_t> collisions=voronota::apollota::SearchForSphericalCollisions::find_all_collisions(bsh, spheres[i]);
					bool passed=false;
					for(std::size_t j=0;j<collisions.size() && !passed;j++)
					{
						std::map<std::size_t, int>::const_iterator jt=atom_groups.find(collisions[j]);
						if(jt!=atom_groups.end() && (jt->second)<3 && (jt->second)!=(it->second))
						{
							ids.insert(i);
							passed=true;
						}
					}
				}
			}
		}

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		if(!name.empty())
		{
			data_manager.selection_manager().set_atoms_selection(name, ids);
		}

		if(mark)
		{
			data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_unmark(true));
			data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_mark(true), ids);
		}

		Result result;
		result.ids=ids;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);
		result.selection_name=name;
		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SELECT_ATOMS_CLOSE_TO_INTER_CHAIN_INTERFACES_H_ */
