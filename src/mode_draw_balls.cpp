#include "apollota/basic_operations_on_spheres.h"
#include "apollota/rotation.h"
#include "apollota/search_for_spherical_collisions.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"
#include "auxiliaries/opengl_printer.h"

#include "modescommon/ball_value.h"
#include "modescommon/drawing_utilities.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

void draw_cylinder(
		const apollota::SimpleSphere& a,
		const apollota::SimpleSphere& b,
		const int sides,
		auxiliaries::OpenGLPrinter& opengl_printer)
{
	apollota::SimplePoint pa(a);
	apollota::SimplePoint pb(b);
	const apollota::SimplePoint axis=(pb-pa);
	apollota::Rotation rotation(axis, 0);
	const apollota::SimplePoint first_point=apollota::any_normal_of_vector<apollota::SimplePoint>(rotation.axis);
	const double angle_step=360.0/static_cast<double>(std::min(std::max(sides, 6), 30));
	std::vector<apollota::SimplePoint> contour;
	contour.reserve(sides+1);
	contour.push_back(first_point);
	for(rotation.angle=angle_step;rotation.angle<360;rotation.angle+=angle_step)
	{
		contour.push_back(rotation.rotate<apollota::SimplePoint>(first_point));
	}
	contour.push_back(first_point);
	std::vector<apollota::SimplePoint> vertices;
	std::vector<apollota::SimplePoint> normals;
	vertices.reserve(contour.size()*2);
	normals.reserve(vertices.size());
	for(std::size_t i=0;i<contour.size();i++)
	{
		const apollota::SimplePoint& p=contour[i];
		vertices.push_back(pa+(p*a.r));
		vertices.push_back(pb+(p*b.r));
		normals.push_back(p);
		normals.push_back(p);
	}
	opengl_printer.add_triangle_strip(vertices, normals);
}

void draw_links(
		const std::vector< std::pair<CRAD, BallValue> >& list_of_balls,
		const double ball_collision_radius,
		const double bsh_initial_radius,
		const double ball_drawing_radius,
		const double cylinder_drawing_radius,
		const int cylinder_sides,
		const bool check_sequence,
		const bool drawing_labels,
		auxiliaries::OpenGLPrinter& opengl_printer)
{
	std::vector<apollota::SimpleSphere> spheres(list_of_balls.size());
	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		spheres[i]=apollota::SimpleSphere(list_of_balls[i].second, ball_collision_radius);
	}
	apollota::BoundingSpheresHierarchy bsh(spheres, bsh_initial_radius, 1);
	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		const apollota::SimpleSphere& a=spheres[i];
		const CRAD& a_crad=list_of_balls[i].first;
		if(drawing_labels)
		{
			opengl_printer.add_label(construct_label_from_crad(a_crad));
		}
		opengl_printer.add_sphere(apollota::SimpleSphere(a, ball_drawing_radius));
		std::vector<std::size_t> collisions=apollota::SearchForSphericalCollisions::find_all_collisions(bsh, a);
		for(std::size_t j=0;j<collisions.size();j++)
		{
			const apollota::SimpleSphere& b=spheres[collisions[j]];
			const CRAD& b_crad=list_of_balls[collisions[j]].first;
			if(!check_sequence || (a_crad.chainID==b_crad.chainID && abs(a_crad.resSeq-b_crad.resSeq)<=1))
			draw_cylinder(
					apollota::SimpleSphere(a, cylinder_drawing_radius),
					apollota::SimpleSphere(apollota::sum_of_points<apollota::SimplePoint>(a, b)*0.5, cylinder_drawing_radius),
					cylinder_sides,
					opengl_printer);
		}
	}
}

}

void draw_balls(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'annotation x y z r tags adjuncts')");
	pohw.describe_io("stdout", false, true, "list of balls (line format: 'annotation x y z r tags adjuncts')");

	const std::string representation=poh.argument<std::string>(pohw.describe_option("--representation", "string", "representation name: 'vdw', 'sticks' or 'trace'"), "vdw");
	const std::string drawing_for_pymol=poh.argument<std::string>(pohw.describe_option("--drawing-for-pymol", "string", "file path to output drawing as pymol script"), "");
	const std::string drawing_for_scenejs=poh.argument<std::string>(pohw.describe_option("--drawing-for-scenejs", "string", "file path to output drawing as scenejs script"), "");
	const std::string drawing_name=poh.argument<std::string>(pohw.describe_option("--drawing-name", "string", "graphics object name for drawing output"), "contacts");
	const unsigned int drawing_color=poh.convert_hex_string_to_integer<unsigned int>(poh.argument<std::string>(pohw.describe_option("--drawing-color", "string", "color for drawing output, in hex format, white is 0xFFFFFF"), "0xFFFFFF"));
	const bool drawing_labels=poh.contains_option(pohw.describe_option("--drawing-labels", "", "flag to use labels in drawing if possible"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	typedef std::vector< std::pair<CRAD, BallValue> > ListOfBalls;
	const ListOfBalls list_of_balls=auxiliaries::IOUtilities().read_lines_to_map<ListOfBalls>(std::cin);
	if(list_of_balls.empty())
	{
		throw std::runtime_error("No input.");
	}

	auxiliaries::OpenGLPrinter opengl_printer;
	opengl_printer.add_color(drawing_color);

	if(representation=="vdw")
	{
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const CRAD& crad=list_of_balls[i].first;
			const BallValue& value=list_of_balls[i].second;
			if(drawing_labels)
			{
				opengl_printer.add_label(construct_label_from_crad(crad));
			}
			opengl_printer.add_sphere(value);
		}
	}
	else if(representation=="sticks")
	{
		draw_links(list_of_balls, 0.8, 4.0, 0.3, 0.2, 6, false, drawing_labels, opengl_printer);
	}
	else if(representation=="trace")
	{
		std::vector< std::pair<CRAD, BallValue> > list_of_balls_filtered;
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			if(list_of_balls[i].first.name=="CA")
			{
				list_of_balls_filtered.push_back(list_of_balls[i]);
			}
		}
		draw_links(list_of_balls_filtered, 2.0, 10.0, 0.3, 0.3, 12, true, drawing_labels, opengl_printer);
	}
	else
	{
		throw std::runtime_error("Invalid representation name.");
	}

	if(!drawing_for_pymol.empty())
	{
		std::ofstream foutput(drawing_for_pymol.c_str(), std::ios::out);
		if(foutput.good())
		{
			opengl_printer.print_pymol_script(drawing_name, true, foutput);
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

	auxiliaries::IOUtilities().write_map(list_of_balls, std::cout);
}
