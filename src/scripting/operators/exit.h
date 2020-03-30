#ifndef SCRIPTING_OPERATORS_EXIT_H_
#define SCRIPTING_OPERATORS_EXIT_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class Exit : public OperatorBase<Exit>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	explicit Exit(bool& exit_status) : exit_status_ptr_(&exit_status)
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
		(*exit_status_ptr_)=true;
		Result result;
		return result;
	}

private:
	bool* exit_status_ptr_;
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXIT_H_ */
