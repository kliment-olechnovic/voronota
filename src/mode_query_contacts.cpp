#include <iostream>
#include <stdexcept>
#include <fstream>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/atoms_io.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"
#include "auxiliaries/opengl_printer.h"

#include "modescommon/contact_value.h"
#include "modescommon/matching_utilities.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

std::set<CRADsPair> init_set_of_hbplus_crad_pairs(const std::string& hbplus_file_name, const bool inter_residue_hbplus_tags)
{
	std::set<CRADsPair> set_of_hbplus_crad_pairs;
	if(!hbplus_file_name.empty())
	{
		std::ifstream input_file(hbplus_file_name.c_str(), std::ios::in);
		auxiliaries::AtomsIO::HBPlusReader::Data hbplus_file_data=auxiliaries::AtomsIO::HBPlusReader::read_data_from_file_stream(input_file);
		if(!hbplus_file_data.hbplus_records.empty())
		{
			for(std::vector<auxiliaries::AtomsIO::HBPlusReader::HBPlusRecord>::const_iterator it=hbplus_file_data.hbplus_records.begin();it!=hbplus_file_data.hbplus_records.end();++it)
			{
				const auxiliaries::AtomsIO::HBPlusReader::ShortAtomDescriptor& a=it->first;
				const auxiliaries::AtomsIO::HBPlusReader::ShortAtomDescriptor& b=it->second;
				const CRADsPair crads_pair(CRAD(CRAD::null_num(), a.chainID, a.resSeq, a.resName, a.name, "", ""), CRAD(CRAD::null_num(), b.chainID, b.resSeq, b.resName, b.name, "", ""));
				set_of_hbplus_crad_pairs.insert(inter_residue_hbplus_tags ? CRADsPair(crads_pair.a.without_atom(), crads_pair.b.without_atom()) : crads_pair);
			}
		}
	}
	return set_of_hbplus_crad_pairs;
}

unsigned int calc_string_color_integer(const std::string& str)
{
	const long generator=123456789;
	const long limiter=0xFFFFFF;
	long hash=generator;
	for(std::size_t i=0;i<str.size();i++)
	{
		hash+=static_cast<long>(str[i]+1)*static_cast<long>(i+1)*generator;
	}
	return static_cast<unsigned int>(hash%limiter);
}

unsigned int calc_two_crads_color_integer(const CRAD& a, const CRAD& b)
{
	return calc_string_color_integer(a<b ? (a.str()+b.str()) : (b.str()+a.str()));
}

}

