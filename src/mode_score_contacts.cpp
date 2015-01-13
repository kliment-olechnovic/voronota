#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cmath>

#include "modescommon/assert_options.h"
#include "modescommon/handle_annotations.h"
#include "modescommon/handle_mappings.h"
#include "modescommon/handle_sequences.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

typedef std::pair<std::pair<CRAD, CRAD>, std::string> Interaction;

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

void read_map_of_interactions_areas(std::istream& input, const bool accumulating, std::map<Interaction, double>& map_of_interactions_areas)
{
	while(input.good())
	{
		std::string line;
		std::getline(input, line);
		if(!line.empty())
		{
			std::istringstream line_input(line);
			if(line_input.good())
			{
				std::pair<std::string, std::string> crads_strings;
				std::string conditions;
				double area=0.0;
				line_input >> crads_strings.first >> crads_strings.second >> conditions >> area;
				if(!line_input.fail() && !crads_strings.first.empty() && !crads_strings.second.empty() && !conditions.empty())
				{
					const std::pair<CRAD, CRAD> crads(CRAD::from_str(crads_strings.first), CRAD::from_str(crads_strings.second));
					if(crads.first.valid() && crads.second.valid())
					{
						if(accumulating)
						{
							const std::pair<CRAD, CRAD> crads_without_numbering=modescommon::refine_pair_by_ordering(std::make_pair(crads.first.without_numbering(), crads.second.without_numbering()));
							map_of_interactions_areas[Interaction(crads_without_numbering, conditions)]+=area;
						}
						else
						{
							map_of_interactions_areas[Interaction(crads, conditions)]=area;
						}
					}
				}
			}
		}
	}
}

inline void print_energy_descriptor(const std::string& name, const EnergyDescriptor& ed, std::ostream& output)
{
	output << name << " " << ed.total_area << " " << ed.strange_area << " " << ed.energy << " " << ed.contacts_count << "\n";
}

void print_pair_energy_descriptors_to_file(const std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >& map_of_pair_energy_descriptors, const std::string& filename)
{
	if(!filename.empty())
	{
		std::ofstream foutput(filename.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=map_of_pair_energy_descriptors.begin();it!=map_of_pair_energy_descriptors.end();++it)
			{
				print_energy_descriptor(it->first.first.str()+" "+it->first.second.str(), it->second, foutput);
			}
		}
	}
}

void print_single_energy_descriptors_to_file(const std::map<CRAD, EnergyDescriptor>& map_of_single_energy_descriptors, const std::string& filename)
{
	if(!filename.empty())
	{
		std::ofstream foutput(filename.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map<CRAD, EnergyDescriptor>::const_iterator it=map_of_single_energy_descriptors.begin();it!=map_of_single_energy_descriptors.end();++it)
			{
				print_energy_descriptor(it->first.str(), it->second, foutput);
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
		list_of_option_descriptions.push_back(OD("--input-file-list", "", "flag to read file list from stdin"));
		list_of_option_descriptions.push_back(OD("--potential-file", "string", "file path to output potential values"));
		list_of_option_descriptions.push_back(OD("--solvent-factor", "number", "solvent factor value"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
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
			if(!file_path.empty())
			{
				std::ifstream finput(file_path.c_str(), std::ios::in);
				if(finput.good())
				{
					read_map_of_interactions_areas(finput, true, map_of_interactions_total_areas);
				}
			}
		}
	}
	else
	{
		read_map_of_interactions_areas(std::cin, true, map_of_interactions_total_areas);
	}

	for(std::map<Interaction, double>::const_iterator it=map_of_interactions_total_areas.begin();it!=map_of_interactions_total_areas.end();++it)
	{
		const Interaction& interaction=it->first;
		const double area=it->second;
		map_of_crads_total_areas[interaction.first.first]+=area;
		map_of_crads_total_areas[interaction.first.second]+=area;
		map_of_conditions_total_areas[interaction.second]+=area;
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
		const double ax=map_of_crads_total_areas[interaction.first.first];
		const double bx=map_of_crads_total_areas[interaction.first.second];
		const double cx=map_of_conditions_total_areas[interaction.second];
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
				foutput << interaction.first.first.str() << " " << interaction.first.second.str() << " " << interaction.second << " "<< it->second.first << "\n";
			}
		}
	}

	for(std::map< Interaction, std::pair<double, double> >::const_iterator it=result.begin();it!=result.end();++it)
	{
		const Interaction& interaction=it->first;
		std::cout << interaction.first.first.str() << " " << interaction.first.second.str() << " " << interaction.second << " "<< it->second.second << "\n";
	}
}

