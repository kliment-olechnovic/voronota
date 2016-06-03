#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

}

void query_contacts_solvation_values(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of model contacts (line format: 'annotation1 annotation2 area')");
	pohw.describe_io("stdout", false, true, "list of solvation values (line format: 'annotation sas_area non_sas_area')");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const std::map<CRADsPair, double> map_of_contacts=auxiliaries::IOUtilities().read_lines_to_map< std::map<CRADsPair, double> >(std::cin);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No contacts input.");
	}

	std::map<CRAD, std::pair<double, double> > map_crad_to_solvation;

	for(std::map<CRADsPair, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		const CRADsPair& crads=it->first;
		const double area=it->second;
		if(crads.a!=CRAD::solvent() && crads.b==CRAD::solvent())
		{
			map_crad_to_solvation[crads.a].first+=area;
		}
		else if(crads.a==CRAD::solvent() && crads.b!=CRAD::solvent())
		{
			map_crad_to_solvation[crads.b].first+=area;
		}
		else if(crads.a!=CRAD::solvent() && crads.b!=CRAD::solvent())
		{
			map_crad_to_solvation[crads.a].second+=area;
			map_crad_to_solvation[crads.b].second+=area;
		}
	}

	for(std::map<CRAD, std::pair<double, double> >::const_iterator it=map_crad_to_solvation.begin();it!=map_crad_to_solvation.end();++it)
	{
		std::cout << it->first << " " << it->second.first << " " << it->second.second << "\n";
	}
}
