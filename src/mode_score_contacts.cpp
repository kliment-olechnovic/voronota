#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cmath>
#include <numeric>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"
#include "auxiliaries/io_utilities.h"

#include "modescommon/generic_utilities.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

struct InteractionName
{
	CRADsPair crads;
	std::string tag;

	InteractionName()
	{
	}

	InteractionName(const CRADsPair& crads, const std::string& tag) : crads(crads), tag(tag)
	{
	}

	bool operator==(const InteractionName& v) const
	{
		return (crads==v.crads && tag==v.tag);
	}

	bool operator<(const InteractionName& v) const
	{
		return ((crads<v.crads) || (crads==v.crads && tag<v.tag));
	}
};

inline std::ostream& operator<<(std::ostream& output, const InteractionName& v)
{
	output << v.crads << " " << v.tag;
	return output;
}

inline std::istream& operator>>(std::istream& input, InteractionName& v)
{
	input >> v.crads >> v.tag;
	return input;
}

struct EnergyDescriptor
{
	double total_area;
	double strange_area;
	double energy;
	int contacts_count;

	EnergyDescriptor() : total_area(0), strange_area(0), energy(0), contacts_count(0)
	{
	}

	void add(const EnergyDescriptor& ed)
	{
		total_area+=ed.total_area;
		strange_area+=ed.strange_area;
		energy+=ed.energy;
		contacts_count+=ed.contacts_count;
	}
};

inline std::ostream& operator<<(std::ostream& output, const EnergyDescriptor& v)
{
	output << v.total_area << " " << v.strange_area << " " << v.energy << " " << v.contacts_count;
	return output;
}

inline std::istream& operator>>(std::istream& input, EnergyDescriptor& v)
{
	input >> v.total_area >> v.strange_area >> v.energy >> v.contacts_count;
	return input;
}

struct NormalDistributionParameters
{
	double mean;
	double sd;

	NormalDistributionParameters() : mean(0.0), sd(1.0)
	{
	}

	NormalDistributionParameters(const double mean, const double sd) : mean(mean), sd(sd)
	{
	}
};

inline std::istream& operator>>(std::istream& input, NormalDistributionParameters& v)
{
	input >> v.mean >> v.sd;
	return input;
}

inline bool check_crads_pair_for_peptide_bond(const CRADsPair& crads)
{
	return (((crads.a.name=="C" && crads.b.name=="N" && crads.a.resSeq<crads.b.resSeq) || (crads.a.name=="N" && crads.b.name=="C" && crads.b.resSeq<crads.a.resSeq))
			&& CRAD::match_with_sequence_separation_interval(crads.a, crads.b, 0, 1, false));
}

inline CRAD generalize_crad(const CRAD& input_crad)
{
	CRAD crad=input_crad.without_numbering();
	if(crad.resName=="ARG" && (crad.name=="NH1" || crad.name=="NH2"))
	{
		crad.name="NH1";
	}
	else if(crad.resName=="ASP" && (crad.name=="OD1" || crad.name=="OD2"))
	{
		crad.name="OD1";
	}
	else if(crad.resName=="GLU" && (crad.name=="OE1" || crad.name=="OE2"))
	{
		crad.name="OE1";
	}
	else if(crad.resName=="PHE" && (crad.name=="CD1" || crad.name=="CD2"))
	{
		crad.name="CD1";
	}
	else if(crad.resName=="PHE" && (crad.name=="CE1" || crad.name=="CE2"))
	{
		crad.name="CE1";
	}
	else if(crad.resName=="TYR" && (crad.name=="CD1" || crad.name=="CD2"))
	{
		crad.name="CD1";
	}
	else if(crad.resName=="TYR" && (crad.name=="CE1" || crad.name=="CE2"))
	{
		crad.name="CE1";
	}
	else if(crad.name=="OXT")
	{
		crad.name="O";
	}
	return crad;
}

inline CRAD simplify_crad(const CRAD& input_crad)
{
	if(input_crad==CRAD::solvent())
	{
		return input_crad;
	}
	else
	{
		return CRAD("any");
	}
}

inline CRADsPair simplify_crads_pair(const CRADsPair& input_crads)
{
	return CRADsPair(simplify_crad(input_crads.a), simplify_crad(input_crads.b));
}

