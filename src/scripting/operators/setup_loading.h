#ifndef SCRIPTING_OPERATORS_SETUP_LOADING_H_
#define SCRIPTING_OPERATORS_SETUP_LOADING_H_

#include "common.h"
#include "../loading_of_data.h"

namespace scripting
{

namespace operators
{

class SetupLoading
{
public:
	struct Result
	{
		const Result& write(HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	LoadingOfData::Configuration config;
	std::string radii_file;
	double default_radius;
	bool only_default_radius;

	SetupLoading() : default_radius(LoadingOfData::Configuration::recommended_default_radius()), only_default_radius(false)
	{
	}

	SetupLoading& init(CommandInput& input)
	{
		config=LoadingOfData::Configuration();
		config.include_heteroatoms=input.get_flag("include-heteroatoms");
		config.include_hydrogens=input.get_flag("include-hydrogens");
		config.multimodel_chains=input.get_flag("as-assembly");
		radii_file=input.get_value_or_default<std::string>("radii-file", "");
		default_radius=input.get_value_or_default<double>("default-radius", LoadingOfData::Configuration::recommended_default_radius());
		only_default_radius=input.get_flag("same-radius-for-all");
		return (*this);
	}

	Result run() const
	{
		LoadingOfData::Configuration config_filled=config;

		if(!radii_file.empty())
		{
			InputSelector finput_selector(radii_file);
			std::istream& radii_file_stream=finput_selector.stream();
			if(!radii_file_stream.good())
			{
				throw std::runtime_error(std::string("Failed to read radii file."));
			}
			config_filled.atom_radius_assigner=LoadingOfData::Configuration::generate_atom_radius_assigner(default_radius, only_default_radius, &radii_file_stream);
		}
		else
		{
			config_filled.atom_radius_assigner=LoadingOfData::Configuration::generate_atom_radius_assigner(default_radius, only_default_radius, 0);
		}

		LoadingOfData::Configuration::setup_default_configuration(config_filled);

		Result result;

		return result;
	}
};

}

}




#endif /* SCRIPTING_OPERATORS_SETUP_LOADING_H_ */
