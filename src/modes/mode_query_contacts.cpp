#include <iostream>
#include <stdexcept>
#include <fstream>

#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/atoms_io.h"
#include "../auxiliaries/opengl_printer.h"

#include "../common/chain_residue_atom_descriptor.h"
#include "../common/contact_value.h"
#include "../common/matching_utilities.h"

namespace
{

typedef voronota::common::ChainResidueAtomDescriptor CRAD;
typedef voronota::common::ChainResidueAtomDescriptorsPair CRADsPair;

std::set<CRADsPair> init_set_of_hbplus_crad_pairs(const std::string& hbplus_file_name, const bool inter_residue_hbplus_tags)
{
	std::set<CRADsPair> set_of_hbplus_crad_pairs;
	if(!hbplus_file_name.empty())
	{
		std::ifstream input_file(hbplus_file_name.c_str(), std::ios::in);
		voronota::auxiliaries::AtomsIO::HBPlusReader::Data hbplus_file_data=voronota::auxiliaries::AtomsIO::HBPlusReader::read_data_from_file_stream(input_file);
		if(!hbplus_file_data.hbplus_records.empty())
		{
			for(std::vector<voronota::auxiliaries::AtomsIO::HBPlusReader::HBPlusRecord>::const_iterator it=hbplus_file_data.hbplus_records.begin();it!=hbplus_file_data.hbplus_records.end();++it)
			{
				const voronota::auxiliaries::AtomsIO::HBPlusReader::ShortAtomDescriptor& a=it->first;
				const voronota::auxiliaries::AtomsIO::HBPlusReader::ShortAtomDescriptor& b=it->second;
				const CRADsPair crads_pair(CRAD(CRAD::null_num(), a.chainID, a.resSeq, a.resName, a.name, "", ""), CRAD(CRAD::null_num(), b.chainID, b.resSeq, b.resName, b.name, "", ""));
				set_of_hbplus_crad_pairs.insert(inter_residue_hbplus_tags ? CRADsPair(crads_pair.a.without_atom(), crads_pair.b.without_atom()) : crads_pair);
			}
		}
	}
	return set_of_hbplus_crad_pairs;
}

void apply_renaming_map_on_contacts(const std::string& renaming_map, std::map<CRADsPair, voronota::common::ContactValue>& map_of_contacts)
{
	const std::map<CRAD, CRAD> renaming_map_of_crads=voronota::auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRAD, CRAD> >(renaming_map);
	if(!renaming_map_of_crads.empty())
	{
		std::map< CRADsPair, voronota::common::ContactValue > map_of_renamed_contacts;
		for(std::map< CRADsPair, voronota::common::ContactValue >::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			CRAD crads[2]={it->first.a, it->first.b};
			for(int i=0;i<2;i++)
			{
				CRAD& crad=crads[i];
				std::map<CRAD, CRAD>::const_iterator renaming_it=renaming_map_of_crads.find(crad.without_numbering());
				if(renaming_it!=renaming_map_of_crads.end())
				{
					crad.resName=renaming_it->second.resName;
					crad.name=renaming_it->second.name;
				}
			}
			map_of_renamed_contacts[CRADsPair(crads[0], crads[1])].add(it->second);
		}
		map_of_contacts=map_of_renamed_contacts;
	}
}

void sum_contacts_into_inter_residue_contacts(const std::string& summing_exceptions, std::map<CRADsPair, voronota::common::ContactValue>& map_of_contacts)
{
	const std::set<CRAD> summing_exceptions_set_of_crads=voronota::auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRAD> >(summing_exceptions);
	std::map< CRADsPair, voronota::common::ContactValue > map_of_reduced_contacts;
	for(std::map< CRADsPair, voronota::common::ContactValue >::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		const CRADsPair& raw_crads=it->first;
		const bool exclude_a=(!summing_exceptions_set_of_crads.empty() && voronota::common::MatchingUtilities::match_crad_with_set_of_crads(false, raw_crads.a, summing_exceptions_set_of_crads));
		const bool exclude_b=(!summing_exceptions_set_of_crads.empty() && voronota::common::MatchingUtilities::match_crad_with_set_of_crads(false, raw_crads.b, summing_exceptions_set_of_crads));
		const CRADsPair crads((exclude_a ? raw_crads.a : raw_crads.a.without_atom()), (exclude_b ? raw_crads.b : raw_crads.b.without_atom()), raw_crads.reversed_display);
		if(!(crads.a==crads.b))
		{
			map_of_reduced_contacts[crads].add(it->second);
		}
	}
	map_of_contacts=map_of_reduced_contacts;
}

}

