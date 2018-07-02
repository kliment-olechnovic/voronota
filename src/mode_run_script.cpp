#include "auxiliaries/program_options_handler.h"

#include "common/commanding_manager_for_atoms_and_contacts.h"

namespace
{

void run_loop(std::istream& input)
{
	common::CommandingManagerForAtomsAndContacts manager;

	manager.add_representations_of_atoms(std::vector<std::string>(1, "atoms"));
	manager.set_atoms_representation_implemented_always(0, true);

	manager.add_representations_of_contacts(std::vector<std::string>(1, "contacts"));
	manager.set_contacts_representation_implemented_always(0, true);

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
				const common::CommandingManagerForAtomsAndContacts::ScriptRecord script_record=manager.execute_script(line);
				for(std::size_t i=0;i<script_record.output_command_records.size();i++)
				{
					script_record.output_command_records[i].print(std::cout, "\n> ");
				}
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
