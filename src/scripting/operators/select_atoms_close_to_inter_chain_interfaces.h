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

		std::vector< std::set<std::size_t> > atom_groups;

		if(!selection_expresion_for_atoms_a.empty() || !selection_expresion_for_atoms_b.empty())
		{
			const std::set<std::size_t> atom_ids_a=data_manager.selection_manager().select_atoms(SelectionManager::Query((selection_expresion_for_atoms_a.empty() ? std::string("[]") : selection_expresion_for_atoms_a), false));

			if(atom_ids_a.empty())
			{
				throw std::runtime_error(std::string("No first atoms selected."));
			}

			const std::set<std::size_t> atom_ids_b=data_manager.selection_manager().select_atoms(SelectionManager::Query((selection_expresion_for_atoms_b.empty() ? std::string("[]") : selection_expresion_for_atoms_b), false));

			if(atom_ids_b.empty())
			{
				throw std::runtime_error(std::string("No second atoms selected."));
			}

			atom_groups.reserve(2);
			atom_groups.push_back(atom_ids_a);
			atom_groups.push_back(atom_ids_b);
		}

		std::set<std::size_t> ids;

		{
			std::vector<apollota::SimpleSphere> spheres(data_manager.atoms().size());
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				spheres[i]=voronota::apollota::SimpleSphere(data_manager.atoms()[i].value, data_manager.atoms()[i].value.r+probe);
			}

			if(atom_groups.empty())
			{
				const apollota::BoundingSpheresHierarchy bsh(spheres, 3.5, 1);

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
				std::vector<bool> atom_validity(data_manager.atoms().size(), false);
				for(std::set<std::size_t>::const_iterator it=base_ids.begin();it!=base_ids.end();++it)
				{
					atom_validity[*it]=true;
				}

				for(int group=0;group<2;group++)
				{
					bool group_validity=false;
					for(std::set<std::size_t>::const_iterator it=atom_groups[group].begin();it!=atom_groups[group].end() && !group_validity;++it)
					{
						group_validity=atom_validity[*it];
					}

					if(group_validity)
					{
						const int antigroup=(group==0 ? 1 : 0);
						std::vector<apollota::SimpleSphere> antigroup_spheres;
						antigroup_spheres.reserve(atom_groups[antigroup].size());
						for(std::set<std::size_t>::const_iterator it=atom_groups[antigroup].begin();it!=atom_groups[antigroup].end();++it)
						{
							const std::size_t i=(*it);
							antigroup_spheres.push_back(voronota::apollota::SimpleSphere(data_manager.atoms()[i].value, data_manager.atoms()[i].value.r+probe));
						}
						const apollota::BoundingSpheresHierarchy bsh(antigroup_spheres, 3.5, 1);

						for(std::set<std::size_t>::const_iterator it=atom_groups[group].begin();it!=atom_groups[group].end();++it)
						{
							const std::size_t i=(*it);
							if(atom_validity[i] && !voronota::apollota::SearchForSphericalCollisions::find_any_collision(bsh, spheres[i]).empty())
							{
								ids.insert(i);
							}
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
