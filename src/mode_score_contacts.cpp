#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cmath>

#include "modescommon/assert_options.h"
#include "modescommon/handle_annotations.h"
#include "modescommon/handle_mappings.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

struct EnergyDescriptor
{
	double total_area;
	double strange_area;
	double energy;
	double contacts_count;

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

struct EnergyScore
{
	double normalized_energy;
	double energy_score;
	double actuality_score;
	double quality_score;

	EnergyScore() : normalized_energy(0.0), energy_score(0.0), actuality_score(0.0), quality_score(0.0)
	{
	}
};

struct EnergyScoreCalculationParameter
{
	double contact_mean_area;
	double erf_mean;
	double erf_sd;

	EnergyScoreCalculationParameter(const double contact_mean_area, const double erf_mean, const double erf_sd) : contact_mean_area(contact_mean_area), erf_mean(erf_mean), erf_sd(erf_sd)
	{
	}
};

inline EnergyScore calculate_energy_score_from_energy_descriptor(const EnergyDescriptor& ed, const EnergyScoreCalculationParameter& escp)
{
	static const double square_root_of_two=sqrt(2.0);
	EnergyScore es;
	if(ed.total_area>0.0)
	{
		es.normalized_energy=ed.energy/(ed.contacts_count*escp.contact_mean_area);
		es.energy_score=1.0-(0.5*(1.0+erf((es.normalized_energy-escp.erf_mean)/(square_root_of_two*escp.erf_sd))));
		es.actuality_score=1.0-(ed.strange_area/ed.total_area);
		es.quality_score=(es.energy_score*es.actuality_score);
	}
	return es;
}

inline void print_score(const std::string& name, const EnergyDescriptor& ed, const EnergyScoreCalculationParameter& escp, std::ostream& output)
{
	const EnergyScore es=calculate_energy_score_from_energy_descriptor(ed, escp);
	output << name << " ";
	output << es.quality_score << " " << es.normalized_energy << " " << es.energy_score << " " << es.actuality_score << " ";
	output << ed.total_area << " " << ed.strange_area << " " << ed.energy << " " << ed.contacts_count << "\n";
}

void print_pair_scores_to_file(const std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >& map_of_pair_energy_descriptors, const EnergyScoreCalculationParameter& escp, const std::string& filename)
{
	if(!filename.empty())
	{
		std::ofstream foutput(filename.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=map_of_pair_energy_descriptors.begin();it!=map_of_pair_energy_descriptors.end();++it)
			{
				print_score(it->first.first.str()+" "+it->first.second.str(), it->second, escp, foutput);
			}
		}
	}
}

void print_single_scores_to_file(const std::map<CRAD, EnergyDescriptor>& map_of_single_energy_descriptors, const EnergyScoreCalculationParameter& escp, const std::string& filename)
{
	if(!filename.empty())
	{
		std::ofstream foutput(filename.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map<CRAD, EnergyDescriptor>::const_iterator it=map_of_single_energy_descriptors.begin();it!=map_of_single_energy_descriptors.end();++it)
			{
				print_score(it->first.str(), it->second, escp, foutput);
			}
		}
	}
}

void print_single_scores_summary(const std::string& name, const std::map<CRAD, EnergyDescriptor>& map_of_single_energy_descriptors, const EnergyScoreCalculationParameter& escp, std::ostream& output)
{
	EnergyScore es_sum;
	for(std::map<CRAD, EnergyDescriptor>::const_iterator it=map_of_single_energy_descriptors.begin();it!=map_of_single_energy_descriptors.end();++it)
	{
		const EnergyDescriptor& ed=it->second;
		const EnergyScore es=calculate_energy_score_from_energy_descriptor(ed, escp);
		es_sum.quality_score+=es.quality_score;
		es_sum.normalized_energy+=es.normalized_energy;
		es_sum.energy_score+=es.energy_score;
		es_sum.actuality_score+=es.actuality_score;
	}
	output << name << " " << map_of_single_energy_descriptors.size() << " ";
	output << es_sum.quality_score << " " << es_sum.normalized_energy << " " << es_sum.energy_score << " " << es_sum.actuality_score << "\n";
}

}

