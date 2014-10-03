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

struct CADDescriptor
{
	double target_area_sum;
	double model_area_sum;
	double raw_differences_sum;
	double constrained_differences_sum;

	CADDescriptor() : target_area_sum(0), model_area_sum(0), raw_differences_sum(), constrained_differences_sum(0)
	{
	}

	void add(const double target_area, const double model_area)
	{
		target_area_sum+=target_area;
		model_area_sum+=model_area;
		raw_differences_sum+=fabs(target_area-model_area);
		constrained_differences_sum+=std::min(fabs(target_area-model_area), target_area);
	}

	void add(const CADDescriptor& cadd)
	{
		target_area_sum+=cadd.target_area_sum;
		model_area_sum+=cadd.target_area_sum;
		raw_differences_sum+=cadd.raw_differences_sum;
		constrained_differences_sum+=cadd.constrained_differences_sum;
	}

	bool scorable() const
	{
		return (target_area_sum>0.0);
	}

	double score() const
	{
		return (1.0-(constrained_differences_sum/target_area_sum));
	}
};

std::map< std::pair<CRAD, CRAD>, double > summarize_pair_mapping_of_values(const std::map< std::pair<CRAD, CRAD>, double >& map)
{
	std::map< std::pair<CRAD, CRAD>, double > result;
	for(std::map< std::pair<CRAD, CRAD>, double >::const_iterator it=map.begin();it!=map.end();++it)
	{
		const std::pair<CRAD, CRAD>& crads=it->first;
		result[modescommon::refine_pair_by_ordering(std::make_pair(crads.first.without_atom(), crads.second.without_atom()))]+=it->second;
	}
	return result;
}

std::map< std::pair<CRAD, CRAD>, std::pair<double, double> > combine_two_pair_mappings_of_values(const std::map< std::pair<CRAD, CRAD>, double >& map1, const std::map< std::pair<CRAD, CRAD>, double >& map2)
{
	std::map< std::pair<CRAD, CRAD>, std::pair<double, double> > result;
	for(std::map< std::pair<CRAD, CRAD>, double >::const_iterator it=map1.begin();it!=map1.end();++it)
	{
		result[it->first].first=it->second;
	}
	for(std::map< std::pair<CRAD, CRAD>, double >::const_iterator it=map2.begin();it!=map2.end();++it)
	{
		result[it->first].second=it->second;
	}
	return result;
}

std::map< std::pair<CRAD, CRAD>, CADDescriptor > construct_map_of_cad_descriptors(const std::map< std::pair<CRAD, CRAD>, std::pair<double, double> >& map_of_value_pairs)
{
	std::map< std::pair<CRAD, CRAD>, CADDescriptor > result;
	for(std::map< std::pair<CRAD, CRAD>, std::pair<double, double> >::const_iterator it=map_of_value_pairs.begin();it!=map_of_value_pairs.end();++it)
	{
		result[it->first].add(it->second.first, it->second.second);
	}
	return result;
}

CADDescriptor construct_global_cad_descriptor(const std::map< std::pair<CRAD, CRAD>, CADDescriptor >& map_of_descriptors)
{
	CADDescriptor result;
	for(std::map< std::pair<CRAD, CRAD>, CADDescriptor >::const_iterator it=map_of_descriptors.begin();it!=map_of_descriptors.end();++it)
	{
		result.add(it->second);
	}
	return result;
}

inline void print_score(const std::string& name, const CADDescriptor& cadd, std::ostream& output)
{
	if(cadd.scorable())
	{
		output << name << " ";
		output << cadd.score() << " " << cadd.target_area_sum << " " << cadd.model_area_sum << " " << cadd.raw_differences_sum << " " << cadd.constrained_differences_sum << "\n";
	}
}

void print_pair_scores_to_file(const std::map< std::pair<CRAD, CRAD>, CADDescriptor >& map_of_pair_cad_descriptors, const std::string& filename)
{
	if(!filename.empty())
	{
		std::ofstream foutput(filename.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map< std::pair<CRAD, CRAD>, CADDescriptor >::const_iterator it=map_of_pair_cad_descriptors.begin();it!=map_of_pair_cad_descriptors.end();++it)
			{
				print_score(it->first.first.str()+" "+it->first.second.str(), it->second, foutput);
			}
		}
	}
}

