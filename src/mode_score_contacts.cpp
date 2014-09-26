#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cmath>

#include "modescommon/assert_options.h"
#include "modescommon/handle_annotations.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

struct EnergyDescriptor
{
	double total_area;
	double strange_area;
	double energy;

	EnergyDescriptor() : total_area(0), strange_area(0), energy(0)
	{
	}

	void add(const EnergyDescriptor& ed)
	{
		total_area+=ed.total_area;
		strange_area+=ed.strange_area;
		energy+=ed.energy;
	}
};

std::map<CRAD, EnergyDescriptor> construct_single_energy_descriptors_from_pair_energy_descriptors(const std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >& map_of_pair_energy_descrptors, const int depth)
{
	std::map< CRAD, std::set<CRAD> > graph;
	for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=map_of_pair_energy_descrptors.begin();it!=map_of_pair_energy_descrptors.end();++it)
	{
		const std::pair<CRAD, CRAD>& crads=it->first;
		if(!(crads.first==crads.second || crads.first==CRAD::solvent() || crads.second==CRAD::solvent()))
		{
			graph[crads.first].insert(crads.second);
			graph[crads.second].insert(crads.first);
		}
	}
	for(int i=0;i<depth;i++)
	{
		std::map< CRAD, std::set<CRAD> > expanded_graph=graph;
		for(std::map< CRAD, std::set<CRAD> >::const_iterator graph_it=graph.begin();graph_it!=graph.end();++graph_it)
		{
			const CRAD& center=graph_it->first;
			const std::set<CRAD>& neighbors=graph_it->second;
			std::set<CRAD>& expandable_neighbors=expanded_graph[center];
			for(std::set<CRAD>::const_iterator neighbors_it=neighbors.begin();neighbors_it!=neighbors.end();neighbors_it++)
			{
				const std::set<CRAD>& neighbor_neighbors=graph[*neighbors_it];
				expandable_neighbors.insert(neighbor_neighbors.begin(), neighbor_neighbors.end());
			}
			expandable_neighbors.erase(center);
		}
		graph=expanded_graph;
	}
	std::map<CRAD, EnergyDescriptor> map_of_single_energy_descriptors;
	for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=map_of_pair_energy_descrptors.begin();it!=map_of_pair_energy_descrptors.end();++it)
	{
		const std::pair<CRAD, CRAD>& crads=it->first;
		const std::set<CRAD>& related_crads1=graph[crads.first];
		const std::set<CRAD>& related_crads2=graph[crads.second];
		std::set<CRAD> related_crads=related_crads1;
		related_crads.insert(related_crads2.begin(), related_crads2.end());
		for(std::set<CRAD>::const_iterator jt=related_crads.begin();jt!=related_crads.end();++jt)
		{
			map_of_single_energy_descriptors[*jt].add(it->second);
		}
	}
	return map_of_single_energy_descriptors;
}

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

inline EnergyScore calculate_energy_score_from_energy_descriptor(const EnergyDescriptor& ed, const double erf_mean, const double erf_sd)
{
	static const double square_root_of_two=sqrt(2.0);
	EnergyScore es;
	if(ed.total_area>0.0)
	{
		es.normalized_energy=ed.energy/ed.total_area;
		es.energy_score=1.0-(0.5*(1.0+erf((es.normalized_energy-erf_mean)/(square_root_of_two*erf_sd))));
		es.actuality_score=1.0-(ed.strange_area/ed.total_area);
		es.quality_score=(es.energy_score*es.actuality_score);
	}
	return es;
}

inline void print_score(const std::string& name, const EnergyDescriptor& ed, const double erf_mean, const double erf_sd, std::ostream& output)
{
	const EnergyScore es=calculate_energy_score_from_energy_descriptor(ed, erf_mean, erf_sd);
	output << name << " ";
	output << es.quality_score << " " << es.normalized_energy << " " << es.energy_score << " " << es.actuality_score << " ";
	output << ed.total_area << " " << ed.strange_area << " " << ed.energy << "\n";
}

void print_pair_scores_to_file(const std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >& map_of_pair_energy_descriptors, const double erf_mean, const double erf_sd, const std::string& filename)
{
	if(!filename.empty())
	{
		std::ofstream foutput(filename.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=map_of_pair_energy_descriptors.begin();it!=map_of_pair_energy_descriptors.end();++it)
			{
				print_score(it->first.first.str()+" "+it->first.second.str(), it->second, erf_mean, erf_sd, foutput);
			}
		}
	}
}

void print_single_scores_to_file(const std::map<CRAD, EnergyDescriptor>& map_of_single_energy_descriptors, const double erf_mean, const double erf_sd, const std::string& filename)
{
	if(!filename.empty())
	{
		std::ofstream foutput(filename.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map<CRAD, EnergyDescriptor>::const_iterator it=map_of_single_energy_descriptors.begin();it!=map_of_single_energy_descriptors.end();++it)
			{
				print_score(it->first.str(), it->second, erf_mean, erf_sd, foutput);
			}
		}
	}
}

}

