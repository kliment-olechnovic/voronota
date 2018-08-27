#include "auxiliaries/program_options_handler.h"

#include "common/commanding_manager_for_atoms_and_contacts.h"

namespace
{

struct CommandRecordPrinter
{
	void operator ()(const common::scripting::CommandingManagerForAtomsAndContacts::CommandRecord& cr) const
	{
		std::cout << "\n> " << cr.command << std::endl;
		std::cout << cr.output_text_data;
		std::cout << cr.output_log;
		if(!cr.output_error.empty())
		{
			std::cout << "Error: " << cr.output_error << "\n";
		}
		std::cout << std::endl;
	}
};

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

	std::istreambuf_iterator<char> eos;
	std::string script(std::istreambuf_iterator<char>(std::cin), eos);

	common::scripting::CommandingManagerForAtomsAndContacts manager;

	manager.add_representations_of_atoms(std::vector<std::string>(1, "atoms"));
	manager.set_atoms_representation_implemented_always(0, true);

	manager.add_representations_of_contacts(std::vector<std::string>(1, "contacts"));
	manager.set_contacts_representation_implemented_always(0, true);

	manager.execute_script(script, false, CommandRecordPrinter());
}

