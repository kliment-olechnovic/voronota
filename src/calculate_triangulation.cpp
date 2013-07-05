#include <iostream>

#include "apollota/triangulation.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/command_line_options.h"

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

void calculate_triangulation(const auxiliaries::CommandLineOptions& clo)
{
	clo.check_allowed_options("--epsilon: --bounding-spheres-hierarchy-first-radius: --allow-redundant-quadruples --skip-output --print-log --check");

	if(clo.isopt("--epsilon"))
	{
		const double epsilon=clo.arg<double>("--epsilon");
		if(epsilon<=0.0)
		{
			throw std::runtime_error("Epsilon is not greater than 0.");
		}
		apollota::comparison_epsilon_reference()=epsilon;
	}

	const double bounding_spheres_hierarchy_first_radius=clo.isopt("--bounding-spheres-hierarchy-first-radius") ? clo.arg<double>("--bounding-spheres-hierarchy-first-radius") : 3.5;
	if(bounding_spheres_hierarchy_first_radius<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy first radius is not greater than 1.");
	}

	const bool allow_redundant_quadruples=clo.isopt("--allow-redundant-quadruples");
	const bool skip_output=clo.isopt("--skip-output");
	const bool print_log=clo.isopt("--print-log");
	const bool check=clo.isopt("--check");

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
