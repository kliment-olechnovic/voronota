#include <iostream>
#include <stdexcept>
#include <fstream>
#include <set>

#include "auxiliaries/opengl_printer.h"

#include "modescommon_assert_options.h"
#include "modescommon_handle_contact.h"

namespace
{

typedef modescommon::contact::Comment Comment;
typedef modescommon::contact::ContactValue ContactValue;

bool match_comment_with_member_descriptors(const Comment& comment, const std::vector<std::string>& positive_descriptors, const std::vector<std::string>& negative_descriptors)
{
	for(std::size_t i=0;i<positive_descriptors.size();i++)
	{
		if(!Comment::match_with_member_descriptor(comment, positive_descriptors[i]))
		{
			return false;
		}
	}
	for(std::size_t i=0;i<negative_descriptors.size();i++)
	{
		if(Comment::match_with_member_descriptor(comment, negative_descriptors[i]))
		{
			return false;
		}
	}
	return true;
}

bool match_two_comments_with_set_of_name_pairs(const Comment& a, const Comment& b, const std::set< std::pair<Comment, Comment> >& set_of_name_pairs)
{
	if(set_of_name_pairs.count(std::make_pair(a, b))>0 || set_of_name_pairs.count(std::make_pair(b, a))>0)
	{
		return true;
	}
	for(std::set< std::pair<Comment, Comment> >::const_iterator it=set_of_name_pairs.begin();it!=set_of_name_pairs.end();++it)
	{
		if((a.contains(it->first) && b.contains(it->second)) || (b.contains(it->first) && a.contains(it->second)))
		{
			return true;
		}
	}
	return false;
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

unsigned int calc_two_comments_color_integer(const Comment& a, const Comment& b)
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
		list_of_option_descriptions.push_back(OD("--match-both", "string", "selection for first and second contacting groups"));
		list_of_option_descriptions.push_back(OD("--match-both-not", "string", "negative selection for first and second contacting groups"));
		list_of_option_descriptions.push_back(OD("--match-min-seq-sep", "number", "minimum residue sequence separation"));
		list_of_option_descriptions.push_back(OD("--match-max-seq-sep", "number", "maximum residue sequence separation"));
		list_of_option_descriptions.push_back(OD("--match-min-area", "number", "minimum contact area"));
		list_of_option_descriptions.push_back(OD("--match-max-area", "number", "maximum contact area"));
		list_of_option_descriptions.push_back(OD("--match-min-dist", "number", "minimum distance"));
		list_of_option_descriptions.push_back(OD("--match-max-dist", "number", "maximum distance"));
		list_of_option_descriptions.push_back(OD("--match-tags", "string", "comma-separated list of tags to match"));
		list_of_option_descriptions.push_back(OD("--match-tags-not", "string", "comma-separated list of tags to not match"));
		list_of_option_descriptions.push_back(OD("--match-external-annotations", "string", "file path to input matchable annotation pairs"));
		list_of_option_descriptions.push_back(OD("--no-solvent", "", "flag to not include solvent accessible areas"));
		list_of_option_descriptions.push_back(OD("--invert", "", "flag to invert selection"));
		list_of_option_descriptions.push_back(OD("--drop-tags", "", "flag to drop all tags from input"));
		list_of_option_descriptions.push_back(OD("--add-tag", "string", "add tag instead of filtering"));
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
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 area distance tags [graphics]')\n";
			std::cerr << "stdout  ->  list of contacts (line format: 'annotation1 annotation2 area distance tags [graphics]')\n";
			return;
		}
	}

	const char selection_list_sep='&';
	const std::vector<std::string> match_first=poh.argument_vector<std::string>("--match-first", selection_list_sep);
	const std::vector<std::string> match_first_not=poh.argument_vector<std::string>("--match-first-not", selection_list_sep);
	const std::vector<std::string> match_second=poh.argument_vector<std::string>("--match-second", selection_list_sep);
	const std::vector<std::string> match_second_not=poh.argument_vector<std::string>("--match-second-not", selection_list_sep);
	const std::vector<std::string> match_both=poh.argument_vector<std::string>("--match-both", selection_list_sep);
	const std::vector<std::string> match_both_not=poh.argument_vector<std::string>("--match-both-not", selection_list_sep);
	const int match_min_sequence_separation=poh.argument<int>("--match-min-seq-sep", Comment::null_num());
	const int match_max_sequence_separation=poh.argument<int>("--match-max-seq-sep", Comment::null_num());
	const double match_min_area=poh.argument<double>("--match-min-area", std::numeric_limits<double>::min());
	const double match_max_area=poh.argument<double>("--match-max-area", std::numeric_limits<double>::max());
	const double match_min_dist=poh.argument<double>("--match-min-dist", std::numeric_limits<double>::min());
	const double match_max_dist=poh.argument<double>("--match-max-dist", std::numeric_limits<double>::max());
	const std::vector<std::string> match_tags=poh.argument_vector<std::string>("--match-tags", ',');
	const std::vector<std::string> match_tags_not=poh.argument_vector<std::string>("--match-tags-not", ',');
	const std::string match_external_annotations=poh.argument<std::string>("--match-external-annotations", "");
	const bool no_solvent=poh.contains_option("--no-solvent");
	const bool invert=poh.contains_option("--invert");
	const bool drop_tags=poh.contains_option("--drop-tags");
	const std::string add_tag=poh.argument<std::string>("--add-tag", "");
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

	std::map< std::pair<Comment, Comment>, ContactValue > map_of_contacts;
	auxiliaries::read_lines_to_container(std::cin, "", modescommon::contact::add_contacts_record_from_stream_to_map, map_of_contacts);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	if(drop_tags)
	{
		for(std::map< std::pair<Comment, Comment>, ContactValue >::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			it->second.tags.clear();
		}
	}

	if(inter_residue)
	{
		std::map< std::pair<Comment, Comment>, ContactValue > map_of_reduced_contacts;
		for(std::map< std::pair<Comment, Comment>, ContactValue >::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const std::pair<Comment, Comment> comments(it->first.first.without_atom(), it->first.second.without_atom());
			if(!(comments.second==comments.first))
			{
				map_of_reduced_contacts[modescommon::contact::refine_pair(comments, map_of_reduced_contacts.count(modescommon::contact::refine_pair(comments, true))>0)].add(it->second);
			}
		}
		map_of_contacts=map_of_reduced_contacts;
	}

	std::set< std::pair<Comment, Comment> > matchable_set_of_name_pairs;
	if(!match_external_annotations.empty())
	{
		std::ifstream input_file(match_external_annotations.c_str(), std::ios::in);
		auxiliaries::read_lines_to_container(input_file, "", modescommon::contact::add_contacts_name_pair_from_stream_to_set, matchable_set_of_name_pairs);
	}

	std::map< std::pair<Comment, Comment>, ContactValue > output_map_of_contacts;

	for(std::map< std::pair<Comment, Comment>, ContactValue >::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		const std::pair<Comment, Comment>& comments=it->first;
		const ContactValue& value=it->second;
		bool passed=false;
		if(
				value.area>=match_min_area && value.area<=match_max_area &&
				value.dist>=match_min_dist && value.dist<=match_max_dist &&
				(!no_solvent || !(comments.first==Comment::solvent() || comments.second==Comment::solvent())) &&
				Comment::match_with_sequence_separation_interval(comments.first, comments.second, match_min_sequence_separation, match_max_sequence_separation, true) &&
				match_comment_with_member_descriptors(comments.first, match_both, match_both_not) &&
				match_comment_with_member_descriptors(comments.second, match_both, match_both_not) &&
				(match_tags.empty() || std::find_first_of(value.tags.begin(), value.tags.end(), match_tags.begin(), match_tags.end())!=value.tags.end()) &&
				(match_tags_not.empty() || std::find_first_of(value.tags.begin(), value.tags.end(), match_tags_not.begin(), match_tags_not.end())==value.tags.end()) &&
				(matchable_set_of_name_pairs.empty() || match_two_comments_with_set_of_name_pairs(comments.first, comments.second, matchable_set_of_name_pairs))
		)
		{
			const bool matched_first_second=(match_comment_with_member_descriptors(comments.first, match_first, match_first_not) && match_comment_with_member_descriptors(comments.second, match_second, match_second_not));
			const bool matched_second_first=(match_comment_with_member_descriptors(comments.second, match_first, match_first_not) && match_comment_with_member_descriptors(comments.first, match_second, match_second_not));
			passed=(matched_first_second || matched_second_first);
			if(passed && !invert)
			{
				output_map_of_contacts[modescommon::contact::refine_pair(comments, !matched_first_second)]=value;
			}
		}
		if(!passed && invert)
		{
			output_map_of_contacts[comments]=value;
		}
	}

	if(!add_tag.empty())
	{
		for(std::map< std::pair<Comment, Comment>, ContactValue >::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const std::pair<Comment, Comment>& comments=it->first;
			if(output_map_of_contacts.count(comments)>0 || output_map_of_contacts.count(modescommon::contact::refine_pair(comments, true))>0)
			{
				it->second.tag(add_tag);
			}
			modescommon::contact::print_contact_record(it->first, it->second, preserve_graphics, std::cout);
		}
	}
	else
	{
		for(std::map< std::pair<Comment, Comment>, ContactValue >::const_iterator it=output_map_of_contacts.begin();it!=output_map_of_contacts.end();++it)
		{
			modescommon::contact::print_contact_record(it->first, it->second, preserve_graphics, std::cout);
		}

		if(!output_map_of_contacts.empty() && !(drawing_for_pymol.empty() && drawing_for_jmol.empty() && drawing_for_scenejs.empty()))
		{
			auxiliaries::OpenGLPrinter opengl_printer;
			opengl_printer.add_color(drawing_color);
			opengl_printer.add_alpha(drawing_alpha);
			for(std::map< std::pair<Comment, Comment>, ContactValue >::const_iterator it=output_map_of_contacts.begin();it!=output_map_of_contacts.end();++it)
			{
				const std::pair<Comment, Comment>& comments=it->first;
				const ContactValue& value=it->second;
				if(!value.graphics.empty())
				{
					if(drawing_labels)
					{
						opengl_printer.add_label(comments.first.str()+"<->"+comments.second.str());
					}
					if(drawing_random_colors)
					{
						opengl_printer.add_color(calc_two_comments_color_integer(comments.first, comments.second));
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
}