void print_single_scores_to_file(const std::map<CRAD, CADDescriptor>& map_of_single_cad_descriptors, const std::string& filename)
{
	if(!filename.empty())
	{
		std::ofstream foutput(filename.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map<CRAD, CADDescriptor>::const_iterator it=map_of_single_cad_descriptors.begin();it!=map_of_single_cad_descriptors.end();++it)
			{
				print_score(it->first.str(), it->second, foutput);
			}
		}
	}
}

}

void compare_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--target-contacts-file", "string", "file path to input target contacts", true));
		list_of_option_descriptions.push_back(OD("--inter-atom-scores-file", "string", "file path to output inter-atom scores"));
		list_of_option_descriptions.push_back(OD("--inter-residue-scores-file", "string", "file path to output inter-residue scores"));
		list_of_option_descriptions.push_back(OD("--atom-scores-file", "string", "file path to output atom scores"));
		list_of_option_descriptions.push_back(OD("--residue-scores-file", "string", "file path to output residue scores"));
		list_of_option_descriptions.push_back(OD("--depth", "number", "local neighborhood depth"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of model contacts (line format: 'annotation1 annotation2 area')\n";
			std::cerr << "stdout  ->  two lines of global scores (atom-level and residue-level)\n";
			return;
		}
	}

	const std::string target_contacts_file=poh.argument<std::string>("--target-contacts-file");
	const std::string inter_atom_scores_file=poh.argument<std::string>("--inter-atom-scores-file", "");
	const std::string inter_residue_scores_file=poh.argument<std::string>("--inter-residue-scores-file", "");
	const std::string atom_scores_file=poh.argument<std::string>("--atom-scores-file", "");
	const std::string residue_scores_file=poh.argument<std::string>("--residue-scores-file", "");
	const int depth=poh.argument<int>("--depth", 0);

	std::map< std::pair<CRAD, CRAD>, double > map_of_contacts;
	{
		auxiliaries::read_lines_to_container(std::cin, modescommon::add_chain_residue_atom_descriptors_pair_value_from_stream_to_map<false>, map_of_contacts);
		if(map_of_contacts.empty())
		{
			throw std::runtime_error("No contacts input.");
		}
	}

	std::map< std::pair<CRAD, CRAD>, double > map_of_target_contacts;
	{
		if(!target_contacts_file.empty())
		{
			std::ifstream finput(target_contacts_file.c_str(), std::ios::in);
			auxiliaries::read_lines_to_container(finput, modescommon::add_chain_residue_atom_descriptors_pair_value_from_stream_to_map<false>, map_of_target_contacts);
		}
		if(map_of_target_contacts.empty())
		{
			throw std::runtime_error("No target contacts input.");
		}
	}

	const std::map< std::pair<CRAD, CRAD>, CADDescriptor > map_of_inter_atom_cad_descriptors=construct_map_of_cad_descriptors(combine_two_pair_mappings_of_values(map_of_target_contacts, map_of_contacts));
	print_pair_scores_to_file(map_of_inter_atom_cad_descriptors, inter_atom_scores_file);

	const std::map< std::pair<CRAD, CRAD>, CADDescriptor > map_of_inter_residue_cad_descriptors=construct_map_of_cad_descriptors(combine_two_pair_mappings_of_values(summarize_pair_mapping_of_values(map_of_target_contacts), summarize_pair_mapping_of_values(map_of_contacts)));
	print_pair_scores_to_file(map_of_inter_residue_cad_descriptors, inter_residue_scores_file);

	if(!atom_scores_file.empty())
	{
		print_single_scores_to_file(modescommon::construct_single_mapping_of_descriptors_from_pair_mapping_of_descriptors(map_of_inter_atom_cad_descriptors, depth), atom_scores_file);
	}

	if(!residue_scores_file.empty())
	{
		print_single_scores_to_file(modescommon::construct_single_mapping_of_descriptors_from_pair_mapping_of_descriptors(map_of_inter_residue_cad_descriptors, depth), residue_scores_file);
	}

	print_score("atom_level_global", construct_global_cad_descriptor(map_of_inter_atom_cad_descriptors), std::cout);
	print_score("residue_level_global", construct_global_cad_descriptor(map_of_inter_residue_cad_descriptors), std::cout);
}
