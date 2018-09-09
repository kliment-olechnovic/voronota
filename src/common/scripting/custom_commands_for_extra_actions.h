#ifndef COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_EXTRA_ACTIONS_H_
#define COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_EXTRA_ACTIONS_H_

#include "../../auxiliaries/time_utilities.h"

#include "generic_command_for_extra_actions.h"

namespace common
{

namespace scripting
{

class CustomsCommandsForExtraActions
{
public:
	class reset_time : public GenericCommandForExtraActions
	{
	public:
		explicit reset_time(auxiliaries::ElapsedProcessorTime& elapsed_processor_time) :
			elapsed_processor_time_(elapsed_processor_time)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.input.assert_nothing_unusable();
			elapsed_processor_time_.reset();
		}

	private:
		auxiliaries::ElapsedProcessorTime& elapsed_processor_time_;
	};

	class print_time : public GenericCommandForExtraActions
	{
	public:
		explicit print_time(auxiliaries::ElapsedProcessorTime& elapsed_processor_time) :
			elapsed_processor_time_(elapsed_processor_time)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			const bool reset=cargs.input.get_flag("reset");
			cargs.input.assert_nothing_unusable();
			cargs.output_for_log << "Elapsed time: " << elapsed_processor_time_.elapsed_miliseconds() << " ms\n";
			if(reset)
			{
				elapsed_processor_time_.reset();
			}
		}

	private:
		auxiliaries::ElapsedProcessorTime& elapsed_processor_time_;
	};
};

}

}

#endif /* COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_EXTRA_ACTIONS_H_ */

