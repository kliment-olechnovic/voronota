#include "auxiliaries/program_options_handler.h"

#include "common/filtering_of_atoms_and_contacts.h"

void demo_apilike_functionality(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "input file");
	pohw.describe_io("stdout", false, true, "demo output");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	common::ConstructionOfAtomicBalls::collect_atomic_balls_from_file collect_atomic_balls_from_file;

	std::vector<common::ConstructionOfAtomicBalls::AtomicBall> atomic_balls;

	if(collect_atomic_balls_from_file(std::cin, atomic_balls))
	{
		std::clog<< atomic_balls.size() << " atoms" << std::endl;

		common::ConstructionOfContacts::construct_bundle_of_contact_information construct_bundle_of_contact_information;
		construct_bundle_of_contact_information.calculate_volumes=true;

		common::ConstructionOfContacts::BundleOfContactInformation bundle_of_contact_information;
		if(construct_bundle_of_contact_information(common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(atomic_balls), bundle_of_contact_information))
		{
			std::clog << bundle_of_contact_information.contacts.size() << " contacts\n";

			common::FilteringOfAtomsAndContacts::SelectionManager selection_manager(atomic_balls, bundle_of_contact_information.contacts);

			std::clog << selection_manager.select_atoms("{match c<A>}", false).size() << " atoms in chain A" << std::endl;
			std::clog << selection_manager.select_atoms("{match c<B>}", false).size() << " atoms in chain B" << std::endl;
			std::clog << selection_manager.select_contacts("{atom-first {match c<A>} atom-second {match c<B>}}", false).size() << " contacts between A and B" << std::endl;
			selection_manager.set_atoms_selection("chainA", selection_manager.select_atoms("{match c<A>}", false));
			selection_manager.set_atoms_selection("chainB", selection_manager.select_atoms("{match c<B>}", false));
			std::clog << selection_manager.select_contacts("{atom-first {selection chainA} atom-second {selection chainB}}", false).size() << " contacts between A and B" << std::endl;
			std::clog << selection_manager.select_contacts("{min-seq-sep 1}", false).size() << " drawable contacts" << std::endl;
			std::clog << selection_manager.select_contacts("{max-seq-sep 0}", false).size() << " non-drawable contacts" << std::endl;
			std::clog << selection_manager.select_contacts("{atom-second {match c<solvent>}}", false).size() << " solvent contacts" << std::endl;
			std::clog << selection_manager.select_contacts("not {no-solvent}", false).size() << " solvent contacts" << std::endl;
		}
	}
}
