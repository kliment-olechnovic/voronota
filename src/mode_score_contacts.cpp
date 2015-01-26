#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cmath>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"
#include "auxiliaries/io_utilities.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

typedef std::pair<CRADsPair, std::string> Interaction0;

struct Interaction
{
	CRADsPair crads;
	std::string tag;

	Interaction()
	{
	}

	Interaction(const CRADsPair& crads, const std::string& tag) : crads(crads), tag(tag)
	{
	}

	bool operator==(const Interaction& v) const
	{
		return (crads==v.crads && tag==v.tag);
	}

	bool operator<(const Interaction& v) const
	{
		return ((crads<v.crads) || (crads==v.crads && tag<v.tag));
	}
};

inline std::ostream& operator<<(std::ostream& output, const Interaction& v)
{
	output << v.crads << " " << v.tag;
	return output;
}

inline std::istream& operator>>(std::istream& input, Interaction& v)
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

inline bool read_and_accumulate_to_map_of_interactions_areas(std::istream& input, std::map<Interaction, double>& map_of_interactions_areas)
{
	Interaction interaction;
	double area;
	input >> interaction >> area;
	if(!input.fail())
	{
		const CRADsPair crads_without_numbering(interaction.crads.a.without_numbering(), interaction.crads.b.without_numbering());
		map_of_interactions_areas[Interaction(crads_without_numbering, interaction.tag)]+=area;
		return true;
	}
	return false;
}

std::map<CRAD, double> smooth_residue_scores_along_sequence(const std::map<CRAD, double>& raw_scores, const unsigned int window)
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

}

void score_contacts_potential(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--input-file-list", "", "flag to read file list from stdin"));
		list_of_option_descriptions.push_back(OD("--potential-file", "string", "file path to output potential values"));
		list_of_option_descriptions.push_back(OD("--solvent-factor", "number", "solvent factor value"));
		if(!poh.assert(list_of_option_descriptions, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 conditions area')\n";
			std::cerr << "stdout  ->  line of contact type area summaries (line format: 'annotation1 annotation2 conditions area')\n";
			return;
		}
	}

	const bool input_file_list=poh.contains_option("--input-file-list");
	const std::string potential_file=poh.argument<std::string>("--potential-file", "");
	const double solvent_factor=poh.argument<double>("--solvent-factor", 1.0);

	std::map<Interaction, double> map_of_interactions_total_areas;
	std::map<CRAD, double> map_of_crads_total_areas;
	std::map<std::string, double> map_of_conditions_total_areas;
	double sum_of_all_areas=0.0;

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

	for(std::map<Interaction, double>::const_iterator it=map_of_interactions_total_areas.begin();it!=map_of_interactions_total_areas.end();++it)
	{
		const Interaction& interaction=it->first;
		const double area=it->second;
		map_of_crads_total_areas[interaction.crads.a]+=area;
		map_of_crads_total_areas[interaction.crads.b]+=area;
		map_of_conditions_total_areas[interaction.tag]+=area;
		sum_of_all_areas+=area;
	}

	if(solvent_factor>0.0)
	{
		std::map<CRAD, double>::iterator it=map_of_crads_total_areas.find(CRAD::solvent());
		if(it!=map_of_crads_total_areas.end())
		{
			const double additional_area=solvent_factor*(it->second);
			it->second+=additional_area;
			map_of_conditions_total_areas["."]+=additional_area;
			sum_of_all_areas+=additional_area;
		}
	}

	std::map< Interaction, std::pair<double, double> > result;
	for(std::map<Interaction, double>::const_iterator it=map_of_interactions_total_areas.begin();it!=map_of_interactions_total_areas.end();++it)
	{
		const Interaction& interaction=it->first;
		const double abc=it->second;
		const double ax=map_of_crads_total_areas[interaction.crads.a];
		const double bx=map_of_crads_total_areas[interaction.crads.b];
		const double cx=map_of_conditions_total_areas[interaction.tag];
		if(abc>0.0 && ax>0.0 && bx>0.0 && cx>0.0)
		{
			const double potential_value=(0.0-log((abc*sum_of_all_areas*sum_of_all_areas)/(ax*bx*cx)));
			result[interaction]=std::make_pair(potential_value, abc);
		}
	}

	if(!potential_file.empty())
	{
		std::ofstream foutput(potential_file.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map< Interaction, std::pair<double, double> >::const_iterator it=result.begin();it!=result.end();++it)
			{
				const Interaction& interaction=it->first;
				foutput << interaction << " " << it->second.first << "\n";
			}
		}
	}

	for(std::map< Interaction, std::pair<double, double> >::const_iterator it=result.begin();it!=result.end();++it)
	{
		const Interaction& interaction=it->first;
		std::cout << interaction << " " << it->second.second << "\n";
	}
}

