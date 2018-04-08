#include "auxiliaries/program_options_handler.h"

#include "common/commanding_manager_for_atoms_and_contacts.h"
#include "common/summary_of_contacts.h"

namespace
{

void execute(common::CommandingManagerForAtomsAndContacts& manager, const std::string& command)
{
	if(command.empty())
	{
		return;
	}

	std::ostream& output=std::clog;
	common::CommandingManagerForAtomsAndContacts::CommandOutputSink sink;

	output << "\n> " << command << std::endl;
	const common::CommandingManagerForAtomsAndContacts::CommandRecord record=manager.execute(command, sink);
	output << sink.output_stream.str();
	output << record.output_log;
	if(!record.output_error.empty())
	{
		output << "Error: " << record.output_error << "\n";
	}
	output << std::endl;
}

}

void run_voromqa_define(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "summary of contacts");
	pohw.describe_io("stdout", false, true, "summary of contacts");

	const std::string list_of_atom_files=poh.argument<std::string>(pohw.describe_option("--list-of-atom-files", "string", "file path to input list of atom-files"), "");
	const bool include_hydrogens=poh.contains_option(pohw.describe_option("--include-hydrogens", "", "flag to include hydrogen atoms"));
	const bool tag_peripherial=poh.contains_option(pohw.describe_option("--tag-peripherial", "", "flag to tag peripheral contacts"));

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
			execute(manager, std::string("load-atoms --file '")+filename+"' --as-assembly --include-heteroatoms"+(include_hydrogens ? " --include-hydrogens" : ""));
			execute(manager, std::string("restrict-atoms {--match R<LEU,ALA,GLY,VAL,GLU,SER,LYS,ILE,ASP,THR,ARG,PRO,ASN,PHE,GLN,TYR,HIS,MET,TRP,CYS,MSE>}"));
			execute(manager, std::string("construct-contacts --tag-centrality")+(tag_peripherial ? " --tag-peripherial" : ""));
			if(!manager.contacts().empty())
			{
				summary_of_contacts.add(manager.atoms(), manager.contacts(), 1.0);
			}
		}
	}

	summary_of_contacts.write(std::cout);
}

