#include "../auxiliaries/program_options_handler.h"

void demo_blank(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "nothing");
	pohw.describe_io("stdout", false, true, "output");

	std::cout << std::endl;
}