void score_contacts_energy(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--potential-file", "string", "file path to input potential values", true));
		list_of_option_descriptions.push_back(OD("--ignorable-max-seq-sep", "number", "maximum residue sequence separation for ignorable contacts"));
		list_of_option_descriptions.push_back(OD("--inter-atom-scores-file", "string", "file path to output inter-atom scores"));
		list_of_option_descriptions.push_back(OD("--inter-residue-scores-file", "string", "file path to output inter-residue scores"));
		list_of_option_descriptions.push_back(OD("--atom-scores-file", "string", "file path to output atom scores"));
		list_of_option_descriptions.push_back(OD("--residue-scores-file", "string", "file path to output residue scores"));
		list_of_option_descriptions.push_back(OD("--depth", "number", "neighborhood normalization depth"));
		if(!poh.assert(list_of_option_descriptions, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 conditions area')\n";
			std::cerr << "stdout  ->  global scores\n";
			return;
		}
	}

	const std::string potential_file=poh.argument<std::string>("--potential-file");
	const int ignorable_max_seq_sep=poh.argument<int>("--ignorable-max-seq-sep", 1);
	const std::string inter_atom_scores_file=poh.argument<std::string>("--inter-atom-scores-file", "");
	const std::string inter_residue_scores_file=poh.argument<std::string>("--inter-residue-scores-file", "");
	const std::string atom_scores_file=poh.argument<std::string>("--atom-scores-file", "");
	const std::string residue_scores_file=poh.argument<std::string>("--residue-scores-file", "");
	const int depth=poh.argument<int>("--depth", 2);

	std::map<Interaction, double> map_of_contacts;
	{
		auxiliaries::IOUtilities().read_lines_to_map_container(std::cin, map_of_contacts);
		if(map_of_contacts.empty())
		{
			throw std::runtime_error("No contacts input.");
		}
	}

	std::map<Interaction, double> map_of_potential_values;
	{
		auxiliaries::IOUtilities().read_file_lines_to_map_container(potential_file, map_of_potential_values);
		if(map_of_potential_values.empty())
		{
			throw std::runtime_error("No potential values input.");
		}
	}

	std::map<CRADsPair, EnergyDescriptor> inter_atom_energy_descriptors;
	{
		for(std::map<Interaction, double>::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const CRADsPair& crads=it->first.crads;
			EnergyDescriptor& ed=inter_atom_energy_descriptors[crads];
			if(!CRAD::match_with_sequence_separation_interval(crads.a, crads.b, 0, ignorable_max_seq_sep, false))
			{
				ed.total_area=it->second;
				ed.contacts_count=1;
				std::map<Interaction, double>::const_iterator potential_value_it=
						map_of_potential_values.find(Interaction(CRADsPair(crads.a.without_numbering(), crads.b.without_numbering()), it->first.tag));
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
		auxiliaries::IOUtilities().write_map_container_to_file(inter_atom_energy_descriptors, inter_atom_scores_file);
	}

	std::map< CRADsPair, EnergyDescriptor > inter_residue_energy_descriptors;
	{
		for(std::map< CRADsPair, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
		{
			const CRADsPair& crads=it->first;
			inter_residue_energy_descriptors[CRADsPair(crads.a.without_atom(), crads.b.without_atom())].add(it->second);
		}
		auxiliaries::IOUtilities().write_map_container_to_file(inter_residue_energy_descriptors, inter_residue_scores_file);
	}

	const std::map<CRAD, EnergyDescriptor> atom_energy_descriptors=auxiliaries::ChainResidueAtomDescriptorsGraphOperations::accumulate_mapped_values_by_graph_neighbors(inter_atom_energy_descriptors, depth);
	auxiliaries::IOUtilities().write_map_container_to_file(atom_energy_descriptors, atom_scores_file);

	const std::map<CRAD, EnergyDescriptor> residue_energy_descriptors=auxiliaries::ChainResidueAtomDescriptorsGraphOperations::accumulate_mapped_values_by_graph_neighbors(inter_residue_energy_descriptors, depth);
	auxiliaries::IOUtilities().write_map_container_to_file(residue_energy_descriptors, residue_scores_file);

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
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--default-mean", "number", "default mean parameter"));
		list_of_option_descriptions.push_back(OD("--default-sd", "number", "default standard deviation parameter"));
		list_of_option_descriptions.push_back(OD("--means-and-sds-file", "string", "file path to input atomic mean and sd parameters"));
		list_of_option_descriptions.push_back(OD("--mean-shift", "number", "mean shift in standard deviations"));
		list_of_option_descriptions.push_back(OD("--smoothing-window", "number", "window to smooth residue quality scores along sequence"));
		list_of_option_descriptions.push_back(OD("--atom-scores-file", "string", "file path to output atom scores"));
		list_of_option_descriptions.push_back(OD("--residue-scores-file", "string", "file path to output residue scores"));
		if(!poh.assert(list_of_option_descriptions, false))
		{
			std::cerr << "stdin   <-  list of atom energy descriptors\n";
			std::cerr << "stdout  ->  average local score\n";
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

	std::map<CRAD, EnergyDescriptor> atom_energy_descriptors;
	auxiliaries::IOUtilities().read_lines_to_map_container(std::cin, atom_energy_descriptors);
	if(atom_energy_descriptors.empty())
	{
		throw std::runtime_error("No input.");
	}

	std::map<CRAD, NormalDistributionParameters> means_and_sds;
	auxiliaries::IOUtilities().read_file_lines_to_map_container(mean_and_sds_file, means_and_sds);

	std::map<CRAD, double> atom_quality_scores;
	for(std::map<CRAD, EnergyDescriptor>::const_iterator it=atom_energy_descriptors.begin();it!=atom_energy_descriptors.end();++it)
	{
		const CRAD& crad=it->first;
		const EnergyDescriptor& ed=it->second;
		if(ed.total_area>0.0)
		{
			const double actuality_score=(1.0-(ed.strange_area/ed.total_area));
			const double normalized_energy=(ed.energy/ed.total_area);
			const bool detailed_mean_and_sd=(means_and_sds.count(crad.without_numbering())>0);
			const double mean=(detailed_mean_and_sd ? means_and_sds[crad.without_numbering()].mean : default_mean);
			const double sd=(detailed_mean_and_sd ? means_and_sds[crad.without_numbering()].sd : default_sd);
			const double adjusted_normalized_energy=((normalized_energy-mean)/sd);
			const double energy_score=(1.0-(0.5*(1.0+erf((adjusted_normalized_energy-mean_shift)/sqrt(2.0)))));
			atom_quality_scores[crad]=(energy_score*actuality_score);
		}
		else
		{
			atom_quality_scores[crad]=0.0;
		}
	}

	if(!atom_scores_file.empty())
	{
		std::ofstream foutput(atom_scores_file.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map<CRAD, double>::const_iterator it=atom_quality_scores.begin();it!=atom_quality_scores.end();++it)
			{
				foutput << it->first.str() << " " <<  it->second << "\n";
			}
		}
	}

	if(!residue_scores_file.empty())
	{
		std::ofstream foutput(residue_scores_file.c_str(), std::ios::out);
		if(foutput.good())
		{
			std::map<CRAD, std::pair<int, double> > residue_atom_summed_scores;
			for(std::map<CRAD, double>::const_iterator it=atom_quality_scores.begin();it!=atom_quality_scores.end();++it)
			{
				std::pair<int, double>& residue_value=residue_atom_summed_scores[it->first.without_atom()];
				residue_value.first++;
				residue_value.second+=it->second;
			}
			std::map<CRAD, double> residue_atomic_scores;
			for(std::map<CRAD, std::pair<int, double> >::const_iterator it=residue_atom_summed_scores.begin();it!=residue_atom_summed_scores.end();++it)
			{
				const std::pair<int, double>& residue_value=it->second;
				residue_atomic_scores[it->first]=((residue_value.first>0) ? (residue_value.second/static_cast<double>(residue_value.first)) : 0.0);
			}
			if(smoothing_window>0)
			{
				residue_atomic_scores=smooth_residue_scores_along_sequence(residue_atomic_scores, smoothing_window);
			}
			for(std::map<CRAD, double>::const_iterator it=residue_atomic_scores.begin();it!=residue_atomic_scores.end();++it)
			{
				foutput << it->first.str() << " " << it->second << "\n";
			}
		}
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