inline bool read_and_accumulate_to_map_of_interactions_areas(std::istream& input, std::map<InteractionName, double>& map_of_interactions_areas)
{
	InteractionName interaction;
	double area;
	input >> interaction >> area;
	if(!input.fail())
	{
		const CRADsPair generalized_crads(generalize_crad(interaction.crads.a), generalize_crad(interaction.crads.b));
		map_of_interactions_areas[InteractionName(generalized_crads, interaction.tag)]+=area;
		return true;
	}
	return false;
}

std::map<CRAD, double> average_atom_scores_by_residue(const std::map<CRAD, double>& atom_scores)
{
	std::map<CRAD, std::pair<int, double> > summed_scores;
	for(std::map<CRAD, double>::const_iterator it=atom_scores.begin();it!=atom_scores.end();++it)
	{
		std::pair<int, double>& value=summed_scores[it->first.without_atom()];
		value.first++;
		value.second+=it->second;
	}
	std::map<CRAD, double> average_scores;
	for(std::map<CRAD, std::pair<int, double> >::const_iterator it=summed_scores.begin();it!=summed_scores.end();++it)
	{
		const std::pair<int, double>& value=it->second;
		average_scores[it->first]=((value.first>0) ? (value.second/static_cast<double>(value.first)) : 0.0);
	}
	return average_scores;
}

std::map<CRAD, double> smooth_residue_scores_along_sequence(const std::map<CRAD, double>& raw_scores, const unsigned int window)
{
	if(window>0)
	{
		std::vector< std::pair<CRAD, double> > v(raw_scores.size());
		std::copy(raw_scores.begin(), raw_scores.end(), v.begin());
		std::vector< std::pair<CRAD, double> > sv=v;
		for(std::size_t i=0;i<v.size();i++)
		{
			const int start=std::max(0, (static_cast<int>(i)-static_cast<int>(window)));
			const int end=std::min(static_cast<int>(v.size())-1, (static_cast<int>(i)+static_cast<int>(window)));
			double sum_of_weighted_values=0.0;
			double sum_of_weights=0.0;
			for(int j=start;j<=end;j++)
			{
				if(v[i].first.chainID==v[j].first.chainID)
				{
					double ndist=fabs(static_cast<double>(static_cast<int>(i)-j))/static_cast<double>(window);
					double weight=(1.0-(ndist*ndist));
					sum_of_weights+=weight;
					sum_of_weighted_values+=v[j].second*weight;
				}
			}
			if(sum_of_weights>0.0)
			{
				sv[i].second=(sum_of_weighted_values/sum_of_weights);
			}
		}
		return std::map<CRAD, double>(sv.begin(), sv.end());
	}
	else
	{
		return raw_scores;
	}
}

}

