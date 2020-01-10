#include <iostream>

#include "../apollota/triangulation.h"
#include "../apollota/triangulation_output.h"

#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"

void calculate_vertices(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'x y z r')");
	pohw.describe_io("stdout", false, true, "list of Voronoi vertices, i.e. quadruples with tangent spheres (line format: 'q1 q2 q3 q4 x y z r')");

	const bool print_log=poh.contains_option(pohw.describe_option("--print-log", "", "flag to print log of calculations"));
	const bool exclude_hidden_balls=poh.contains_option(pohw.describe_option("--exclude-hidden-balls", "", "flag to exclude hidden input balls"));
	const bool include_surplus_quadruples=poh.contains_option(pohw.describe_option("--include-surplus-quadruples", "", "flag to include surplus quadruples"));
	const bool link=poh.contains_option(pohw.describe_option("--link", "", "flag to output links between vertices"));
	const double init_radius_for_BSH=poh.argument<double>(pohw.describe_option("--init-radius-for-BSH", "number", "initial radius for bounding sphere hierarchy"), 3.5);
	const bool check=poh.contains_option(pohw.describe_option("--check", "", "flag to slowly check the resulting vertices (used only for testing)"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	if(init_radius_for_BSH<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy initial radius should be greater than 1.");
	}

	const std::vector<voronota::apollota::SimpleSphere> spheres=voronota::auxiliaries::IOUtilities().read_lines_to_set< std::vector<voronota::apollota::SimpleSphere> >(std::cin);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	const voronota::apollota::Triangulation::Result triangulation_result=voronota::apollota::Triangulation::construct_result(spheres, init_radius_for_BSH, exclude_hidden_balls, include_surplus_quadruples);

	if(link)
	{
		voronota::apollota::TriangulationOutput::print_vertices_vector_with_vertices_graph(voronota::apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map), voronota::apollota::Triangulation::construct_vertices_graph(spheres, triangulation_result.quadruples_map), std::cout);
	}
	else
	{
		voronota::apollota::TriangulationOutput::print_vertices_vector(voronota::apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map), std::cout);
	}

	if(print_log)
	{
		std::clog << "balls " <<  spheres.size() << "\n";
		voronota::apollota::TriangulationOutput::print_status(triangulation_result, std::clog);
	}

	if(check)
	{
		std::clog << "check " << (voronota::apollota::Triangulation::check_quadruples_map(spheres, triangulation_result.quadruples_map)) << "\n";
	}
}
