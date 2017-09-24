#include "auxiliaries/program_options_handler.h"

#include "common/manipulation_manager_for_atoms_and_contacts.h"

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
	manager.execute("read-atoms file tests/input/single/structure.pdb heteroatoms", std::cout);
	manager.execute("query-atoms use '{tags het adjuncts tf=0:10}' print", std::cout);
	manager.execute("restrict-atoms use '{tags-not het}'", std::cout);
	manager.execute("construct-contacts render", std::cout);
	manager.execute("query-contacts use '{atom-first {match R<PHE>} atom-second {match R<PHE>} min-area 5.0 min-seq-sep 1}' print name cs1", std::cout);
	manager.execute("query-atoms use '{match r<64>&A<C,N,O,CA,CB>}' print name as1", std::cout);
	manager.execute("manage-selections-of-atoms list", std::cout);
	manager.execute("manage-selections-of-contacts list", std::cout);
}
