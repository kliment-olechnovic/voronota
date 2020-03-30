#ifndef SCRIPTING_OPERATORS_RESET_TIME_H_
#define SCRIPTING_OPERATORS_RESET_TIME_H_

#include "../operators_common.h"
#include "../../auxiliaries/time_utilities.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ResetTime : public OperatorBase<ResetTime>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	explicit ResetTime(auxiliaries::ElapsedProcessorTime& elapsed_processor_time) : elapsed_processor_time_ptr_(&elapsed_processor_time)
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
		elapsed_processor_time_ptr_->reset();
		Result result;
		return result;
	}

private:
	auxiliaries::ElapsedProcessorTime* elapsed_processor_time_ptr_;
};

}

}

}

#endif /* SCRIPTING_OPERATORS_RESET_TIME_H_ */
