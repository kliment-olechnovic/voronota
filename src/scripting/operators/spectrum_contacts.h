#ifndef SCRIPTING_OPERATORS_SPECTRUM_CONTACTS_H_
#define SCRIPTING_OPERATORS_SPECTRUM_CONTACTS_H_

#include <cstdint>

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SpectrumContacts : public OperatorBase<SpectrumContacts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfContacts contacts_summary;
		double min_value;
		double max_value;
		int number_of_values;
		double mean_of_values;
		double sd_of_values;

		Result() : min_value(0.0), max_value(0.0), number_of_values(0), mean_of_values(0.0), sd_of_values(0.0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			VariantObject& info=heterostorage.variant_object.object("spectrum_summary");
			info.value("min_value")=min_value;
			info.value("max_value")=max_value;
			info.value("number_of_values")=number_of_values;
			info.value("mean_of_values")=mean_of_values;
			info.value("sd_of_values")=sd_of_values;
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::vector<std::string> representation_names;
	std::string adjunct;
	std::string by;
	std::string scheme;
	bool additive_coloring;
	bool as_z_scores;
	bool min_val_present;
	double min_val;
	bool max_val_present;
	double max_val;
	bool only_summarize;

	SpectrumContacts() : additive_coloring(false),as_z_scores(false), min_val_present(false), min_val(0.0), max_val_present(false), max_val(1.0), only_summarize(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
		adjunct=input.get_value_or_default<std::string>("adjunct", "");
		by=adjunct.empty() ? input.get_value<std::string>("by") : std::string("adjunct");
		scheme=input.get_value_or_default<std::string>("scheme", "reverse-rainbow");
		additive_coloring=input.get_flag("additive-coloring");
		as_z_scores=input.get_flag("as-z-scores");
		min_val_present=input.is_option("min-val");
		min_val=input.get_value_or_default<double>("min-val", (as_z_scores ? -2.0 : 0.0));
		max_val_present=input.is_option("max-val");
		max_val=input.get_value_or_default<double>("max-val", (as_z_scores ? 2.0 : 1.0));
		only_summarize=input.get_flag("only-summarize");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("rep", CDOD::DATATYPE_STRING_ARRAY, "representation names", ""));
		doc.set_option_decription(CDOD("adjunct", CDOD::DATATYPE_STRING, "adjunct name", ""));
		doc.set_option_decription(CDOD("by", CDOD::DATATYPE_STRING, "spectrum source ID", ""));
		doc.set_option_decription(CDOD("scheme", CDOD::DATATYPE_STRING, "coloring scheme", "reverse-rainbow"));
		doc.set_option_decription(CDOD("additive-coloring", CDOD::DATATYPE_BOOL, "flag to sum new color with current color in RGB space"));
		doc.set_option_decription(CDOD("as-z-scores", CDOD::DATATYPE_BOOL, "flag to convert values to z-scores"));
		doc.set_option_decription(CDOD("min-val", CDOD::DATATYPE_FLOAT, "min value", 0.0));
		doc.set_option_decription(CDOD("max-val", CDOD::DATATYPE_FLOAT, "max value", 1.0));
		doc.set_option_decription(CDOD("only-summarize", CDOD::DATATYPE_BOOL, "flag to only summarize values"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();
		data_manager.assert_contacts_representations_availability();

		const std::set<std::size_t> representation_ids=data_manager.contacts_representation_descriptor().ids_by_names(representation_names);

		if(by!="area" && by!="residue-area" && by!="chain-area" && by!="adjunct" && by!="dist-centers" && by!="dist-balls" && by!="seq-sep" && by!="residue-ids" && by!="atom-ids" && by!="residue-ids-simplified" && by!="atom-ids-simplified" && by!="chain-ids")
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

		const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		std::map<std::size_t, double> map_of_ids_values;

		if(by=="adjunct")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::map<std::string, double>& adjuncts=data_manager.contacts()[*it].value.props.adjuncts;
				std::map<std::string, double>::const_iterator jt=adjuncts.find(adjunct);
				if(jt!=adjuncts.end())
				{
					map_of_ids_values[*it]=jt->second;
				}
			}
		}
		else if(by=="area")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				map_of_ids_values[*it]=data_manager.contacts()[*it].value.area;
			}
		}
		else if(by=="residue-area")
		{
			std::map<common::ChainResidueAtomDescriptorsPair, double> residue_ids_to_values;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const Contact& contact=data_manager.contacts()[*it];
				residue_ids_to_values[common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), contact).without_some_info(true, true, false, false)]+=contact.value.area;
			}
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				map_of_ids_values[*it]=residue_ids_to_values[common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), data_manager.contacts()[*it]).without_some_info(true, true, false, false)];
			}
		}
		else if(by=="chain-area")
		{
			std::map<common::ChainResidueAtomDescriptorsPair, double> chain_ids_to_values;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const Contact& contact=data_manager.contacts()[*it];
				const common::ChainResidueAtomDescriptorsPair raw_crads=common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), data_manager.contacts()[*it]);
				const common::ChainResidueAtomDescriptorsPair crads=common::ChainResidueAtomDescriptorsPair(common::ChainResidueAtomDescriptor(raw_crads.a.chainID), common::ChainResidueAtomDescriptor(raw_crads.b.chainID));
				chain_ids_to_values[crads]+=contact.value.area;
			}
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const common::ChainResidueAtomDescriptorsPair raw_crads=common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), data_manager.contacts()[*it]);
				const common::ChainResidueAtomDescriptorsPair crads=common::ChainResidueAtomDescriptorsPair(common::ChainResidueAtomDescriptor(raw_crads.a.chainID), common::ChainResidueAtomDescriptor(raw_crads.b.chainID));
				map_of_ids_values[*it]=chain_ids_to_values[crads];
			}
		}
		else if(by=="dist-centers")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				map_of_ids_values[*it]=data_manager.contacts()[*it].value.dist;
			}
		}
		else if(by=="dist-balls")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::size_t id0=data_manager.contacts()[*it].ids[0];
				const std::size_t id1=data_manager.contacts()[*it].ids[1];
				if(data_manager.contacts()[*it].solvent())
				{
					map_of_ids_values[*it]=(data_manager.contacts()[*it].value.dist-data_manager.atoms()[id0].value.r)/3.0*2.0;
				}
				else
				{
					map_of_ids_values[*it]=apollota::minimal_distance_from_sphere_to_sphere(data_manager.atoms()[id0].value, data_manager.atoms()[id1].value);
				}
			}
		}
		else if(by=="seq-sep")
		{
			double max_seq_sep=0.0;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::size_t id0=data_manager.contacts()[*it].ids[0];
				const std::size_t id1=data_manager.contacts()[*it].ids[1];
				if(data_manager.atoms()[id0].crad.chainID==data_manager.atoms()[id1].crad.chainID)
				{
					const double seq_sep=fabs(static_cast<double>(data_manager.atoms()[id0].crad.resSeq-data_manager.atoms()[id1].crad.resSeq));
					map_of_ids_values[*it]=seq_sep;
					max_seq_sep=((max_seq_sep<seq_sep) ? seq_sep : max_seq_sep);
				}
			}
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::size_t id0=data_manager.contacts()[*it].ids[0];
				const std::size_t id1=data_manager.contacts()[*it].ids[1];
				if(data_manager.atoms()[id0].crad.chainID!=data_manager.atoms()[id1].crad.chainID)
				{
					map_of_ids_values[*it]=max_seq_sep+1.0;
				}
			}
		}
		else if(by=="residue-ids")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const common::ChainResidueAtomDescriptorsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), data_manager.contacts()[*it]).without_some_info(true, true, false, false);
				map_of_ids_values[*it]=static_cast<double>(crads.hash_value())/static_cast<double>(std::numeric_limits<std::uint32_t>::max());
			}
		}
		else if(by=="atom-ids")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const common::ChainResidueAtomDescriptorsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), data_manager.contacts()[*it]).without_some_info(true, false, false, false);
				map_of_ids_values[*it]=static_cast<double>(crads.hash_value())/static_cast<double>(std::numeric_limits<std::uint32_t>::max());
			}
		}
		else if(by=="residue-ids-simplified")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				common::ChainResidueAtomDescriptorsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), data_manager.contacts()[*it]).without_some_info(true, true, false, true);
				crads.a.chainID.clear();
				crads.b.chainID.clear();
				map_of_ids_values[*it]=static_cast<double>(crads.hash_value())/static_cast<double>(std::numeric_limits<std::uint32_t>::max());
			}
		}
		else if(by=="atom-ids-simplified")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				common::ChainResidueAtomDescriptorsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), data_manager.contacts()[*it]).without_some_info(true, false, false, true);
				crads.a.chainID.clear();
				crads.b.chainID.clear();
				map_of_ids_values[*it]=static_cast<double>(crads.hash_value())/static_cast<double>(std::numeric_limits<std::uint32_t>::max());
			}
		}
		else if(by=="chain-ids")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const common::ChainResidueAtomDescriptorsPair raw_crads=common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), data_manager.contacts()[*it]);
				const common::ChainResidueAtomDescriptorsPair crads(common::ChainResidueAtomDescriptor(raw_crads.a.chainID), common::ChainResidueAtomDescriptor(raw_crads.b.chainID));
				map_of_ids_values[*it]=static_cast<double>(crads.hash_value())/static_cast<double>(std::numeric_limits<std::uint32_t>::max());
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

		OperatorsUtilities::calculate_spectrum_info(
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
			dsu.additive_color=additive_coloring;
			if(scheme=="random")
			{
				std::map<double, auxiliaries::ColorUtilities::ColorInteger> map_of_values_colors;
				for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
				{
					map_of_values_colors[it->second]=0;
				}
				for(std::map<double, auxiliaries::ColorUtilities::ColorInteger>::iterator it=map_of_values_colors.begin();it!=map_of_values_colors.end();++it)
				{
					it->second=OperatorsUtilities::get_next_random_color();
				}
				for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
				{
					dsu.color=map_of_values_colors[it->second];
					data_manager.update_contacts_display_state(dsu, it->first);
				}
			}
			else
			{
				for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
				{
					dsu.color=auxiliaries::ColorUtilities::color_from_gradient(scheme, it->second);
					data_manager.update_contacts_display_state(dsu, it->first);
				}
			}
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);
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

}

#endif /* SCRIPTING_OPERATORS_SPECTRUM_CONTACTS_H_ */
