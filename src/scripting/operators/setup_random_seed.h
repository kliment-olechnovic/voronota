#ifndef SCRIPTING_OPERATORS_SETUP_RANDOM_SEED_H_
#define SCRIPTING_OPERATORS_SETUP_RANDOM_SEED_H_

#include <cstdlib>
#include <ctime>

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetupRandomSeed : public OperatorBase<SetupRandomSeed>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	unsigned int seed;

	SetupRandomSeed() : seed(0)
	{
	}

	void initialize(CommandInput& input)
	{
		seed=input.get_value_or_default<unsigned int>("seed", 0);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("seed", CDOD::DATATYPE_INT, "integer seed value, 0 to use current time", 0));
	}

	Result run(void*) const
	{
		if(seed==0)
		{
			std::srand(std::time(0));
		}
		else
		{
			std::srand(seed);
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SETUP_RANDOM_SEED_H_ */
