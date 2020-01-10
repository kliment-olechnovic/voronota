#ifndef APOLLOTA_ROLLING_TOPOLOGY_H_
#define APOLLOTA_ROLLING_TOPOLOGY_H_

#include <list>
#include <vector>
#include <set>

#include "tangent_sphere_of_three_spheres.h"
#include "polar_sorting.h"
#include "rotation.h"

namespace voronota
{

namespace apollota
{

class RollingTopology
{
public:
	struct RollingPoint
	{
		std::size_t generator;
		SimpleSphere tangent;
	};

	struct RollingStrip
	{
		RollingPoint start;
		RollingPoint end;
	};

	struct RollingDescriptor
	{
		std::size_t a_id;
		std::size_t b_id;
		bool possible;
		SimpleSphere circle;
		SimplePoint axis;
		bool detached;
		std::set<std::size_t> generators;
		std::list<RollingStrip> strips;
		std::vector<SimplePoint> breaks;
	};

	static inline RollingDescriptor calculate_rolling_descriptor(
			const std::vector<SimpleSphere>& spheres,
			const std::size_t a_id,
			const std::size_t b_id,
			const std::set<std::size_t>& neighbor_ids,
			const std::set<std::size_t>& a_neighbor_ids,
			const std::set<std::size_t>& b_neighbor_ids,
			const double probe)
	{
		RollingDescriptor result;
		result.a_id=a_id;
		result.b_id=b_id;
		const SimpleSphere& a=spheres[a_id];
		const SimpleSphere& b=spheres[b_id];
		result.possible=sphere_intersects_sphere_with_expansion(a, b, probe*2);
		if(result.possible)
		{
			result.circle=intersection_circle_of_two_spheres<SimpleSphere>(SimpleSphere(a, a.r+probe), SimpleSphere(b, b.r+probe));
			result.axis=sub_of_points<SimplePoint>(a, b).unit();
			if(result.circle.r<probe)
			{
				result.breaks.resize(2);
				const double dl=sqrt((probe*probe)-(result.circle.r*result.circle.r));
				result.breaks[0]=sum_of_points<SimplePoint>(result.circle, point_and_number_product<PODPoint>(result.axis, dl));
				result.breaks[1]=sum_of_points<SimplePoint>(result.circle, point_and_number_product<PODPoint>(result.axis, 0.0-dl));
			}
			result.detached=true;
			for(std::set<std::size_t>::const_iterator c_id_it=neighbor_ids.begin();c_id_it!=neighbor_ids.end() && result.detached;++c_id_it)
			{
				const SimpleSphere expanded_c(spheres[*c_id_it], spheres[*c_id_it].r+probe);
				const double distance_to_circle_plane=signed_distance_from_point_to_plane(result.circle, result.axis, expanded_c);
				if(fabs(distance_to_circle_plane)<expanded_c.r)
				{
					const SimpleSphere projected_c(SimplePoint(expanded_c)-(result.axis*distance_to_circle_plane), sqrt((expanded_c.r*expanded_c.r)-(distance_to_circle_plane*distance_to_circle_plane)));
					if(sphere_intersects_sphere(result.circle, projected_c))
					{
						result.detached=false;
					}
				}
			}
			if(!result.detached)
			{
				std::list<RollingPoint> rolling_points;
				for(std::set<std::size_t>::const_iterator c_id_it=neighbor_ids.begin();c_id_it!=neighbor_ids.end();++c_id_it)
				{
					const std::vector<SimpleSphere> tangents=TangentSphereOfThreeSpheres::calculate(a, b, spheres[*c_id_it], probe);
					if(tangents.size()==2)
					{
						const double allowed_tangent_error=0.00001;
						for(std::vector<SimpleSphere>::const_iterator tangent_it=tangents.begin();tangent_it!=tangents.end();++tangent_it)
						{
							bool tangent_empty=true;
							for(std::set<std::size_t>::const_iterator d_id_it=neighbor_ids.begin();d_id_it!=neighbor_ids.end() && tangent_empty;++d_id_it)
							{
								if(d_id_it!=c_id_it)
								{
									tangent_empty=!sphere_intersects_sphere_with_expansion((*tangent_it), spheres[*d_id_it], 0.0-allowed_tangent_error);
								}
							}
							for(std::set<std::size_t>::const_iterator d_id_it=a_neighbor_ids.begin();d_id_it!=a_neighbor_ids.end() && tangent_empty;++d_id_it)
							{
								if(d_id_it!=c_id_it && (*d_id_it)!=a_id && (*d_id_it)!=b_id)
								{
									tangent_empty=!sphere_intersects_sphere_with_expansion((*tangent_it), spheres[*d_id_it], 0.0-allowed_tangent_error);
								}
							}
							for(std::set<std::size_t>::const_iterator d_id_it=b_neighbor_ids.begin();d_id_it!=b_neighbor_ids.end() && tangent_empty;++d_id_it)
							{
								if(d_id_it!=c_id_it && (*d_id_it)!=a_id && (*d_id_it)!=b_id)
								{
									tangent_empty=!sphere_intersects_sphere_with_expansion((*tangent_it), spheres[*d_id_it], 0.0-allowed_tangent_error);
								}
							}
							if(tangent_empty)
							{
								result.generators.insert(*c_id_it);
								RollingPoint rolling_point;
								rolling_point.tangent=(*tangent_it);
								rolling_point.generator=(*c_id_it);
								rolling_points.push_back(rolling_point);
							}
						}
					}
				}
				if(rolling_points.size()>=2 && rolling_points.size()%2==0)
				{
					std::vector< std::pair<SimpleSphere, std::pair<bool, std::size_t> > > circular_mapping;
					{
						circular_mapping.reserve(rolling_points.size()*2);
						for(std::set<std::size_t>::const_iterator c_id_it=result.generators.begin();c_id_it!=result.generators.end();++c_id_it)
						{
							circular_mapping.push_back(std::make_pair(spheres[*c_id_it], std::make_pair(false, (*c_id_it))));
						}
						for(std::list<RollingPoint>::const_iterator rolling_point_it=rolling_points.begin();rolling_point_it!=rolling_points.end();++rolling_point_it)
						{
							circular_mapping.push_back(std::make_pair(rolling_point_it->tangent, std::make_pair(true, rolling_point_it->generator)));
						}
					}
					PolarSorting::sort_mapping(result.circle, result.axis, circular_mapping.begin(), circular_mapping.end());
					for(std::size_t i=0;i<circular_mapping.size();i++)
					{
						const std::size_t j=((i+1<circular_mapping.size()) ? (i+1) : 0);
						if(circular_mapping[i].second.first && circular_mapping[j].second.first)
						{
							RollingStrip rolling_strip;
							rolling_strip.start.tangent=circular_mapping[i].first;
							rolling_strip.start.generator=circular_mapping[i].second.second;
							rolling_strip.end.tangent=circular_mapping[j].first;
							rolling_strip.end.generator=circular_mapping[j].second.second;
							result.strips.push_back(rolling_strip);
						}
					}
				}
			}
		}
		return result;
	}

