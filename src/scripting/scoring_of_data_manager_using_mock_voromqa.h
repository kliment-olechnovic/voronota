#ifndef SCRIPTING_SCORING_OF_DATA_MANAGER_USING_MOCK_VOROMQA_H_
#define SCRIPTING_SCORING_OF_DATA_MANAGER_USING_MOCK_VOROMQA_H_

#include "../common/contacts_scoring_utilities.h"
#include "../common/conversion_of_descriptors.h"

#include "io_selectors.h"
#include "data_manager.h"
#include "primitive_chemistry_annotation.h"

namespace voronota
{

namespace scripting
{

class ScoringOfDataManagerUsingMockVoroMQA
{
public:
	class Configuration
	{
	public:
		std::map<common::InteractionName, double> potential_values;

		bool valid() const
		{
			return (!potential_values.empty());
		}

		static const Configuration& get_default_configuration()
		{
			return get_default_configuration_mutable();
		}

		static bool setup_default_configuration(const std::string& potential_file)
		{
			if(potential_file.empty())
			{
				return false;
			}

			std::map<common::InteractionName, double> potential_values;
			{
				InputSelector potential_file_input_selector(potential_file);
				potential_values=auxiliaries::IOUtilities().read_lines_to_map< std::map<common::InteractionName, double> >(potential_file_input_selector.stream());
				if(potential_values.empty())
				{
					return false;
				}
			}

			get_default_configuration_mutable().potential_values.swap(potential_values);

			return true;
		}

		static bool generate_default_configuration(const std::string& original_potential_file, const std::string& original_potential_areas_file)
		{
			if(original_potential_file.empty() || original_potential_areas_file.empty())
			{
				return false;
			}

			std::map<common::InteractionName, double> original_potential_values;
			{
				InputSelector original_potential_file_input_selector(original_potential_file);
				original_potential_values=auxiliaries::IOUtilities().read_lines_to_map< std::map<common::InteractionName, double> >(original_potential_file_input_selector.stream());
				if(original_potential_values.empty())
				{
					return false;
				}
			}

			std::map<common::InteractionName, double> original_potential_area_values;
			{
				InputSelector original_potential_areas_file_input_selector(original_potential_areas_file);
				original_potential_area_values=auxiliaries::IOUtilities().read_lines_to_map< std::map<common::InteractionName, double> >(original_potential_areas_file_input_selector.stream());
				if(original_potential_area_values.empty())
				{
					return false;
				}
			}

			std::map<common::InteractionName, double> potential_values=generate_configuration_potential_values(original_potential_values, original_potential_area_values);
			if(potential_values.empty())
			{
				return false;
			}

			get_default_configuration_mutable().potential_values.swap(potential_values);

			return true;
		}

		bool export_to_file(const std::string& output_file) const
		{
			if(!valid())
			{
				return false;
			}

			OutputSelector output_selector(output_file);
			std::ostream& output=output_selector.stream();

			if(!output.good())
			{
				return false;
			}

			auxiliaries::IOUtilities().write_map(potential_values, output);

			return true;
		}

	private:
		static Configuration& get_default_configuration_mutable()
		{
			static Configuration configuration;
			return configuration;
		}
	};

	static std::map<common::InteractionName, double> generate_configuration_potential_values(const std::map<common::InteractionName, double>& map_of_standard_potential_values, const std::map<common::InteractionName, double>& map_of_standard_potential_areas)
	{
		std::map<common::InteractionName, double> configuration_potential_values;
		std::map<common::InteractionName, std::pair<double, double> > collected_values;
		for(std::map<common::InteractionName, double>::const_iterator it=map_of_standard_potential_values.begin();it!=map_of_standard_potential_values.end();++it)
		{
			const double potential_value=it->second;
			std::map<common::InteractionName, double>::const_iterator potential_areas_it=map_of_standard_potential_areas.find(it->first);
			const double potential_value_area=(potential_areas_it!=map_of_standard_potential_areas.end() ? potential_areas_it->second : 0.0);
			std::pair<double, double>& potential_value_info=collected_values[generate_mock_interaction_name(it->first)];
			potential_value_info.first+=(potential_value*potential_value_area);
			potential_value_info.second+=potential_value_area;
		}
		for(std::map<common::InteractionName, std::pair<double, double> >::const_iterator it=collected_values.begin();it!=collected_values.end();++it)
		{
			const std::pair<double, double>& potential_value_info=it->second;
			if(potential_value_info.second>0.0)
			{
				configuration_potential_values[it->first]=(potential_value_info.first/potential_value_info.second);
			}
		}
		return configuration_potential_values;
	}

	static double get_potential_value(const Configuration& configuration, const DataManager& data_manager, const Contact& contact)
	{
		if(!configuration.valid())
		{
			throw std::runtime_error(std::string("Invalid configuration."));
		}
		std::map<common::InteractionName, double>::const_iterator it=configuration.potential_values.find(generate_mock_interaction_name(data_manager, contact));
		return ((it!=configuration.potential_values.end()) ? it->second : 0.0);
	}

	static double get_potential_value(const DataManager& data_manager, const Contact& contact)
	{
		return get_potential_value(Configuration::get_default_configuration(), data_manager, contact);
	}

	static std::pair<double, double> get_split_to_sas_potential_values(const Configuration& configuration, const DataManager& data_manager, const Contact& contact)
	{
		if(!configuration.valid())
		{
			throw std::runtime_error(std::string("Invalid configuration."));
		}
		const common::ChainResidueAtomDescriptorsPair crads_ab=common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), contact);
		std::map<common::InteractionName, double>::const_iterator it_a=configuration.potential_values.find(generate_mock_interaction_name(common::ChainResidueAtomDescriptorsPair(crads_ab.a, common::ChainResidueAtomDescriptor::solvent()), "."));
		std::map<common::InteractionName, double>::const_iterator it_b=(contact.solvent() ? it_a : configuration.potential_values.find(generate_mock_interaction_name(common::ChainResidueAtomDescriptorsPair(crads_ab.b, common::ChainResidueAtomDescriptor::solvent()), ".")));
		return std::pair<double, double>(((it_a!=configuration.potential_values.end()) ? it_a->second : 0.0), ((it_b!=configuration.potential_values.end()) ? it_b->second : 0.0));
	}

	static std::pair<double, double> get_split_to_sas_potential_values(const DataManager& data_manager, const Contact& contact)
	{
		return get_split_to_sas_potential_values(Configuration::get_default_configuration(), data_manager, contact);
	}

private:
	static common::InteractionName generate_mock_interaction_name(const common::ChainResidueAtomDescriptorsPair& crads, const std::string category)
	{
		if(!PrimitiveChemistryAnnotation::Configuration::get_default_configuration().valid())
		{
			throw std::runtime_error(std::string("Invalid chemistry annotating configuration."));
		}

		common::ChainResidueAtomDescriptorsPair mock_crads;
		mock_crads.a.name=PrimitiveChemistryAnnotation::get_knodle_atom_type_string(crads.a);
		mock_crads.b.name=(crads.b!=common::ChainResidueAtomDescriptor::solvent() ? PrimitiveChemistryAnnotation::get_knodle_atom_type_string(crads.b) : "solvent");
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

};

}

}

#endif /* SCRIPTING_SCORING_OF_DATA_MANAGER_USING_MOCK_VOROMQA_H_ */
