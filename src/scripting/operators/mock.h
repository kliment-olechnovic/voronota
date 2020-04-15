#ifndef SCRIPTING_OPERATORS_MOCK_H_
#define SCRIPTING_OPERATORS_MOCK_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class Mock : public OperatorBase<Mock>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::vector<std::string> strings;

	Mock()
	{
	}

	void initialize(CommandInput&)
	{
	}

	void document(CommandDocumentation&) const
	{
	}

	Result run(void*) const
	{
		Result result;
		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_MOCK_H_ */
