#include <iostream>

#include "auxiliaries/program_options_handler.h"

void generate_demo(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		if(!poh.assert(list_of_option_descriptions, false))
		{
			return;
		}
	}
}