void query_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
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
	const bool inter_residue=poh.contains_option(pohw.describe_option("--inter-residue", "", "flag to convert input to inter-residue contacts"));
	const bool summarize=poh.contains_option(pohw.describe_option("--summarize", "", "flag to output only summary of contacts"));
	enabled_output_of_ContactValue_graphics()=poh.contains_option(pohw.describe_option("--preserve-graphics", "", "flag to preserve graphics in output"));
	const std::string drawing_for_pymol=poh.argument<std::string>(pohw.describe_option("--drawing-for-pymol", "string", "file path to output drawing as pymol script"), "");
	const std::string drawing_for_jmol=poh.argument<std::string>(pohw.describe_option("--drawing-for-jmol", "string", "file path to output drawing as jmol script"), "");
	const std::string drawing_for_scenejs=poh.argument<std::string>(pohw.describe_option("--drawing-for-scenejs", "string", "file path to output drawing as scenejs script"), "");
	const std::string drawing_name=poh.argument<std::string>(pohw.describe_option("--drawing-name", "string", "graphics object name for drawing output"), "contacts");
	const unsigned int drawing_color=poh.convert_hex_string_to_integer<unsigned int>(poh.argument<std::string>(pohw.describe_option("--drawing-color", "string", "color for drawing output, in hex format, white is 0xFFFFFF"), "0xFFFFFF"));
	const std::string drawing_adjunct_gradient=poh.argument<std::string>(pohw.describe_option("--drawing-adjunct-gradient", "string", "adjunct name to use for gradient-based coloring"), "");
	const bool drawing_reverse_gradient=poh.contains_option(pohw.describe_option("--drawing-reverse-gradient", "", "flag to use reversed gradient for drawing"));
	const bool drawing_random_colors=poh.contains_option(pohw.describe_option("--drawing-random-colors", "", "flag to use random color for each drawn contact"));
	const double drawing_alpha=poh.argument<double>(pohw.describe_option("--drawing-alpha", "number", "alpha opacity value for drawing output"), 1.0);
	const bool drawing_labels=poh.contains_option(pohw.describe_option("--drawing-labels", "", "flag to use labels in drawing if possible"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::map<CRADsPair, ContactValue> map_of_contacts;
	auxiliaries::IOUtilities().read_lines_to_map(std::cin, map_of_contacts);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	if(inter_residue)
	{
		std::map< CRADsPair, ContactValue > map_of_reduced_contacts;
		for(std::map< CRADsPair, ContactValue >::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const CRADsPair crads(it->first.a.without_atom(), it->first.b.without_atom());
			if(!(crads.a==crads.b))
			{
				map_of_reduced_contacts[crads].add(it->second);
			}
		}
		map_of_contacts=map_of_reduced_contacts;
	}

	std::map<CRADsPair, std::map<CRADsPair, ContactValue>::iterator> selected_contacts;
	{
		const std::set<CRAD> matchable_external_first_set_of_crads=auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRAD> >(match_external_first);
		const std::set<CRAD> matchable_external_second_set_of_crads=auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRAD> >(match_external_second);
		const std::set<CRADsPair> matchable_external_set_of_crad_pairs=auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRADsPair> >(match_external_pairs);

		for(std::map< CRADsPair, ContactValue >::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const CRADsPair& crads=it->first;
			const ContactValue& value=it->second;
			bool passed=false;
			if(
					value.area>=match_min_area && value.area<=match_max_area &&
					value.dist>=match_min_dist && value.dist<=match_max_dist &&
					(!no_solvent || !(crads.a==CRAD::solvent() || crads.b==CRAD::solvent())) &&
					(!no_same_chain || crads.a.chainID!=crads.b.chainID) &&
					CRAD::match_with_sequence_separation_interval(crads.a, crads.b, match_min_sequence_separation, match_max_sequence_separation, true) &&
					MatchingUtilities::match_set_of_tags(value.props.tags, match_tags, match_tags_not) &&
					MatchingUtilities::match_map_of_adjuncts(value.props.adjuncts, match_adjuncts, match_adjuncts_not) &&
					(match_external_pairs.empty() || MatchingUtilities::match_crads_pair_with_set_of_crads_pairs(crads, matchable_external_set_of_crad_pairs))
			)
			{
				const bool matched_first_second=(
						MatchingUtilities::match_crad(crads.a, match_first, match_first_not) &&
						MatchingUtilities::match_crad(crads.b, match_second, match_second_not) &&
						(match_external_first.empty() || MatchingUtilities::match_crad_with_set_of_crads(crads.a, matchable_external_first_set_of_crads)) &&
						(match_external_second.empty() || MatchingUtilities::match_crad_with_set_of_crads(crads.b, matchable_external_second_set_of_crads)));
				const bool matched_second_first=matched_first_second || (
						MatchingUtilities::match_crad(crads.b, match_first, match_first_not) &&
						MatchingUtilities::match_crad(crads.a, match_second, match_second_not) &&
						(match_external_first.empty() || MatchingUtilities::match_crad_with_set_of_crads(crads.b, matchable_external_first_set_of_crads)) &&
						(match_external_second.empty() || MatchingUtilities::match_crad_with_set_of_crads(crads.a, matchable_external_second_set_of_crads)));
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

	const bool update_mode=(drop_tags || drop_adjuncts || !set_tags.empty() || !set_adjuncts.empty() || !set_external_adjuncts.empty() || !set_hbplus_tags.empty() || !set_distance_bins_tags.empty());
	if(update_mode && !selected_contacts.empty())
	{
		const std::map<CRADsPair, double> map_of_external_adjunct_values=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRADsPair, double> >(set_external_adjuncts);
		const std::set<CRADsPair> set_of_hbplus_crad_pairs=init_set_of_hbplus_crad_pairs(set_hbplus_tags, inter_residue_hbplus_tags);
		const std::set<double> distance_thresholds_for_bins=(set_distance_bins_tags.empty() ? std::set<double>() : auxiliaries::IOUtilities(';').read_string_lines_to_set< std::set<double> >(set_distance_bins_tags));

		for(std::map<CRADsPair, std::map<CRADsPair, ContactValue>::iterator>::iterator selected_map_it=selected_contacts.begin();selected_map_it!=selected_contacts.end();++selected_map_it)
		{
			std::map<CRADsPair, ContactValue>::iterator it=selected_map_it->second;
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
					std::map< CRADsPair, double >::const_iterator adjunct_value_it=map_of_external_adjunct_values.find(it->first);
					if(adjunct_value_it!=map_of_external_adjunct_values.end())
					{
						it->second.props.adjuncts[set_external_adjuncts_name]=adjunct_value_it->second;
					}
				}
				if(!set_of_hbplus_crad_pairs.empty())
				{
					if(MatchingUtilities::match_crads_pair_with_set_of_crads_pairs(it->first, set_of_hbplus_crad_pairs))
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

	if(summarize)
	{
		ContactValue summary;
		for(std::map<CRADsPair, std::map<CRADsPair, ContactValue>::iterator>::const_iterator it=selected_contacts.begin();it!=selected_contacts.end();++it)
		{
			summary.add(it->second->second);
			summary.graphics.clear();
		}
		std::cout << CRADsPair(CRAD("any"), CRAD("any")) << " " << summary << "\n";
	}
	else
	{
		if(update_mode)
		{
			auxiliaries::IOUtilities().write_map(map_of_contacts, std::cout);
		}
		else
		{
			for(std::map<CRADsPair, std::map<CRADsPair, ContactValue>::iterator>::const_iterator it=selected_contacts.begin();it!=selected_contacts.end();++it)
			{
				std::cout << it->first << " " << it->second->second << "\n";
			}
		}
	}

	if(!selected_contacts.empty() && !(drawing_for_pymol.empty() && drawing_for_jmol.empty() && drawing_for_scenejs.empty()))
	{
		auxiliaries::OpenGLPrinter opengl_printer;
		opengl_printer.add_color(drawing_color);
		opengl_printer.add_alpha(drawing_alpha);
		for(std::map<CRADsPair, std::map<CRADsPair, ContactValue>::iterator>::const_iterator it=selected_contacts.begin();it!=selected_contacts.end();++it)
		{
			const CRADsPair& crads=it->first;
			const ContactValue& value=it->second->second;
			if(!value.graphics.empty())
			{
				if(drawing_labels)
				{
					std::string label=("("+crads.a.str()+")&("+crads.b.str()+")");
					std::replace(label.begin(), label.end(), '<', '[');
					std::replace(label.begin(), label.end(), '>', ']');
					opengl_printer.add_label(label);
				}

				if(!drawing_adjunct_gradient.empty())
				{
					if(value.props.adjuncts.count(drawing_adjunct_gradient)>0)
					{
						const double gradient_value=value.props.adjuncts.find(drawing_adjunct_gradient)->second;
						opengl_printer.add_color_from_blue_white_red_gradient(drawing_reverse_gradient ? (1.0-gradient_value) : gradient_value);
					}
					else
					{
						opengl_printer.add_color(drawing_color);
					}
				}
				else if(drawing_random_colors)
				{
					opengl_printer.add_color(calc_two_crads_color_integer(crads.a, crads.b));
				}

				opengl_printer.add(value.graphics);
			}
		}
		if(!drawing_for_pymol.empty())
		{
			std::ofstream foutput(drawing_for_pymol.c_str(), std::ios::out);
			if(foutput.good())
			{
				opengl_printer.print_pymol_script(drawing_name, true, foutput);
			}
		}
		if(!drawing_for_jmol.empty())
		{
			std::ofstream foutput(drawing_for_jmol.c_str(), std::ios::out);
			if(foutput.good())
			{
				opengl_printer.print_jmol_script(drawing_name, foutput);
			}
		}
		if(!drawing_for_scenejs.empty())
		{
			std::ofstream foutput(drawing_for_scenejs.c_str(), std::ios::out);
			if(foutput.good())
			{
				opengl_printer.print_scenejs_script(drawing_name, true, foutput);
			}
		}
	}
}
