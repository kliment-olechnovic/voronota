#ifndef SCRIPTING_OPERATORS_SETUP_AKBPS_LAYERED_H_
#define SCRIPTING_OPERATORS_SETUP_AKBPS_LAYERED_H_

#include "../operators_common.h"
#include "../scoring_of_data_manager_interfaces_using_area_kbps_layered.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetupAKBPsLayered : public OperatorBase<SetupAKBPsLayered>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string potentials_file;
	std::string summing_weights_file;

	SetupAKBPsLayered()
	{
	}

	void initialize(CommandInput& input)
	{
		potentials_file=input.get_value<std::string>("potentials");
		summing_weights_file=input.get_value_or_default<std::string>("summing-weights", std::string());
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("potentials", CDOD::DATATYPE_STRING, "path to file with potentials values"));
		doc.set_option_decription(CDOD("summing-weights", CDOD::DATATYPE_STRING, "path to file with summing weights values", ""));
	}

	Result run(void*) const
	{
		if(!ScoringOfDataManagerInterfacesUsingAreaKBPsLayered::Configuration::setup_default_configuration(potentials_file, summing_weights_file))
		{
			throw std::runtime_error(std::string("Failed to setup AKBPs-Layered."));
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SETUP_AKBPS_LAYERED_H_ */
