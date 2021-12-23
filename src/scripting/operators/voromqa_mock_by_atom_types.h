#ifndef SCRIPTING_OPERATORS_VOROMQA_MOCK_BY_ATOM_TYPES_H_
#define SCRIPTING_OPERATORS_VOROMQA_MOCK_BY_ATOM_TYPES_H_

#include "../operators_common.h"
#include "../scoring_of_data_manager_using_voromqa.h"
#include "../primitive_chemistry_annotation.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class VoroMQAMockByAtomTypes : public OperatorBase<VoroMQAMockByAtomTypes>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string adjunct_contact_energy;

	VoroMQAMockByAtomTypes()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		adjunct_contact_energy=input.get_value<std::string>("adj-contact-energy");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("adj-contact-energy", CDOD::DATATYPE_STRING, "name of output adjunct for raw energy values"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(adjunct_contact_energy, false);

		if(!PrimitiveChemistryAnnotation::Configuration::get_default_configuration().valid())
		{
			throw std::runtime_error(std::string("Invalid chemistry annotating configuration."));
		}

		if(!ScoringOfDataManagerUsingVoroMQA::Configuration::get_default_configuration().valid())
		{
			throw std::runtime_error(std::string("Invalid VoroMQA configuration."));
		}

		const std::set<std::size_t> contact_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(contact_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		std::map<std::size_t, double> contact_values;

		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const std::size_t contact_id=(*it);
			const Contact& contact=data_manager.contacts()[contact_id];
			contact_values[contact_id]=contact.value.area*get_simplified_contact_potential_value(generate_mock_interaction_name(data_manager, contact));
		}

		for(std::map<std::size_t, double>::const_iterator it=contact_values.begin();it!=contact_values.end();++it)
		{
			std::map<std::string, double>& contact_adjuncts=data_manager.contact_adjuncts_mutable(it->first);
			contact_adjuncts[adjunct_contact_energy]=it->second;
		}

		Result result;

		return result;
	}

private:
	static common::InteractionName generate_mock_interaction_name(const common::ChainResidueAtomDescriptorsPair& crads, const std::string category)
	{
		common::ChainResidueAtomDescriptorsPair mock_crads;
		mock_crads.a.name=PrimitiveChemistryAnnotation::get_knodle_atom_type_string(crads.a);
		mock_crads.b.name=(crads.b!=common::ChainResidueAtomDescriptor::solvent() ? PrimitiveChemistryAnnotation::get_knodle_atom_type_string(crads.b) : "solvent");
//		mock_crads.a.name=PrimitiveChemistryAnnotation::get_CNOSP_atom_type_string(PrimitiveChemistryAnnotation::get_CNOSP_atom_type_number(crads.a));
//		mock_crads.b.name=(crads.b!=common::ChainResidueAtomDescriptor::solvent() ? PrimitiveChemistryAnnotation::get_CNOSP_atom_type_string(PrimitiveChemistryAnnotation::get_CNOSP_atom_type_number(crads.b)) : "solvent");
		return common::InteractionName(common::ChainResidueAtomDescriptorsPair(mock_crads.a, mock_crads.b), category);
	}

	static common::InteractionName generate_mock_interaction_name(const DataManager& data_manager, const Contact& contact)
	{
		std::string category;
		if(contact.solvent())
		{
			category=".";
		}
		else
		{
			if(contact.value.props.tags.count("central")>0)
			{
				category="central_";
			}
			category+="sep2";
		}
		return generate_mock_interaction_name(common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), contact), category);
	}

	static common::InteractionName generate_mock_interaction_name(const common::InteractionName& standard_interaction_name)
	{
		return generate_mock_interaction_name(standard_interaction_name.crads,  standard_interaction_name.tag);
	}

	static void init_simplified_map_of_potential_values_naively(std::map<common::InteractionName, double>& map_of_potential_values)
	{
		const std::map<common::InteractionName, double>& map_of_standard_potential_values=ScoringOfDataManagerUsingVoroMQA::Configuration::get_default_configuration().potential_values.front();
		std::map<common::InteractionName, std::vector<double> > collected_values;
		for(std::map<common::InteractionName, double>::const_iterator it=map_of_standard_potential_values.begin();it!=map_of_standard_potential_values.end();++it)
		{
			bool allowed=true;
			allowed=allowed && it->first.crads.a.name!="C";
			allowed=allowed && it->first.crads.a.name!="CA";
			allowed=allowed && it->first.crads.a.name!="N";
			allowed=allowed && it->first.crads.a.name!="O";
			allowed=allowed && it->first.crads.b.name!="C";
			allowed=allowed && it->first.crads.b.name!="CA";
			allowed=allowed && it->first.crads.b.name!="N";
			allowed=allowed && it->first.crads.b.name!="O";
			if(allowed)
			{
				collected_values[generate_mock_interaction_name(it->first)].push_back(it->second);
			}
		}
		for(std::map<common::InteractionName, std::vector<double> >::const_iterator it=collected_values.begin();it!=collected_values.end();++it)
		{
			const std::vector<double>& values=it->second;
			if(!values.empty())
			{
				double sum_value=0.0;
				double sum_squared_value=0.0;
				for(std::size_t i=0;i<values.size();i++)
				{
					sum_value+=values[i];
					sum_squared_value+=(values[i]*values[i]);
				}
				const double mean_value=(sum_value/values.size());
				const double mean_squared_value=(sum_squared_value/values.size());
				const double standard_deviation_value=std::sqrt(mean_squared_value-(mean_value*mean_value));
				if(standard_deviation_value>0.0)
				{
					double filtered_sum_value=0.0;
					int filtered_count=0;
					for(std::size_t i=0;i<values.size();i++)
					{
						const double abs_z_score=std::abs((values[i]-mean_value)/standard_deviation_value);
						if(abs_z_score<2.0)
						{
							filtered_sum_value+=values[i];
							filtered_count++;
						}
					}
					if(filtered_count>0)
					{
						map_of_potential_values[it->first]=(filtered_sum_value/filtered_count);
					}
					else
					{
						map_of_potential_values[it->first]=mean_value;
					}
				}
				else
				{
					map_of_potential_values[it->first]=mean_value;
				}
			}
		}
	}

	static double get_simplified_contact_potential_value(const common::InteractionName& mock_interaction_name)
	{
		static std::map<common::InteractionName, double> map_of_potential_values;
		if(map_of_potential_values.empty())
		{
			init_simplified_map_of_potential_values_naively(map_of_potential_values);
		}
		std::map<common::InteractionName, double>::const_iterator it=map_of_potential_values.find(mock_interaction_name);
		return ((it!=map_of_potential_values.end()) ? it->second : 0.0);
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_VOROMQA_MOCK_BY_ATOM_TYPES_H_ */
