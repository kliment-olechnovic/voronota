#include <iostream>

#include "apollota/triangulation.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/command_line_options_handler.h"

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

void calculate_triangulation(const auxiliaries::CommandLineOptionsHandler& clo, const bool print_help)
{
	{
		typedef auxiliaries::CommandLineOptionsHandler Clo;
		Clo::MapOfOptionDescriptions map_of_option_descriptions;
		map_of_option_descriptions["--bounding-spheres-hierarchy-first-radius"]=Clo::OptionDescription(true, "initial radius for bounding sphere hierarchy");
		map_of_option_descriptions["--allow-redundant-quadruples"]=Clo::OptionDescription(false, "flag to include redundant quadruples");
		map_of_option_descriptions["--skip-output"]=Clo::OptionDescription(false, "flag to disable output of the resulting triangulation");
		map_of_option_descriptions["--print-log"]=Clo::OptionDescription(false, "flag to print log of calculations");
		map_of_option_descriptions["--check"]=Clo::OptionDescription(false, "flag to explicitely check the resulting triangulation (takes time, used only for testing)");
		if(print_help)
		{
			Clo::print_map_of_option_descriptions(map_of_option_descriptions, "    ", std::cerr);
			return;
		}
		else
		{
			clo.compare_with_map_of_option_descriptions(map_of_option_descriptions);
		}
	}

	const double bounding_spheres_hierarchy_first_radius=clo.argument<double>("--bounding-spheres-hierarchy-first-radius", 3.5);
	if(bounding_spheres_hierarchy_first_radius<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy first radius is not greater than 1.");
	}

	const bool allow_redundant_quadruples=clo.contains_option("--allow-redundant-quadruples");
	const bool skip_output=clo.contains_option("--skip-output");
	const bool print_log=clo.contains_option("--print-log");
	const bool check=clo.contains_option("--check");

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
