#ifndef DRAWING_LINKS_H_
#define DRAWING_LINKS_H_

#include "../apollota/search_for_spherical_collisions.h"

#include "drawing_cylinder.h"
#include "ball_value.h"

namespace
{

void draw_links(
		const std::vector< std::pair<CRAD, BallValue> >& list_of_balls,
		const double ball_collision_radius,
		const double bsh_initial_radius,
		const double ball_drawing_radius,
		const double cylinder_drawing_radius,
		const int cylinder_sides,
		const bool check_sequence,
		const DrawingParametersWrapper& drawing_parameters_wrapper,
		auxiliaries::OpenGLPrinter& opengl_printer)
{
	std::vector<apollota::SimpleSphere> spheres(list_of_balls.size());
	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		spheres[i]=apollota::SimpleSphere(list_of_balls[i].second, ball_collision_radius);
	}
	apollota::BoundingSpheresHierarchy bsh(spheres, bsh_initial_radius, 1);
	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		const apollota::SimpleSphere& a=spheres[i];
		const CRAD& a_crad=list_of_balls[i].first;
		drawing_parameters_wrapper.process(a_crad, list_of_balls[i].second.props.adjuncts, opengl_printer);
		opengl_printer.add_sphere(apollota::SimpleSphere(a, ball_drawing_radius));
		std::vector<std::size_t> collisions=apollota::SearchForSphericalCollisions::find_all_collisions(bsh, a);
		for(std::size_t j=0;j<collisions.size();j++)
		{
			const std::size_t collision_id=collisions[j];
			if(i!=collision_id)
			{
				const apollota::SimpleSphere& b=spheres[collision_id];
				const CRAD& b_crad=list_of_balls[collision_id].first;
				if(!check_sequence || (a_crad.chainID==b_crad.chainID && abs(a_crad.resSeq-b_crad.resSeq)<=1))
				draw_cylinder(
						apollota::SimpleSphere(a, cylinder_drawing_radius),
						apollota::SimpleSphere(apollota::sum_of_points<apollota::SimplePoint>(a, b)*0.5, cylinder_drawing_radius),
						cylinder_sides,
						opengl_printer);
			}
		}
	}
}

void draw_trace(
		const std::vector< std::pair<CRAD, BallValue> >& list_of_balls,
		const std::string& atom_name,
		const double max_distance,
		const double drawing_radius,
		const DrawingParametersWrapper& drawing_parameters_wrapper,
		auxiliaries::OpenGLPrinter& opengl_printer)
{
	std::vector< std::pair<CRAD, BallValue> > list_of_balls_filtered;
	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		if(list_of_balls[i].first.name==atom_name)
		{
			list_of_balls_filtered.push_back(list_of_balls[i]);
		}
	}
	draw_links(list_of_balls_filtered, max_distance*0.5, 10.0, drawing_radius, drawing_radius, 12, true, drawing_parameters_wrapper, opengl_printer);
}

}

#endif /* DRAWING_LINKS_H_ */