void score_contacts_potential(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--output-summed-areas", "", "flag to output summed areas instead of potential values"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 area')\n";
			std::cerr << "stdout  ->  list of potential values (line format: 'annotation1 annotation2 value')\n";
			return;
		}
	}

	const bool output_summed_areas=poh.contains_option("--output-summed-areas");

	std::map< std::pair<CRAD, CRAD>, double > map_of_total_areas;
	auxiliaries::read_lines_to_container(std::cin, modescommon::add_chain_residue_atom_descriptors_pair_value_from_stream_to_map<true>, map_of_total_areas);
	if(map_of_total_areas.empty())
	{
		throw std::runtime_error("No contacts input.");
	}

	std::map<CRAD, double> map_of_generalized_total_areas;
	double sum_of_all_areas=0.0;
	for(std::map< std::pair<CRAD, CRAD>, double >::iterator it=map_of_total_areas.begin();it!=map_of_total_areas.end();++it)
	{
		const std::pair<CRAD, CRAD>& crads=it->first;
		map_of_generalized_total_areas[crads.first]+=(it->second);
		if(!(crads.first==crads.second))
		{
			map_of_generalized_total_areas[crads.second]+=(it->second);
		}
		sum_of_all_areas+=(it->second);
	}

	for(std::map< std::pair<CRAD, CRAD>, double >::iterator it=map_of_total_areas.begin();it!=map_of_total_areas.end();++it)
	{
		const std::pair<CRAD, CRAD>& crads=it->first;
		const double ab=it->second;
		if(output_summed_areas)
		{
			std::cout << crads.first.str() << " " << crads.second.str() << " " << ab << "\n";
		}
		else
		{
			const double ax=map_of_generalized_total_areas[crads.first];
			const double bx=map_of_generalized_total_areas[crads.second];
			if(ab>0.0 && ax>0.0 && bx>0.0)
			{
				const double potential_value=(0.0-log((ab*sum_of_all_areas)/(ax*bx)));
				std::cout << crads.first.str() << " " << crads.second.str() << " " << potential_value << "\n";
			}
		}
	}
}

void score_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--potential-file", "string", "file path to input potential values", true));
		list_of_option_descriptions.push_back(OD("--inter-atom-scores-file", "string", "file path to output inter-atom scores"));
		list_of_option_descriptions.push_back(OD("--inter-residue-scores-file", "string", "file path to output inter-residue scores"));
		list_of_option_descriptions.push_back(OD("--atom-scores-file", "string", "file path to output atom scores"));
		list_of_option_descriptions.push_back(OD("--residue-scores-file", "string", "file path to output residue scores"));
		list_of_option_descriptions.push_back(OD("--depth", "number", "neighborhood normalization depth"));
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
	const std::string inter_atom_scores_file=poh.argument<std::string>("--inter-atom-scores-file", "");
	const std::string inter_residue_scores_file=poh.argument<std::string>("--inter-residue-scores-file", "");
	const std::string atom_scores_file=poh.argument<std::string>("--atom-scores-file", "");
	const std::string residue_scores_file=poh.argument<std::string>("--residue-scores-file", "");
	const int depth=poh.argument<int>("--depth", 1);
	const double erf_mean=poh.argument<double>("--erf-mean", 0.3);
	const double erf_sd=poh.argument<double>("--erf-sd", 0.2);

	std::map< std::pair<CRAD, CRAD>, double > map_of_contacts;
	{
		auxiliaries::read_lines_to_container(std::cin, modescommon::add_chain_residue_atom_descriptors_pair_value_from_stream_to_map<false>, map_of_contacts);
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
				auxiliaries::read_lines_to_container(finput, modescommon::add_chain_residue_atom_descriptors_pair_value_from_stream_to_map<false>, map_of_potential_values);
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
		print_pair_scores_to_file(inter_atom_energy_descriptors, erf_mean, erf_sd, inter_atom_scores_file);
	}

	std::map< std::pair<CRAD, CRAD>, EnergyDescriptor > inter_residue_energy_descriptors;
	{
		for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			inter_residue_energy_descriptors[modescommon::refine_pair_by_ordering(std::make_pair(crads.first.without_atom(), crads.second.without_atom()))].add(it->second);
		}
		print_pair_scores_to_file(inter_residue_energy_descriptors, erf_mean, erf_sd, inter_residue_scores_file);
	}

	if(!atom_scores_file.empty())
	{
		print_single_scores_to_file(construct_single_energy_descriptors_from_pair_energy_descriptors(inter_atom_energy_descriptors, depth), erf_mean, erf_sd, atom_scores_file);
	}

	if(!residue_scores_file.empty())
	{
		print_single_scores_to_file(construct_single_energy_descriptors_from_pair_energy_descriptors(inter_residue_energy_descriptors, depth), erf_mean, erf_sd, residue_scores_file);
	}

	{
		EnergyDescriptor global_ed;
		for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
		{
			global_ed.add(it->second);
		}
		print_score("global", global_ed, erf_mean, erf_sd, std::cout);
	}
}
