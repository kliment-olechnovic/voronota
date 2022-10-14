#ifndef SCRIPTING_OPERATORS_SET_GLOBAL_ADJUNCT_BY_POOLING_H_
#define SCRIPTING_OPERATORS_SET_GLOBAL_ADJUNCT_BY_POOLING_H_

#include <numeric>

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetGlobalAdjunctByPooling : public OperatorBase<SetGlobalAdjunctByPooling>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms used_source_atoms_summary;
		double pooling_result_value;

		Result() : pooling_result_value(0.0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(used_source_atoms_summary, heterostorage.variant_object.object("used_source_atoms_summary"));
			heterostorage.variant_object.value("pooling_result_value")=pooling_result_value;
		}
	};

	SelectionManager::Query parameters_for_selecting_source_atoms;
	std::string source_name;
	std::string destination_name;
	std::string pooling_mode;

	SetGlobalAdjunctByPooling()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_source_atoms=OperatorsUtilities::read_generic_selecting_query("source-atoms-", "[]", input);
		source_name=input.get_value<std::string>("source-name");
		destination_name=input.get_value<std::string>("destination-name");
		pooling_mode=input.get_value<std::string>("pooling-mode");
		assert_adjunct_name_input(source_name, false);
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query("source-atoms-", "[]", doc);
		doc.set_option_decription(CDOD("source-name", CDOD::DATATYPE_STRING, "source adjunct name"));
		doc.set_option_decription(CDOD("destination-name", CDOD::DATATYPE_STRING, "destination adjunct name"));
		doc.set_option_decription(CDOD("pooling-mode", CDOD::DATATYPE_STRING, "pooling mode, possible values: mean, sum, min, max, count"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_adjunct_name_input(source_name, false);
		assert_adjunct_name_input(destination_name, false);

		if(pooling_mode!="mean" && pooling_mode!="sum" && pooling_mode!="min" && pooling_mode!="max" && pooling_mode!="count")
		{
			throw std::runtime_error(std::string("Invalid pooling mode, valid options are: 'mean', 'sum', 'min', 'max', 'count'."));
		}

		const std::set<std::size_t> source_atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_source_atoms);
		if(source_atom_ids.empty())
		{
			throw std::runtime_error(std::string("No source atoms selected."));
		}

		std::vector<double> source_values;
		source_values.reserve(source_atom_ids.size());
		std::set<std::size_t> used_source_atom_ids;

		for(std::set<std::size_t>::const_iterator it=source_atom_ids.begin();it!=source_atom_ids.end();++it)
		{
			const std::size_t atom_id=(*it);
			const Atom& atom=data_manager.atoms()[atom_id];
			std::map<std::string, double>::const_iterator jt=atom.value.props.adjuncts.find(source_name);
			if(jt!=atom.value.props.adjuncts.end())
			{
				source_values.push_back(jt->second);
				used_source_atom_ids.insert(atom_id);
			}
		}

		double pooling_result_value=0.0;

		if(source_values.empty())
		{
			if(pooling_mode!="count")
			{
				throw std::runtime_error(std::string("No values found."));
			}
		}
		else
		{
			if(pooling_mode=="mean")
			{
				pooling_result_value=std::accumulate(source_values.begin(), source_values.end(), 0.0)/static_cast<double>(source_values.size());
			}
			else if(pooling_mode=="sum")
			{
				pooling_result_value=std::accumulate(source_values.begin(), source_values.end(), 0.0);
			}
			else if(pooling_mode=="min")
			{
				pooling_result_value=(*std::min_element(source_values.begin(), source_values.end()));
			}
			else if(pooling_mode=="max")
			{
				pooling_result_value=(*std::max_element(source_values.begin(), source_values.end()));
			}
			else if(pooling_mode=="count")
			{
				pooling_result_value=static_cast<double>(source_values.size());
			}
			else
			{
				throw std::runtime_error(std::string("Invalid pooling mode."));
			}
		}

		data_manager.global_numeric_adjuncts_mutable()[destination_name]=pooling_result_value;

		Result result;
		result.used_source_atoms_summary=SummaryOfAtoms(data_manager.atoms(), used_source_atom_ids);
		result.pooling_result_value=pooling_result_value;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_GLOBAL_ADJUNCT_BY_POOLING_H_ */

