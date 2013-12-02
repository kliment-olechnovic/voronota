#include <iostream>

#include "apollota/triangulation.h"

#include "modes_commons.h"

void calculate_triangulation(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--print-log"].init("", "flag to print log of calculations");
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions full_map_of_option_descriptions=basic_map_of_option_descriptions;
		full_map_of_option_descriptions["--exclude-hidden-balls"].init("", "flag to exclude hidden input balls");
		full_map_of_option_descriptions["--include-surplus-quadruples"].init("", "flag to include surplus quadruples");
		full_map_of_option_descriptions["--skip-output"].init("", "flag to disable output of the resulting triangulation");
		full_map_of_option_descriptions["--init-radius-for-BSH"].init("number", "initial radius for bounding sphere hierarchy");
		full_map_of_option_descriptions["--check"].init("", "flag to slowly check the resulting triangulation (used only for testing)");
		if(poh.contains_option("--help") || poh.contains_option("--help-full"))
		{
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions(poh.contains_option("--help-full") ? full_map_of_option_descriptions : basic_map_of_option_descriptions, std::cerr);
			std::cerr << "\n";
			std::cerr << "  stdin   <-  list of balls (line format: 'x y z r # comments')\n";
			std::cerr << "  stdout  ->  list of quadruples with tangent spheres (line format: 'q1 q2 q3 q4 x y z r')\n";
			return;
		}
		else
		{
			poh.compare_with_map_of_option_descriptions(full_map_of_option_descriptions);
		}
	}

	const bool exclude_hidden_balls=poh.contains_option("--exclude-hidden-balls");

	const bool include_surplus_quadruples=poh.contains_option("--include-redundant-quadruples");

	const bool skip_output=poh.contains_option("--skip-output");

	const bool print_log=poh.contains_option("--print-log");

	const bool check=poh.contains_option("--check");

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

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, init_radius_for_BSH, exclude_hidden_balls, include_surplus_quadruples, false, false);

	if(!skip_output)
	{
		apollota::Triangulation::print_quadruples_map(triangulation_result.quadruples_map, std::cout);
	}

	if(print_log)
	{
		std::clog << "balls " <<  spheres.size() << "\n";
		triangulation_result.print_status(std::clog);
	}

	if(check)
	{
		std::clog << "check " << (apollota::Triangulation::check_quadruples_map(spheres, triangulation_result.quadruples_map)) << "\n";
	}
}
