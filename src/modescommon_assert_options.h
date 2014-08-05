#ifndef MODESCOMMON_ASSERT_OPTIONS_H_
#define MODESCOMMON_ASSERT_OPTIONS_H_

#include "auxiliaries/program_options_handler.h"

namespace modescommon
{

inline bool assert_options(
		const std::vector<auxiliaries::ProgramOptionsHandler::OptionDescription>& basic_list_of_option_descriptions,
		const auxiliaries::ProgramOptionsHandler& poh,
		const bool allow_unrecognized_options)
{
	if(poh.contains_option("--help"))
	{
		if(!basic_list_of_option_descriptions.empty())
		{
			auxiliaries::ProgramOptionsHandler::print_list_of_option_descriptions("", basic_list_of_option_descriptions, std::cerr);
		}
		return false;
	}
	else
	{
		poh.compare_with_list_of_option_descriptions(basic_list_of_option_descriptions, allow_unrecognized_options);
		return true;
	}
}

}

#endif /* MODESCOMMON_ASSERT_OPTIONS_H_ */
