#include <iostream>
#include <utility>
#include <stdexcept>

#include "apollota/constrained_contacts_construction.h"
#include "apollota/opengl_printer.h"

#include "modes_commons.h"

namespace
{

class Comment
{
public:
	Comment() : serial(null_num()), resSeq(null_num())
	{
	}

	static Comment solvent()
	{
		Comment v;
		v.chainID="solvent";
		return v;
	}

	static Comment from_str(const std::string& input_str)
	{
		Comment v;
		std::string refined_input_str=input_str;
		for(std::size_t i=0;i<refined_input_str.size();i++)
		{
			char& s=refined_input_str[i];
			if(s==vend || s==vbegin)
			{
				s=' ';
			}
		}
		std::istringstream input(refined_input_str);
		while(input.good())
		{
			std::string marker;
			input >> marker;
			if(marker=="c") { input >> v.chainID; }
			else if(marker=="r") { input >> v.resSeq; }
			else if(marker=="i") { input >> v.iCode; }
			else if(marker=="a") { input >> v.serial; }
			else if(marker=="l") { input >> v.altLoc; }
			else if(marker=="rn") { input >> v.resName; }
			else if(marker=="an") { input >> v.name; }
		}
		if(!(v.valid() && v.str()==input_str))
		{
			throw std::runtime_error(std::string("Invalid comment string '")+input_str+"'.");
		}
		return v;
	}

	static bool match_with_member_descriptor(const Comment& comment, const std::string& input_str)
	{
		const char vsep=',';
		const char vinterval=':';
		std::ostringstream control_output;
		bool matched=false;
		std::string refined_input_str=input_str;
		for(std::size_t i=0;i<refined_input_str.size();i++)
		{
			char& s=refined_input_str[i];
			if(s==vend || s==vbegin || s==vsep)
			{
				s=' ';
			}
		}
		std::istringstream input(refined_input_str);
		std::string marker;
		input >> marker;
		if(!input.fail() && (marker=="c" || marker=="r" || marker=="i" || marker=="a" || marker=="l" || marker=="rn" || marker=="an"))
		{
			control_output << marker << vbegin;
			bool need_sep=false;
			while(input.good())
			{
				std::string token;
				input >> token;
				if(!input.fail() && !token.empty())
				{
					if(need_sep) { control_output << vsep; } else { need_sep=true; }
					if(marker=="r" || marker=="a")
					{
						std::size_t pinterval=token.find(vinterval);
						if(pinterval!=std::string::npos)
						{
							token[pinterval]=' ';
							std::istringstream token_input(token);
							std::pair<int, int> value(0, 0);
							token_input >> value.first >> value.second;
							if(!token_input.fail() && value.first<=value.second)
							{
								control_output << value.first << vinterval << value.second;
								if(marker=="r") { matched=matched || (comment.resSeq>=value.first && comment.resSeq<=value.second); }
								else if(marker=="a") { matched=matched || (comment.serial>=value.first && comment.serial<=value.second); }
							}
						}
						else
						{
							std::istringstream token_input(token);
							int value=0;
							token_input >> value;
							if(!token_input.fail())
							{
								control_output << value;
								if(marker=="r") { matched=matched || (comment.resSeq==value); }
								else if(marker=="a") { matched=matched || (comment.serial==value); }
							}
						}
					}
					else
					{
						control_output << token;
						if(marker=="c") { matched=matched || (comment.chainID==token); }
						else if(marker=="i") { matched=matched || (comment.iCode==token); }
						else if(marker=="l") { matched=matched || (comment.altLoc==token); }
						else if(marker=="rn") { matched=matched || (comment.resName==token); }
						else if(marker=="an") { matched=matched || (comment.name==token); }
					}
				}
			}
			control_output << vend;
		}
		const std::string control_output_str=control_output.str();
		if(control_output_str.empty() || control_output_str!=input_str)
		{
			throw std::runtime_error(std::string("Invalid match descriptor '")+input_str+"'.");
		}
		return matched;
	}

