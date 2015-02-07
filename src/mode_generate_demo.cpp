#include <iostream>

#include "auxiliaries/program_options_handler.h"

void generate_demo(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		if(!poh.assert(ods, false))
		{
			return;
		}
	}
}