void score_contacts(const auxiliaries::ProgramOptionsHandler& poh)
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
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
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
	const int depth=poh.argument<int>("--depth", 1);

	std::map<Interaction, double> map_of_contacts;
	{
		read_map_of_interactions_areas(std::cin, false, map_of_contacts);
		if(map_of_contacts.empty())
		{
			throw std::runtime_error("No contacts input.");
		}
	}

	std::map<Interaction, double> map_of_potential_values;
	{
		if(!potential_file.empty())
		{
			std::ifstream finput(potential_file.c_str(), std::ios::in);
			if(finput.good())
			{
				read_map_of_interactions_areas(finput, false, map_of_potential_values);
			}
		}
		if(map_of_potential_values.empty())
		{
			throw std::runtime_error("No potential values input.");
		}
	}

	std::map<std::pair<CRAD, CRAD>, EnergyDescriptor> inter_atom_energy_descriptors;
	{
		for(std::map<Interaction, double>::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first.first;
			EnergyDescriptor& ed=inter_atom_energy_descriptors[crads];
			if(!CRAD::match_with_sequence_separation_interval(crads.first, crads.second, 0, ignorable_max_seq_sep, false))
			{
				ed.total_area=it->second;
				ed.contacts_count=1;
				std::map<Interaction, double>::const_iterator potential_value_it=
						map_of_potential_values.find(Interaction(modescommon::refine_pair_by_ordering(std::make_pair(crads.first.without_numbering(), crads.second.without_numbering())), it->first.second));
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
		print_pair_energy_descriptors_to_file(inter_atom_energy_descriptors, inter_atom_scores_file);
	}

	std::map< std::pair<CRAD, CRAD>, EnergyDescriptor > inter_residue_energy_descriptors;
	{
		for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			inter_residue_energy_descriptors[modescommon::refine_pair_by_ordering(std::make_pair(crads.first.without_atom(), crads.second.without_atom()))].add(it->second);
		}
		print_pair_energy_descriptors_to_file(inter_residue_energy_descriptors, inter_residue_scores_file);
	}

	const std::map< CRAD, std::set<CRAD> > atom_graph=modescommon::construct_graph_from_pair_mapping_of_descriptors(inter_atom_energy_descriptors, depth);
	const std::map<CRAD, EnergyDescriptor> atom_energy_descriptors=modescommon::construct_single_mapping_of_descriptors_from_pair_mapping_of_descriptors(inter_atom_energy_descriptors, atom_graph);
	print_single_energy_descriptors_to_file(atom_energy_descriptors, atom_scores_file);

	const std::map< CRAD, std::set<CRAD> > residue_graph=modescommon::construct_graph_from_pair_mapping_of_descriptors(inter_residue_energy_descriptors, depth);
	const std::map<CRAD, EnergyDescriptor> residue_energy_descriptors=modescommon::construct_single_mapping_of_descriptors_from_pair_mapping_of_descriptors(inter_residue_energy_descriptors, residue_graph);
	print_single_energy_descriptors_to_file(residue_energy_descriptors, residue_scores_file);

	{
		EnergyDescriptor global_ed;
		for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
		{
			global_ed.add(it->second);
		}
		print_energy_descriptor("global", global_ed, std::cout);
	}
}
