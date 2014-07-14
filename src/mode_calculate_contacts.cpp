#include <iostream>
#include <utility>
#include <stdexcept>
#include <fstream>

#include "apollota/constrained_contacts_construction.h"

#include "auxiliaries/chain_residue_atom_comment.h"
#include "auxiliaries/opengl_printer.h"

#include "modes_commons.h"

namespace
{

typedef auxiliaries::ChainResidueAtomComment Comment;

bool add_contacts_record_from_stream_to_map(std::istream& input, std::map< std::pair<Comment, Comment>, std::pair<double, std::string> >& map_of_records)
{
	std::pair<std::string, std::string> comment_strings;
	std::pair<double, std::string> value(0.0, std::string());
	input >> comment_strings.first >> comment_strings.second >> value.first;
	if(input.good())
	{
		std::getline(input, value.second);
	}
	if(!input.fail() && !comment_strings.first.empty() && !comment_strings.second.empty())
	{
		std::pair<Comment, Comment> comments(Comment::from_str(comment_strings.first), Comment::from_str(comment_strings.second));
		if(comments.first.valid() && comments.second.valid())
		{
			if(comments.second<comments.first)
			{
				std::swap(comments.first, comments.second);
			}
			map_of_records[comments]=value;
			return true;
		}
	}
	return false;
}

std::string draw_iter_atom_contact(
		const std::vector<apollota::SimpleSphere>& spheres,
		const apollota::Triangulation::VerticesVector& vertices_vector,
		const apollota::TriangulationQueries::PairsMap& pairs_vertices,
		const std::size_t a_id,
		const std::size_t b_id,
		const double probe,
		const double step,
		const int projections)
{
	auxiliaries::OpenGLPrinter opengl_printer;
	if(a_id<spheres.size() && b_id<spheres.size())
	{
		apollota::TriangulationQueries::PairsMap::const_iterator pairs_vertices_it=pairs_vertices.find(apollota::Pair(a_id, b_id));
		if(pairs_vertices_it!=pairs_vertices.end())
		{
			const std::list<apollota::ConstrainedContactContour::Contour> contours=apollota::ConstrainedContactContour::construct_contact_contours(
					spheres, vertices_vector, pairs_vertices_it->second, a_id, b_id, probe, step, projections);
			for(std::list<apollota::ConstrainedContactContour::Contour>::const_iterator contours_it=contours.begin();contours_it!=contours.end();++contours_it)
			{
				const apollota::ConstrainedContactContour::Contour& contour=(*contours_it);
				const std::vector<apollota::SimplePoint> outline=apollota::ConstrainedContactContour::collect_points_from_contour(contour);
				opengl_printer.add_triangle_fan(
						apollota::HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(apollota::mass_center<apollota::SimplePoint>(outline.begin(), outline.end()), spheres[a_id], spheres[b_id]),
						outline,
						apollota::sub_of_points<apollota::SimplePoint>(spheres[b_id], spheres[a_id]).unit());
			}
		}
	}
	return opengl_printer.str();
}

std::string draw_solvent_contact(
		const std::vector<apollota::SimpleSphere>& spheres,
		const apollota::Triangulation::VerticesVector& vertices_vector,
		const apollota::TriangulationQueries::IDsMap& ids_vertices,
		const std::size_t a_id,
		const double probe,
		const apollota::SubdividedIcosahedron& sih)
{
	auxiliaries::OpenGLPrinter opengl_printer;
	if(a_id<spheres.size())
	{
		apollota::TriangulationQueries::IDsMap::const_iterator ids_vertices_it=ids_vertices.find(a_id);
		if(ids_vertices_it!=ids_vertices.end())
		{
			const apollota::ConstrainedContactRemainder::Remainder remainder=apollota::ConstrainedContactRemainder::construct_contact_remainder(
					spheres, vertices_vector, ids_vertices_it->second, a_id, probe, sih);
			for(apollota::ConstrainedContactRemainder::Remainder::const_iterator remainder_it=remainder.begin();remainder_it!=remainder.end();++remainder_it)
			{
				std::vector<apollota::SimplePoint> ts(3);
				std::vector<apollota::SimplePoint> ns(3);
				for(int i=0;i<3;i++)
				{
					ts[i]=remainder_it->p[i];
					ns[i]=apollota::sub_of_points<apollota::SimplePoint>(ts[i], spheres[a_id]).unit();
				}
				opengl_printer.add_triangle_strip(ts, ns);
			}
		}
	}
	return opengl_printer.str();
}

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

}

