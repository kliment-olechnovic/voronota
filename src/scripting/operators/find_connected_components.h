#ifndef SCRIPTING_OPERATORS_FIND_CONNECTED_COMPONENTS_H_
#define SCRIPTING_OPERATORS_FIND_CONNECTED_COMPONENTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class FindConnectedComponents : public OperatorBase<FindConnectedComponents>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;
		VariantObject components_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.variant_object.object("components_summary")=components_summary;
		}
	};

	SelectionManager::Query parameters_for_selecting_atoms_core;
	SelectionManager::Query parameters_for_selecting_atoms_all;
	SelectionManager::Query parameters_for_selecting_contacts;
	std::string adjunct_component_number;

	FindConnectedComponents()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_atoms_core=OperatorsUtilities::read_generic_selecting_query("atoms-core-", "[]", input);
		parameters_for_selecting_atoms_all=OperatorsUtilities::read_generic_selecting_query("atoms-all-", parameters_for_selecting_atoms_core.expression_string, input);
		parameters_for_selecting_contacts=OperatorsUtilities::read_generic_selecting_query("contacts-", "[]", input);
		adjunct_component_number=input.get_value_or_default<std::string>("adj-component-number", "component_number");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query("atoms-core-", "[]", doc);
		OperatorsUtilities::document_read_generic_selecting_query("atoms-all-", "[]", doc);
		OperatorsUtilities::document_read_generic_selecting_query("contacts-", "[]", doc);
		doc.set_option_decription(CDOD("adj-component-number", CDOD::DATATYPE_STRING, "atom adjunct name for result values", "component_number"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(adjunct_component_number, false);

		const std::set<std::size_t> core_atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms_core);
		if(core_atom_ids.empty())
		{
			throw std::runtime_error(std::string("No core atoms selected."));
		}

		std::set<std::size_t> all_atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms_all);
		all_atom_ids.insert(core_atom_ids.begin(), core_atom_ids.end());
		if(all_atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		const std::set<std::size_t> contact_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting_contacts);
		if(contact_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		std::vector<int> atoms_enabled(data_manager.atoms().size(), 0);
		for(std::set<std::size_t>::const_iterator it=all_atom_ids.begin();it!=all_atom_ids.end();++it)
		{
			atoms_enabled[*it]=1;
		}
		for(std::set<std::size_t>::const_iterator it=core_atom_ids.begin();it!=core_atom_ids.end();++it)
		{
			atoms_enabled[*it]=2;
		}

		std::vector< std::set<std::size_t> > graph(data_manager.atoms().size());
		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];
			if(!contact.solvent() && atoms_enabled[contact.ids[0]]>0 && atoms_enabled[contact.ids[1]]>0)
			{
				graph[contact.ids[0]].insert(contact.ids[1]);
				graph[contact.ids[1]].insert(contact.ids[0]);
			}
		}

		std::vector<int> atoms_component_nums(data_manager.atoms().size(), 0);
		{
			int current_component_num=0;
			for(std::set<std::size_t>::const_iterator start_it=core_atom_ids.begin();start_it!=core_atom_ids.end();++start_it)
			{
				const std::size_t start_id=(*start_it);
				if(atoms_enabled[start_id]>0 && atoms_component_nums[start_id]==0)
				{
					++current_component_num;
					atoms_component_nums[start_id]=current_component_num;
					std::vector<std::size_t> stack;
					stack.push_back(start_id);
					while(!stack.empty())
					{
						const std::size_t central_id=stack.back();
						stack.pop_back();
						for(std::set<std::size_t>::const_iterator neighbor_it=graph[central_id].begin();neighbor_it!=graph[central_id].end();++neighbor_it)
						{
							const std::size_t neignbor_id=(*neighbor_it);
							if(atoms_enabled[neignbor_id]>0 && atoms_component_nums[neignbor_id]==0)
							{
								atoms_component_nums[neignbor_id]=current_component_num;
								stack.push_back(neignbor_id);
							}
						}
					}
				}
			}
		}

		VariantObject components_info;

		{
			std::map<int, int> map_of_component_nums_to_counts;
			for(std::size_t i=0;i<atoms_component_nums.size();i++)
			{
				if(atoms_component_nums[i]>0)
				{
					map_of_component_nums_to_counts[atoms_component_nums[i]]++;
				}
			}

			std::map<int, int> map_of_counts_to_ranks;
			for(std::map<int, int>::const_iterator it=map_of_component_nums_to_counts.begin();it!=map_of_component_nums_to_counts.end();++it)
			{
				map_of_counts_to_ranks[it->second]=0;
			}
			{
				int rank=0;
				for(std::map<int, int>::reverse_iterator it=map_of_counts_to_ranks.rbegin();it!=map_of_counts_to_ranks.rend();++it)
				{
					++rank;
					it->second=rank;
				}
			}

			for(std::size_t i=0;i<atoms_component_nums.size();i++)
			{
				if(atoms_component_nums[i]>0)
				{
					atoms_component_nums[i]=map_of_counts_to_ranks[map_of_component_nums_to_counts[atoms_component_nums[i]]];
				}
			}

			components_info.value("number_of_components")=map_of_counts_to_ranks.size();
			for(std::map<int, int>::reverse_iterator it=map_of_counts_to_ranks.rbegin();it!=map_of_counts_to_ranks.rend();++it)
			{
				VariantObject obj;
				obj.value("rank")=it->second;
				obj.value("size")=it->first;
				components_info.objects_array("component_sizes").push_back(obj);
			}
		}

		if(!adjunct_component_number.empty())
		{
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				data_manager.atom_adjuncts_mutable(i).erase(adjunct_component_number);
			}

			for(std::set<std::size_t>::const_iterator it=all_atom_ids.begin();it!=all_atom_ids.end();++it)
			{
				const std::size_t central_id=(*it);
				if(atoms_component_nums[central_id]>0)
				{
					data_manager.atom_adjuncts_mutable(central_id)[adjunct_component_number]=atoms_component_nums[central_id];
				}
			}
		}

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), all_atom_ids);
		result.components_summary=components_info;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_FIND_CONNECTED_COMPONENTS_H_ */
