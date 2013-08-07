#include <iostream>

#include "apollota/triangulation.h"
#include "apollota/splitting_set_of_spheres.h"

#include "modes_commons.h"

void calculate_triangulation_in_parallel(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--depth"].init("number", "depth of recursive input subdivision");
		basic_map_of_option_descriptions["--skip-output"].init("", "flag to disable output of the resulting triangulation");
		basic_map_of_option_descriptions["--print-log"].init("", "flag to print log of calculations");
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions full_map_of_option_descriptions=basic_map_of_option_descriptions;
		full_map_of_option_descriptions["--init-radius-for-BSH"].init("number", "initial radius for bounding sphere hierarchy");
		if(poh.contains_option("--help") || poh.contains_option("--help-full"))
		{
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions(poh.contains_option("--help-full") ? full_map_of_option_descriptions : basic_map_of_option_descriptions, std::cerr);
			std::cerr << "\n";
			std::cerr << "  stdin   <-  list of balls (line format: 'x y z r')\n";
			std::cerr << "  stdout  ->  list of quadruples with tangent spheres (line format: 'q1 q2 q3 q4 x y z r')\n";
			return;
		}
		else
		{
			poh.compare_with_map_of_option_descriptions(full_map_of_option_descriptions);
		}
	}

	const unsigned int depth=poh.argument<double>("--depth", 2);

	const bool skip_output=poh.contains_option("--skip-output");

	const bool print_log=poh.contains_option("--print-log");

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

	apollota::Triangulation::QuadruplesMap result_quadruples_map;
	const apollota::BoundingSpheresHierarchy bsh(spheres, init_radius_for_BSH, 1);
	const std::vector< std::vector<std::size_t> > ids=apollota::SplittingSetOfSpheres::split(spheres, depth);
	for(std::size_t i=0;i<ids.size();i++)
	{
		apollota::Triangulation::merge_quadruples_maps(apollota::Triangulation::construct_result_for_admittance_set(bsh, ids[i]).quadruples_map, result_quadruples_map);
	}

	if(!skip_output)
	{
		apollota::Triangulation::print_quadruples_map(result_quadruples_map, std::cout);
	}

	if(print_log)
	{
		std::clog << "balls " <<  spheres.size() << "\n";

		std::clog << "parts " <<  ids.size() << " :";
		for(std::size_t i=0;i<ids.size();i++)
		{
			std::clog << " " << ids[i].size();
		}
		std::clog << "\n";

		std::clog << "quadruples " <<  result_quadruples_map.size() << "\n";
		std::clog << "tangent_spheres " <<  apollota::Triangulation::count_tangent_spheres_in_quadruples_map(result_quadruples_map) << "\n";
	}
}

