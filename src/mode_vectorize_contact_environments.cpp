#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

#include "modescommon/contacts_scoring_utilities.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

std::string generate_generalized_crad_file_name(const CRAD& crad, const std::string& prefix)
{
	const CRAD generalized_crad=generalize_crad(crad);
	return (prefix+generalized_crad.resName+"_"+generalized_crad.name);
}

}

void vectorize_contact_environments(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 area')");
	pohw.describe_io("stdout", false, true, "table of environments");

	const std::string names_file=poh.argument<std::string>(pohw.describe_option("--names-file", "string", "file path to environment names list", true), "");
	const bool inter_residue=poh.contains_option(pohw.describe_option("--inter-residue", "", "flag to use inter-residue contacts"));
	const bool normalize=poh.contains_option(pohw.describe_option("--normalize", "", "flag to normalize output"));
	const bool binarize=poh.contains_option(pohw.describe_option("--binarize", "", "flag to binarize output"));
	const std::string output_files_prefix=poh.argument<std::string>(pohw.describe_option("--output-files-prefix", "string", "file path prefix for per-type output"), "");
	const bool append=poh.contains_option(pohw.describe_option("--append", "", "flag to append to per-type output files"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::map<CRADsPair, double> map_of_contacts=auxiliaries::IOUtilities().read_lines_to_map< std::map<CRADsPair, double> >(std::cin);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No contacts input.");
	}
	if(inter_residue)
	{
		std::map<CRADsPair, double> map_of_reduced_contacts;
		for(std::map<CRADsPair, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const CRADsPair& raw_crads=it->first;
			const CRADsPair crads(raw_crads.a.without_atom(), raw_crads.b.without_atom());
			if(!(crads.a==crads.b))
			{
				map_of_reduced_contacts[crads]=it->second;
			}
		}
		map_of_contacts=map_of_reduced_contacts;
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
			refined_set_of_names.insert(inter_residue ? generalize_crad(*it).without_atom() : generalize_crad(*it));
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

	std::map<std::string, std::ofstream*> map_of_output_file_streams;

	for(std::map<CRAD, std::vector<double> >::const_iterator it=map_of_environments.begin();it!=map_of_environments.end();++it)
	{
		const CRAD& crad=it->first;
		const std::vector<double>& environment=it->second;
		double sum_of_areas=0.0;
		for(std::size_t i=0;i<environment.size();i++)
		{
			sum_of_areas+=environment[i];
		}
		if(sum_of_areas>0.0)
		{
			std::cout << crad;
			for(std::size_t i=0;i<environment.size();i++)
			{
				double output_value=environment[i];
				if(binarize)
				{
					output_value=(output_value>0.0 ? 1.0 : 0.0);
				}
				else if(normalize)
				{
					output_value=(output_value/sum_of_areas);
				}
				std::cout << " " << output_value;
			}
			std::cout << "\n";
			if(!output_files_prefix.empty())
			{
				const std::string output_file_name=generate_generalized_crad_file_name(crad, output_files_prefix);
				std::map<std::string, std::ofstream*>::iterator map_of_output_file_streams_it=map_of_output_file_streams.find(output_file_name);
				if(map_of_output_file_streams_it==map_of_output_file_streams.end())
				{
					map_of_output_file_streams_it=map_of_output_file_streams.insert(std::make_pair(output_file_name, new std::ofstream(output_file_name.c_str(), (append ? std::ios::app : std::ios::out)))).first;
				}
				std::ofstream& foutput=(*(map_of_output_file_streams_it->second));
				foutput << std::fixed;
				for(std::size_t i=0;i<environment.size();i++)
				{
					double output_value=environment[i];
					if(binarize)
					{
						output_value=(output_value>0.0 ? 1.0 : 0.0);
					}
					else if(normalize)
					{
						output_value=(output_value/sum_of_areas);
					}
					if(binarize)
					{
						foutput.precision(0);
					}
					else
					{
						if(fabs(output_value)<0.005)
						{
							foutput.precision(0);
						}
						else
						{
							foutput.precision(2);
						}
					}
					foutput << output_value << ((i+1<environment.size()) ? " " : "\n");
				}
			}
		}
	}

	for(std::map<std::string, std::ofstream*>::iterator it=map_of_output_file_streams.begin();it!=map_of_output_file_streams.end();++it)
	{
		delete (it->second);
	}
}
