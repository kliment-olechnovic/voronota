#include "../apollota/subdivided_icosahedron.h"

#include "../auxiliaries/program_options_handler.h"

namespace
{

}

void rotational_optimization_demo(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "x y z");
	pohw.describe_io("stdout", false, true, "log");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	voronota::apollota::SimplePoint goal;
	std::cin >> goal.x >> goal.y >> goal.z;
	const double goal_length=goal.module();

	std::cout << "goal " << 0.0 << " " << goal.x << " " << goal.y << " " << goal.z << std::endl;

	voronota::apollota::SubdividedIcosahedron sih(2);
	sih.fit_into_sphere(voronota::apollota::SimplePoint(0, 0, 0), 1);
	std::size_t min_dist_id=0;
	double min_dist=0.0;
	int number_of_checks=0;
	while(number_of_checks==0 || min_dist>0.000001)
	{
		std::size_t start_id=0;
		if(number_of_checks>0)
		{
			start_id=sih.vertices().size();
			sih.grow(min_dist_id, true);
		}
		for(std::size_t i=start_id;i<sih.vertices().size();i++)
		{
			const double dist=voronota::apollota::distance_from_point_to_point(sih.vertices()[i]*goal_length, goal);
			if(number_of_checks==0 || dist<min_dist)
			{
				min_dist_id=i;
				min_dist=dist;
			}
			number_of_checks++;
		}
		const voronota::apollota::SimplePoint result=sih.vertices()[min_dist_id]*goal_length;
		std::cout << "result " << min_dist << " " << result.x << " " << result.y << " " << result.z << " edge " << sih.calc_max_edge_length() << std::endl;
	}

	std::cout << "checks " << number_of_checks << std::endl;
}
