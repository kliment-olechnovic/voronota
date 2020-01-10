#ifndef SCRIPTING_OPERATORS_SETUP_VOROMQA_H_
#define SCRIPTING_OPERATORS_SETUP_VOROMQA_H_

#include "../operators_common.h"
#include "../scoring_of_data_manager_using_voromqa.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetupVoroMQA : public OperatorBase<SetupVoroMQA>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string potential_file;
	std::string means_and_sds_file;

	SetupVoroMQA()
	{
	}

	void initialize(CommandInput& input)
	{
		potential_file=input.get_value<std::string>("potential");
		means_and_sds_file=input.get_value<std::string>("means-and-sds");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("potential", CDOD::DATATYPE_STRING, "path to file with potential values"));
		doc.set_option_decription(CDOD("means-and-sds", CDOD::DATATYPE_STRING, "path to file with means and sds"));
	}

	Result run(void*&) const
	{
		if(!ScoringOfDataManagerUsingVoroMQA::Configuration::setup_default_configuration(potential_file, means_and_sds_file))
		{
			throw std::runtime_error(std::string("Failed to setup VoroMQA."));
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SETUP_VOROMQA_H_ */
