#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"

#include "../common/contacts_scoring_utilities.h"

namespace
{

typedef voronota::common::ChainResidueAtomDescriptor CRAD;
typedef voronota::common::ChainResidueAtomDescriptorsPair CRADsPair;
typedef voronota::common::InteractionName InteractionName;

inline bool read_and_accumulate_to_map_of_interactions_areas(std::istream& input, std::map<InteractionName, double>& map_of_interactions_areas)
{
	InteractionName interaction;
	double area;
	input >> interaction >> area;
	if(!input.fail())
	{
		map_of_interactions_areas[InteractionName(voronota::common::generalize_crads_pair(interaction.crads), interaction.tag)]+=area;
		return true;
	}
	return false;
}

inline bool read_to_map_of_crads_pairs_stats(std::istream& input, std::map< CRADsPair, std::pair<double, double> >& map_of_crads_pairs_stats)
{
	CRAD a;
	CRAD b;
	double p1=0.0;
	double p2=0.0;
	input >> a >> b >> p1 >> p2;
	if(!input.fail())
	{
		map_of_crads_pairs_stats[CRADsPair(a, b)]=std::make_pair(p1, p2);
		return true;
	}
	return false;
}

inline CRAD simplify_crad(const CRAD& input_crad)
{
	if(input_crad==CRAD::solvent())
	{
		return input_crad;
	}
	else
	{
		return CRAD::any();
	}
}

inline CRADsPair simplify_crads_pair(const CRADsPair& input_crads)
{
	return CRADsPair(simplify_crad(input_crads.a), simplify_crad(input_crads.b));
}

}