	bool valid() const
	{
		return !((chainID.empty()) ||
				(serial!=null_num() && resSeq==null_num()) ||
				(resName.empty() && resSeq!=null_num()) ||
				(!resName.empty() && resSeq==null_num()) ||
				(name.empty() && serial!=null_num()) ||
				(!name.empty() && serial==null_num()));
	}

	Comment without_atom() const
	{
		Comment v=(*this);
		v.serial=null_num();
		v.altLoc.clear();
		v.name.clear();
		return v;
	}

	Comment without_residue() const
	{
		Comment v=without_atom();
		v.resSeq=null_num();
		v.iCode.clear();
		v.resName.clear();
		return v;
	}

	bool operator==(const Comment& v) const
	{
		return (serial==v.serial && resSeq==v.resSeq && chainID==v.chainID && iCode==v.iCode && altLoc==v.altLoc && resName==v.resName && name==v.name);
	}

	bool operator<(const Comment& v) const
	{
		if(chainID<v.chainID) { return true; }
		else if(chainID==v.chainID)
		{
			if(resSeq<v.resSeq) { return true; }
			else if(resSeq==v.resSeq)
			{
				if(iCode<v.iCode) { return true; }
				else if(iCode==v.iCode)
				{
					if(serial<v.serial) { return true; }
					else if(serial==v.serial)
					{
						if(altLoc<v.altLoc) { return true; }
						else if(altLoc==v.altLoc)
						{
							if(resName<v.resName) { return true; }
							else if(resName==v.resName)
							{
								return (name<v.name);
							}
						}
					}
				}
			}
		}
		return false;
	}

	std::string str() const
	{
		const bool with_residue=(resSeq!=null_num());
		const bool with_residue_and_atom=(with_residue && (serial!=null_num()));
		std::ostringstream output;
		output << "c" << vbegin << chainID << vend;
		if(with_residue)
		{
			output << "r" << vbegin << resSeq << vend;
			if(!iCode.empty())
			{
				output << "i" << vbegin << iCode << vend;
			}
		}
		if(with_residue_and_atom)
		{
			output << "a" << vbegin << serial << vend;
			if(!altLoc.empty())
			{
				output << "l" << vbegin << altLoc << vend;
			}
		}
		if(with_residue)
		{
			output << "rn" << vbegin << resName << vend;
		}
		if(with_residue_and_atom)
		{
			output << "an" << vbegin << name << vend;
		}
		return output.str();
	}

	friend bool add_sphere_and_comments_from_stream_to_vectors(std::istream&, std::pair< std::vector<apollota::SimpleSphere>*, std::vector<Comment>* >&);

private:
	static const char vbegin='[';
	static const char vend=']';

	static int null_num()
	{
		return std::numeric_limits<int>::min();
	}

