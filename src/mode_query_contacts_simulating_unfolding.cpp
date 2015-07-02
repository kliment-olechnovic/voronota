#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

#include "modescommon/contact_value.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

}

void query_contacts_simulating_unfolding(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts [graphics]')");
	pohw.describe_io("stdout", false, true, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts')");

	const int match_max_sequence_separation=poh.argument<int>(pohw.describe_option("--max-seq-sep", "number", "maximum untouchable residue sequence separation", true), 0);

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	enabled_output_of_ContactValue_graphics()=false;

	const std::map<CRADsPair, ContactValue> map_of_contacts=auxiliaries::IOUtilities().read_lines_to_map< std::map<CRADsPair, ContactValue> >(std::cin);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	std::map<CRADsPair, ContactValue> result;
	for(std::map< CRADsPair, ContactValue >::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		const CRADsPair& crads=it->first;
		const ContactValue& value=it->second;
		if(CRAD::match_with_sequence_separation_interval(crads.a, crads.b, 0, match_max_sequence_separation, true) && !crads.contains(CRAD::solvent()))
		{
			result[crads]=value;
		}
		else
		{
			const CRAD* crads_components[2]={&crads.a, &crads.b};
			for(int i=0;i<2;i++)
			{
				const CRAD& crad=(*(crads_components[i]));
				if(crad!=CRAD::solvent())
				{
					ContactValue& solvent_value=result[CRADsPair(crad, CRAD::solvent())];
					solvent_value.area+=value.area;
					solvent_value.dist=std::max(solvent_value.dist, value.dist);
				}
			}
		}
	}

	auxiliaries::IOUtilities().write_map(result, std::cout);
}
