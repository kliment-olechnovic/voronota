#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cmath>

#include "modescommon/assert_options.h"
#include "modescommon/handle_contact.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

template<bool Additive>
inline bool add_contact_value_from_stream_to_map_of_contacts_values(std::istream& input, std::map< std::pair<CRAD, CRAD>, double >& map_of_values)
{
	std::pair<std::string, std::string> name_strings;
	double value;
	input >> name_strings.first >> name_strings.second >> value;
	if(!input.fail() && !name_strings.first.empty() && !name_strings.second.empty())
	{
		std::pair<CRAD, CRAD> names(CRAD::from_str(name_strings.first), CRAD::from_str(name_strings.second));
		if(Additive)
		{
			names.first=names.first.without_numbering();
			names.second=names.second.without_numbering();
		}
		if(names.first.valid() && names.second.valid())
		{
			double& left_value=map_of_values[modescommon::refine_pair_by_ordering(names)];
			left_value=(Additive ? (left_value+value) : value);
			return true;
		}
	}
	return false;
}

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

inline double calculate_score_from_energy_descriptor(const EnergyDescriptor& ed, const double erf_mean, const double erf_sd)
{
	static const double square_root_of_two=sqrt(2.0);
	if(ed.total_area>0.0)
	{
		const double normalized_energy=ed.energy/ed.total_area;
		const double completeness_score=1.0-(ed.strange_area/ed.total_area);
		const double energy_quality_score=1.0-(0.5*(1.0+erf((normalized_energy-erf_mean)/(square_root_of_two*erf_sd))));
		return (energy_quality_score*completeness_score);
	}
	return 0.0;
}

}

void score_contacts_potential(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 area')\n";
			std::cerr << "stdout  ->  list of potential values (line format: 'annotation1 annotation2 value')\n";
			return;
		}
	}

	std::map< std::pair<CRAD, CRAD>, double > map_of_total_areas;
	auxiliaries::read_lines_to_container(std::cin, add_contact_value_from_stream_to_map_of_contacts_values<true>, map_of_total_areas);
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
		const double ax=map_of_generalized_total_areas[crads.first];
		const double bx=map_of_generalized_total_areas[crads.second];
		if(ab>0.0 && ax>0.0 && bx>0.0)
		{
			const double potential_value=(ab*sum_of_all_areas)/(ax*bx);
			std::cout << crads.first.str() << " " << crads.second.str() << potential_value << "\n";
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
		list_of_option_descriptions.push_back(OD("--residue-scores-file", "string", "file path to output residue scores"));
		list_of_option_descriptions.push_back(OD("--depth", "number", "neighborhood normalization depth"));
		list_of_option_descriptions.push_back(OD("--erf-mean", "number", "mean parameter for error function"));
		list_of_option_descriptions.push_back(OD("--erf-sd", "number", "sd parameter for error function"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 area')\n";
			std::cerr << "stdout  ->  line of global scores (line format: straight_global average_local)\n";
			return;
		}
	}

	const std::string potential_file=poh.argument<std::string>("--potential-file");
	const std::string inter_atom_scores_file=poh.argument<std::string>("--inter-atom-scores-file", "");
	const std::string inter_residue_scores_file=poh.argument<std::string>("--inter-residue-scores-file", "");
	const std::string residue_scores_file=poh.argument<std::string>("--residue-scores-file", "");
	const int depth=poh.argument<int>("--depth", 1);
	const double erf_mean=poh.argument<double>("--erf-mean", 0.3);
	const double erf_sd=poh.argument<double>("--erf-sd", 0.2);

	std::map< std::pair<CRAD, CRAD>, double > map_of_contacts;
	{
		auxiliaries::read_lines_to_container(std::cin, add_contact_value_from_stream_to_map_of_contacts_values<false>, map_of_contacts);
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
				auxiliaries::read_lines_to_container(finput, add_contact_value_from_stream_to_map_of_contacts_values<false>, map_of_potential_values);
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
	}

	std::map< std::pair<CRAD, CRAD>, EnergyDescriptor > inter_residue_energy_descriptors;
	{
		for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			inter_residue_energy_descriptors[modescommon::refine_pair_by_ordering(std::make_pair(crads.first.without_atom(), crads.second.without_atom()))].add(it->second);
		}
	}

	std::map<CRAD, EnergyDescriptor> residue_energy_descriptors;
	{
		std::map< CRAD, std::set<CRAD> > residue_graph;
		for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_residue_energy_descriptors.begin();it!=inter_residue_energy_descriptors.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			if(!(crads.first==crads.second || crads.first==CRAD::solvent() || crads.second==CRAD::solvent()))
			{
				residue_graph[crads.first].insert(crads.second);
				residue_graph[crads.second].insert(crads.first);
			}
		}
		for(int i=0;i<depth;i++)
		{
			std::map< CRAD, std::set<CRAD> > expanded_residue_graph=residue_graph;
			for(std::map< CRAD, std::set<CRAD> >::const_iterator residue_graph_it=residue_graph.begin();residue_graph_it!=residue_graph.end();++residue_graph_it)
			{
				const CRAD& center=residue_graph_it->first;
				const std::set<CRAD>& neighbors=residue_graph_it->second;
				std::set<CRAD>& expandable_neighbors=expanded_residue_graph[center];
				for(std::set<CRAD>::const_iterator neighbors_it=neighbors.begin();neighbors_it!=neighbors.end();neighbors_it++)
				{
					const std::set<CRAD>& neighbor_neighbors=residue_graph[*neighbors_it];
					expandable_neighbors.insert(neighbor_neighbors.begin(), neighbor_neighbors.end());
				}
				expandable_neighbors.erase(center);
			}
			residue_graph=expanded_residue_graph;
		}
		for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_residue_energy_descriptors.begin();it!=inter_residue_energy_descriptors.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			const std::set<CRAD>& related_residues1=residue_graph[crads.first];
			const std::set<CRAD>& related_residues2=residue_graph[crads.second];
			std::set<CRAD> related_residues=related_residues1;
			related_residues.insert(related_residues2.begin(), related_residues2.end());
			for(std::set<CRAD>::const_iterator jt=related_residues.begin();jt!=related_residues.end();++jt)
			{
				residue_energy_descriptors[*jt].add(it->second);
			}
		}
	}

	if(!inter_atom_scores_file.empty())
	{
		std::ofstream foutput(inter_atom_scores_file.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
			{
				foutput << it->first.first.str() << " " << it->first.second.str() << " " << calculate_score_from_energy_descriptor(it->second, erf_mean, erf_sd) << "\n";
			}
		}
	}

	if(!inter_residue_scores_file.empty())
	{
		std::ofstream foutput(inter_residue_scores_file.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_residue_energy_descriptors.begin();it!=inter_residue_energy_descriptors.end();++it)
			{
				foutput << it->first.first.str() << " " << it->first.second.str() << " " << calculate_score_from_energy_descriptor(it->second, erf_mean, erf_sd) << "\n";
			}
		}
	}

	if(!residue_scores_file.empty())
	{
		std::ofstream foutput(residue_scores_file.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map<CRAD, EnergyDescriptor>::const_iterator it=residue_energy_descriptors.begin();it!=residue_energy_descriptors.end();++it)
			{
				foutput << it->first.str() << " " << calculate_score_from_energy_descriptor(it->second, erf_mean, erf_sd) << "\n";
			}
		}
	}
}
