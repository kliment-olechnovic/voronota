#include "../auxiliaries/program_options_handler.h"

#include "../common/chain_residue_atom_descriptor.h"
#include "../common/contact_value.h"

namespace
{

typedef voronota::common::ChainResidueAtomDescriptor CRAD;
typedef voronota::common::ChainResidueAtomDescriptorsPair CRADsPair;

struct ResidueValuesSummary
{
	int count;
	int min;
	int max;
	int sum;

	ResidueValuesSummary() : count(0), min(0), max(0), sum(0)
	{
	}

	void record(const int val)
	{
		if(count<1)
		{
			count=1;
			min=val;
			max=val;
			sum=val;
		}
		else
		{
			count++;
			min=std::min(min, val);
			max=std::max(max, val);
			sum+=val;
		}
	}

	double average() const
	{
		return (count>0 ? (static_cast<double>(sum)/static_cast<double>(count)) : 0);
	}
};

}

void query_contacts_depth_values(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2')");
	pohw.describe_io("stdout", false, true, "list of depth values (line format: 'annotation depth')");

	const std::string residue_info=poh.argument<std::string>(pohw.describe_option("--residue-info", "string", "file path to output residue info"), "");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const std::set<CRADsPair> set_of_contacts=voronota::auxiliaries::IOUtilities().read_lines_to_set< std::set<CRADsPair> >(std::cin);
	if(set_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	const std::map<CRAD, int> map_crad_to_depth=voronota::common::ChainResidueAtomDescriptorsGraphOperations::calculate_burial_depth_values(set_of_contacts);

	voronota::auxiliaries::IOUtilities().write_map(map_crad_to_depth, std::cout);

	if(!map_crad_to_depth.empty() && !residue_info.empty())
	{
		std::map<CRAD, ResidueValuesSummary> map_crad_to_depth_summary;
		for(std::map<CRAD, int>::const_iterator it=map_crad_to_depth.begin();it!=map_crad_to_depth.end();++it)
		{
			map_crad_to_depth_summary[it->first.without_atom()].record(it->second);
		}
		std::ofstream foutput(residue_info.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map<CRAD, ResidueValuesSummary>::const_iterator it=map_crad_to_depth_summary.begin();it!=map_crad_to_depth_summary.end();++it)
			{
				foutput << it->first << " " << it->second.average() << " " << it->second.min << " " << it->second.max << " " << it->second.count << "\n";
			}
		}
	}
}