void score_contacts_potential(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--potential-file", "string", "file path to output potential values", true));
		list_of_option_descriptions.push_back(OD("--defaulting-max-seq-sep", "number", "maximum residue sequence separation for defaulting contacts"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 area')\n";
			return;
		}
	}

	const std::string potential_file=poh.argument<std::string>("--potential-file");
	const int defaulting_max_seq_sep=poh.argument<int>("--defaulting-max-seq-sep", 1);

	std::map< std::pair<CRAD, CRAD>, double > map_of_considered_total_areas;
	std::map<CRAD, double> map_of_generalized_total_areas;
	double sum_of_all_areas=0.0;

	while(std::cin.good())
	{
		std::string line;
		std::getline(std::cin, line);
		if(!line.empty())
		{
			std::istringstream input(line);
			if(input.good())
			{
				std::pair<std::string, std::string> crads_strings;
				double area=0.0;
				input >> crads_strings.first >> crads_strings.second >> area;
				if(!input.fail() && !crads_strings.first.empty() && !crads_strings.second.empty())
				{
					const std::pair<CRAD, CRAD> crads(CRAD::from_str(crads_strings.first), CRAD::from_str(crads_strings.second));
					if(crads.first.valid() && crads.second.valid())
					{
						const std::pair<CRAD, CRAD> crads_without_numbering=modescommon::refine_pair_by_ordering(std::make_pair(crads.first.without_numbering(), crads.second.without_numbering()));
						if(!CRAD::match_with_sequence_separation_interval(crads.first, crads.second, 0, defaulting_max_seq_sep, false))
						{
							map_of_considered_total_areas[crads_without_numbering]+=area;
						}
						map_of_generalized_total_areas[crads_without_numbering.first]+=area;
						map_of_generalized_total_areas[crads_without_numbering.second]+=area;
						sum_of_all_areas+=area;
					}
				}
			}
		}
	}

	std::map< std::pair<CRAD, CRAD>, std::pair<double, double> > result;
	for(std::map< std::pair<CRAD, CRAD>, double >::const_iterator it=map_of_considered_total_areas.begin();it!=map_of_considered_total_areas.end();++it)
	{
		const std::pair<CRAD, CRAD>& crads=it->first;
		const double ab=it->second;
		const double ax=map_of_generalized_total_areas[crads.first];
		const double bx=map_of_generalized_total_areas[crads.second];
		if(ab>0.0 && ax>0.0 && bx>0.0)
		{
			const double potential_value=(0.0-log((ab*sum_of_all_areas)/(ax*bx)));
			result[crads]=std::make_pair(potential_value, ab);
		}
	}

	std::ofstream foutput(potential_file.c_str(), std::ios::out);
	if(foutput.good())
	{
		for(std::map< std::pair<CRAD, CRAD>, std::pair<double, double> >::const_iterator it=result.begin();it!=result.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			foutput << crads.first.str() << " " << crads.second.str() << " " << it->second.first << "\n";
		}
	}
}

