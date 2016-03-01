#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

#include "modescommon/contact_value.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

}

void query_contacts_depth_values(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2')");
	pohw.describe_io("stdout", false, true, "list of depth values (line format: 'annotation depth')");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const std::set<CRADsPair> set_of_contacts=auxiliaries::IOUtilities().read_lines_to_set< std::set<CRADsPair> >(std::cin);
	if(set_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	std::map<CRAD, int> map_crad_to_depth;
	int level_count=0;

	for(std::set<CRADsPair>::const_iterator it=set_of_contacts.begin();it!=set_of_contacts.end();++it)
	{
		const CRADsPair& crads=(*it);
		if(crads.b==CRAD::solvent())
		{
			map_crad_to_depth[crads.a]=1;
			level_count++;
		}
	}

	for(int depth=1;level_count>0;depth++)
	{
		level_count=0;
		for(std::set<CRADsPair>::const_iterator it=set_of_contacts.begin();it!=set_of_contacts.end();++it)
		{
			const CRADsPair& crads=(*it);
			if(!crads.contains(CRAD::solvent()))
			{
				const int depth_a=map_crad_to_depth[crads.a];
				const int depth_b=map_crad_to_depth[crads.b];
				if(depth_a==depth && depth_b==0)
				{
					map_crad_to_depth[crads.b]=(depth+1);
					level_count++;
				}
				else if(depth_b==depth && depth_a==0)
				{
					map_crad_to_depth[crads.a]=(depth+1);
					level_count++;
				}
			}
		}
	}

	for(std::set<CRADsPair>::const_iterator it=set_of_contacts.begin();it!=set_of_contacts.end();++it)
	{
		const CRADsPair& crads=(*it);
		if(crads.a!=CRAD::solvent() && map_crad_to_depth.find(crads.a)==map_crad_to_depth.end())
		{
			map_crad_to_depth[crads.a]=1;
		}
		if(crads.b!=CRAD::solvent() && map_crad_to_depth.find(crads.b)==map_crad_to_depth.end())
		{
			map_crad_to_depth[crads.b]=1;
		}
	}

	auxiliaries::IOUtilities().write_map(map_crad_to_depth, std::cout);
}
