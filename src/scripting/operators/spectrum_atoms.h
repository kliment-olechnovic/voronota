#ifndef SCRIPTING_OPERATORS_SPECTRUM_ATOMS_H_
#define SCRIPTING_OPERATORS_SPECTRUM_ATOMS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class SpectrumAtoms : public OperatorBase<SpectrumAtoms>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;
		double min_value;
		double max_value;
		int number_of_values;
		double mean_of_values;
		double sd_of_values;

		Result() : min_value(0.0), max_value(0.0), number_of_values(0), mean_of_values(0.0), sd_of_values(0.0)
		{
		}

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			VariantObject& info=heterostorage.variant_object.object("spectrum_summary");
			info.value("min_value")=min_value;
			info.value("max_value")=max_value;
			info.value("number_of_values")=number_of_values;
			info.value("mean_of_values")=mean_of_values;
			info.value("sd_of_values")=sd_of_values;
			return (*this);
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::vector<std::string> representation_names;
	std::string adjunct;
	std::string by;
	std::string scheme;
	bool as_z_scores;
	bool min_val_present;
	double min_val;
	bool max_val_present;
	double max_val;
	bool only_summarize;

	SpectrumAtoms() : as_z_scores(false), min_val_present(false), min_val(0.0), max_val_present(false), max_val(1.0), only_summarize(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
		adjunct=input.get_value_or_default<std::string>("adjunct", "");
		by=adjunct.empty() ? input.get_value_or_default<std::string>("by", "residue-number") : std::string("adjunct");
		scheme=input.get_value_or_default<std::string>("scheme", "reverse-rainbow");
		as_z_scores=input.get_flag("as-z-scores");
		min_val_present=input.is_option("min-val");
		min_val=input.get_value_or_default<double>("min-val", (as_z_scores ? -2.0 : 0.0));
		max_val_present=input.is_option("max-val");
		max_val=input.get_value_or_default<double>("max-val", (as_z_scores ? 2.0 : 1.0));
		only_summarize=input.get_flag("only-summarize");
	}

	void document(CommandDocumentation& doc) const
	{
		Utilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("rep", CDOD::DATATYPE_STRING_ARRAY, "representation names", ""));
		doc.set_option_decription(CDOD("adjunct", CDOD::DATATYPE_STRING, "adjunct name", ""));
		doc.set_option_decription(CDOD("by", CDOD::DATATYPE_STRING, "spectrum source ID", "residue-number"));
		doc.set_option_decription(CDOD("scheme", CDOD::DATATYPE_STRING, "coloring scheme", "reverse-rainbow"));
		doc.set_option_decription(CDOD("as-z-scores", CDOD::DATATYPE_BOOL, "flag to convert values to z-scores"));
		doc.set_option_decription(CDOD("min-val", CDOD::DATATYPE_FLOAT, "min value", 0.0));
		doc.set_option_decription(CDOD("max-val", CDOD::DATATYPE_FLOAT, "max value", 1.0));
		doc.set_option_decription(CDOD("only-summarize", CDOD::DATATYPE_BOOL, "flag to only summarize values"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();
		data_manager.assert_atoms_representations_availability();

		const std::set<std::size_t> representation_ids=data_manager.atoms_representation_descriptor().ids_by_names(representation_names);

		if(by!="residue-number" && by!="adjunct" && by!="chain" && by!="residue-id")
		{
			throw std::runtime_error(std::string("Invalid 'by' value '")+by+"'.");
		}

		if(by=="adjunct" && adjunct.empty())
		{
			throw std::runtime_error(std::string("No adjunct name provided."));
		}

		if(by!="adjunct" && !adjunct.empty())
		{
			throw std::runtime_error(std::string("Adjunct name provided when coloring not by adjunct."));
		}

		if(scheme!="random" && !auxiliaries::ColorUtilities::color_valid(auxiliaries::ColorUtilities::color_from_gradient(scheme, 0.5)))
		{
			throw std::runtime_error(std::string("Invalid 'scheme' value '")+scheme+"'.");
		}

		if(min_val_present && max_val_present && max_val<=min_val)
		{
			throw std::runtime_error(std::string("Minimum and maximum values do not define range."));
		}

		const std::set<std::size_t> ids=data_manager.filter_atoms_drawable_implemented_ids(
				representation_ids,
				data_manager.selection_manager().select_atoms(parameters_for_selecting),
				false);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable atoms selected."));
		}

		std::map<std::size_t, double> map_of_ids_values;

		if(by=="adjunct")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::map<std::string, double>& adjuncts=data_manager.atoms()[*it].value.props.adjuncts;
				std::map<std::string, double>::const_iterator jt=adjuncts.find(adjunct);
				if(jt!=adjuncts.end())
				{
					map_of_ids_values[*it]=jt->second;
				}
			}
		}
		else if(by=="residue-number")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				map_of_ids_values[*it]=data_manager.atoms()[*it].crad.resSeq;
			}
		}
		else if(by=="chain")
		{
			std::map<std::string, double> chains_to_values;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				chains_to_values[data_manager.atoms()[*it].crad.chainID]=0.5;
			}
			if(chains_to_values.size()>1)
			{
				int i=0;
				for(std::map<std::string, double>::iterator it=chains_to_values.begin();it!=chains_to_values.end();++it)
				{
					it->second=static_cast<double>(i)/static_cast<double>(chains_to_values.size()-1);
					i++;
				}
			}
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				map_of_ids_values[*it]=chains_to_values[data_manager.atoms()[*it].crad.chainID];
			}
		}
		else if(by=="residue-id")
		{
			std::map<common::ChainResidueAtomDescriptor, double> residue_ids_to_values;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				residue_ids_to_values[data_manager.atoms()[*it].crad.without_atom()]=0.5;
			}
			if(residue_ids_to_values.size()>1)
			{
				int i=0;
				for(std::map<common::ChainResidueAtomDescriptor, double>::iterator it=residue_ids_to_values.begin();it!=residue_ids_to_values.end();++it)
				{
					it->second=static_cast<double>(i)/static_cast<double>(residue_ids_to_values.size()-1);
					i++;
				}
			}
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				map_of_ids_values[*it]=residue_ids_to_values[data_manager.atoms()[*it].crad.without_atom()];
			}
		}

		if(map_of_ids_values.empty())
		{
			throw std::runtime_error(std::string("Nothing colorable."));
		}

		double min_val_actual=0.0;
		double max_val_actual=0.0;
		int num_of_vals=0;
		double mean_of_values=0.0;
		double sd_of_values=0.0;

		Utilities::calculate_spectrum_info(
				as_z_scores,
				min_val_present,
				min_val,
				max_val_present,
				max_val,
				min_val_actual,
				max_val_actual,
				num_of_vals,
				mean_of_values,
				sd_of_values,
				map_of_ids_values);

		if(!only_summarize)
		{
			DataManager::DisplayStateUpdater dsu;
			dsu.visual_ids=representation_ids;
			if(scheme=="random")
			{
				std::map<double, auxiliaries::ColorUtilities::ColorInteger> map_of_values_colors;
				for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
				{
					map_of_values_colors[it->second]=0;
				}
				for(std::map<double, auxiliaries::ColorUtilities::ColorInteger>::iterator it=map_of_values_colors.begin();it!=map_of_values_colors.end();++it)
				{
					it->second=Utilities::get_next_random_color();
				}
				for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
				{
					dsu.color=map_of_values_colors[it->second];
					data_manager.update_atoms_display_state(dsu, it->first);
				}
			}
			else
			{
				for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
				{
					dsu.color=auxiliaries::ColorUtilities::color_from_gradient(scheme, it->second);
					data_manager.update_atoms_display_state(dsu, it->first);
				}
			}
		}

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);
		result.min_value=min_val_actual;
		result.max_value=max_val_actual;
		result.number_of_values=num_of_vals;
		result.mean_of_values=mean_of_values;
		result.sd_of_values=sd_of_values;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_SPECTRUM_ATOMS_H_ */