	static std::vector<SimplePoint> construct_rolling_circle_approximation(const RollingDescriptor& rolling_descriptor, const double angle_step)
	{
		return construct_circular_arc_approximation_from_axis_and_start(
				SimplePoint(rolling_descriptor.circle),
				rolling_descriptor.axis,
				any_normal_of_vector<SimplePoint>(rolling_descriptor.axis)*rolling_descriptor.circle.r,
				pi_value()*2,
				angle_step);
	}

	static std::vector<SimplePoint> construct_rolling_strip_approximation(const RollingDescriptor& rolling_descriptor, const RollingStrip& rolling_strip, const double angle_step)
	{
		const SimplePoint base(rolling_descriptor.circle);
		const SimplePoint start_vector=(SimplePoint(rolling_strip.start.tangent)-base);
		const SimplePoint end_vector=(SimplePoint(rolling_strip.end.tangent)-base);
		const double angle=directed_angle(SimplePoint(0, 0, 0), start_vector, end_vector, rolling_descriptor.axis);
		return construct_circular_arc_approximation_from_axis_and_start(
				base,
				rolling_descriptor.axis,
				start_vector,
				angle,
				angle_step);
	}

	static std::vector<SimplePoint> construct_circular_arc_approximation_from_start_and_end(const SimplePoint& center, const SimplePoint& start_vector, const SimplePoint& end_vector, const double angle_step)
	{
		const double angle=min_angle(SimplePoint(0, 0, 0), start_vector, end_vector);
		const int steps=static_cast<int>(floor(angle/angle_step)+1.0);
		return construct_circular_arc_approximation_from_start_and_end(center, start_vector, end_vector, steps);
	}

	static std::vector<SimplePoint> construct_circular_arc_approximation_from_start_and_end(const SimplePoint& base, const SimplePoint& start_vector, const SimplePoint& end_vector, const int steps)
	{
		SimplePoint axis=(start_vector&end_vector).unit();
		double angle=directed_angle(SimplePoint(0, 0, 0), start_vector, end_vector, axis);
		if(angle>pi_value())
		{
			axis=axis.inverted();
			angle=(pi_value()*2-angle);
		}
		return construct_circular_arc_approximation_from_axis_and_start(base, axis, start_vector, angle, steps);
	}

	static std::vector<SimplePoint> construct_circular_arc_approximation_from_axis_and_start(const SimplePoint& base, const SimplePoint& axis, const SimplePoint& start_vector, const double angle, const double angle_step)
	{
		const int steps=static_cast<int>(floor(angle/angle_step)+1.0);
		return construct_circular_arc_approximation_from_axis_and_start(base, axis, start_vector, angle, steps);
	}

	static std::vector<SimplePoint> construct_circular_arc_approximation_from_axis_and_start(const SimplePoint& base, const SimplePoint& axis, const SimplePoint& start_vector, const double angle, const int steps)
	{
		std::vector<SimplePoint> result;
		Rotation rotation(axis, 0, true);
		const double adjusted_angle_step=angle/static_cast<double>(steps);
		result.reserve(steps+1);
		for(int i=0;i<=steps;i++)
		{
			rotation.angle=adjusted_angle_step*static_cast<double>(i);
			result.push_back(base+rotation.rotate<SimplePoint>(start_vector));
		}
		return result;
	}
};

}

}

#endif /* APOLLOTA_ROLLING_TOPOLOGY_H_ */