void score_contacts_potential(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 conditions area')");
	pohw.describe_io("stdout", false, true, "line of contact type area summaries (line format: 'annotation1 annotation2 conditions area')");

	const bool input_file_list=poh.contains_option(pohw.describe_option("--input-file-list", "", "flag to read file list from stdin"));
	const std::string input_contributions=poh.argument<std::string>(pohw.describe_option("--input-contributions", "string", "file path to input contact types contributions"), "");
	const std::string input_fixed_types=poh.argument<std::string>(pohw.describe_option("--input-fixed-types", "string", "file path to input fixed types"), "");
	const std::string input_seq_pairs_stats=poh.argument<std::string>(pohw.describe_option("--input-seq-pairs-stats", "string", "file path to input sequence pairings statistics"), "");
	const std::string potential_file=poh.argument<std::string>(pohw.describe_option("--potential-file", "string", "file path to output potential values"), "");
	const std::string probabilities_file=poh.argument<std::string>(pohw.describe_option("--probabilities-file", "string", "file path to output observed and expected probabilities"), "");
	const std::string single_areas_file=poh.argument<std::string>(pohw.describe_option("--single-areas-file", "string", "file path to output single type total areas"), "");
	const std::string contributions_file=poh.argument<std::string>(pohw.describe_option("--contributions-file", "string", "file path to output contact types contributions"), "");
	const double multiply_areas=poh.argument<double>(pohw.describe_option("--multiply-areas", "number", "coefficient to multiply output areas"), -1.0);
	const std::string toggling_list=poh.argument<std::string>(pohw.describe_option("--toggling-list", "string", "list of toggling subtags"), "");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const std::set<std::string> toggling_subtags=(toggling_list.empty() ? std::set<std::string>() : voronota::auxiliaries::IOUtilities(';').read_string_lines_to_set< std::set<std::string> >(toggling_list));

	std::map<InteractionName, double> map_of_interactions_total_areas;

	if(input_file_list)
	{
		while(std::cin.good())
		{
			std::string file_path;
			std::cin >> file_path;
			voronota::auxiliaries::IOUtilities().read_file_lines_to_container(file_path, read_and_accumulate_to_map_of_interactions_areas, map_of_interactions_total_areas);
		}
	}
	else
	{
		voronota::auxiliaries::IOUtilities().read_lines_to_container(std::cin, read_and_accumulate_to_map_of_interactions_areas, map_of_interactions_total_areas);
	}

	std::map<CRAD, double> map_of_crads_total_areas;
	std::map< CRADsPair, std::set<std::string> > map_of_crads_possible_subtags;
	std::map<InteractionName, double> map_of_subtags_total_areas;
	double sum_of_solvent_areas=0.0;
	double sum_of_nonsolvent_areas=0.0;

	for(std::map<InteractionName, double>::const_iterator it=map_of_interactions_total_areas.begin();it!=map_of_interactions_total_areas.end();++it)
	{
		const InteractionName& interaction=it->first;
		const double area=it->second;
		map_of_crads_total_areas[interaction.crads.a]+=area;
		map_of_crads_total_areas[interaction.crads.b]+=area;
		if(interaction.crads.b==CRAD::solvent())
		{
			sum_of_solvent_areas+=area;
		}
		else
		{
			sum_of_nonsolvent_areas+=area;
			const std::set<std::string> subtags=voronota::auxiliaries::IOUtilities(';').read_string_lines_to_set< std::set<std::string> >(interaction.tag);
			map_of_crads_possible_subtags[interaction.crads].insert(subtags.begin(), subtags.end());
			for(std::set<std::string>::const_iterator subtags_it=subtags.begin();subtags_it!=subtags.end();++subtags_it)
			{
				map_of_subtags_total_areas[InteractionName(simplify_crads_pair(interaction.crads), *subtags_it)]+=area;
			}
		}
	}

	const double sum_of_all_areas=(sum_of_solvent_areas+sum_of_nonsolvent_areas*2.0);
	const double sum_of_contact_areas=(sum_of_solvent_areas+sum_of_nonsolvent_areas);

	std::map<InteractionName, double> map_of_subtags_contributions;
	if(!input_contributions.empty())
	{
		voronota::auxiliaries::IOUtilities().read_file_lines_to_map(input_contributions, map_of_subtags_contributions);
		if(map_of_subtags_contributions.empty())
		{
			throw std::runtime_error("No valid contributions input.");
		}
	}
	else
	{
		std::map<InteractionName, double> map_of_subtags_possible_areas;
		for(std::map<InteractionName, double>::const_iterator it=map_of_interactions_total_areas.begin();it!=map_of_interactions_total_areas.end();++it)
		{
			const InteractionName& interaction=it->first;
			const double area=it->second;
			const std::set<std::string>& subtags=map_of_crads_possible_subtags[interaction.crads];
			for(std::set<std::string>::const_iterator subtags_it=subtags.begin();subtags_it!=subtags.end();++subtags_it)
			{
				map_of_subtags_possible_areas[InteractionName(simplify_crads_pair(interaction.crads), *subtags_it)]+=area;
			}
		}

		for(std::map<InteractionName, double>::const_iterator it=map_of_subtags_total_areas.begin();it!=map_of_subtags_total_areas.end();++it)
		{
			if(toggling_subtags.count(it->first.tag)>0)
			{
				map_of_subtags_contributions[it->first]=(it->second/map_of_subtags_possible_areas[it->first]);
			}
			else
			{
				map_of_subtags_contributions[it->first]=(it->second/sum_of_nonsolvent_areas);
			}
		}

		map_of_subtags_contributions[InteractionName(CRADsPair(CRAD::any(), CRAD::solvent()), "solvent")]=(sum_of_solvent_areas/sum_of_contact_areas);
	}

	std::map< CRADsPair, std::pair<double, double> > seqsep_modifiers;
	if(!input_seq_pairs_stats.empty())
	{
		std::map< CRADsPair, std::pair<double, double> > seq_pairs_stats;
		voronota::auxiliaries::IOUtilities().read_file_lines_to_container(input_seq_pairs_stats, read_to_map_of_crads_pairs_stats, seq_pairs_stats);
		if(!seq_pairs_stats.empty())
		{
			double sep1_contribution=0.0;
			for(std::map<InteractionName, double>::const_iterator it=map_of_subtags_contributions.begin();it!=map_of_subtags_contributions.end();++it)
			{
				const InteractionName& interaction=it->first;
				if(interaction.tag.find("sep1")!=std::string::npos)
				{
					sep1_contribution+=it->second;
				}
			}
			for(std::map< CRADsPair, std::pair<double, double> >::const_iterator it=seq_pairs_stats.begin();it!=seq_pairs_stats.end();++it)
			{
				const double a=(it->second.first/it->second.second);
				const double b=(1-a*sep1_contribution)/(1-sep1_contribution);
				seqsep_modifiers[it->first]=std::make_pair(a, b);
			}
		}
	}

	const double solvent_contribution=map_of_subtags_contributions[InteractionName(CRADsPair(CRAD::any(), CRAD::solvent()), "solvent")];

	std::map< InteractionName, std::pair<double, double> > result;
	std::map< InteractionName, std::pair<double, double> > probabilities;
	for(std::map<InteractionName, double>::const_iterator it=map_of_interactions_total_areas.begin();it!=map_of_interactions_total_areas.end();++it)
	{
		const InteractionName& interaction=it->first;
		const double abc=it->second;
		if(abc>0.0)
		{
			const double p_obs=(abc/sum_of_contact_areas);
			double p_exp=0.0;
			if(interaction.crads.b==CRAD::solvent())
			{
				p_exp=(map_of_crads_total_areas[interaction.crads.a]/sum_of_all_areas)*(solvent_contribution);
			}
			else
			{
				p_exp=(map_of_crads_total_areas[interaction.crads.a]/sum_of_all_areas)*(map_of_crads_total_areas[interaction.crads.b]/sum_of_all_areas)*(1.0-solvent_contribution)*(interaction.crads.a==interaction.crads.b ? 1.0 : 2.0);
				const std::set<std::string> subtags=voronota::auxiliaries::IOUtilities(';').read_string_lines_to_set< std::set<std::string> >(interaction.tag);
				for(std::set<std::string>::const_iterator subtags_it=subtags.begin();subtags_it!=subtags.end();++subtags_it)
				{
					p_exp*=map_of_subtags_contributions[InteractionName(simplify_crads_pair(interaction.crads), *subtags_it)];
				}
				for(std::set<std::string>::const_iterator toggling_subtags_it=toggling_subtags.begin();toggling_subtags_it!=toggling_subtags.end();++toggling_subtags_it)
				{
					if(map_of_crads_possible_subtags[interaction.crads].count(*toggling_subtags_it)>0 && subtags.count(*toggling_subtags_it)==0)
					{
						p_exp*=(1.0-map_of_subtags_contributions[InteractionName(simplify_crads_pair(interaction.crads), *toggling_subtags_it)]);
					}
				}
				if(!seqsep_modifiers.empty() && interaction.tag.find("sep")!=std::string::npos)
				{
					std::map< CRADsPair, std::pair<double, double> >::const_iterator seqsep_modifiers_it=seqsep_modifiers.find(CRADsPair(interaction.crads.a.without_atom(), interaction.crads.b.without_atom()));
					if(seqsep_modifiers_it!=seqsep_modifiers.end())
					{
						if(interaction.tag.find("sep1")!=std::string::npos)
						{
							p_exp*=seqsep_modifiers_it->second.first;
						}
						else
						{
							p_exp*=seqsep_modifiers_it->second.second;
						}
					}
				}
			}
			if(p_exp>0.0)
			{
				result[interaction]=std::make_pair(log(p_exp/p_obs), abc);
				probabilities[interaction]=std::make_pair(p_obs, p_exp);
			}
		}
	}

	if(!input_fixed_types.empty())
	{
		std::set<InteractionName> fixed_types;
		voronota::auxiliaries::IOUtilities().read_file_lines_to_set(input_fixed_types, fixed_types);
		if(fixed_types.empty())
		{
			throw std::runtime_error("No valid fixed types input.");
		}

		double max_potential_value=0.0;
		for(std::map< InteractionName, std::pair<double, double> >::const_iterator it=result.begin();it!=result.end();++it)
		{
			max_potential_value=std::max(max_potential_value, it->second.first);
		}

		std::map< InteractionName, std::pair<double, double> > fixed_result;
		for(std::set<InteractionName>::const_iterator it=fixed_types.begin();it!=fixed_types.end();++it)
		{
			const InteractionName& iname=(*it);
			std::map< InteractionName, std::pair<double, double> >::const_iterator result_it=result.find(iname);
			fixed_result[iname]=(result_it!=result.end() ? result_it->second : std::make_pair(max_potential_value, 0.0));
		}
		result=fixed_result;
	}

	if(!potential_file.empty())
	{
		std::ofstream foutput(potential_file.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map< InteractionName, std::pair<double, double> >::const_iterator it=result.begin();it!=result.end();++it)
			{
				const InteractionName& interaction=it->first;
				foutput << interaction << " " << it->second.first << "\n";
			}
		}
	}

	if(!probabilities_file.empty())
	{
		std::ofstream foutput(probabilities_file.c_str(), std::ios::out);
		if(foutput.good())
		{
			double sum_obs_nonsolvent=0.0;
			double sum_obs_solvent=0.0;
			double sum_exp_nonsolvent=0.0;
			double sum_exp_solvent=0.0;
			for(std::map< InteractionName, std::pair<double, double> >::const_iterator it=probabilities.begin();it!=probabilities.end();++it)
			{
				const InteractionName& interaction=it->first;
				foutput << interaction << " " << it->second.first <<  " " << it->second.second << "\n";
				if(interaction.crads.b==CRAD::solvent())
				{
					sum_obs_solvent+=it->second.first;
					sum_exp_solvent+=it->second.second;
				}
				else
				{
					sum_obs_nonsolvent+=it->second.first;
					sum_exp_nonsolvent+=it->second.second;
				}
			}
			foutput << InteractionName(CRADsPair(CRAD("nonsolvent"), CRAD("nonsolvent")), ".") << " " << sum_obs_nonsolvent << " " << sum_exp_nonsolvent << "\n";
			foutput << InteractionName(CRADsPair(CRAD("nonsolvent"), CRAD::solvent()), ".") << " " << sum_obs_solvent << " " << sum_exp_solvent << "\n";
			foutput << InteractionName(CRADsPair(CRAD::any(), CRAD::any()), ".") << " " << (sum_obs_nonsolvent+sum_obs_solvent) << " " << (sum_exp_nonsolvent+sum_exp_solvent) << "\n";
		}
	}

	voronota::auxiliaries::IOUtilities().write_map_to_file(map_of_crads_total_areas, single_areas_file);

	voronota::auxiliaries::IOUtilities().write_map_to_file(map_of_subtags_contributions, contributions_file);

	for(std::map< InteractionName, std::pair<double, double> >::const_iterator it=result.begin();it!=result.end();++it)
	{
		const InteractionName& interaction=it->first;
		std::cout << interaction << " " << (multiply_areas>0.0 ? (it->second.second*multiply_areas) : it->second.second) << "\n";
	}
}