void score_contacts_potential(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--input-file-list", "", "flag to read file list from stdin"));
		ods.push_back(OD("--input-contributions", "string", "file path to input contact types contributions"));
		ods.push_back(OD("--input-fixed-types", "string", "file path to input fixed types"));
		ods.push_back(OD("--potential-file", "string", "file path to output potential values"));
		ods.push_back(OD("--probabilities-file", "string", "file path to output observed and expected probabilities"));
		ods.push_back(OD("--single-areas-file", "string", "file path to output single type total areas"));
		ods.push_back(OD("--contributions-file", "string", "file path to output contact types contributions"));
		ods.push_back(OD("--multiply-areas", "number", "coefficient to multiply output areas"));
		ods.push_back(OD("--toggling-list", "string", "list of toggling subtags"));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 conditions area')");
			poh.print_io_description("stdout", false, true, "line of contact type area summaries (line format: 'annotation1 annotation2 conditions area')");
			return;
		}
	}

	const bool input_file_list=poh.contains_option("--input-file-list");
	const std::string input_contributions=poh.argument<std::string>("--input-contributions", "");
	const std::string input_fixed_types=poh.argument<std::string>("--input-fixed-types", "");
	const std::string potential_file=poh.argument<std::string>("--potential-file", "");
	const std::string probabilities_file=poh.argument<std::string>("--probabilities-file", "");
	const std::string single_areas_file=poh.argument<std::string>("--single-areas-file", "");
	const std::string contributions_file=poh.argument<std::string>("--contributions-file", "");
	const double multiply_areas=poh.argument<double>("--multiply-areas", -1.0);
	const std::string toggling_list=poh.argument<std::string>("--toggling-list", "");

	const std::set<std::string> toggling_subtags=(toggling_list.empty() ? std::set<std::string>() : auxiliaries::IOUtilities(';').read_string_lines_to_set< std::set<std::string> >(toggling_list));

	std::map<InteractionName, double> map_of_interactions_total_areas;

	if(input_file_list)
	{
		while(std::cin.good())
		{
			std::string file_path;
			std::cin >> file_path;
			auxiliaries::IOUtilities().read_file_lines_to_container(file_path, read_and_accumulate_to_map_of_interactions_areas, map_of_interactions_total_areas);
		}
	}
	else
	{
		auxiliaries::IOUtilities().read_lines_to_container(std::cin, read_and_accumulate_to_map_of_interactions_areas, map_of_interactions_total_areas);
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
			const std::set<std::string> subtags=auxiliaries::IOUtilities(';').read_string_lines_to_set< std::set<std::string> >(interaction.tag);
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
		auxiliaries::IOUtilities().read_file_lines_to_map(input_contributions, map_of_subtags_contributions);
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

		map_of_subtags_contributions[InteractionName(CRADsPair(CRAD("any"), CRAD::solvent()), "solvent")]=(sum_of_solvent_areas/sum_of_contact_areas);
	}

	const double solvent_contribution=map_of_subtags_contributions[InteractionName(CRADsPair(CRAD("any"), CRAD::solvent()), "solvent")];

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
				const std::set<std::string> subtags=auxiliaries::IOUtilities(';').read_string_lines_to_set< std::set<std::string> >(interaction.tag);
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
		auxiliaries::IOUtilities().read_file_lines_to_set(input_fixed_types, fixed_types);
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
			foutput << InteractionName(CRADsPair(CRAD("any"), CRAD("any")), ".") << " " << (sum_obs_nonsolvent+sum_obs_solvent) << " " << (sum_exp_nonsolvent+sum_exp_solvent) << "\n";
		}
	}

	auxiliaries::IOUtilities().write_map_to_file(map_of_crads_total_areas, single_areas_file);

	auxiliaries::IOUtilities().write_map_to_file(map_of_subtags_contributions, contributions_file);

	for(std::map< InteractionName, std::pair<double, double> >::const_iterator it=result.begin();it!=result.end();++it)
	{
		const InteractionName& interaction=it->first;
		std::cout << interaction << " " << (multiply_areas>0.0 ? (it->second.second*multiply_areas) : it->second.second) << "\n";
	}
}

void score_contacts_energy(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--potential-file", "string", "file path to input potential values", true));
		ods.push_back(OD("--ignorable-max-seq-sep", "number", "maximum residue sequence separation for ignorable contacts"));
		ods.push_back(OD("--inter-atom-scores-file", "string", "file path to output inter-atom scores"));
		ods.push_back(OD("--atom-scores-file", "string", "file path to output atom scores"));
		ods.push_back(OD("--depth", "number", "neighborhood normalization depth"));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 conditions area')");
			poh.print_io_description("stdout", false, true, "global scores");
			return;
		}
	}

	const std::string potential_file=poh.argument<std::string>("--potential-file");
	const int ignorable_max_seq_sep=poh.argument<int>("--ignorable-max-seq-sep", 1);
	const std::string inter_atom_scores_file=poh.argument<std::string>("--inter-atom-scores-file", "");
	const std::string atom_scores_file=poh.argument<std::string>("--atom-scores-file", "");
	const int depth=poh.argument<int>("--depth", 2);

	const std::map<InteractionName, double> map_of_contacts=auxiliaries::IOUtilities().read_lines_to_map< std::map<InteractionName, double> >(std::cin);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No contacts input.");
	}

	const std::map<InteractionName, double> map_of_potential_values=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<InteractionName, double> >(potential_file);
	if(map_of_potential_values.empty())
	{
		throw std::runtime_error("No potential values input.");
	}

	std::map<CRADsPair, EnergyDescriptor> inter_atom_energy_descriptors;
	{
		for(std::map<InteractionName, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const CRADsPair& crads=it->first.crads;
			EnergyDescriptor& ed=inter_atom_energy_descriptors[crads];
			if(!CRAD::match_with_sequence_separation_interval(crads.a, crads.b, 0, ignorable_max_seq_sep, false) && !check_crads_pair_for_peptide_bond(crads))
			{
				ed.total_area=(it->second);
				ed.contacts_count=1;
				std::map<InteractionName, double>::const_iterator potential_value_it=
						map_of_potential_values.find(InteractionName(CRADsPair(generalize_crad(crads.a), generalize_crad(crads.b)), it->first.tag));
				if(potential_value_it!=map_of_potential_values.end())
				{
					ed.energy=ed.total_area*(potential_value_it->second);
				}
				else
				{
					ed.strange_area=ed.total_area;
				}
			}
		}
		auxiliaries::IOUtilities().write_map_to_file(inter_atom_energy_descriptors, inter_atom_scores_file);
	}

	const std::map<CRAD, EnergyDescriptor> atom_energy_descriptors=auxiliaries::ChainResidueAtomDescriptorsGraphOperations::accumulate_mapped_values_by_graph_neighbors(inter_atom_energy_descriptors, depth);
	auxiliaries::IOUtilities().write_map_to_file(atom_energy_descriptors, atom_scores_file);

	{
		EnergyDescriptor global_ed;
		for(std::map< CRADsPair, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
		{
			global_ed.add(it->second);
		}
		std::cout << "global " << global_ed << "\n";
	}
}

