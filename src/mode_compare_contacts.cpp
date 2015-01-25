#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cmath>

#include "auxiliaries/program_options_handler.h"

#include "modescommon/handle_annotations.h"
#include "modescommon/handle_mappings.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

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
		model_area_sum+=cadd.model_area_sum;
		raw_differences_sum+=cadd.raw_differences_sum;
		constrained_differences_sum+=cadd.constrained_differences_sum;
	}

	double score() const
	{
		return ((target_area_sum>0.0) ? (1.0-(constrained_differences_sum/target_area_sum)) : 0.0);
	}
};

inline bool& detailed_output_of_CADDescriptor()
{
	static bool detailed_output=false;
	return detailed_output;
}

inline std::ostream& operator<<(std::ostream& output, const CADDescriptor& cadd)
{
	output << cadd.score();
	if(detailed_output_of_CADDescriptor())
	{
		output << " " << cadd.target_area_sum << " " << cadd.model_area_sum << " " << cadd.raw_differences_sum << " " << cadd.constrained_differences_sum;
	}
	return output;
}

std::map< CRADsPair, double > summarize_pair_mapping_of_values(const std::map< CRADsPair, double >& map)
{
	std::map< CRADsPair, double > result;
	for(std::map< CRADsPair, double >::const_iterator it=map.begin();it!=map.end();++it)
	{
		const CRADsPair& crads=it->first;
		result[CRADsPair(crads.a.without_atom(), crads.b.without_atom())]+=it->second;
	}
	return result;
}

std::map< CRADsPair, std::pair<double, double> > combine_two_pair_mappings_of_values(const std::map< CRADsPair, double >& map1, const std::map< CRADsPair, double >& map2)
{
	std::map< CRADsPair, std::pair<double, double> > result;
	for(std::map< CRADsPair, double >::const_iterator it=map1.begin();it!=map1.end();++it)
	{
		result[it->first].first=it->second;
	}
	for(std::map< CRADsPair, double >::const_iterator it=map2.begin();it!=map2.end();++it)
	{
		result[it->first].second=it->second;
	}
	return result;
}

std::map< CRADsPair, CADDescriptor > construct_map_of_cad_descriptors(const std::map< CRADsPair, std::pair<double, double> >& map_of_value_pairs)
{
	std::map< CRADsPair, CADDescriptor > result;
	for(std::map< CRADsPair, std::pair<double, double> >::const_iterator it=map_of_value_pairs.begin();it!=map_of_value_pairs.end();++it)
	{
		result[it->first].add(it->second.first, it->second.second);
	}
	return result;
}

CADDescriptor construct_global_cad_descriptor(const std::map< CRADsPair, CADDescriptor >& map_of_descriptors)
{
	CADDescriptor result;
	for(std::map< CRADsPair, CADDescriptor >::const_iterator it=map_of_descriptors.begin();it!=map_of_descriptors.end();++it)
	{
		result.add(it->second);
	}
	return result;
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
		list_of_option_descriptions.push_back(OD("--detailed-output", "", "flag to enable detailed output"));
		if(!poh.assert(list_of_option_descriptions, false))
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
	detailed_output_of_CADDescriptor()=poh.contains_option("--detailed-output");

	std::map< CRADsPair, double > map_of_contacts;
	{
		auxiliaries::read_lines_to_map_container(std::cin, map_of_contacts);
		if(map_of_contacts.empty())
		{
			throw std::runtime_error("No contacts input.");
		}
	}

	std::map< CRADsPair, double > map_of_target_contacts;
	{
		auxiliaries::read_file_lines_to_map_container(target_contacts_file, map_of_target_contacts);
		if(map_of_target_contacts.empty())
		{
			throw std::runtime_error("No target contacts input.");
		}
	}

	const std::map< CRADsPair, CADDescriptor > map_of_inter_atom_cad_descriptors=construct_map_of_cad_descriptors(combine_two_pair_mappings_of_values(map_of_target_contacts, map_of_contacts));
	auxiliaries::write_map_container_to_file(map_of_inter_atom_cad_descriptors, inter_atom_scores_file);

	const std::map< CRADsPair, CADDescriptor > map_of_inter_residue_cad_descriptors=construct_map_of_cad_descriptors(combine_two_pair_mappings_of_values(summarize_pair_mapping_of_values(map_of_target_contacts), summarize_pair_mapping_of_values(map_of_contacts)));
	auxiliaries::write_map_container_to_file(map_of_inter_residue_cad_descriptors, inter_residue_scores_file);

	if(!atom_scores_file.empty())
	{
		auxiliaries::write_map_container_to_file(modescommon::construct_single_mapping_of_descriptors_from_mapping_of_descriptors_pairs(map_of_inter_atom_cad_descriptors, modescommon::construct_graph_from_mapping_of_descriptors_pairs(map_of_inter_atom_cad_descriptors, depth)), atom_scores_file);
	}

	if(!residue_scores_file.empty())
	{
		auxiliaries::write_map_container_to_file(modescommon::construct_single_mapping_of_descriptors_from_mapping_of_descriptors_pairs(map_of_inter_residue_cad_descriptors, modescommon::construct_graph_from_mapping_of_descriptors_pairs(map_of_inter_residue_cad_descriptors, depth)), residue_scores_file);
	}

	std::cout << "atom_level_global " << construct_global_cad_descriptor(map_of_inter_atom_cad_descriptors) << "\n";
	std::cout << "residue_level_global " << construct_global_cad_descriptor(map_of_inter_residue_cad_descriptors) << "\n";
}
