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
	std::string potential_alt_file;
	std::string means_and_sds_file;
	bool faster;

	SetupVoroMQA() : faster(false)
	{
	}

	void initialize(CommandInput& input)
	{
		potential_file=input.get_value<std::string>("potential");
		potential_alt_file=input.get_value_or_default<std::string>("potential-alt", "");
		means_and_sds_file=input.get_value<std::string>("means-and-sds");
		faster=input.get_flag("faster");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("potential", CDOD::DATATYPE_STRING, "path to file with potential values"));
		doc.set_option_decription(CDOD("potential-alt", CDOD::DATATYPE_STRING, "path to file with potential alternative values", ""));
		doc.set_option_decription(CDOD("means-and-sds", CDOD::DATATYPE_STRING, "path to file with means and sds"));
		doc.set_option_decription(CDOD("faster", CDOD::DATATYPE_BOOL, "flag to load potentials assuming the default type descriptors"));
	}

	Result run(void*) const
	{
		if(!ScoringOfDataManagerUsingVoroMQA::Configuration::setup_default_configuration(potential_file, potential_alt_file, means_and_sds_file, faster))
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
