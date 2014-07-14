#include <iostream>

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
	Quadron(const Comment& a, const Comment& b, const Comment& c, const Comment& d, const double volume) : comments_(4), volume_(volume)
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

	double get_volume() const
	{
		return volume_;
	}

	bool is_of_same_sequence_separation_interval(const int min_sep, const int max_sep) const
	{
		return (Comment::match_with_sequence_separation_interval(comments_[0], comments_[1], min_sep, max_sep, false) &&
				Comment::match_with_sequence_separation_interval(comments_[0], comments_[2], min_sep, max_sep, false) &&
				Comment::match_with_sequence_separation_interval(comments_[0], comments_[3], min_sep, max_sep, false) &&
				Comment::match_with_sequence_separation_interval(comments_[1], comments_[2], min_sep, max_sep, false) &&
				Comment::match_with_sequence_separation_interval(comments_[1], comments_[3], min_sep, max_sep, false) &&
				Comment::match_with_sequence_separation_interval(comments_[2], comments_[3], min_sep, max_sep, false));
	}

private:
	std::vector<Comment> comments_;
	double volume_;
};

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
			Quadron quadron(input_spheres_comments[q.get(0)], input_spheres_comments[q.get(1)], input_spheres_comments[q.get(2)], input_spheres_comments[q.get(3)], ((4.0/3.0)*apollota::Rotation::pi()*s.r*s.r*s.r));
			if(!quadron.is_of_same_sequence_separation_interval(0, 0))
			{
				std::cout << quadron.get_comments()[0].str() << " " << quadron.get_comments()[1].str() << " " << quadron.get_comments()[2].str() << " " << quadron.get_comments()[3].str() << " " << quadron.get_volume();
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
