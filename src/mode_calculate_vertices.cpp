#include <iostream>

#include "apollota/triangulation.h"
#include "apollota/triangulation_output.h"

#include "modes_commons.h"

void calculate_vertices(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--print-log"].init("", "flag to print log of calculations");
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions advanced_map_of_option_descriptions;
		advanced_map_of_option_descriptions["--exclude-hidden-balls"].init("", "flag to exclude hidden input balls");
		advanced_map_of_option_descriptions["--include-surplus-quadruples"].init("", "flag to include surplus quadruples");
		advanced_map_of_option_descriptions["--init-radius-for-BSH"].init("number", "initial radius for bounding sphere hierarchy");
		advanced_map_of_option_descriptions["--check"].init("", "flag to slowly check the resulting vertices (used only for testing)");
		advanced_map_of_option_descriptions["--link"].init("", "flag to output links between vertices");
		if(!modes_commons::assert_options(basic_map_of_option_descriptions, advanced_map_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of balls (line format: 'x y z r # comments')\n";
			std::cerr << "stdout  ->  list of Voronoi vertices, i.e. quadruples with tangent spheres (line format: 'q1 q2 q3 q4 x y z r')\n";
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

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
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