	int serial;
	std::string chainID;
	int resSeq;
	std::string resName;
	std::string name;
	std::string altLoc;
	std::string iCode;
};

bool add_sphere_and_comments_from_stream_to_vectors(std::istream& input, std::pair< std::vector<apollota::SimpleSphere>*, std::vector<Comment>* >& spheres_with_comments)
{
	apollota::SimpleSphere sphere;
	input >> sphere.x >> sphere.y >> sphere.z >> sphere.r;
	std::string separator;
	input >> separator;
	if(!input.fail() && separator=="#")
	{
		Comment comment;
		input >> comment.serial >> comment.chainID >> comment.resSeq >> comment.resName >> comment.name;
		if(input.good())
		{
			input >> comment.altLoc >> comment.iCode;
		}
		if(!input.fail())
		{
			if(comment.altLoc.find_first_of(".?")==0)
			{
				comment.altLoc.clear();
			}
			if(comment.iCode.find_first_of(".?")==0)
			{
				comment.iCode.clear();
			}
			if(comment.valid())
			{
				spheres_with_comments.first->push_back(sphere);
				spheres_with_comments.second->push_back(comment);
				return true;
			}
		}
	}
	return false;
}

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
	apollota::OpenGLPrinter opengl_printer;
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
	apollota::OpenGLPrinter opengl_printer;
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
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--print-log"].init("", "flag to print log of calculations");
		basic_map_of_option_descriptions["--annotate"].init("", "flag to annotate contacts using balls comments");
		basic_map_of_option_descriptions["--probe"].init("number", "probe radius");
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions full_map_of_option_descriptions=basic_map_of_option_descriptions;
		full_map_of_option_descriptions["--exclude-hidden-balls"].init("", "flag to exclude hidden input balls");
		full_map_of_option_descriptions["--init-radius-for-BSH"].init("number", "initial radius for bounding sphere hierarchy");
		full_map_of_option_descriptions["--step"].init("number", "curve step length");
		full_map_of_option_descriptions["--projections"].init("number", "curve optimization depth");
		full_map_of_option_descriptions["--sih-depth"].init("number", "spherical surface optimization depth");
		full_map_of_option_descriptions["--max-dist"].init("number", "maximal distance to record, may exceed probe diameter");
		full_map_of_option_descriptions["--draw"].init("", "flag to output graphics for annotated contacts");
		if(poh.contains_option("--help") || poh.contains_option("--help-full"))
		{
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions(poh.contains_option("--help-full") ? full_map_of_option_descriptions : basic_map_of_option_descriptions, std::cerr);
			std::cerr << "\n";
			std::cerr << "  stdin   <-  list of balls\n";
			std::cerr << "                (default line format: 'x y z r # comments')\n";
			std::cerr << "                (annotated line format: 'x y z r # atomSerial chainID resSeq resName atomName [altLoc iCode]')\n";
			std::cerr << "  stdout  ->  list of contacts\n";
			std::cerr << "                (default line format: 'b1 b2 distance area')\n";
			std::cerr << "                (annotated line format: 'annotation1 annotation2 area [graphics]')\n";
			return;
		}
		else
		{
			poh.compare_with_map_of_option_descriptions(full_map_of_option_descriptions);
		}
	}

	const bool print_log=poh.contains_option("--print-log");
	const bool annotate=poh.contains_option("--annotate");
	const bool exclude_hidden_balls=poh.contains_option("--exclude-hidden-balls");
	const double init_radius_for_BSH=std::max(1.0, poh.argument<double>("--init-radius-for-BSH", 3.5));
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
		auxiliaries::read_lines_to_container(std::cin, "", add_sphere_and_comments_from_stream_to_vectors, spheres_with_comments);
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

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, init_radius_for_BSH, exclude_hidden_balls, false);
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
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--inter-residue"].init("", "flag to convert to inter-residue contacts");
		basic_map_of_option_descriptions["--inter-chain"].init("", "flag to convert to inter-chain contacts");
		basic_map_of_option_descriptions["--match-first"].init("string", "selection for first contacting group");
		basic_map_of_option_descriptions["--match-first-not"].init("string", "negative selection for first contacting group");
		basic_map_of_option_descriptions["--match-second"].init("string", "selection for second contacting group");
		basic_map_of_option_descriptions["--match-second-not"].init("string", "negative selection for second contacting group");
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions full_map_of_option_descriptions=basic_map_of_option_descriptions;
		full_map_of_option_descriptions["--drawing-for-pymol"].init("", "flag to output drawing as pymol script");
		full_map_of_option_descriptions["--drawing-for-jmol"].init("", "flag to output drawing as jmol script");
		full_map_of_option_descriptions["--drawing-name"].init("string", "graphics object name for drawing output");
		full_map_of_option_descriptions["--drawing-color"].init("hex", "color for drawing output");
		full_map_of_option_descriptions["--drawing-random-colors"].init("", "flag to use random color for each drawn contact");
		full_map_of_option_descriptions["--drawing-alpha"].init("number", "alpha opacity value for drawing output");
		if(poh.contains_option("--help") || poh.contains_option("--help-full"))
		{
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions(poh.contains_option("--help-full") ? full_map_of_option_descriptions : basic_map_of_option_descriptions, std::cerr);
			std::cerr << "\n";
			std::cerr << "  stdin   <-  list of contacts (line format: 'annotation1 annotation2 area [graphics]')\n";
			std::cerr << "  stdout  ->  list of contacts (line format: 'annotation1 annotation2 area [graphics]')\n";
			return;
		}
		else
		{
			poh.compare_with_map_of_option_descriptions(full_map_of_option_descriptions);
		}
	}

	const bool inter_residue=poh.contains_option("--inter-residue");
	const bool inter_chain=poh.contains_option("--inter-chain");
	const char selection_list_sep='&';
	const std::vector<std::string> match_first=poh.argument_vector<std::string>("--match-first", selection_list_sep);
	const std::vector<std::string> match_first_not=poh.argument_vector<std::string>("--match-first-not", selection_list_sep);
	const std::vector<std::string> match_second=poh.argument_vector<std::string>("--match-second", selection_list_sep);
	const std::vector<std::string> match_second_not=poh.argument_vector<std::string>("--match-second-not", selection_list_sep);
	const bool drawing_for_pymol=poh.contains_option("--drawing-for-pymol");
	const bool drawing_for_jmol=poh.contains_option("--drawing-for-jmol");
	const bool drawing=(drawing_for_pymol || drawing_for_jmol);
	const std::string drawing_name=poh.argument<std::string>("--drawing-name", "contacts");
	const unsigned int drawing_color=auxiliaries::ProgramOptionsHandler::convert_hex_string_to_integer<unsigned int>(poh.argument<std::string>("--drawing-color", "0xFFFFFF"));
	const bool drawing_random_colors=poh.contains_option("--drawing-random-colors");
	const double drawing_alpha=poh.argument<double>("--drawing-alpha", 1.0);

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

	apollota::OpenGLPrinter opengl_printer;
	opengl_printer.add_color(drawing_color);
	opengl_printer.add_alpha(drawing_alpha);

	for(std::map< std::pair<Comment, Comment>, std::pair<double, std::string> >::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		const std::pair<Comment, Comment>& comments=it->first;
		const bool matched_first_second=(match_comment_with_member_descriptors(comments.first, match_first, match_first_not) && match_comment_with_member_descriptors(comments.second, match_second, match_second_not));
		const bool matched_second_first=(match_comment_with_member_descriptors(comments.second, match_first, match_first_not) && match_comment_with_member_descriptors(comments.first, match_second, match_second_not));
		if(matched_first_second || matched_second_first)
		{
			const std::pair<double, std::string>& value=it->second;
			if(drawing)
			{
				if(drawing_random_colors)
				{
					opengl_printer.add_color(calc_string_color_integer(comments.first.str()+comments.second.str()));
				}
				opengl_printer.add(value.second);
			}
			else
			{
				if(matched_first_second)
				{
					output_map_of_contacts[comments]=value;
				}
				else
				{
					output_map_of_contacts[std::make_pair(comments.second, comments.first)]=value;
				}
			}
		}
	}

	if(drawing)
	{
		if(opengl_printer.empty())
		{
			throw std::runtime_error("No graphics input.");
		}
		if(drawing_for_pymol)
		{
			opengl_printer.print_pymol_script(drawing_name, true, std::cout);
		}
		if(drawing_for_jmol)
		{
			opengl_printer.print_jmol_script(drawing_name, std::cout);
		}
	}
	else
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
			if(!value.second.empty())
			{
				std::cout << value.second;
			}
			std::cout << "\n";
		}
	}
}
