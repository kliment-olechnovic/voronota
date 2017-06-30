#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cmath>
#include <algorithm>

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
	double model_target_area_sum;

	CADDescriptor() : target_area_sum(0), model_area_sum(0), raw_differences_sum(), constrained_differences_sum(0), model_target_area_sum(0)
	{
	}

	void add(const double target_area, const double model_area)
	{
		target_area_sum+=target_area;
		model_area_sum+=model_area;
		raw_differences_sum+=fabs(target_area-model_area);
		constrained_differences_sum+=std::min(fabs(target_area-model_area), target_area);
		model_target_area_sum+=(target_area>0.0 ? model_area : 0.0);
	}

	void add(const CADDescriptor& cadd)
	{
		target_area_sum+=cadd.target_area_sum;
		model_area_sum+=cadd.model_area_sum;
		raw_differences_sum+=cadd.raw_differences_sum;
		constrained_differences_sum+=cadd.constrained_differences_sum;
		model_target_area_sum+=cadd.model_target_area_sum;
	}

	double score() const
	{
		return ((target_area_sum>0.0) ? (1.0-(constrained_differences_sum/target_area_sum)) : -1.0);
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
				<< " " << cadd.model_target_area_sum;
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

CADDescriptor construct_global_cad_descriptor(const std::map<CRADsPair, double>& map_of_target_contacts, const std::map<CRADsPair, double>& map_of_contacts, const bool ignore_residue_names)
{
	return construct_global_cad_descriptor(
			construct_map_of_cad_descriptors(
					combine_two_pair_mappings_of_values(
							summarize_pair_mapping_of_values(map_of_target_contacts, ignore_residue_names),
							summarize_pair_mapping_of_values(map_of_contacts, ignore_residue_names))));
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

std::string rename_by_map(const std::map<std::string, std::string>& map_of_renamings, const std::string& name)
{
	std::map<std::string, std::string>::const_iterator it=map_of_renamings.find(name);
	if(it!=map_of_renamings.end())
	{
		return it->second;
	}
	else
	{
		return name;
	}
}

std::map<CRADsPair, double> rename_chains_in_map_of_contacts(const std::map<CRADsPair, double>& map_of_contacts, const std::map<std::string, std::string>& map_of_renamings)
{
	if(map_of_renamings.empty())
	{
		return map_of_contacts;
	}
	std::map<CRADsPair, double> result;
	for(std::map<CRADsPair, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		CRAD a=it->first.a;
		CRAD b=it->first.b;
		a.chainID=rename_by_map(map_of_renamings, a.chainID);
		b.chainID=rename_by_map(map_of_renamings, b.chainID);
		result[CRADsPair(a, b)]+=it->second;
	}
	return result;
}

std::map<CRADsPair, double> select_contacts_with_defined_chain_names(const std::map<CRADsPair, double>& map_of_contacts)
{
	std::map<CRADsPair, double> result;
	for(std::map<CRADsPair, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		if(!it->first.a.chainID.empty() || !it->first.b.chainID.empty())
		{
			result[it->first]=it->second;
		}
	}
	return result;
}

std::vector<std::string> get_sorted_chain_names_from_map_of_contacts(const std::map<CRADsPair, double>& map_of_contacts)
{
	std::set<std::string> set_of_names;
	for(std::map<CRADsPair, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		if(it->first.a!=CRAD::solvent() && it->first.a!=CRAD::any())
		{
			set_of_names.insert(it->first.a.chainID);
		}
		if(it->first.b!=CRAD::solvent() && it->first.b!=CRAD::any())
		{
			set_of_names.insert(it->first.b.chainID);
		}
	}
	return std::vector<std::string>(set_of_names.begin(), set_of_names.end());
}

std::map<std::string, std::string> generate_map_of_renamings_from_two_lists(const std::vector<std::string>& left, const std::vector<std::string>& right)
{
	std::map<std::string, std::string> result;
	if(left.size()==right.size())
	{
		for(std::size_t i=0;i<left.size();i++)
		{
			result[left[i]]=right[i];
		}
	}
	return result;
}

void remap_chains_optimally(
		const std::map<CRADsPair, double>& map_of_target_contacts,
		const bool ignore_residue_names,
		const std::string& remapped_chains_file,
		const bool print_log,
		std::map<CRADsPair, double>& map_of_contacts)
{
	const std::vector<std::string> chain_names=get_sorted_chain_names_from_map_of_contacts(map_of_contacts);
	if(chain_names.size()<2)
	{
		return;
	}
	if(chain_names.size()<=5)
	{
		std::pair<std::map<std::string, std::string>, double> best_renaming(generate_map_of_renamings_from_two_lists(chain_names, chain_names), 0.0);
		std::vector<std::string> permutated_chain_names=chain_names;
		do
		{
			const std::map<std::string, std::string> map_of_renamings=generate_map_of_renamings_from_two_lists(chain_names, permutated_chain_names);
			const double score=construct_global_cad_descriptor(map_of_target_contacts, rename_chains_in_map_of_contacts(map_of_contacts, map_of_renamings), ignore_residue_names).score();
			if(score>best_renaming.second)
			{
				best_renaming.first=map_of_renamings;
				best_renaming.second=score;
			}
			if(print_log)
			{
				for(std::size_t i=0;i<permutated_chain_names.size();i++)
				{
					std::cerr << permutated_chain_names[i] << " ";
				}
				std::cerr << " " << score << "\n";
			}
		}
		while(std::next_permutation(permutated_chain_names.begin(), permutated_chain_names.end()));
		map_of_contacts=rename_chains_in_map_of_contacts(map_of_contacts, best_renaming.first);
		if(print_log)
		{
			std::cerr << "remapping:\n";
			auxiliaries::IOUtilities().write_map(best_renaming.first, std::cerr);
		}
		auxiliaries::IOUtilities().write_map_to_file(best_renaming.first, remapped_chains_file);
	}
	else
	{
		std::map<std::string, std::string> map_of_renamings;
		std::map<std::string, std::string> map_of_renamings_in_target;
		for(std::size_t i=0;i<chain_names.size();i++)
		{
			map_of_renamings[chain_names[i]]=std::string();
			map_of_renamings_in_target[chain_names[i]]=std::string();
		}
		std::set<std::string> set_of_free_chains_left(chain_names.begin(), chain_names.end());
		std::set<std::string> set_of_free_chains_right(chain_names.begin(), chain_names.end());
		while(!set_of_free_chains_left.empty())
		{
			std::pair<std::string, std::string> best_pair(*set_of_free_chains_left.begin(), *set_of_free_chains_right.begin());
			double best_score=0.0;
			for(std::set<std::string>::const_iterator it_left=set_of_free_chains_left.begin();it_left!=set_of_free_chains_left.end();++it_left)
			{
				for(std::set<std::string>::const_iterator it_right=set_of_free_chains_right.begin();it_right!=set_of_free_chains_right.end();++it_right)
				{
					std::map<std::string, std::string> new_map_of_renamings=map_of_renamings;
					std::map<std::string, std::string> new_map_of_renamings_in_target=map_of_renamings_in_target;
					new_map_of_renamings[*it_left]=(*it_right);
					new_map_of_renamings_in_target[*it_right]=(*it_right);
					const CADDescriptor cad_descriptor=construct_global_cad_descriptor(
							select_contacts_with_defined_chain_names(rename_chains_in_map_of_contacts(map_of_target_contacts, new_map_of_renamings_in_target)),
							select_contacts_with_defined_chain_names(rename_chains_in_map_of_contacts(map_of_contacts, new_map_of_renamings)),
							ignore_residue_names);
					const double score=cad_descriptor.score()*cad_descriptor.target_area_sum;
					if(score>best_score)
					{
						best_pair=std::make_pair(*it_left, *it_right);
						best_score=score;
					}
					if(print_log)
					{
						std::cerr << (*it_left) << " " << (*it_right) << "  " << score << "\n";
					}
				}
			}
			if(print_log)
			{
				std::cerr << best_pair.first << " " << best_pair.second << "  " << best_score << " fixed\n";
			}
			map_of_renamings[best_pair.first]=best_pair.second;
			map_of_renamings_in_target[best_pair.second]=best_pair.second;
			set_of_free_chains_left.erase(best_pair.first);
			set_of_free_chains_right.erase(best_pair.second);
		}
		map_of_contacts=rename_chains_in_map_of_contacts(map_of_contacts, map_of_renamings);
		if(print_log)
		{
			std::cerr << "remapping:\n";
			auxiliaries::IOUtilities().write_map(map_of_renamings, std::cerr);
		}
		auxiliaries::IOUtilities().write_map_to_file(map_of_renamings, remapped_chains_file);
	}
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
	const bool residue_level_only=poh.contains_option(pohw.describe_option("--residue-level-only", "", "flag to output only residue-level results"));
	detailed_output_of_CADDescriptor()=poh.contains_option(pohw.describe_option("--detailed-output", "", "flag to enable detailed output"));
	const std::string chains_renaming_file=poh.argument<std::string>(pohw.describe_option("--chains-renaming-file", "string", "file path to input chains renaming"), "");
	const bool remap_chains=poh.contains_option(pohw.describe_option("--remap-chains", "", "flag to calculate optimal chains remapping"));
	const bool remap_chains_log=poh.contains_option(pohw.describe_option("--remap-chains-log", "", "flag output remapping progress to stderr"));
	const std::string remapped_chains_file=poh.argument<std::string>(pohw.describe_option("--remapped-chains-file", "string", "file path to output calculated chains remapping"), "");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::map<CRADsPair, double> map_of_contacts=auxiliaries::IOUtilities().read_lines_to_map< std::map<CRADsPair, double> >(std::cin);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No contacts input.");
	}

	const std::map<CRADsPair, double> map_of_target_contacts=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRADsPair, double> >(target_contacts_file);
	if(map_of_target_contacts.empty())
	{
		throw std::runtime_error("No target contacts input.");
	}

	if(!chains_renaming_file.empty())
	{
		const std::map<std::string, std::string> map_of_renamings=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<std::string, std::string> >(chains_renaming_file);
		if(!map_of_renamings.empty())
		{
			map_of_contacts=rename_chains_in_map_of_contacts(map_of_contacts, map_of_renamings);
		}
	}

	if(remap_chains)
	{
		remap_chains_optimally(map_of_target_contacts, ignore_residue_names, remapped_chains_file, remap_chains_log, map_of_contacts);
	}

	if(!residue_level_only)
	{
		const std::map< CRADsPair, CADDescriptor > map_of_inter_atom_cad_descriptors=construct_map_of_cad_descriptors(
				combine_two_pair_mappings_of_values(map_of_target_contacts, map_of_contacts));
		auxiliaries::IOUtilities().write_map_to_file(map_of_inter_atom_cad_descriptors, inter_atom_scores_file);

		const std::map<CRAD, CADDescriptor> map_of_atom_cad_descriptors=filter_map_of_cad_descriptors_by_target_presence(
				auxiliaries::ChainResidueAtomDescriptorsGraphOperations::accumulate_mapped_values_by_graph_neighbors(map_of_inter_atom_cad_descriptors, depth));
		auxiliaries::IOUtilities().write_map_to_file(map_of_atom_cad_descriptors, atom_scores_file);

		std::cout << "atom_level_global " << construct_global_cad_descriptor(map_of_inter_atom_cad_descriptors) << "\n";
		std::cout << "atom_average_local " << calculate_average_score_from_map_of_cad_descriptors(map_of_atom_cad_descriptors) << "\n";
		std::cout << "atom_count " << map_of_atom_cad_descriptors.size() << "\n";
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
		std::cout << "residue_count " << map_of_residue_cad_descriptors.size() << "\n";
	}
}
