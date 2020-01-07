#ifndef SCRIPTING_OPERATORS_PRINT_TIME_H_
#define SCRIPTING_OPERATORS_PRINT_TIME_H_

#include "../operators_common.h"
#include "../../auxiliaries/time_utilities.h"

namespace scripting
{

namespace operators
{

class PrintTime : public OperatorBase<PrintTime>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		double elapsed_miliseconds;

		Result() : elapsed_miliseconds(0.0)
		{
		}

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("elapsed_miliseconds")=elapsed_miliseconds;
			return (*this);
		}
	};

	bool reset;

	explicit PrintTime(auxiliaries::ElapsedProcessorTime& elapsed_processor_time) : reset(false), elapsed_processor_time_ptr_(&elapsed_processor_time)
	{
	}

	PrintTime& init(CommandInput& input)
	{
		reset=input.get_flag("reset");
		return (*this);
	}

	Result run(void*&) const
	{
		Result result;
		result.elapsed_miliseconds=elapsed_processor_time_ptr_->elapsed_miliseconds();
		if(reset)
		{
			elapsed_processor_time_ptr_->reset();
		}
		return result;
	}

private:
	auxiliaries::ElapsedProcessorTime* elapsed_processor_time_ptr_;
};

}

}

#endif /* SCRIPTING_OPERATORS_PRINT_TIME_H_ */
