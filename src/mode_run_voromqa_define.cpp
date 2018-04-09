#include "auxiliaries/program_options_handler.h"

#include "common/commanding_manager_for_atoms_and_contacts.h"
#include "common/summary_of_contacts.h"

void run_voromqa_define(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "summary of contacts");
	pohw.describe_io("stdout", false, true, "summary of contacts");

	const std::string list_of_atom_files=poh.argument<std::string>(pohw.describe_option("--list-of-atom-files", "string", "file path to input list of atom-files"), "");
	const bool include_hydrogens=poh.contains_option(pohw.describe_option("--include-hydrogens", "", "flag to include hydrogen atoms"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	common::SummaryOfContacts summary_of_contacts;
	summary_of_contacts.read(std::cin);

	if(!list_of_atom_files.empty())
	{
		const std::set<std::string> filenames=auxiliaries::IOUtilities().read_file_lines_to_set< std::set<std::string> >(list_of_atom_files);
		for(std::set<std::string>::const_iterator it=filenames.begin();it!=filenames.end();++it)
		{
			const std::string& filename=(*it);

			common::CommandingManagerForAtomsAndContacts manager;

			manager.execute_verbosely(std::clog, std::string("load-atoms --format pdb --file '")+filename+"' --as-assembly --include-heteroatoms"+(include_hydrogens ? " --include-hydrogens" : ""));
			manager.execute_verbosely(std::clog, std::string("restrict-atoms {--match R<LEU,ALA,GLY,VAL,GLU,SER,LYS,ILE,ASP,THR,ARG,PRO,ASN,PHE,GLN,TYR,HIS,MET,TRP,CYS,MSE>}"));

			const double uniqueness_of_chains=common::ConstructionOfPrimaryStructure::estimate_uniqueness_of_chains(manager.primary_structure_info(), 0.9);
			std::clog << "number_of_chains = " << manager.primary_structure_info().chains.size() << "\n";
			std::clog << "uniqueness_of_chains = " << uniqueness_of_chains << "\n";

			manager.execute_verbosely(std::clog, std::string("construct-contacts --tag-centrality --tag-peripherial"));

			if(!manager.contacts().empty())
			{
				summary_of_contacts.add(manager.atoms(), manager.contacts(), uniqueness_of_chains);
			}
		}
	}

	summary_of_contacts.write(std::cout);
}

