#ifndef VIEWER_OPERATORS_SLEEP_H_
#define VIEWER_OPERATORS_SLEEP_H_

#include "../operators_common.h"
#include "../../../../src/auxiliaries/time_utilities.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Sleep : public scripting::OperatorBase<Sleep>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	double milliseconds;

	Sleep() : milliseconds(0.0)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		milliseconds=input.get_value_or_default<double>("milliseconds", 40.0);
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("milliseconds", CDOD::DATATYPE_FLOAT, "milliseconds", 40.0));
	}

	Result run(void*) const
	{
		if(milliseconds>0.0)
		{
			uv::ViewerApplication::instance_refresh_frame();

			auxiliaries::ElapsedProcessorTime elapsed_processor_time;
			double elapsed_miliseconds=0.0;
			elapsed_processor_time.reset();
			do
			{
				elapsed_miliseconds=elapsed_processor_time.elapsed_miliseconds();
			}
			while(elapsed_miliseconds<milliseconds);

			if(milliseconds>17.0)
			{
				uv::ViewerApplication::instance_refresh_frame();
			}
		}
		Result result;
		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_SLEEP_H_ */
