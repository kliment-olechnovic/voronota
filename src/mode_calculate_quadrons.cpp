#include <iostream>
#include <fstream>

#include "apollota/triangulation.h"

#include "auxiliaries/chain_residue_atom_comment.h"
#include "auxiliaries/opengl_printer.h"

#include "modes_commons.h"

namespace
{

typedef auxiliaries::ChainResidueAtomComment Comment;

class Quadron
{
public:
	Quadron(const Comment& a, const Comment& b, const Comment& c, const Comment& d) : comments_(4)
	{
		comments_[0]=a;
		comments_[1]=b;
		comments_[2]=c;
		comments_[3]=d;
		std::sort(comments_.begin(), comments_.end());
	}

	const std::vector<Comment>& get_comments() const
	{
		return comments_;
	}

	bool is_in_single_seq_group(const int max_sep) const
	{
		return (Comment::match_with_sequence_separation_interval(comments_[0], comments_[1], Comment::null_num(), max_sep, false) &&
				Comment::match_with_sequence_separation_interval(comments_[0], comments_[2], Comment::null_num(), max_sep, false) &&
				Comment::match_with_sequence_separation_interval(comments_[0], comments_[3], Comment::null_num(), max_sep, false) &&
				Comment::match_with_sequence_separation_interval(comments_[1], comments_[2], Comment::null_num(), max_sep, false) &&
				Comment::match_with_sequence_separation_interval(comments_[1], comments_[3], Comment::null_num(), max_sep, false) &&
				Comment::match_with_sequence_separation_interval(comments_[2], comments_[3], Comment::null_num(), max_sep, false));
	}

	bool operator==(const Quadron& v) const
	{
		for(int i=0;i<4;i++)
		{
			if(!(comments_[i]==v.comments_[i]))
			{
				return false;
			}
		}
		return true;
	}

	bool operator<(const Quadron& v) const
	{
		for(int i=0;i<4;i++)
		{
			if(comments_[i]<v.comments_[i])
			{
				return true;
			}
			else if(!(comments_[i]==v.comments_[i]))
			{
				return false;
			}
		}
		return false;
	}

private:
	std::vector<Comment> comments_;
};

bool add_quadron_record_from_stream_to_map(std::istream& input, std::map< Quadron, std::pair<double, std::string> >& map_of_records)
{
	std::vector<Comment> comments(4);
	std::pair<double, std::string> value;
	for(int i=0;i<4;i++)
	{
		std::string token;
		input >> token;
		if(token.empty())
		{
			return false;
		}
		else
		{
			comments[i]=(Comment::from_str(token));
			if(!comments[i].valid())
			{
				return false;
			}
		}
	}
	input >> value.first;
	if(input.good())
	{
		std::getline(input, value.second);
	}
	if(!input.fail())
	{
		map_of_records[Quadron(comments[0], comments[1], comments[2], comments[3])]=value;
		return true;
	}
	return false;
}

}

void calculate_quadrons(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--probe", "number", "probe radius"));
		list_of_option_descriptions.push_back(OD("--exclude-hidden-balls", "", "flag to exclude hidden input balls"));
		list_of_option_descriptions.push_back(OD("--draw", "", "flag to output graphics"));
		if(!modes_commons::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of balls (line format: 'x y z r # comments')\n";
			std::cerr << "stdout  ->  list of quadrons (line format: 'annotation1 annotation2 annotation3 annotation4 volume [graphics]')\n";
			return;
		}
	}

	const bool exclude_hidden_balls=poh.contains_option("--exclude-hidden-balls");
	const double probe=std::max(0.01, std::min(14.0, poh.argument<double>("--probe", 1.4)));
	const bool draw=poh.contains_option("--draw");

	std::vector<apollota::SimpleSphere> spheres;
	std::vector<Comment> input_spheres_comments;
	{
		std::pair< std::vector<apollota::SimpleSphere>*, std::vector<Comment>* > spheres_with_comments(&spheres, &input_spheres_comments);
		auxiliaries::read_lines_to_container(std::cin, "", modes_commons::add_sphere_and_comments_from_stream_to_vectors<apollota::SimpleSphere, Comment>, spheres_with_comments);
	}
	if(spheres.size()!=input_spheres_comments.size())
	{
		throw std::runtime_error("Number of comments does not match number of spheres.");
	}
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, exclude_hidden_balls, false);
	const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);

	for(apollota::Triangulation::VerticesVector::const_iterator it=vertices_vector.begin();it!=vertices_vector.end();++it)
	{
		const apollota::Quadruple& q=it->first;
		apollota::SimpleSphere s=it->second;
		if(s.r<probe && q.get(0)<input_spheres_comments.size() && q.get(1)<input_spheres_comments.size() && q.get(2)<input_spheres_comments.size() && q.get(3)<input_spheres_comments.size())
		{
			s.r=(probe-s.r);
			const double volume=((4.0/3.0)*apollota::Rotation::pi()*s.r*s.r*s.r);
			Quadron quadron(input_spheres_comments[q.get(0)], input_spheres_comments[q.get(1)], input_spheres_comments[q.get(2)], input_spheres_comments[q.get(3)]);
			if(!quadron.is_in_single_seq_group(0))
			{
				std::cout << quadron.get_comments()[0].str() << " " << quadron.get_comments()[1].str() << " " << quadron.get_comments()[2].str() << " " << quadron.get_comments()[3].str() << " " << volume;
				if(draw)
				{
					auxiliaries::OpenGLPrinter opengl_printer;
					opengl_printer.add_sphere(s);
					std::cout << " " << opengl_printer.str();
				}
				std::cout << "\n";
			}
		}
	}
}

