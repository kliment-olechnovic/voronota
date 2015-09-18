#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

#include "modescommon/contacts_scoring_utilities.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

}

void vectorize_contact_environments(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 area')");
	pohw.describe_io("stdout", false, true, "table of environments");

	const std::string names_file=poh.argument<std::string>(pohw.describe_option("--names-file", "string", "file path to environment names list", true), "");
	const bool binarize=poh.contains_option(pohw.describe_option("--binarize", "", "flag to binarize output"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const std::map<CRADsPair, double> map_of_contacts=auxiliaries::IOUtilities().read_lines_to_map< std::map<CRADsPair, double> >(std::cin);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No contacts input.");
	}

	std::set<CRAD> refined_set_of_names;
	{
		const std::set<CRAD> set_of_names=auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRAD> >(names_file);
		if(set_of_names.empty())
		{
			throw std::runtime_error("No environment names input.");
		}
		for(std::set<CRAD>::const_iterator it=set_of_names.begin();it!=set_of_names.end();++it)
		{
			refined_set_of_names.insert(generalize_crad(*it));
		}
	}

	std::map<CRAD, std::size_t> map_of_names_ids;
	{
		std::size_t id=0;
		for(std::set<CRAD>::const_iterator it=refined_set_of_names.begin();it!=refined_set_of_names.end();++it)
		{
			map_of_names_ids[*it]=(id++);
		}
	}

	std::map<CRAD, std::vector<double> > map_of_environments;
	for(std::map<CRADsPair, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		const CRADsPair& crads=it->first;
		const double area=it->second;
		if(area>0.0)
		{
			for(int i=0;i<2;i++)
			{
				const CRAD& crad1=(i==0 ? crads.a : crads.b);
				const CRAD& crad2=(i==0 ? crads.b : crads.a);
				if(crad1!=CRAD::solvent())
				{
					std::vector<double>& environment=map_of_environments[crad1];
					environment.resize(map_of_names_ids.size(), 0.0);
					std::map<CRAD, std::size_t>::const_iterator map_of_names_ids_it=map_of_names_ids.find(generalize_crad(crad2));
					if(map_of_names_ids_it!=map_of_names_ids.end())
					{
						environment[map_of_names_ids_it->second]+=area;
					}
				}
			}
		}
	}

	for(std::map<CRAD, std::vector<double> >::const_iterator it=map_of_environments.begin();it!=map_of_environments.end();++it)
	{
		const CRAD& crad=it->first;
		const std::vector<double>& environment=it->second;
		int environment_diversity=0;
		for(std::size_t i=0;i<environment.size();i++)
		{
			if(environment[i]>0.0)
			{
				environment_diversity++;
			}
		}
		if(environment_diversity>0)
		{
			std::cout << crad << " " << generalize_crad(crad) << " " << environment_diversity;
			for(std::size_t i=0;i<environment.size();i++)
			{
				std::cout << " " << (binarize ? (environment[i]>0.0 ? 1.0 : 0.0) : environment[i]);
			}
			std::cout << "\n";
		}
	}
}
