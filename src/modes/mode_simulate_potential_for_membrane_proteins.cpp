#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"

#include "../common/contacts_scoring_utilities.h"

namespace
{
	typedef voronota::common::ChainResidueAtomDescriptor CRAD;
	typedef voronota::common::ChainResidueAtomDescriptorsPair CRADsPair;
	typedef voronota::common::InteractionName InteractionName;
}

void simulate_potential_for_membrane_proteins(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "normal potential");
	pohw.describe_io("stdout", false, true, "simulated potential");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::map<InteractionName, double> normal_potential=voronota::auxiliaries::IOUtilities().read_lines_to_map< std::map<InteractionName, double> >(std::cin);

	double sum_of_sas_values=0.0;
	int num_of_sas_values=0;
	for(std::map<InteractionName, double>::const_iterator it=normal_potential.begin();it!=normal_potential.end();++it)
	{
		if(it->first.crads.b==CRAD::solvent())
		{
			sum_of_sas_values+=it->second;
			num_of_sas_values++;
		}
	}

	std::set<CRAD> hydrophobic_crads;
	for(std::map<InteractionName, double>::const_iterator it=normal_potential.begin();it!=normal_potential.end();++it)
	{
		const CRAD crads[2]={it->first.crads.a, it->first.crads.b};
		for(int i=0;i<2;i++)
		{
			if((crads[i].resName=="LEU" || crads[i].resName=="ILE") && crads[i].name!="N" && crads[i].name!="C" && crads[i].name!="O" && crads[i].name!="CA")
			{
				hydrophobic_crads.insert(crads[i]);
			}
		}
	}

	std::map<InteractionName, double> simulated_potential=normal_potential;
	for(std::map<InteractionName, double>::iterator it=simulated_potential.begin();it!=simulated_potential.end();++it)
	{
		const CRADsPair& crads=it->first.crads;
		if(crads.b==CRAD::solvent())
		{
			double sum=0.0;
			int counter=0;
			for(std::set<CRAD>::const_iterator jt=hydrophobic_crads.begin();jt!=hydrophobic_crads.end();++jt)
			{
				sum+=normal_potential[InteractionName(CRADsPair(crads.a, (*jt)), "sep2")];
				counter++;
				sum+=normal_potential[InteractionName(CRADsPair(crads.a, (*jt)), "central_sep2")];
				counter++;
			}
			it->second=sum/static_cast<double>(counter);
		}
	}

	double sum_of_simulated_sas_values=0.0;
	for(std::map<InteractionName, double>::const_iterator it=simulated_potential.begin();it!=simulated_potential.end();++it)
	{
		if(it->first.crads.b==CRAD::solvent())
		{
			sum_of_simulated_sas_values+=it->second;
		}
	}

	const double shift_for_simulated_sas_values=(sum_of_sas_values-sum_of_simulated_sas_values)/static_cast<double>(num_of_sas_values);
	for(std::map<InteractionName, double>::iterator it=simulated_potential.begin();it!=simulated_potential.end();++it)
	{
		if(it->first.crads.b==CRAD::solvent())
		{
			it->second+=shift_for_simulated_sas_values;
		}
	}

	voronota::auxiliaries::IOUtilities().write_map(simulated_potential, std::cout);
}
