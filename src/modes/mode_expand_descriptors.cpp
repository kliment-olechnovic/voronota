#include "../auxiliaries/program_options_handler.h"

#include "../common/chain_residue_atom_descriptor.h"

namespace
{

typedef voronota::common::ChainResidueAtomDescriptor CRAD;

inline std::string crad_to_expanded_str(const CRAD& crad)
{
	std::ostringstream output;
	output << (crad.chainID.empty() ? std::string(".") : crad.chainID) << " ";
	if(crad.resSeq==CRAD::null_num()) { output << "." << " "; } else { output << crad.resSeq << " "; }
	output << (crad.iCode.empty() ? std::string(".") : crad.iCode) << " ";
	if(crad.serial==CRAD::null_num()) { output << "." << " "; } else { output << crad.serial << " "; }
	output << (crad.altLoc.empty() ? std::string(".") : crad.altLoc) << " ";
	output << (crad.resName.empty() ? std::string(".") : crad.resName) << " ";
	output << (crad.name.empty() ? std::string(".") : crad.name);
	return output.str();
}

}

void expand_descriptors(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "any text containing atom descriptors");
	pohw.describe_io("stdout", false, true, "text with each atom descriptor expanded to 'chainID resSeq iCode serial altLoc resName name'");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	while(std::cin.good())
	{
		std::string line;
		std::getline(std::cin, line);
		if(!std::cin.fail())
		{
			if(!line.empty())
			{
				std::size_t i=0;
				while(i<line.size())
				{
					i=line.find_first_of("crialAR", i);
					if(i!=std::string::npos)
					{
						std::size_t j=line.find_first_of(" \n\t", i+1);
						if(j==std::string::npos && line[line.size()-1]=='>')
						{
							j=line.size();
						}
						if((j-1)<line.size() && line[j-1]=='>')
						{
							CRAD crad;
							if(CRAD::from_str(line.substr(i, j-i), crad).empty())
							{
								const std::string expanded_str=crad_to_expanded_str(crad);
								line.replace(i, j-i, expanded_str);
								i=i+expanded_str.size();
							}
							else
							{
								i=j;
							}
						}
						else
						{
							i=std::string::npos;
						}
					}
				}
			}
			std::cout << line << "\n";
		}
	}
}