void calculate_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--print-log", "", "flag to print log of calculations"));
		list_of_option_descriptions.push_back(OD("--annotate", "", "flag to annotate contacts using balls comments"));
		list_of_option_descriptions.push_back(OD("--probe", "number", "probe radius"));
		list_of_option_descriptions.push_back(OD("--exclude-hidden-balls", "", "flag to exclude hidden input balls"));
		list_of_option_descriptions.push_back(OD("--step", "number", "curve step length"));
		list_of_option_descriptions.push_back(OD("--projections", "number", "curve optimization depth"));
		list_of_option_descriptions.push_back(OD("--sih-depth", "number", "spherical surface optimization depth"));
		list_of_option_descriptions.push_back(OD("--max-dist", "number", "maximal distance to record, may exceed probe diameter"));
		list_of_option_descriptions.push_back(OD("--draw", "", "flag to output graphics for annotated contacts"));
		if(!modes_commons::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of balls\n";
			std::cerr << "              (default line format: 'x y z r # comments')\n";
			std::cerr << "              (annotated line format: 'x y z r # atomSerial chainID resSeq resName atomName [altLoc iCode]')\n";
			std::cerr << "stdout  ->  list of contacts\n";
			std::cerr << "              (default line format: 'b1 b2 distance area')\n";
			std::cerr << "              (annotated line format: 'annotation1 annotation2 area [graphics]')\n";
			return;
		}
	}

	const bool print_log=poh.contains_option("--print-log");
	const bool annotate=poh.contains_option("--annotate");
	const bool exclude_hidden_balls=poh.contains_option("--exclude-hidden-balls");
	const double probe=std::max(0.01, std::min(14.0, poh.argument<double>("--probe", 1.4)));
	const double step=std::max(0.05, std::min(0.5, poh.argument<double>("--step", 0.2)));
	const int projections=std::max(1, std::min(10, poh.argument<int>("--projections", 5)));
	const int sih_depth=std::max(1, std::min(5, poh.argument<int>("--sih-depth", 3)));
	const double max_dist=std::max(0.0, std::min(14.0*4.0, poh.argument<double>("--max-dist", probe*4.0)));
	const bool draw=poh.contains_option("--draw");

	std::vector<apollota::SimpleSphere> spheres;
	std::vector<Comment> input_spheres_comments;
	if(annotate)
	{
		std::pair< std::vector<apollota::SimpleSphere>*, std::vector<Comment>* > spheres_with_comments(&spheres, &input_spheres_comments);
		auxiliaries::read_lines_to_container(std::cin, "", modes_commons::add_sphere_and_comments_from_stream_to_vectors<apollota::SimpleSphere, Comment>, spheres_with_comments);
		if(spheres.size()!=input_spheres_comments.size())
		{
			throw std::runtime_error("Number of comments does not match number of spheres.");
		}
	}
	else
	{
		auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
	}
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	const std::size_t input_spheres_count=spheres.size();
	const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::ConstrainedContactsConstruction::construct_artificial_boundary(spheres, probe*2.0);
	spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, exclude_hidden_balls, false);
	const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);
	const apollota::TriangulationQueries::IDsMap ids_map=apollota::TriangulationQueries::collect_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map);

	std::map<apollota::Pair, std::pair<double, double> > interactions_map;
	for(apollota::TriangulationQueries::IDsMap::const_iterator it=ids_map.begin();it!=ids_map.end();++it)
	{
		const std::size_t a_id=it->first;
		if(a_id<input_spheres_count)
		{
			for(std::set<std::size_t>::const_iterator jt=it->second.begin();jt!=it->second.end();++jt)
			{
				const std::size_t b_id=(*jt);
				if(a_id<b_id && b_id<input_spheres_count)
				{
					const double dist=apollota::minimal_distance_from_sphere_to_sphere(spheres[a_id], spheres[b_id]);
					if(dist<=max_dist)
					{
						interactions_map[apollota::Pair(a_id, b_id)].first=dist;
					}
				}
			}
		}
	}

	const std::map<apollota::Pair, double> constrained_contacts=apollota::ConstrainedContactsConstruction::construct_contacts(spheres, vertices_vector, probe, step, projections);
	for(std::map<apollota::Pair, double>::const_iterator it=constrained_contacts.begin();it!=constrained_contacts.end();++it)
	{
		if(it->first.get(0)<input_spheres_count && it->first.get(1)<input_spheres_count)
		{
			interactions_map[it->first].second=it->second;
		}
	}

	const std::map<std::size_t, double> constrained_contact_remainders=apollota::ConstrainedContactsConstruction::construct_contact_remainders(spheres, vertices_vector, probe, sih_depth);
	for(std::map<std::size_t, double>::const_iterator it=constrained_contact_remainders.begin();it!=constrained_contact_remainders.end();++it)
	{
		if(it->first<input_spheres_count)
		{
			interactions_map[apollota::Pair(it->first, it->first)]=std::make_pair(-(spheres[it->first].r*2.0), it->second);
		}
	}

	if(!input_spheres_comments.empty())
	{
		const apollota::TriangulationQueries::PairsMap pairs_vertices=(draw ? apollota::TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector) : apollota::TriangulationQueries::PairsMap());
		const apollota::TriangulationQueries::IDsMap ids_vertices=(draw ? apollota::TriangulationQueries::collect_vertices_map_from_vertices_vector(vertices_vector) : apollota::TriangulationQueries::IDsMap());
		const apollota::SubdividedIcosahedron sih(draw ? sih_depth : 0);

		for(std::map<apollota::Pair, std::pair<double, double> >::const_iterator it=interactions_map.begin();it!=interactions_map.end();++it)
		{
			const double area=it->second.second;
			if(area>0.0)
			{
				const std::size_t a_id=it->first.get(0);
				const std::size_t b_id=it->first.get(1);
				if(!(a_id!=b_id && input_spheres_comments[a_id].without_atom()==input_spheres_comments[b_id].without_atom()))
				{
					const bool reverse=input_spheres_comments[b_id]<input_spheres_comments[a_id];
					std::cout << input_spheres_comments[reverse ? b_id : a_id].str() << " " << (a_id==b_id ? Comment::solvent().str() : input_spheres_comments[reverse ? a_id : b_id].str()) << " " << area;
					if(draw)
					{
						std::cout << " " << (a_id==b_id ?
								draw_solvent_contact(spheres, vertices_vector, ids_vertices, a_id, probe, sih) :
								draw_iter_atom_contact(spheres, vertices_vector, pairs_vertices, a_id, b_id, probe, step, projections));
					}
					std::cout << "\n";
				}
			}
		}
	}
	else
	{
		for(std::map<apollota::Pair, std::pair<double, double> >::const_iterator it=interactions_map.begin();it!=interactions_map.end();++it)
		{
			std::cout << it->first.get(0) << " " << it->first.get(1) << " " << it->second.first << " " << it->second.second<< "\n";
		}
	}

	if(print_log)
	{
		std::clog << "balls " << input_spheres_count << "\n";
		std::clog << "probe " << probe << "\n";
		std::clog << "step " << step << "\n";
		std::clog << "projections " << projections << "\n";
		std::clog << "sih_depth " << sih_depth << "\n";
		std::clog << "max_dist " << max_dist << "\n";
		std::clog << "output_pairs " << interactions_map.size() << "\n";
		std::clog << "contacts_count_internal " << constrained_contacts.size() << "\n";
		std::clog << "contacts_count_external " << constrained_contact_remainders.size() << "\n";

		double contacts_sum_internal=0.0;
		for(std::map<apollota::Pair, double>::const_iterator it=constrained_contacts.begin();it!=constrained_contacts.end();++it)
		{
			if(it->first.get(0)<input_spheres_count && it->first.get(1)<input_spheres_count)
			{
				contacts_sum_internal+=it->second;
			}
		}

		double contacts_sum_external=0.0;
		for(std::map<std::size_t, double>::const_iterator it=constrained_contact_remainders.begin();it!=constrained_contact_remainders.end();++it)
		{
			if(it->first<input_spheres_count)
			{
				contacts_sum_external+=it->second;
			}
		}

		std::clog << "contacts_sum_internal " << contacts_sum_internal << "\n";
		std::clog << "contacts_sum_external " << contacts_sum_external << "\n";
	}
}

