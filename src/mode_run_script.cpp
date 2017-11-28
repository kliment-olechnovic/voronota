#include "auxiliaries/program_options_handler.h"

#include "common/manipulation_manager_for_atoms_and_contacts.h"

namespace
{

void execute(common::ManipulationManagerForAtomsAndContacts& manager, const std::string& command)
{
	if(command.empty())
	{
		return;
	}

	std::ostream& output=std::cout;
	common::ManipulationManagerForAtomsAndContacts::CommandOutputSink sink;

	output << "\n> " << command << std::endl;
	const common::ManipulationManagerForAtomsAndContacts::CommandRecord record=manager.execute(command, sink);
	output << sink.output_stream.str();
	output << record.output_log;
	if(!record.output_error.empty())
	{
		output << "Error: " << record.output_error << "\n";
	}
	output << std::endl;
}

void run_loop(std::istream& input)
{
	common::ManipulationManagerForAtomsAndContacts manager;

	while(input.good())
	{
		std::string line;
		std::getline(input, line);
		if(!line.empty())
		{
			{
				const std::size_t comments_pos=line.find("#", 0);
				if(comments_pos!=std::string::npos)
				{
					line=line.substr(0, comments_pos);
				}
			}
			if(!line.empty())
			{
				execute(manager, line);
			}
		}
	}
}

}

void run_script(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "script as plain text");
	pohw.describe_io("stdout", false, true, "output on script execution");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	run_loop(std::cin);
}
