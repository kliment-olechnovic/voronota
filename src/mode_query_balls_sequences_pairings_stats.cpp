#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

#include "modescommon/ball_value.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

}

void query_balls_sequences_pairings_stats(const auxiliaries::ProgramOptionsHandler& poh)
{
	typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls files");
	pohw.describe_io("stdout", false, true, "list of sequences pairings stats");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::map<CRAD, double> map_of_single_stats;
	double sum_of_single_stats=0.0;
	std::map<CRADsPair, double> map_of_pairing_stats;
	double sum_of_pairings_stats=0.0;

	while(std::cin.good())
	{
		std::string balls_file;
		std::cin >> balls_file;
		std::vector<CRAD> list_of_balls;
		auxiliaries::IOUtilities().read_file_lines_to_set(balls_file, list_of_balls);
		std::map< std::string, std::set<CRAD> > chains;
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			chains[list_of_balls[i].chainID].insert(list_of_balls[i].without_atom());
		}
		for(std::map< std::string, std::set<CRAD> >::const_iterator chains_it=chains.begin();chains_it!=chains.end();++chains_it)
		{
			std::vector<CRAD> residues(chains_it->second.begin(), chains_it->second.end());
			for(std::size_t i=0;i<residues.size();i++)
			{
				map_of_single_stats[residues[i].without_numbering()]+=1;
				sum_of_single_stats+=1;
			}
			for(std::size_t i=0;i+1<residues.size();i++)
			{
				if(CRAD::match_with_sequence_separation_interval(residues[i], residues[i+1], 1, 1, false))
				{
					map_of_pairing_stats[CRADsPair(residues[i].without_numbering(), residues[i+1].without_numbering())]+=1;
					sum_of_pairings_stats+=1;
				}
			}
		}
	}

	double sum_of_p_obs=0.0;
	double sum_of_p_exp=0.0;
	for(std::map<CRADsPair, double>::const_iterator it=map_of_pairing_stats.begin();it!=map_of_pairing_stats.end();++it)
	{
		const CRADsPair& crads=it->first;
		const double pairing_stat=it->second;
		const double single1_stat=map_of_single_stats[crads.a];
		const double single2_stat=map_of_single_stats[crads.b];
		const double p_obs=(pairing_stat/sum_of_pairings_stats);
		const double p_exp=((single1_stat/sum_of_single_stats)*(single2_stat/sum_of_single_stats)*(crads.a==crads.b ? 1.0 : 2.0));
		sum_of_p_obs+=p_obs;
		sum_of_p_exp+=p_exp;
		std::cout << crads << " " << p_obs << " " << p_exp << "\n";
	}
	std::cout << CRADsPair(CRAD::any(), CRAD::any()) << " " << sum_of_p_obs << " " << sum_of_p_exp << "\n";
}
