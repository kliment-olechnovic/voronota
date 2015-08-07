#include "apollota/constrained_contacts_construction.h"
#include "apollota/spheres_boundary_construction.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/opengl_printer.h"
#include "auxiliaries/program_options_handler.h"

void generate_demo(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'x y z r')");
	pohw.describe_io("stdout", false, true, "drawing script fo PyMol");

	const double probe=poh.restrict_value_in_range(0.01, 14.0, poh.argument<double>(pohw.describe_option("--probe", "number", "probe radius"), 1.4));
	const std::string representation=poh.argument<std::string>(pohw.describe_option("--representation", "string", "representation name"), "");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const double step=0.2;
	const int projections=5;

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::IOUtilities().read_lines_to_set(std::cin, spheres);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

//	const std::size_t input_spheres_count=spheres.size();
	const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(spheres, probe*2.0);
	spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, false, false);
	const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);

	std::vector< std::pair<apollota::Pair, apollota::ConstrainedContactContour::Contour> > surface_contours_vector;
	std::vector<int> marks;
	apollota::ConstrainedContactsConstruction::construct_surface_contours(spheres, vertices_vector, probe, step, projections, surface_contours_vector, marks);

	if(representation=="")
	{
		auxiliaries::OpenGLPrinter opengl_printer;
		opengl_printer.add_color(0xFFFF00);
		for(std::size_t i=0;i<surface_contours_vector.size();i++)
		{
//			const apollota::Pair& pair=surface_contours_vector[i].first;
			const apollota::ConstrainedContactContour::Contour& contour=surface_contours_vector[i].second;
			std::vector<apollota::SimplePoint> points;
			points.reserve(contour.size());
			for(apollota::ConstrainedContactContour::Contour::const_iterator contour_it=contour.begin();contour_it!=contour.end();++contour_it)
			{
				points.push_back(contour_it->p);
			}
			opengl_printer.add_color(((marks[i]+1)*123456789)%(0xFFFFFF));
			opengl_printer.add_line_strip(points);
		}
		opengl_printer.print_pymol_script("surface_curves", true, std::cout);
	}
}
