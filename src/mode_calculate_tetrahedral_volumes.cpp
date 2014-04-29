#include <iostream>

#include "apollota/triangulation.h"

#include "modes_commons.h"

void calculate_tetrahedral_volumes(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--probes"].init("list", "list of probe radius values");
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions full_map_of_option_descriptions=basic_map_of_option_descriptions;
		full_map_of_option_descriptions["--exclude-hidden-balls"].init("", "flag to exclude hidden input balls");
		full_map_of_option_descriptions["--init-radius-for-BSH"].init("number", "initial radius for bounding sphere hierarchy");
		if(poh.contains_option("--help") || poh.contains_option("--help-full"))
		{
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions(poh.contains_option("--help-full") ? full_map_of_option_descriptions : basic_map_of_option_descriptions, std::cerr);
			std::cerr << "\n";
			std::cerr << "  stdin   <-  list of balls (line format: 'x y z r # comments')\n";
			std::cerr << "  stdout  ->  full volume and probe-shaped volumes\n";
			return;
		}
		else
		{
			poh.compare_with_map_of_option_descriptions(full_map_of_option_descriptions);
		}
	}

	const std::vector<double> probes=poh.argument_vector<double>("--probes");

	const bool exclude_hidden_balls=poh.contains_option("--exclude-hidden-balls");

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

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, init_radius_for_BSH, exclude_hidden_balls, false);

	double full_volume=0.0;
	std::vector<double> shape_volumes(probes.size(), 0.0);
	for(apollota::Triangulation::QuadruplesMap::const_iterator it=triangulation_result.quadruples_map.begin();it!=triangulation_result.quadruples_map.end();++it)
	{
		const apollota::Quadruple& q=it->first;
		const double volume=fabs(apollota::signed_volume_of_tetrahedron(spheres[q.get(0)], spheres[q.get(1)], spheres[q.get(2)], spheres[q.get(3)]));
		full_volume+=volume;
		for(std::size_t i=0;i<probes.size();i++)
		{
			const double max_dist=probes[i]*2.0;
			if(apollota::minimal_distance_from_sphere_to_sphere(spheres[q.get(0)], spheres[q.get(1)])<max_dist &&
					apollota::minimal_distance_from_sphere_to_sphere(spheres[q.get(0)], spheres[q.get(2)])<max_dist &&
					apollota::minimal_distance_from_sphere_to_sphere(spheres[q.get(0)], spheres[q.get(3)])<max_dist &&
					apollota::minimal_distance_from_sphere_to_sphere(spheres[q.get(1)], spheres[q.get(2)])<max_dist &&
					apollota::minimal_distance_from_sphere_to_sphere(spheres[q.get(1)], spheres[q.get(3)])<max_dist &&
					apollota::minimal_distance_from_sphere_to_sphere(spheres[q.get(2)], spheres[q.get(3)])<max_dist)
			{
				shape_volumes[i]+=volume;
			}
		}
	}

	std::cout << "balls quadruples full_volume";
	for(std::size_t i=0;i<probes.size();i++)
	{
		std::cout << " probe_" << probes[i];
	}
	std::cout << "\n";
	std::cout << spheres.size() << " " << triangulation_result.quadruples_map.size() << " " << full_volume;
	for(std::size_t i=0;i<shape_volumes.size();i++)
	{
		std::cout << " " << shape_volumes[i];
	}
	std::cout << "\n";
}