void score_contacts_quality(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--default-mean", "number", "default mean parameter"));
		ods.push_back(OD("--default-sd", "number", "default standard deviation parameter"));
		ods.push_back(OD("--means-and-sds-file", "string", "file path to input atomic mean and sd parameters"));
		ods.push_back(OD("--mean-shift", "number", "mean shift in standard deviations"));
		ods.push_back(OD("--smoothing-window", "number", "window to smooth residue quality scores along sequence"));
		ods.push_back(OD("--atom-scores-file", "string", "file path to output atom scores"));
		ods.push_back(OD("--residue-scores-file", "string", "file path to output residue scores"));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false, "list of atom energy descriptors");
			poh.print_io_description("stdout", false, true, "average local score");
			return;
		}
	}

	const double default_mean=poh.argument<double>("--default-mean", 0.4);
	const double default_sd=poh.argument<double>("--default-sd", 0.3);
	const std::string mean_and_sds_file=poh.argument<std::string>("--means-and-sds-file", "");
	const double mean_shift=poh.argument<double>("--mean-shift", 0.0);
	const unsigned int smoothing_window=poh.argument<unsigned int>("--smoothing-window", 0);
	const std::string atom_scores_file=poh.argument<std::string>("--atom-scores-file", "");
	const std::string residue_scores_file=poh.argument<std::string>("--residue-scores-file", "");

	const std::map<CRAD, EnergyDescriptor> atom_energy_descriptors=auxiliaries::IOUtilities().read_lines_to_map< std::map<CRAD, EnergyDescriptor> >(std::cin);
	if(atom_energy_descriptors.empty())
	{
		throw std::runtime_error("No input.");
	}

	const std::map<CRAD, NormalDistributionParameters> means_and_sds=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRAD, NormalDistributionParameters> >(mean_and_sds_file);

	std::map<CRAD, double> atom_quality_scores;
	for(std::map<CRAD, EnergyDescriptor>::const_iterator it=atom_energy_descriptors.begin();it!=atom_energy_descriptors.end();++it)
	{
		const CRAD& crad=it->first;
		const EnergyDescriptor& ed=it->second;
		if(ed.total_area>0.0)
		{
			const double actuality_score=(1.0-(ed.strange_area/ed.total_area));
			const double normalized_energy=(ed.energy/ed.total_area);
			std::map<CRAD, NormalDistributionParameters>::const_iterator mean_and_sd_it=means_and_sds.find(generalize_crad(crad));
			const double mean=(mean_and_sd_it!=means_and_sds.end() ? mean_and_sd_it->second.mean : default_mean);
			const double sd=(mean_and_sd_it!=means_and_sds.end() ? mean_and_sd_it->second.sd : default_sd);
			const double adjusted_normalized_energy=((normalized_energy-mean)/sd);
			const double energy_score=(1.0-(0.5*(1.0+erf((adjusted_normalized_energy-mean_shift)/sqrt(2.0)))));
			atom_quality_scores[crad]=(energy_score*actuality_score);
		}
		else
		{
			atom_quality_scores[crad]=0.0;
		}
	}

	auxiliaries::IOUtilities().write_map_to_file(atom_quality_scores, atom_scores_file);

	if(!residue_scores_file.empty())
	{
		auxiliaries::IOUtilities().write_map_to_file(
				smooth_residue_scores_along_sequence(average_atom_scores_by_residue(atom_quality_scores), smoothing_window),
				residue_scores_file);
	}

	if(!atom_quality_scores.empty())
	{
		double sum=0.0;
		for(std::map<CRAD, double>::const_iterator it=atom_quality_scores.begin();it!=atom_quality_scores.end();++it)
		{
			sum+=it->second;
		}
		std::cout << (sum/static_cast<double>(atom_quality_scores.size())) << "\n";
	}
	else
	{
		std::cout << "0\n";
	}
}
