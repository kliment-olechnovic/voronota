#include <iostream>
#include <stdexcept>
#include <fstream>

#include "auxiliaries/opengl_printer.h"

#include "modescommon/assert_options.h"
#include "modescommon/handle_contact.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef modescommon::ContactValue ContactValue;

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
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--match-first", "string", "selection for first contacting group"));
		list_of_option_descriptions.push_back(OD("--match-first-not", "string", "negative selection for first contacting group"));
		list_of_option_descriptions.push_back(OD("--match-second", "string", "selection for second contacting group"));
		list_of_option_descriptions.push_back(OD("--match-second-not", "string", "negative selection for second contacting group"));
		list_of_option_descriptions.push_back(OD("--match-min-seq-sep", "number", "minimum residue sequence separation"));
		list_of_option_descriptions.push_back(OD("--match-max-seq-sep", "number", "maximum residue sequence separation"));
		list_of_option_descriptions.push_back(OD("--match-min-area", "number", "minimum contact area"));
		list_of_option_descriptions.push_back(OD("--match-max-area", "number", "maximum contact area"));
		list_of_option_descriptions.push_back(OD("--match-min-dist", "number", "minimum distance"));
		list_of_option_descriptions.push_back(OD("--match-max-dist", "number", "maximum distance"));
		list_of_option_descriptions.push_back(OD("--match-tags", "string", "tags to match"));
		list_of_option_descriptions.push_back(OD("--match-tags-not", "string", "tags to not match"));
		list_of_option_descriptions.push_back(OD("--match-adjuncts", "string", "adjuncts intervals to match"));
		list_of_option_descriptions.push_back(OD("--match-adjuncts-not", "string", "adjuncts intervals to not match"));
		list_of_option_descriptions.push_back(OD("--match-external-annotations", "string", "file path to input matchable annotation pairs"));
		list_of_option_descriptions.push_back(OD("--no-solvent", "", "flag to not include solvent accessible areas"));
		list_of_option_descriptions.push_back(OD("--invert", "", "flag to invert selection"));
		list_of_option_descriptions.push_back(OD("--drop-tags", "", "flag to drop all tags from input"));
		list_of_option_descriptions.push_back(OD("--set-tags", "string", "set tags instead of filtering"));
		list_of_option_descriptions.push_back(OD("--drop-adjuncts", "", "flag to drop all adjuncts from input"));
		list_of_option_descriptions.push_back(OD("--set-adjuncts", "string", "set adjuncts instead of filtering"));
		list_of_option_descriptions.push_back(OD("--inter-residue", "", "flag to convert input to inter-residue contacts"));
		list_of_option_descriptions.push_back(OD("--preserve-graphics", "", "flag to preserve graphics in output"));
		list_of_option_descriptions.push_back(OD("--drawing-for-pymol", "string", "file path to output drawing as pymol script"));
		list_of_option_descriptions.push_back(OD("--drawing-for-jmol", "string", "file path to output drawing as jmol script"));
		list_of_option_descriptions.push_back(OD("--drawing-for-scenejs", "string", "file path to output drawing as scenejs script"));
		list_of_option_descriptions.push_back(OD("--drawing-name", "string", "graphics object name for drawing output"));
		list_of_option_descriptions.push_back(OD("--drawing-color", "string", "color for drawing output, in hex format, white is 0xFFFFFF"));
		list_of_option_descriptions.push_back(OD("--drawing-random-colors", "", "flag to use random color for each drawn contact"));
		list_of_option_descriptions.push_back(OD("--drawing-alpha", "number", "alpha opacity value for drawing output"));
		list_of_option_descriptions.push_back(OD("--drawing-labels", "", "flag to use labels in drawing if possible"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts [graphics]')\n";
			std::cerr << "stdout  ->  list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts [graphics]')\n";
			return;
		}
	}

	const std::string match_first=poh.argument<std::string>("--match-first", "");
	const std::string match_first_not=poh.argument<std::string>("--match-first-not", "");
	const std::string match_second=poh.argument<std::string>("--match-second", "");
	const std::string match_second_not=poh.argument<std::string>("--match-second-not", "");
	const int match_min_sequence_separation=poh.argument<int>("--match-min-seq-sep", CRAD::null_num());
	const int match_max_sequence_separation=poh.argument<int>("--match-max-seq-sep", CRAD::null_num());
	const double match_min_area=poh.argument<double>("--match-min-area", std::numeric_limits<double>::min());
	const double match_max_area=poh.argument<double>("--match-max-area", std::numeric_limits<double>::max());
	const double match_min_dist=poh.argument<double>("--match-min-dist", std::numeric_limits<double>::min());
	const double match_max_dist=poh.argument<double>("--match-max-dist", std::numeric_limits<double>::max());
	const std::string match_tags=poh.argument<std::string>("--match-tags", "");
	const std::string match_tags_not=poh.argument<std::string>("--match-tags-not", "");
	const std::string match_adjuncts=poh.argument<std::string>("--match-adjuncts", "");
	const std::string match_adjuncts_not=poh.argument<std::string>("--match-adjuncts-not", "");
	const std::string match_external_annotations=poh.argument<std::string>("--match-external-annotations", "");
	const bool no_solvent=poh.contains_option("--no-solvent");
	const bool invert=poh.contains_option("--invert");
	const bool drop_tags=poh.contains_option("--drop-tags");
	const std::string set_tags=poh.argument<std::string>("--set-tags", "");
	const bool drop_adjuncts=poh.contains_option("--drop-adjuncts");
	const std::string set_adjuncts=poh.argument<std::string>("--set-adjuncts", "");
	const bool inter_residue=poh.contains_option("--inter-residue");
	const std::string drawing_for_pymol=poh.argument<std::string>("--drawing-for-pymol", "");
	const std::string drawing_for_jmol=poh.argument<std::string>("--drawing-for-jmol", "");
	const std::string drawing_for_scenejs=poh.argument<std::string>("--drawing-for-scenejs", "");
	const std::string drawing_name=poh.argument<std::string>("--drawing-name", "contacts");
	const unsigned int drawing_color=auxiliaries::ProgramOptionsHandler::convert_hex_string_to_integer<unsigned int>(poh.argument<std::string>("--drawing-color", "0xFFFFFF"));
	const bool drawing_random_colors=poh.contains_option("--drawing-random-colors");
	const double drawing_alpha=poh.argument<double>("--drawing-alpha", 1.0);
	const bool drawing_labels=poh.contains_option("--drawing-labels");
	const bool preserve_graphics=poh.contains_option("--preserve-graphics");

	std::map< std::pair<CRAD, CRAD>, ContactValue > map_of_contacts;
	auxiliaries::read_lines_to_container(std::cin, modescommon::add_contact_record_from_stream_to_map, map_of_contacts);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	if(drop_tags || drop_adjuncts)
	{
		for(std::map< std::pair<CRAD, CRAD>, ContactValue >::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			if(drop_tags)
			{
				it->second.tags.clear();
			}
			if(drop_adjuncts)
			{
				it->second.adjuncts.clear();
			}
		}
	}

	if(inter_residue)
	{
		std::map< std::pair<CRAD, CRAD>, ContactValue > map_of_reduced_contacts;
		for(std::map< std::pair<CRAD, CRAD>, ContactValue >::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const std::pair<CRAD, CRAD> crads(it->first.first.without_atom(), it->first.second.without_atom());
			if(!(crads.second==crads.first))
			{
				map_of_reduced_contacts[modescommon::refine_pair_by_ordering(crads)].add(it->second);
			}
		}
		map_of_contacts=map_of_reduced_contacts;
	}

	std::set< std::pair<CRAD, CRAD> > matchable_external_set_of_crad_pairs;
	if(!match_external_annotations.empty())
	{
		std::ifstream input_file(match_external_annotations.c_str(), std::ios::in);
		auxiliaries::read_lines_to_container(input_file, modescommon::add_chain_residue_atom_descriptors_pair_from_stream_to_set, matchable_external_set_of_crad_pairs);
	}

	std::map< std::pair<CRAD, CRAD>, ContactValue > output_map_of_contacts;

	for(std::map< std::pair<CRAD, CRAD>, ContactValue >::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		const std::pair<CRAD, CRAD>& crads=it->first;
		const ContactValue& value=it->second;
		bool passed=false;
		if(
				value.area>=match_min_area && value.area<=match_max_area &&
				value.dist>=match_min_dist && value.dist<=match_max_dist &&
				(!no_solvent || !(crads.first==CRAD::solvent() || crads.second==CRAD::solvent())) &&
				CRAD::match_with_sequence_separation_interval(crads.first, crads.second, match_min_sequence_separation, match_max_sequence_separation, true) &&
				modescommon::match_set_of_tags(value.tags, match_tags, match_tags_not) &&
				modescommon::match_map_of_adjuncts(value.adjuncts, match_adjuncts, match_adjuncts_not) &&
				(matchable_external_set_of_crad_pairs.empty() || modescommon::match_chain_residue_atom_descriptors_pair_with_set_of_descriptors_pairs(crads, matchable_external_set_of_crad_pairs))
		)
		{
			const bool matched_first_second=(modescommon::match_chain_residue_atom_descriptor(crads.first, match_first, match_first_not) && modescommon::match_chain_residue_atom_descriptor(crads.second, match_second, match_second_not));
			const bool matched_second_first=(modescommon::match_chain_residue_atom_descriptor(crads.second, match_first, match_first_not) && modescommon::match_chain_residue_atom_descriptor(crads.first, match_second, match_second_not));
			passed=(matched_first_second || matched_second_first);
			if(passed && !invert)
			{
				output_map_of_contacts[modescommon::refine_pair(crads, !matched_first_second)]=value;
			}
		}
		if(!passed && invert)
		{
			output_map_of_contacts[crads]=value;
		}
	}

	if(!set_tags.empty() || !set_adjuncts.empty())
	{
		for(std::map< std::pair<CRAD, CRAD>, ContactValue >::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			if(output_map_of_contacts.count(crads)>0 || output_map_of_contacts.count(modescommon::refine_pair(crads, true))>0)
			{
				modescommon::update_set_of_tags(it->second.tags, set_tags);
				modescommon::update_map_of_adjuncts(it->second.adjuncts, set_adjuncts);
			}
			modescommon::print_contact_record(it->first, it->second, preserve_graphics, std::cout);
		}
	}
	else
	{
		for(std::map< std::pair<CRAD, CRAD>, ContactValue >::const_iterator it=output_map_of_contacts.begin();it!=output_map_of_contacts.end();++it)
		{
			modescommon::print_contact_record(it->first, it->second, preserve_graphics, std::cout);
		}
	}

	if(!output_map_of_contacts.empty() && !(drawing_for_pymol.empty() && drawing_for_jmol.empty() && drawing_for_scenejs.empty()))
	{
		auxiliaries::OpenGLPrinter opengl_printer;
		opengl_printer.add_color(drawing_color);
		opengl_printer.add_alpha(drawing_alpha);
		for(std::map< std::pair<CRAD, CRAD>, ContactValue >::const_iterator it=output_map_of_contacts.begin();it!=output_map_of_contacts.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			const ContactValue& value=it->second;
			if(!value.graphics.empty())
			{
				if(drawing_labels)
				{
					opengl_printer.add_label(crads.first.str()+"<->"+crads.second.str());
				}
				if(drawing_random_colors)
				{
					opengl_printer.add_color(calc_two_crads_color_integer(crads.first, crads.second));
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
