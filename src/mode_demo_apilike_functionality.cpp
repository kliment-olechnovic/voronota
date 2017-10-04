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
	std::ostringstream output_for_content;

	output << "\n> " << command << std::endl;
	const common::ManipulationManagerForAtomsAndContacts::CommandRecord record=manager.execute(command, &output_for_content);
	output << output_for_content.str();
	output << record.output_log;
	if(!record.output_error.empty())
	{
		output << "Error: " << record.output_error << "\n";
	}
	output << std::endl;
}

}

void demo_apilike_functionality(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "none");
	pohw.describe_io("stdout", false, true, "demo output");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	common::ManipulationManagerForAtomsAndContacts manager;
	execute(manager, "load-atoms file tests/input/single/structure");
	execute(manager, "load-atoms file tests/input/single/structure.cif format mmcif");
	execute(manager, "load-atoms file tests/input/single/structure.pdb");
	execute(manager, "load-atoms file tests/input/single/structure.pdb include-heteroatoms");
	execute(manager, "print-atoms use '{tags het adjuncts tf=0:10}'");
	execute(manager, "restrict-atoms use '{tags-not het}'");
	execute(manager, "construct-contacts render-default calculate-volumes");
	execute(manager, "save-atoms file 'tmp/plain_atoms.txt'");
	execute(manager, "save-contacts file 'tmp/plain_contacts.txt'");
	execute(manager, "load-atoms file 'tmp/plain_atoms.txt' format plain");
	execute(manager, "select-contacts");
	execute(manager, "load-contacts file 'tmp/plain_contacts.txt'");
	execute(manager, "select-contacts use '{atom-first {match R<PHE>} atom-second {match R<PHE>} min-area 5.0 min-seq-sep 1}' name cs1");
	execute(manager, "print-contacts use '{selection cs1}' sort-r area");
	execute(manager, "print-contacts use '{no-solvent min-seq-sep 2}' sort-r area limit 3 expand");
	execute(manager, "print-contacts use '{no-solvent min-seq-sep 2}' sort-r area limit 3 expand inter-residue");
	execute(manager, "select-atoms use '{match r<64>&A<C,N,O,CA,CB>}' name as1");
	execute(manager, "print-atoms use '{selection as1}' sort tags");
	execute(manager, "print-atoms use '{match r<64>&A<C,N,O,CA,CB>}' sort atmn expand");
	execute(manager, "rename-selection-of-atoms nosel1 nodel2");
	execute(manager, "delete-selections-of-contacts nosel1");
	execute(manager, "list-selections-of-atoms");
	execute(manager, "list-selections-of-contacts");
	execute(manager, "print-history last 5");
	execute(manager, "print-history");
}