void query_contacts(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts [graphics]')");
	pohw.describe_io("stdout", false, true, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts [graphics]')");

	const std::string match_first=poh.argument<std::string>(pohw.describe_option("--match-first", "string", "selection for first contacting group"), "");
	const std::string match_first_not=poh.argument<std::string>(pohw.describe_option("--match-first-not", "string", "negative selection for first contacting group"), "");
	const std::string match_second=poh.argument<std::string>(pohw.describe_option("--match-second", "string", "selection for second contacting group"), "");
	const std::string match_second_not=poh.argument<std::string>(pohw.describe_option("--match-second-not", "string", "negative selection for second contacting group"), "");
	const int match_min_sequence_separation=poh.argument<int>(pohw.describe_option("--match-min-seq-sep", "number", "minimum residue sequence separation"), CRAD::null_num());
	const int match_max_sequence_separation=poh.argument<int>(pohw.describe_option("--match-max-seq-sep", "number", "maximum residue sequence separation"), CRAD::null_num());
	const double match_min_area=poh.argument<double>(pohw.describe_option("--match-min-area", "number", "minimum contact area"), std::numeric_limits<double>::min());
	const double match_max_area=poh.argument<double>(pohw.describe_option("--match-max-area", "number", "maximum contact area"), std::numeric_limits<double>::max());
	const double match_min_dist=poh.argument<double>(pohw.describe_option("--match-min-dist", "number", "minimum distance"), std::numeric_limits<double>::min());
	const double match_max_dist=poh.argument<double>(pohw.describe_option("--match-max-dist", "number", "maximum distance"), std::numeric_limits<double>::max());
	const std::string match_tags=poh.argument<std::string>(pohw.describe_option("--match-tags", "string", "tags to match"), "");
	const std::string match_tags_not=poh.argument<std::string>(pohw.describe_option("--match-tags-not", "string", "tags to not match"), "");
	const std::string match_adjuncts=poh.argument<std::string>(pohw.describe_option("--match-adjuncts", "string", "adjuncts intervals to match"), "");
	const std::string match_adjuncts_not=poh.argument<std::string>(pohw.describe_option("--match-adjuncts-not", "string", "adjuncts intervals to not match"), "");
	const std::string match_external_first=poh.argument<std::string>(pohw.describe_option("--match-external-first", "string", "file path to input matchable annotations"), "");
	const std::string match_external_second=poh.argument<std::string>(pohw.describe_option("--match-external-second", "string", "file path to input matchable annotations"), "");
	const std::string match_external_pairs=poh.argument<std::string>(pohw.describe_option("--match-external-pairs", "string", "file path to input matchable annotation pairs"), "");
	const bool no_solvent=poh.contains_option(pohw.describe_option("--no-solvent", "", "flag to not include solvent accessible areas"));
	const bool ignore_dist_for_solvent=poh.contains_option(pohw.describe_option("--ignore-dist-for-solvent", "", "flag to ignore distance for solvent contacts"));
	const bool ignore_seq_sep_for_solvent=poh.contains_option(pohw.describe_option("--ignore-seq-sep-for-solvent", "", "flag to ignore sequence separation for solvent contacts"));
	const bool no_same_chain=poh.contains_option(pohw.describe_option("--no-same-chain", "", "flag to not include same chain contacts"));
	const bool invert=poh.contains_option(pohw.describe_option("--invert", "", "flag to invert selection"));
	const bool drop_tags=poh.contains_option(pohw.describe_option("--drop-tags", "", "flag to drop all tags from input"));
	const bool drop_adjuncts=poh.contains_option(pohw.describe_option("--drop-adjuncts", "", "flag to drop all adjuncts from input"));
	const std::string set_tags=poh.argument<std::string>(pohw.describe_option("--set-tags", "string", "set tags instead of filtering"), "");
	const std::string set_hbplus_tags=poh.argument<std::string>(pohw.describe_option("--set-hbplus-tags", "string", "file path to input HBPLUS file"), "");
	const std::string set_distance_bins_tags=poh.argument<std::string>(pohw.describe_option("--set-distance-bins-tags", "string", "list of distance thresholds"), "");
	const bool inter_residue_hbplus_tags=poh.contains_option(pohw.describe_option("--inter-residue-hbplus-tags", "", "flag to set inter-residue H-bond tags"));
	const std::string set_adjuncts=poh.argument<std::string>(pohw.describe_option("--set-adjuncts", "string", "set adjuncts instead of filtering"), "");
	const std::string set_external_adjuncts=poh.argument<std::string>(pohw.describe_option("--set-external-adjuncts", "string", "file path to input external adjuncts"), "");
	const std::string set_external_adjuncts_name=poh.argument<std::string>(pohw.describe_option("--set-external-adjuncts-name", "string", "name for external adjuncts"), "ex");
	const std::string set_external_means=poh.argument<std::string>(pohw.describe_option("--set-external-means", "string", "file path to input external values for averaging"), "");
	const std::string set_external_means_name=poh.argument<std::string>(pohw.describe_option("--set-external-means-name", "string", "name for external means"), "em");
	const std::string renaming_map=poh.argument<std::string>(pohw.describe_option("--renaming-map", "string", "file path to input atoms renaming map"), "");
	const bool inter_residue=poh.contains_option(pohw.describe_option("--inter-residue", "", "flag to convert input to inter-residue contacts"));
	const bool inter_residue_after=poh.contains_option(pohw.describe_option("--inter-residue-after", "", "flag to convert output to inter-residue contacts"));
	const std::string summing_exceptions=poh.argument<std::string>(pohw.describe_option("--summing-exceptions", "string", "file path to input inter-residue summing exceptions annotations"), "");
	const bool summarize=poh.contains_option(pohw.describe_option("--summarize", "", "flag to output only summary of matched contacts"));
	const bool summarize_by_first=poh.contains_option(pohw.describe_option("--summarize-by-first", "", "flag to output only summary of matched contacts by first identifier"));
	voronota::common::enabled_output_of_ContactValue_graphics()=poh.contains_option(pohw.describe_option("--preserve-graphics", "", "flag to preserve graphics in output"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::map<CRADsPair, voronota::common::ContactValue> map_of_contacts;
	voronota::auxiliaries::IOUtilities().read_lines_to_map(std::cin, map_of_contacts);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	if(!renaming_map.empty())
	{
		apply_renaming_map_on_contacts(renaming_map, map_of_contacts);
	}

	if(inter_residue)
	{
		sum_contacts_into_inter_residue_contacts(summing_exceptions, map_of_contacts);
	}

	std::map<CRADsPair, std::map<CRADsPair, voronota::common::ContactValue>::iterator> selected_contacts;

	const std::set<CRAD> matchable_external_first_set_of_crads=voronota::auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRAD> >(match_external_first);
	const std::set<CRAD> matchable_external_second_set_of_crads=voronota::auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRAD> >(match_external_second);
	{
		const std::set<CRADsPair> matchable_external_set_of_crad_pairs=voronota::auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRADsPair> >(match_external_pairs);
		for(std::map< CRADsPair, voronota::common::ContactValue >::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const CRADsPair& crads=it->first;
			const voronota::common::ContactValue& value=it->second;
			bool passed=false;
			if(
					value.area>=match_min_area && value.area<=match_max_area &&
					((value.dist>=match_min_dist && value.dist<=match_max_dist) || (ignore_dist_for_solvent && crads.contains(CRAD::solvent()))) &&
					(!no_solvent || !crads.contains(CRAD::solvent())) &&
					(!no_same_chain || crads.a.chainID!=crads.b.chainID) &&
					((ignore_seq_sep_for_solvent && crads.contains(CRAD::solvent())) || CRAD::match_with_sequence_separation_interval(crads.a, crads.b, match_min_sequence_separation, match_max_sequence_separation, true)) &&
					voronota::common::MatchingUtilities::match_set_of_tags(value.props.tags, match_tags, match_tags_not) &&
					voronota::common::MatchingUtilities::match_map_of_adjuncts(value.props.adjuncts, match_adjuncts, match_adjuncts_not) &&
					(match_external_pairs.empty() || voronota::common::MatchingUtilities::match_crads_pair_with_set_of_crads_pairs(false, crads, matchable_external_set_of_crad_pairs))
			)
			{
				const bool matched_first_second=(
						voronota::common::MatchingUtilities::match_crad(crads.a, match_first, match_first_not) &&
						voronota::common::MatchingUtilities::match_crad(crads.b, match_second, match_second_not) &&
						(match_external_first.empty() || voronota::common::MatchingUtilities::match_crad_with_set_of_crads(false, crads.a, matchable_external_first_set_of_crads)) &&
						(match_external_second.empty() || voronota::common::MatchingUtilities::match_crad_with_set_of_crads(false, crads.b, matchable_external_second_set_of_crads)));
				const bool matched_second_first=matched_first_second || (
						voronota::common::MatchingUtilities::match_crad(crads.b, match_first, match_first_not) &&
						voronota::common::MatchingUtilities::match_crad(crads.a, match_second, match_second_not) &&
						(match_external_first.empty() || voronota::common::MatchingUtilities::match_crad_with_set_of_crads(false, crads.b, matchable_external_first_set_of_crads)) &&
						(match_external_second.empty() || voronota::common::MatchingUtilities::match_crad_with_set_of_crads(false, crads.a, matchable_external_second_set_of_crads)));
				passed=(matched_first_second || matched_second_first);
				if(passed && !invert)
				{
					selected_contacts.insert(selected_contacts.end(), std::make_pair(CRADsPair(crads.a, crads.b, !matched_first_second), it));
				}
			}
			if(!passed && invert)
			{
				selected_contacts.insert(selected_contacts.end(), std::make_pair(crads, it));
			}
		}
	}

	const bool update_mode=(drop_tags || drop_adjuncts || !set_tags.empty() || !set_adjuncts.empty() || !set_external_adjuncts.empty() || !set_external_means.empty() || !set_hbplus_tags.empty() || !set_distance_bins_tags.empty());
	if(update_mode && !selected_contacts.empty())
	{
		const std::map<CRADsPair, double> map_of_external_adjunct_values=voronota::auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRADsPair, double> >(set_external_adjuncts);
		const std::map<CRAD, double> map_of_external_values_for_averaging=voronota::auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRAD, double> >(set_external_means);
		const std::set<CRADsPair> set_of_hbplus_crad_pairs=init_set_of_hbplus_crad_pairs(set_hbplus_tags, inter_residue_hbplus_tags);
		const std::set<double> distance_thresholds_for_bins=(set_distance_bins_tags.empty() ? std::set<double>() : voronota::auxiliaries::IOUtilities(';').read_string_lines_to_set< std::set<double> >(set_distance_bins_tags));

		for(std::map<CRADsPair, std::map<CRADsPair, voronota::common::ContactValue>::iterator>::iterator selected_map_it=selected_contacts.begin();selected_map_it!=selected_contacts.end();++selected_map_it)
		{
			std::map<CRADsPair, voronota::common::ContactValue>::iterator it=selected_map_it->second;
			if(it!=map_of_contacts.end())
			{
				if(drop_tags)
				{
					it->second.props.tags.clear();
				}
				if(drop_adjuncts)
				{
					it->second.props.adjuncts.clear();
				}
				if(!set_tags.empty())
				{
					it->second.props.update_tags(set_tags);
				}
				if(!set_adjuncts.empty())
				{
					it->second.props.update_adjuncts(set_adjuncts);
				}
				if(!map_of_external_adjunct_values.empty())
				{
					const std::pair<bool, double> adjunct_value=voronota::common::MatchingUtilities::match_crads_pair_with_map_of_crads_pairs(false, it->first, map_of_external_adjunct_values);
					if(adjunct_value.first)
					{
						it->second.props.adjuncts[set_external_adjuncts_name]=adjunct_value.second;
					}
				}
				if(!map_of_external_values_for_averaging.empty())
				{
					std::map< CRAD, double >::const_iterator value_a_it=map_of_external_values_for_averaging.find(it->first.a);
					std::map< CRAD, double >::const_iterator value_b_it=map_of_external_values_for_averaging.find(it->first.b);
					if(value_a_it!=map_of_external_values_for_averaging.end() || value_b_it!=map_of_external_values_for_averaging.end())
					{
						const double value_a=(value_a_it!=map_of_external_values_for_averaging.end() ? value_a_it->second : 0.0);
						const double value_b=(value_b_it!=map_of_external_values_for_averaging.end() ? value_b_it->second : 0.0);
						const double mult=((value_a_it!=map_of_external_values_for_averaging.end() && value_b_it!=map_of_external_values_for_averaging.end()) ? 0.5 : 1.0);
						it->second.props.adjuncts[set_external_means_name]=(value_a+value_b)*mult;
					}
				}
				if(!set_of_hbplus_crad_pairs.empty())
				{
					if(voronota::common::MatchingUtilities::match_crads_pair_with_set_of_crads_pairs(false, it->first, set_of_hbplus_crad_pairs))
					{
						it->second.props.tags.insert(inter_residue_hbplus_tags ? "rhb" : "hb");
					}
				}
				if(!distance_thresholds_for_bins.empty())
				{
					std::string bin_tag="d0";
					std::set<double>::const_iterator it_a=distance_thresholds_for_bins.begin();
					if(it->second.dist>=(*it_a))
					{
						bool found=false;
						int bin=1;
						while(it_a!=distance_thresholds_for_bins.end() && !found)
						{
							std::set<double>::const_iterator it_b=it_a; ++it_b;
							if(it->second.dist>=(*it_a) && (it_b==distance_thresholds_for_bins.end() || it->second.dist<(*it_b)))
							{
								std::ostringstream bin_tag_output;
								bin_tag_output << "d" << bin;
								bin_tag=bin_tag_output.str();
								found=true;
							}
							++it_a;
							bin++;
						}
					}
					it->second.props.tags.insert(bin_tag);
				}
			}
		}
	}

	const bool inter_residue_summation_needed=(!inter_residue && inter_residue_after);

	if(summarize)
	{
		voronota::common::ContactValue summary;
		for(std::map<CRADsPair, std::map<CRADsPair, voronota::common::ContactValue>::iterator>::const_iterator it=selected_contacts.begin();it!=selected_contacts.end();++it)
		{
			summary.add(it->second->second);
			summary.graphics.clear();
		}
		std::cout << CRADsPair(CRAD::any(), CRAD::any()) << " " << summary << "\n";
	}
	else if(summarize_by_first)
	{
		std::map< CRADsPair, voronota::common::ContactValue > map_of_summaries;
		for(std::map<CRADsPair, std::map<CRADsPair, voronota::common::ContactValue>::iterator>::const_iterator it=selected_contacts.begin();it!=selected_contacts.end();++it)
		{
			CRAD crads[2]={it->first.a, it->first.b};
			for(int i=0;i<2;i++)
			{
				if(crads[i]!=CRAD::any() && voronota::common::MatchingUtilities::match_crad(crads[i], match_first, match_first_not) && (match_external_first.empty() || voronota::common::MatchingUtilities::match_crad_with_set_of_crads(false, crads[i], matchable_external_first_set_of_crads)))
				{
					voronota::common::ContactValue& cv=map_of_summaries[CRADsPair(crads[i], CRAD::any())];
					cv.add(it->second->second);
					cv.graphics.clear();
				}
			}
		}
		if(inter_residue_summation_needed)
		{
			sum_contacts_into_inter_residue_contacts(summing_exceptions, map_of_summaries);
		}
		voronota::auxiliaries::IOUtilities().write_map(map_of_summaries, std::cout);
	}
	else
	{
		if(!update_mode && !inter_residue_summation_needed)
		{
			for(std::map<CRADsPair, std::map<CRADsPair, voronota::common::ContactValue>::iterator>::const_iterator it=selected_contacts.begin();it!=selected_contacts.end();++it)
			{
				std::cout << it->first << " " << it->second->second << "\n";
			}
		}
		else
		{
			if(!update_mode)
			{
				std::map< CRADsPair, voronota::common::ContactValue > map_of_selected_contacts;
				for(std::map<CRADsPair, std::map<CRADsPair, voronota::common::ContactValue>::iterator>::const_iterator it=selected_contacts.begin();it!=selected_contacts.end();++it)
				{
					map_of_selected_contacts.insert(std::make_pair(it->first, it->second->second));
				}
				map_of_contacts=map_of_selected_contacts;
			}
			if(inter_residue_summation_needed)
			{
				sum_contacts_into_inter_residue_contacts(summing_exceptions, map_of_contacts);
			}
			voronota::auxiliaries::IOUtilities().write_map(map_of_contacts, std::cout);
		}
	}
}
