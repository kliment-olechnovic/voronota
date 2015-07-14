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

struct DrawingParametersWrapper
{
	unsigned int default_color;
	bool adjuncts_rgb;
	bool use_labels;

	DrawingParametersWrapper() : default_color(0xFFFFFF), adjuncts_rgb(false), use_labels(false)
	{
	}

	void process(const CRAD& crad, const BallValue& value, auxiliaries::OpenGLPrinter& opengl_printer) const
	{
		if(use_labels)
		{
			opengl_printer.add_label(construct_label_from_crad(crad));
		}

		if(adjuncts_rgb)
		{
			const bool rp=value.props.adjuncts.count("r")>0;
			const bool gp=value.props.adjuncts.count("g")>0;
			const bool bp=value.props.adjuncts.count("b")>0;
			if(!(rp || gp || bp))
			{
				opengl_printer.add_color(default_color);
			}
			else
			{
				opengl_printer.add_color(
						(rp ? value.props.adjuncts.find("r")->second : 0.0),
						(gp ? value.props.adjuncts.find("g")->second : 0.0),
						(bp ? value.props.adjuncts.find("b")->second : 0.0));
			}
		}
	}
};

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
		const DrawingParametersWrapper& drawing_parameters_wrapper,
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
		drawing_parameters_wrapper.process(a_crad, list_of_balls[i].second, opengl_printer);
		opengl_printer.add_sphere(apollota::SimpleSphere(a, ball_drawing_radius));
		std::vector<std::size_t> collisions=apollota::SearchForSphericalCollisions::find_all_collisions(bsh, a);
		for(std::size_t j=0;j<collisions.size();j++)
		{
			const std::size_t collision_id=collisions[j];
			if(i!=collision_id)
			{
				const apollota::SimpleSphere& b=spheres[collision_id];
				const CRAD& b_crad=list_of_balls[collision_id].first;
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
	DrawingParametersWrapper drawing_parameters_wrapper;
	drawing_parameters_wrapper.default_color=poh.convert_hex_string_to_integer<unsigned int>(poh.argument<std::string>(pohw.describe_option("--default-color", "string", "default color for drawing output, in hex format, white is 0xFFFFFF"), "0xFFFFFF"));
	drawing_parameters_wrapper.adjuncts_rgb=poh.contains_option(pohw.describe_option("--adjuncts-rgb", "", "flag to use RGB color values from adjuncts"));
	drawing_parameters_wrapper.use_labels=poh.contains_option(pohw.describe_option("--use-labels", "", "flag to use labels in drawing if possible"));

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
	opengl_printer.add_color(drawing_parameters_wrapper.default_color);

	if(representation=="vdw")
	{
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const CRAD& crad=list_of_balls[i].first;
			const BallValue& value=list_of_balls[i].second;
			drawing_parameters_wrapper.process(crad, value, opengl_printer);
			opengl_printer.add_sphere(value);
		}
	}
	else if(representation=="sticks")
	{
		draw_links(list_of_balls, 0.8, 4.0, 0.3, 0.2, 6, false, drawing_parameters_wrapper, opengl_printer);
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
		draw_links(list_of_balls_filtered, 2.0, 10.0, 0.3, 0.3, 12, true, drawing_parameters_wrapper, opengl_printer);
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
