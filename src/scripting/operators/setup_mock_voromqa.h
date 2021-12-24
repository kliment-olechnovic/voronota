#ifndef SCRIPTING_OPERATORS_SETUP_MOCK_VOROMQA_H_
#define SCRIPTING_OPERATORS_SETUP_MOCK_VOROMQA_H_

#include "../operators_common.h"
#include "../scoring_of_data_manager_using_mock_voromqa.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetupMockVoroMQA : public OperatorBase<SetupMockVoroMQA>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string potential_file;
	std::vector<std::string> generating_parameters;

	SetupMockVoroMQA()
	{
	}

	void initialize(CommandInput& input)
	{
		potential_file=input.get_value_or_default<std::string>("potential", "");
		generating_parameters=input.get_value_vector_or_default<std::string>("generating-parameters", std::vector<std::string>());
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("potential", CDOD::DATATYPE_STRING, "path to file with potential values"));
		doc.set_option_decription(CDOD("generating-parameters", CDOD::DATATYPE_STRING_ARRAY, "files to generate a configuration", ""));
	}

	Result run(void*) const
	{
		if(potential_file.empty() && generating_parameters.empty())
		{
			throw std::runtime_error(std::string("No options provided."));
		}

		if(!potential_file.empty() && !generating_parameters.empty())
		{
			throw std::runtime_error(std::string("Incompatible options provided."));
		}

		if(!potential_file.empty())
		{
			if(!ScoringOfDataManagerUsingMockVoroMQA::Configuration::setup_default_configuration(potential_file))
			{
				throw std::runtime_error(std::string("Failed to setup mock VoroMQA."));
			}
		}

		if(!generating_parameters.empty())
		{
			if(!(generating_parameters.size()==2 || generating_parameters.size()==3))
			{
				throw std::runtime_error(std::string("Not 2 or 3 generating parameters provided."));
			}

			if(!ScoringOfDataManagerUsingMockVoroMQA::Configuration::generate_default_configuration(generating_parameters[0], generating_parameters[1]))
			{
				throw std::runtime_error(std::string("Failed to generate and setup mock VoroMQA."));
			}

			if(generating_parameters.size()==3)
			{
				if(!ScoringOfDataManagerUsingMockVoroMQA::Configuration::get_default_configuration().export_to_file(generating_parameters[2]))
				{
					throw std::runtime_error(std::string("Failed to export mock VoroMQA configuration."));
				}
			}
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SETUP_MOCK_VOROMQA_H_ */