void calculate_quadrons_query(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--inter-residue", "", "flag to convert to inter-residue contacts"));
		list_of_option_descriptions.push_back(OD("--drawing-for-pymol", "string", "file path to output drawing as pymol script"));
		list_of_option_descriptions.push_back(OD("--drawing-name", "string", "graphics object name for drawing output"));
		list_of_option_descriptions.push_back(OD("--drawing-color", "string", "color for drawing output, in hex format, white is 0xFFFFFF"));
		list_of_option_descriptions.push_back(OD("--preserve-graphics", "", "flag to preserve graphics in output"));
		if(!modes_commons::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 annotation3 annotation4 volume [graphics]')\n";
			std::cerr << "stdout  ->  list of contacts (line format: 'annotation1 annotation2 annotation3 annotation4 volume [graphics]')\n";
			return;
		}
	}

	const bool inter_residue=poh.contains_option("--inter-residue");
	const std::string drawing_for_pymol=poh.argument<std::string>("--drawing-for-pymol", "");
	const bool drawing=!drawing_for_pymol.empty();
	const std::string drawing_name=poh.argument<std::string>("--drawing-name", "quadrons");
	const unsigned int drawing_color=auxiliaries::ProgramOptionsHandler::convert_hex_string_to_integer<unsigned int>(poh.argument<std::string>("--drawing-color", "0xFFFFFF"));
	const bool preserve_graphics=poh.contains_option("--preserve-graphics");

	std::map< Quadron, std::pair<double, std::string> > map_of_quadrons;
	auxiliaries::read_lines_to_container(std::cin, "", add_quadron_record_from_stream_to_map, map_of_quadrons);
	if(map_of_quadrons.empty())
	{
		throw std::runtime_error("No input.");
	}

	if(inter_residue)
	{
		std::map< Quadron, std::pair<double, std::string> > map_of_reduced_quadrons;
		for(std::map< Quadron, std::pair<double, std::string> >::const_iterator it=map_of_quadrons.begin();it!=map_of_quadrons.end();++it)
		{
			const Quadron& q=it->first;
			std::pair<double, std::string>& value=map_of_reduced_quadrons[Quadron(q.get_comments()[0].without_atom(), q.get_comments()[1].without_atom(), q.get_comments()[2].without_atom(), q.get_comments()[3].without_atom())];
			value.first+=it->second.first;
			value.second+=it->second.first;
		}
		map_of_quadrons=map_of_reduced_quadrons;
	}

	auxiliaries::OpenGLPrinter opengl_printer;
	bool opengl_printer_filled=false;
	if(drawing)
	{
		opengl_printer.add_color(drawing_color);
	}

	for(std::map< Quadron, std::pair<double, std::string> >::const_iterator it=map_of_quadrons.begin();it!=map_of_quadrons.end();++it)
	{
		const Quadron& q=it->first;
		const std::pair<double, std::string>& value=it->second;
		std::cout << q.get_comments()[0].str() << " " << q.get_comments()[1].str() << " " << q.get_comments()[2].str() << " " << q.get_comments()[3].str() << " " << value.first;
		if(preserve_graphics && !value.second.empty())
		{
			std::cout << value.second;
		}
		std::cout << "\n";
		if(drawing && !value.second.empty())
		{
			opengl_printer.add(value.second);
			opengl_printer_filled=true;
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
	}
}
