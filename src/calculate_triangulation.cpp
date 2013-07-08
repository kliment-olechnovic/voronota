#include <iostream>

#include "apollota/triangulation.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/program_options_handler.h"

namespace
{

void add_sphere_from_stream_to_vector(std::istream& input, std::vector<apollota::SimpleSphere>& spheres)
{
	apollota::SimpleSphere sphere;
	input >> sphere.x >> sphere.y >> sphere.z >> sphere.r;
	if(!input.fail())
	{
		spheres.push_back(sphere);
	}
}

}

void calculate_triangulation(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler POH;
		POH::MapOfOptionDescriptions map_of_option_descriptions;
		map_of_option_descriptions["--bounding-spheres-hierarchy-first-radius"]=POH::OptionDescription("number", "initial radius for bounding sphere hierarchy");
		map_of_option_descriptions["--allow-redundant-quadruples"]=POH::OptionDescription("", "flag to include redundant quadruples");
		map_of_option_descriptions["--skip-output"]=POH::OptionDescription("", "flag to disable output of the resulting triangulation");
		map_of_option_descriptions["--print-log"]=POH::OptionDescription("", "flag to print log of calculations");
		map_of_option_descriptions["--check"]=POH::OptionDescription("", "flag to explicitly check the resulting triangulation (takes time, used only for testing)");
		if(poh.contains_option("--help"))
		{
			POH::print_map_of_option_descriptions(map_of_option_descriptions, std::cerr);
			return;
		}
		else
		{
			poh.compare_with_map_of_option_descriptions(map_of_option_descriptions);
		}
	}

	const double bounding_spheres_hierarchy_first_radius=poh.argument<double>("--bounding-spheres-hierarchy-first-radius", 3.5);
	if(bounding_spheres_hierarchy_first_radius<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy first radius is not greater than 1.");
	}

	const bool allow_redundant_quadruples=poh.contains_option("--allow-redundant-quadruples");
	const bool skip_output=poh.contains_option("--skip-output");
	const bool print_log=poh.contains_option("--print-log");
	const bool check=poh.contains_option("--check");

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", add_sphere_from_stream_to_vector, spheres);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided.");
	}

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, bounding_spheres_hierarchy_first_radius, allow_redundant_quadruples);

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