void calculate_contacts_query(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--inter-residue", "", "flag to convert to inter-residue contacts"));
		list_of_option_descriptions.push_back(OD("--inter-chain", "", "flag to convert to inter-chain contacts"));
		list_of_option_descriptions.push_back(OD("--match-first", "string", "selection for first contacting group"));
		list_of_option_descriptions.push_back(OD("--match-first-not", "string", "negative selection for first contacting group"));
		list_of_option_descriptions.push_back(OD("--match-second", "string", "selection for second contacting group"));
		list_of_option_descriptions.push_back(OD("--match-second-not", "string", "negative selection for second contacting group"));
		list_of_option_descriptions.push_back(OD("--match-min-seq-sep", "number", "minimum residue sequence separation"));
		list_of_option_descriptions.push_back(OD("--match-max-seq-sep", "number", "maximum residue sequence separation"));
		list_of_option_descriptions.push_back(OD("--drawing-for-pymol", "string", "file path to output drawing as pymol script"));
		list_of_option_descriptions.push_back(OD("--drawing-for-jmol", "string", "file path to output drawing as jmol script"));
		list_of_option_descriptions.push_back(OD("--drawing-name", "string", "graphics object name for drawing output"));
		list_of_option_descriptions.push_back(OD("--drawing-color", "string", "color for drawing output, in hex format, white is 0xFFFFFF"));
		list_of_option_descriptions.push_back(OD("--drawing-random-colors", "", "flag to use random color for each drawn contact"));
		list_of_option_descriptions.push_back(OD("--drawing-alpha", "number", "alpha opacity value for drawing output"));
		list_of_option_descriptions.push_back(OD("--preserve-graphics", "", "flag to preserve graphics in output"));
		if(!modes_commons::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 area [graphics]')\n";
			std::cerr << "stdout  ->  list of contacts (line format: 'annotation1 annotation2 area [graphics]')\n";
			return;
		}
	}

	const bool inter_residue=poh.contains_option("--inter-residue");
	const bool inter_chain=poh.contains_option("--inter-chain");
	const char selection_list_sep='&';
	const std::vector<std::string> match_first=poh.argument_vector<std::string>("--match-first", selection_list_sep);
	const std::vector<std::string> match_first_not=poh.argument_vector<std::string>("--match-first-not", selection_list_sep);
	const std::vector<std::string> match_second=poh.argument_vector<std::string>("--match-second", selection_list_sep);
	const std::vector<std::string> match_second_not=poh.argument_vector<std::string>("--match-second-not", selection_list_sep);
	const int match_min_sequence_separation=poh.argument<int>("--match-min-seq-sep", Comment::null_num());
	const int match_max_sequence_separation=poh.argument<int>("--match-max-seq-sep", Comment::null_num());
	const std::string drawing_for_pymol=poh.argument<std::string>("--drawing-for-pymol", "");
	const std::string drawing_for_jmol=poh.argument<std::string>("--drawing-for-jmol", "");
	const bool drawing=!(drawing_for_pymol.empty() && drawing_for_jmol.empty());
	const std::string drawing_name=poh.argument<std::string>("--drawing-name", "contacts");
	const unsigned int drawing_color=auxiliaries::ProgramOptionsHandler::convert_hex_string_to_integer<unsigned int>(poh.argument<std::string>("--drawing-color", "0xFFFFFF"));
	const bool drawing_random_colors=poh.contains_option("--drawing-random-colors");
	const double drawing_alpha=poh.argument<double>("--drawing-alpha", 1.0);
	const bool preserve_graphics=poh.contains_option("--preserve-graphics");

	std::map< std::pair<Comment, Comment>, std::pair<double, std::string> > map_of_contacts;
	auxiliaries::read_lines_to_container(std::cin, "", add_contacts_record_from_stream_to_map, map_of_contacts);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	if(inter_chain || inter_residue)
	{
		std::map< std::pair<Comment, Comment>, std::pair<double, std::string> > map_of_reduced_contacts;
		for(std::map< std::pair<Comment, Comment>, std::pair<double, std::string> >::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			std::pair<Comment, Comment> comments=it->first;
			if(inter_chain)
			{
				comments.first=comments.first.without_residue();
				comments.second=comments.second.without_residue();
			}
			else if(inter_residue)
			{
				comments.first=comments.first.without_atom();
				comments.second=comments.second.without_atom();
			}
			if(!(comments.second==comments.first))
			{
				if(comments.second<comments.first)
				{
					std::swap(comments.first, comments.second);
				}
				std::pair<double, std::string>& value=map_of_reduced_contacts[comments];
				value.first+=it->second.first;
				value.second+=it->second.second;
			}
		}
		map_of_contacts=map_of_reduced_contacts;
	}

	std::map< std::pair<Comment, Comment>, std::pair<double, std::string> > output_map_of_contacts;

	auxiliaries::OpenGLPrinter opengl_printer;
	bool opengl_printer_filled=false;
	if(drawing)
	{
		opengl_printer.add_color(drawing_color);
		opengl_printer.add_alpha(drawing_alpha);
	}

	for(std::map< std::pair<Comment, Comment>, std::pair<double, std::string> >::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		const std::pair<Comment, Comment>& comments=it->first;
		if(Comment::match_with_sequence_separation_interval(comments.first, comments.second, match_min_sequence_separation, match_max_sequence_separation))
		{
			const bool matched_first_second=(match_comment_with_member_descriptors(comments.first, match_first, match_first_not) && match_comment_with_member_descriptors(comments.second, match_second, match_second_not));
			const bool matched_second_first=(match_comment_with_member_descriptors(comments.second, match_first, match_first_not) && match_comment_with_member_descriptors(comments.first, match_second, match_second_not));
			if(matched_first_second || matched_second_first)
			{
				const std::pair<double, std::string>& value=it->second;

				if(matched_first_second)
				{
					output_map_of_contacts[comments]=value;
				}
				else
				{
					output_map_of_contacts[std::make_pair(comments.second, comments.first)]=value;
				}

				if(drawing && !value.second.empty())
				{
					if(drawing_random_colors)
					{
						opengl_printer.add_color(calc_string_color_integer(comments.first.str()+comments.second.str()));
					}
					opengl_printer.add(value.second);
					opengl_printer_filled=true;
				}
			}
		}
	}

	{
		const std::size_t default_column_width=std::cout.width();
		std::pair<std::size_t, std::size_t> column_width(default_column_width, default_column_width);
		for(std::map< std::pair<Comment, Comment>, std::pair<double, std::string> >::const_iterator it=output_map_of_contacts.begin();it!=output_map_of_contacts.end();++it)
		{
			const std::pair<Comment, Comment>& comments=it->first;
			column_width.first=std::max(column_width.first, comments.first.str().size());
			column_width.second=std::max(column_width.second, comments.second.str().size());
		}
		column_width.first+=2;
		column_width.second+=2;
		for(std::map< std::pair<Comment, Comment>, std::pair<double, std::string> >::const_iterator it=output_map_of_contacts.begin();it!=output_map_of_contacts.end();++it)
		{
			const std::pair<Comment, Comment>& comments=it->first;
			const std::pair<double, std::string>& value=it->second;
			std::cout.width(column_width.first);
			std::cout << std::left << comments.first.str();
			std::cout.width(column_width.second);
			std::cout << std::left << comments.second.str();
			std::cout.width(default_column_width);
			std::cout << value.first;
			if(preserve_graphics && !value.second.empty())
			{
				std::cout << value.second;
			}
			std::cout << "\n";
		}
	}

	if(drawing && opengl_printer_filled)
	{
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
	}
}
