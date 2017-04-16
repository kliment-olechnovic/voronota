#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cmath>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

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
		return ((target_area_sum>0.0) ? (1.0-(constrained_differences_sum/target_area_sum)) : -1.0);
	}

	double bounded_ratio_of_areas() const
	{
		return ((target_area_sum>0.0) ? std::min(1.0, model_area_sum/target_area_sum) : -1.0);
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
		output << " " << cadd.target_area_sum
				<< " " << cadd.model_area_sum
				<< " " << cadd.raw_differences_sum
				<< " " << cadd.constrained_differences_sum
				<< " " << cadd.bounded_ratio_of_areas();
	}
	return output;
}

std::map< CRADsPair, double > summarize_pair_mapping_of_values(const std::map< CRADsPair, double >& map, const bool ignore_residue_names)
{
	std::map< CRADsPair, double > result;
	for(std::map< CRADsPair, double >::const_iterator it=map.begin();it!=map.end();++it)
	{
		const CRADsPair& crads=it->first;
		result[CRADsPair(crads.a.without_some_info(true, true, false, ignore_residue_names), crads.b.without_some_info(true, true, false, ignore_residue_names))]+=it->second;
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

std::map<CRAD, CADDescriptor> filter_map_of_cad_descriptors_by_target_presence(const std::map<CRAD, CADDescriptor>& input_map)
{
	std::map<CRAD, CADDescriptor> result;
	for(std::map<CRAD, CADDescriptor>::const_iterator it=input_map.begin();it!=input_map.end();++it)
	{
		if(it->second.target_area_sum>0.0 && it->first.altLoc!="m" && it->first!=CRAD::any())
		{
			result[it->first]=it->second;
		}
	}
	return result;
}

double calculate_average_score_from_map_of_cad_descriptors(const std::map<CRAD, CADDescriptor>& input_map)
{
	double sum=0.0;
	for(std::map<CRAD, CADDescriptor>::const_iterator it=input_map.begin();it!=input_map.end();++it)
	{
		sum+=it->second.score();
	}
	return (sum/static_cast<double>(input_map.size()));
}

std::map<CRAD, double> collect_scores_from_map_of_cad_descriptors(const std::map<CRAD, CADDescriptor>& input_map)
{
	std::map<CRAD, double> result;
	for(std::map<CRAD, CADDescriptor>::const_iterator it=input_map.begin();it!=input_map.end();++it)
	{
		result[it->first]=it->second.score();
	}
	return result;
}

}

void compare_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of model contacts (line format: 'annotation1 annotation2 area')");
	pohw.describe_io("stdout", false, true, "global scores (atom-level and residue-level)");

	const std::string target_contacts_file=poh.argument<std::string>(pohw.describe_option("--target-contacts-file", "string", "file path to input target contacts", true), "");
	const std::string inter_atom_scores_file=poh.argument<std::string>(pohw.describe_option("--inter-atom-scores-file", "string", "file path to output inter-atom scores"), "");
	const std::string inter_residue_scores_file=poh.argument<std::string>(pohw.describe_option("--inter-residue-scores-file", "string", "file path to output inter-residue scores"), "");
	const std::string atom_scores_file=poh.argument<std::string>(pohw.describe_option("--atom-scores-file", "string", "file path to output atom scores"), "");
	const std::string residue_scores_file=poh.argument<std::string>(pohw.describe_option("--residue-scores-file", "string", "file path to output residue scores"), "");
	const int depth=poh.argument<int>(pohw.describe_option("--depth", "number", "local neighborhood depth"), 0);
	const unsigned int smoothing_window=poh.argument<unsigned int>(pohw.describe_option("--smoothing-window", "number", "window to smooth residue scores along sequence"), 0);
	const std::string smoothed_scores_file=poh.argument<std::string>(pohw.describe_option("--smoothed-scores-file", "string", "file path to output smoothed residue scores"), "");
	const bool ignore_residue_names=poh.contains_option(pohw.describe_option("--ignore-residue-names", "", "flag to consider just residue numbers and ignore residue names"));
	detailed_output_of_CADDescriptor()=poh.contains_option(pohw.describe_option("--detailed-output", "", "flag to enable detailed output"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const std::map<CRADsPair, double> map_of_contacts=auxiliaries::IOUtilities().read_lines_to_map< std::map<CRADsPair, double> >(std::cin);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No contacts input.");
	}

	const std::map<CRADsPair, double> map_of_target_contacts=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRADsPair, double> >(target_contacts_file);
	if(map_of_target_contacts.empty())
	{
		throw std::runtime_error("No target contacts input.");
	}

	{
		const std::map< CRADsPair, CADDescriptor > map_of_inter_atom_cad_descriptors=construct_map_of_cad_descriptors(
				combine_two_pair_mappings_of_values(map_of_target_contacts, map_of_contacts));
		auxiliaries::IOUtilities().write_map_to_file(map_of_inter_atom_cad_descriptors, inter_atom_scores_file);

		const std::map<CRAD, CADDescriptor> map_of_atom_cad_descriptors=filter_map_of_cad_descriptors_by_target_presence(
				auxiliaries::ChainResidueAtomDescriptorsGraphOperations::accumulate_mapped_values_by_graph_neighbors(map_of_inter_atom_cad_descriptors, depth));
		auxiliaries::IOUtilities().write_map_to_file(map_of_atom_cad_descriptors, atom_scores_file);

		std::cout << "atom_level_global " << construct_global_cad_descriptor(map_of_inter_atom_cad_descriptors) << "\n";
		std::cout << "atom_average_local " << calculate_average_score_from_map_of_cad_descriptors(map_of_atom_cad_descriptors) << "\n";
	}

	{
		const std::map< CRADsPair, CADDescriptor > map_of_inter_residue_cad_descriptors=construct_map_of_cad_descriptors(
				combine_two_pair_mappings_of_values(summarize_pair_mapping_of_values(map_of_target_contacts, ignore_residue_names), summarize_pair_mapping_of_values(map_of_contacts, ignore_residue_names)));
		auxiliaries::IOUtilities().write_map_to_file(map_of_inter_residue_cad_descriptors, inter_residue_scores_file);

		const std::map<CRAD, CADDescriptor> map_of_residue_cad_descriptors=filter_map_of_cad_descriptors_by_target_presence(
				auxiliaries::ChainResidueAtomDescriptorsGraphOperations::accumulate_mapped_values_by_graph_neighbors(map_of_inter_residue_cad_descriptors, depth));
		auxiliaries::IOUtilities().write_map_to_file(map_of_residue_cad_descriptors, residue_scores_file);

		if(!smoothed_scores_file.empty())
		{
			auxiliaries::IOUtilities().write_map_to_file(
					auxiliaries::ChainResidueAtomDescriptorsSequenceOperations::smooth_residue_scores_along_sequence(
							collect_scores_from_map_of_cad_descriptors(map_of_residue_cad_descriptors), smoothing_window), smoothed_scores_file);
		}

		std::cout << "residue_level_global " << construct_global_cad_descriptor(map_of_inter_residue_cad_descriptors) << "\n";
		std::cout << "residue_average_local " << calculate_average_score_from_map_of_cad_descriptors(map_of_residue_cad_descriptors) << "\n";
	}
}