void score_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--potential-file", "string", "file path to input potential values", true));
		list_of_option_descriptions.push_back(OD("--defaulting-max-seq-sep", "number", "maximum residue sequence separation for defaulting contacts"));
		list_of_option_descriptions.push_back(OD("--defaulting-potential-value", "number", "potential value to use for defaulting contacts"));
		list_of_option_descriptions.push_back(OD("--inter-atom-scores-file", "string", "file path to output inter-atom scores"));
		list_of_option_descriptions.push_back(OD("--inter-residue-scores-file", "string", "file path to output inter-residue scores"));
		list_of_option_descriptions.push_back(OD("--atom-scores-file", "string", "file path to output atom scores"));
		list_of_option_descriptions.push_back(OD("--residue-scores-file", "string", "file path to output residue scores"));
		list_of_option_descriptions.push_back(OD("--depth", "number", "neighborhood normalization depth"));
		list_of_option_descriptions.push_back(OD("--contact-mean-area", "number", "average area per contact to use for normalization"));
		list_of_option_descriptions.push_back(OD("--erf-mean", "number", "mean parameter for error function"));
		list_of_option_descriptions.push_back(OD("--erf-sd", "number", "sd parameter for error function"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 area')\n";
			std::cerr << "stdout  ->  line of global scores\n";
			return;
		}
	}

	const std::string potential_file=poh.argument<std::string>("--potential-file");
	const int defaulting_max_seq_sep=poh.argument<int>("--defaulting-max-seq-sep", 1);
	const double defaulting_potential_value=poh.argument<int>("--defaulting-potential-value", 0.0);
	const std::string inter_atom_scores_file=poh.argument<std::string>("--inter-atom-scores-file", "");
	const std::string inter_residue_scores_file=poh.argument<std::string>("--inter-residue-scores-file", "");
	const std::string atom_scores_file=poh.argument<std::string>("--atom-scores-file", "");
	const std::string residue_scores_file=poh.argument<std::string>("--residue-scores-file", "");
	const int depth=poh.argument<int>("--depth", 1);
	const double contact_mean_area=poh.argument<double>("--contact-mean-area", 3.8);
	const double erf_mean=poh.argument<double>("--erf-mean", 0.3);
	const double erf_sd=poh.argument<double>("--erf-sd", 0.2);

	const EnergyScoreCalculationParameter escp(contact_mean_area, erf_mean, erf_sd);

	std::map< std::pair<CRAD, CRAD>, double > map_of_contacts;
	{
		auxiliaries::read_lines_to_container(std::cin, modescommon::add_chain_residue_atom_descriptors_pair_value_from_stream_to_map, map_of_contacts);
		if(map_of_contacts.empty())
		{
			throw std::runtime_error("No contacts input.");
		}
	}

	std::map< std::pair<CRAD, CRAD>, double > map_of_potential_values;
	{
		if(!potential_file.empty())
		{
			std::ifstream finput(potential_file.c_str(), std::ios::in);
			if(finput.good())
			{
				auxiliaries::read_lines_to_container(finput, modescommon::add_chain_residue_atom_descriptors_pair_value_from_stream_to_map, map_of_potential_values);
			}
		}
		if(map_of_potential_values.empty())
		{
			throw std::runtime_error("No potential values input.");
		}
	}

	std::map< std::pair<CRAD, CRAD>, EnergyDescriptor > inter_atom_energy_descriptors;
	{
		for(std::map< std::pair<CRAD, CRAD>, double >::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			EnergyDescriptor& ed=inter_atom_energy_descriptors[crads];
			ed.total_area=it->second;
			ed.contacts_count=1.0;
			if(CRAD::match_with_sequence_separation_interval(crads.first, crads.second, 0, defaulting_max_seq_sep, false))
			{
				ed.energy=ed.total_area*defaulting_potential_value;
			}
			else
			{
				std::map< std::pair<CRAD, CRAD>, double >::const_iterator potential_value_it=
						map_of_potential_values.find(modescommon::refine_pair_by_ordering(std::make_pair(crads.first.without_numbering(), crads.second.without_numbering())));
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
		print_pair_scores_to_file(inter_atom_energy_descriptors, escp, inter_atom_scores_file);
	}

	std::map< std::pair<CRAD, CRAD>, EnergyDescriptor > inter_residue_energy_descriptors;
	{
		for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			inter_residue_energy_descriptors[modescommon::refine_pair_by_ordering(std::make_pair(crads.first.without_atom(), crads.second.without_atom()))].add(it->second);
		}
		print_pair_scores_to_file(inter_residue_energy_descriptors, escp, inter_residue_scores_file);
	}

	const std::map<CRAD, EnergyDescriptor> atom_energy_descriptors=modescommon::construct_single_mapping_of_descriptors_from_pair_mapping_of_descriptors(inter_atom_energy_descriptors, depth);
	print_single_scores_to_file(atom_energy_descriptors, escp, atom_scores_file);

	const std::map<CRAD, EnergyDescriptor> residue_energy_descriptors=modescommon::construct_single_mapping_of_descriptors_from_pair_mapping_of_descriptors(inter_residue_energy_descriptors, depth);
	print_single_scores_to_file(residue_energy_descriptors, escp, residue_scores_file);

	{
		EnergyDescriptor global_ed;
		for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
		{
			global_ed.add(it->second);
		}
		print_score("global", global_ed, escp, std::cout);
	}

	print_single_scores_summary("atom_level_summary", atom_energy_descriptors, escp, std::cout);
	print_single_scores_summary("residue_level_summary", residue_energy_descriptors, escp, std::cout);
}
