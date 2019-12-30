#ifndef SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_RESIDUE_POOLING_H_
#define SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_RESIDUE_POOLING_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class SetAdjunctOfAtomsByResiduePooling
{
public:
	struct Result
	{
		SummaryOfAtoms used_source_atoms_summary;
		SummaryOfAtoms destination_atoms_summary;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(used_source_atoms_summary, heterostorage.variant_object.object("used_source_atoms_summary"));
			VariantSerialization::write(destination_atoms_summary, heterostorage.variant_object.object("destination_atoms_summary"));
			return (*this);
		}
	};

	SelectionManager::Query parameters_for_selecting_source_atoms;
	SelectionManager::Query parameters_for_selecting_destination_atoms;
	std::string source_name;
	std::string destination_name;
	std::string pooling_mode;

	SetAdjunctOfAtomsByResiduePooling()
	{
	}

	SetAdjunctOfAtomsByResiduePooling& init(CommandInput& input)
	{
		parameters_for_selecting_source_atoms=Utilities::read_generic_selecting_query("source-atoms-", "[]", input);
		parameters_for_selecting_destination_atoms=Utilities::read_generic_selecting_query("destination-atoms-", "[]", input);
		source_name=input.get_value<std::string>("source-name");
		destination_name=input.get_value<std::string>("destination-name");
		pooling_mode=input.get_value<std::string>("pooling-mode");
		assert_adjunct_name_input(source_name, false);
		assert_adjunct_name_input(destination_name, false);
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_adjunct_name_input(source_name, false);
		assert_adjunct_name_input(destination_name, false);

		if(pooling_mode!="mean" && pooling_mode!="sum" && pooling_mode!="product" && pooling_mode!="min" && pooling_mode!="max")
		{
			throw std::runtime_error(std::string("Invalid pooling mode, valid options are: 'mean', 'sum', 'product', 'min', 'max'."));
		}

		const std::set<std::size_t> source_atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_source_atoms);
		if(source_atom_ids.empty())
		{
			throw std::runtime_error(std::string("No source atoms selected."));
		}

		const std::set<std::size_t> destination_atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_destination_atoms);
		if(destination_atom_ids.empty())
		{
			throw std::runtime_error(std::string("No destination atoms selected."));
		}

		std::map< std::size_t, std::vector<double> > residue_source_values;
		std::set<std::size_t> used_source_atom_ids;

		for(std::set<std::size_t>::const_iterator it=destination_atom_ids.begin();it!=destination_atom_ids.end();++it)
		{
			const std::size_t residue_id=data_manager.primary_structure_info().map_of_atoms_to_residues[*it];
			if(residue_source_values.count(residue_id)==0)
			{
				const std::vector<std::size_t>& residue_atom_ids=data_manager.primary_structure_info().residues[residue_id].atom_ids;
				for(std::size_t i=0;i<residue_atom_ids.size();i++)
				{
					const std::size_t atom_id=residue_atom_ids[i];
					if(source_atom_ids.count(atom_id)>0)
					{
						const Atom& atom=data_manager.atoms()[atom_id];
						std::map<std::string, double>::const_iterator jt=atom.value.props.adjuncts.find(source_name);
						if(jt!=atom.value.props.adjuncts.end())
						{
							residue_source_values[residue_id].push_back(jt->second);
							used_source_atom_ids.insert(atom_id);
						}
					}
				}
			}
		}

		std::map<std::size_t, double> residue_pooled_values;
		for(std::map< std::size_t, std::vector<double> >::const_iterator it=residue_source_values.begin();it!=residue_source_values.end();++it)
		{
			const std::size_t residue_id=it->first;
			const std::vector<double>& values=it->second;
			if(!values.empty())
			{
				if(pooling_mode=="mean")
				{
					residue_pooled_values[residue_id]=std::accumulate(values.begin(), values.end(), 0.0)/values.size();
				}
				else if(pooling_mode=="sum")
				{
					residue_pooled_values[residue_id]=std::accumulate(values.begin(), values.end(), 0.0);
				}
				else if(pooling_mode=="product")
				{
					residue_pooled_values[residue_id]=std::accumulate(values.begin(), values.end(), 1.0, std::multiplies<double>());
				}
				else if(pooling_mode=="min")
				{
					residue_pooled_values[residue_id]=(*std::min_element(values.begin(), values.end()));
				}
				else if(pooling_mode=="max")
				{
					residue_pooled_values[residue_id]=(*std::max_element(values.begin(), values.end()));
				}
				else
				{
					throw std::runtime_error(std::string("Invalid pooling mode."));
				}
			}
		}

		for(std::set<std::size_t>::const_iterator it=destination_atom_ids.begin();it!=destination_atom_ids.end();++it)
		{
			std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
			atom_adjuncts.erase(destination_name);
			const std::size_t residue_id=data_manager.primary_structure_info().map_of_atoms_to_residues[*it];
			if(residue_pooled_values.count(residue_id)>0)
			{
				atom_adjuncts[destination_name]=residue_pooled_values[residue_id];
			}
		}


		Result result;
		result.used_source_atoms_summary=SummaryOfAtoms(data_manager.atoms(), used_source_atom_ids);
		result.destination_atoms_summary=SummaryOfAtoms(data_manager.atoms(), destination_atom_ids);

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_RESIDUE_POOLING_H_ */
