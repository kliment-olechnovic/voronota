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
	manager.execute("restrict-atoms sel [{tags-not het}]", std::cout);
	manager.execute("construct-contacts render", std::cout);

//	common::ConstructionOfAtomicBalls::collect_atomic_balls_from_file collect_atomic_balls_from_file;
//
//	std::vector<common::ConstructionOfAtomicBalls::AtomicBall> atomic_balls;
//
//	if(collect_atomic_balls_from_file(std::cin, atomic_balls))
//	{
//		std::clog << atomic_balls.size() << " atoms" << std::endl;
//
//		common::ConstructionOfContacts::construct_bundle_of_contact_information construct_bundle_of_contact_information;
//		construct_bundle_of_contact_information.calculate_volumes=true;
//
//		common::ConstructionOfContacts::BundleOfContactInformation bundle_of_contact_information;
//		if(construct_bundle_of_contact_information(common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(atomic_balls), bundle_of_contact_information))
//		{
//			std::clog << bundle_of_contact_information.contacts.size() << " contacts\n";
//
//			common::SelectionManagerForAtomsAndContacts selection_manager(&atomic_balls, &bundle_of_contact_information.contacts);
//
//			std::clog << selection_manager.select_atoms("{}").size() << " all atoms" << std::endl;
//			std::clog << selection_manager.select_contacts("{}").size() << " all contacts" << std::endl;
//			std::clog << selection_manager.select_contacts("{min-seq-sep 1}").size() << " drawable contacts" << std::endl;
//			std::clog << selection_manager.select_contacts("{max-seq-sep 0}").size() << " non-drawable contacts" << std::endl;
//
//			std::clog << selection_manager.select_atoms("{match c<A>}").size() << " atoms in chain A" << std::endl;
//			std::clog << selection_manager.select_atoms("{match c<B>}").size() << " atoms in chain B" << std::endl;
//			std::clog << selection_manager.select_contacts("{atom-first  {    match    c  <  A  >    }atom-second{match c<B>}}").size() << " contacts between A and B" << std::endl;
//			selection_manager.set_atoms_selection("chainA", selection_manager.select_atoms("{match c<A>}"));
//			selection_manager.set_atoms_selection("chainB", selection_manager.select_atoms("{match c<B>}"));
//			std::clog << selection_manager.select_contacts("{atom-first {selection chainA} atom-second {selection chainB}}").size() << " contacts between A and B" << std::endl;
//			std::clog << selection_manager.select_contacts("{atom-second {match c<solvent>}}").size() << " solvent contacts" << std::endl;
//			std::clog << selection_manager.select_contacts("not {no-solvent}").size() << " solvent contacts" << std::endl;
//			std::clog << selection_manager.select_atoms("{match A<CA>}").size() << " CA atoms" << std::endl;
//			std::clog << selection_manager.select_atoms("{match A<CA>}", true).size() << " CA atoms, full residues" << std::endl;
//			std::clog << selection_manager.select_contacts("{atom-first{match A<C>} atom-second{match A<N>}}").size() << " contacts between C and N atoms" << std::endl;
//			std::clog << selection_manager.select_contacts("{atom-first{match A<C>} atom-second{match A<N>}}", true).size() << " contacts between C and N atoms, full residues" << std::endl;
//			std::clog << selection_manager.select_atoms("{match A<X>}").size() << " X atoms" << std::endl;
//			std::clog << selection_manager.select_atoms("{match A<X>}", true).size() << " X atoms, full residues" << std::endl;
//		}
//	}
}
