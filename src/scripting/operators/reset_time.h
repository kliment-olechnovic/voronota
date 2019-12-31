#ifndef SCRIPTING_OPERATORS_RESET_TIME_H_
#define SCRIPTING_OPERATORS_RESET_TIME_H_

#include "common.h"
#include "../../auxiliaries/time_utilities.h"

namespace scripting
{

namespace operators
{

class ResetTime
{
public:
	struct Result
	{
		const Result& write(HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	explicit ResetTime(auxiliaries::ElapsedProcessorTime& elapsed_processor_time) : elapsed_processor_time_ptr_(&elapsed_processor_time)
	{
	}

	ResetTime& init(CommandInput&)
	{
		return (*this);
	}

	Result run() const
	{
		elapsed_processor_time_ptr_->reset();
		Result result;
		return result;
	}

private:
	auxiliaries::ElapsedProcessorTime* elapsed_processor_time_ptr_;
};

}

}

#endif /* SCRIPTING_OPERATORS_RESET_TIME_H_ */
