#ifndef SCRIPTING_OPERATORS_EXIT_H_
#define SCRIPTING_OPERATORS_EXIT_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class Exit
{
public:
	struct Result
	{
		const Result& write(HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	explicit Exit(bool& exit_status) : exit_status_ptr_(&exit_status)
	{
	}

	Exit& init(CommandInput&)
	{
		return (*this);
	}

	Result run(void*&) const
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

#endif /* SCRIPTING_OPERATORS_EXIT_H_ */
