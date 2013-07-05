#include <iostream>
#include <sstream>

#include "apollota/triangulation.h"

#include "auxiliaries/command_line_options.h"

namespace
{

std::vector<apollota::SimpleSphere> read_spheres(std::istream& input)
{
	std::vector<apollota::SimpleSphere> result;
	while(input.good())
	{
		std::string line;
		std::getline(input, line);
		line.substr(0, line.find("#", 0));
		if(!line.empty())
		{
			std::istringstream line_input(line);
			apollota::SimpleSphere sphere;
			line_input >> sphere.x >> sphere.y >> sphere.z >> sphere.r;
			if(!line_input.fail())
			{
				result.push_back(sphere);
			}
		}
	}
	return result;
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

	const std::vector<apollota::SimpleSphere> spheres=read_spheres(std::cin);
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
