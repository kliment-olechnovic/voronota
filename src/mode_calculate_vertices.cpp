#include <iostream>

#include "apollota/triangulation.h"
#include "apollota/triangulation_output.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

void calculate_vertices(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--print-log", "", "flag to print log of calculations"));
		ods.push_back(OD("--exclude-hidden-balls", "", "flag to exclude hidden input balls"));
		ods.push_back(OD("--include-surplus-quadruples", "", "flag to include surplus quadruples"));
		ods.push_back(OD("--link", "", "flag to output links between vertices"));
		ods.push_back(OD("--init-radius-for-BSH", "number", "initial radius for bounding sphere hierarchy"));
		ods.push_back(OD("--check", "", "flag to slowly check the resulting vertices (used only for testing)"));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false, "list of balls (line format: 'x y z r')");
			poh.print_io_description("stdout", false, true, "list of Voronoi vertices, i.e. quadruples with tangent spheres (line format: 'q1 q2 q3 q4 x y z r')");
			return;
		}
	}

	const bool print_log=poh.contains_option("--print-log");

	const bool exclude_hidden_balls=poh.contains_option("--exclude-hidden-balls");

	const bool include_surplus_quadruples=poh.contains_option("--include-redundant-quadruples");

	const bool check=poh.contains_option("--check");

	const bool link=poh.contains_option("--link");

	const double init_radius_for_BSH=poh.argument<double>("--init-radius-for-BSH", 3.5);
	if(init_radius_for_BSH<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy initial radius should be greater than 1.");
	}

	const std::vector<apollota::SimpleSphere> spheres=auxiliaries::IOUtilities().read_lines_to_set< std::vector<apollota::SimpleSphere> >(std::cin);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, init_radius_for_BSH, exclude_hidden_balls, include_surplus_quadruples);

	if(link)
	{
		apollota::TriangulationOutput::print_vertices_vector_with_vertices_graph(apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map), apollota::Triangulation::construct_vertices_graph(spheres, triangulation_result.quadruples_map), std::cout);
	}
	else
	{
		apollota::TriangulationOutput::print_vertices_vector(apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map), std::cout);
	}

	if(print_log)
	{
		std::clog << "balls " <<  spheres.size() << "\n";
		apollota::TriangulationOutput::print_status(triangulation_result, std::clog);
	}

	if(check)
	{
		std::clog << "check " << (apollota::Triangulation::check_quadruples_map(spheres, triangulation_result.quadruples_map)) << "\n";
	}
}
