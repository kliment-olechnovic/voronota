#ifndef SCRIPTING_OPERATORS_PRINT_TIME_H_
#define SCRIPTING_OPERATORS_PRINT_TIME_H_

#include "../operators_common.h"
#include "../../auxiliaries/time_utilities.h"

namespace voronota
{

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

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("elapsed_miliseconds")=elapsed_miliseconds;
		}
	};

	bool reset;

	explicit PrintTime(auxiliaries::ElapsedProcessorTime& elapsed_processor_time) : reset(false), elapsed_processor_time_ptr_(&elapsed_processor_time)
	{
	}

	void initialize(CommandInput& input)
	{
		reset=input.get_flag("reset");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("reset", CDOD::DATATYPE_BOOL, "flag to reset time afterwards"));
	}

	Result run(void*) const
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

}

#endif /* SCRIPTING_OPERATORS_PRINT_TIME_H_ */
