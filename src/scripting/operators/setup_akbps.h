#ifndef SCRIPTING_OPERATORS_SETUP_AKBPS_H_
#define SCRIPTING_OPERATORS_SETUP_AKBPS_H_

#include "../operators_common.h"
#include "../scoring_of_data_manager_interfaces_using_area_kbps.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetupAKBPs : public OperatorBase<SetupAKBPs>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string potentials_file;

	SetupAKBPs()
	{
	}

	void initialize(CommandInput& input)
	{
		potentials_file=input.get_value<std::string>("potentials");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("potentials", CDOD::DATATYPE_STRING, "path to file with potentials values"));
	}

	Result run(void*) const
	{
		if(!ScoringOfDataManagerInterfacesUsingAreaKBPs::Configuration::setup_default_configuration(potentials_file))
		{
			throw std::runtime_error(std::string("Failed to setup AKBPs."));
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SETUP_AKBPS_H_ */
