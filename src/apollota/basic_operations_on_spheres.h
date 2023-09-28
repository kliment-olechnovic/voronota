#ifndef APOLLOTA_BASIC_OPERATIONS_ON_SPHERES_H_
#define APOLLOTA_BASIC_OPERATIONS_ON_SPHERES_H_

#include "basic_operations_on_points.h"

namespace voronota
{

namespace apollota
{

template<typename OutputSphereType>
OutputSphereType custom_sphere(const double x, const double y, const double z, const double r)
{
	OutputSphereType result;
	result.x=x;
	result.y=y;
	result.z=z;
	result.r=r;
	return result;
}

template<typename OutputSphereType, typename InputObjectType>
OutputSphereType custom_sphere_from_object(const InputObjectType& o)
{
	return custom_sphere<OutputSphereType>(o.x, o.y, o.z, o.r);
}

template<typename OutputSphereType, typename InputPointType>
OutputSphereType custom_sphere_from_point(const InputPointType& p, const double r)
{
	return custom_sphere<OutputSphereType>(p.x, p.y, p.z, r);
}

template<typename InputSphereTypeA, typename InputSphereTypeB>
bool spheres_equal(const InputSphereTypeA& a, const InputSphereTypeB& b, const double epsilon)
{
	return (equal(a.x, b.x, epsilon) && equal(a.y, b.y, epsilon) && equal(a.z, b.z, epsilon) && equal(a.r, b.r, epsilon));
}

template<typename InputSphereTypeA, typename InputSphereTypeB>
bool spheres_less(const InputSphereTypeA& a, const InputSphereTypeB& b)
{
	if(less(a.x, b.x)) { return true; }
	else if(greater(a.x, b.x)) { return false; }

	if(less(a.y, b.y)) { return true; }
	else if(greater(a.y, b.y)) { return false; }

	if(less(a.z, b.z)) { return true; }
	else if(greater(a.z, b.z)) { return false; }

	if(less(a.r, b.r)) { return true; }
	else if(greater(a.r, b.r)) { return false; }

	return false;
}

template<typename InputPointType, typename InputSphereType>
double minimal_distance_from_point_to_sphere(const InputPointType& a, const InputSphereType& b)
{
	return (distance_from_point_to_point(a, b)-b.r);
}

template<typename InputPointType, typename InputSphereType>
double maximal_distance_from_point_to_sphere(const InputPointType& a, const InputSphereType& b)
{
	return (distance_from_point_to_point(a, b)+b.r);
}

template<typename InputSphereTypeA, typename InputSphereTypeB>
double minimal_distance_from_sphere_to_sphere(const InputSphereTypeA& a, const InputSphereTypeB& b)
{
	return (distance_from_point_to_point(a, b)-a.r-b.r);
}

template<typename InputSphereTypeA, typename InputSphereTypeB, typename InputPointTypeA>
double minimal_distance_from_sphere_to_oriented_circle(const InputSphereTypeA& sphere, const InputSphereTypeB& circle, const InputPointTypeA& circle_axis)
{
	const double signed_dist_from_sphere_center_to_circle_plane=signed_distance_from_point_to_plane(circle, circle_axis, sphere);
	const PODPoint closest_point_on_circle_plane=sub_of_points<PODPoint>(sphere, point_and_number_product<PODPoint>(unit_point<PODPoint>(circle_axis), signed_dist_from_sphere_center_to_circle_plane));
	const double distance_on_circle_plane=distance_from_point_to_point(circle, closest_point_on_circle_plane);
	if(distance_on_circle_plane<circle.r)
	{
		return (std::abs(signed_dist_from_sphere_center_to_circle_plane)-sphere.r);
	}
	double distance_surplus=(circle.r-distance_on_circle_plane);
	return (std::sqrt((signed_dist_from_sphere_center_to_circle_plane*signed_dist_from_sphere_center_to_circle_plane)+(distance_surplus*distance_surplus))-sphere.r);
}

template<typename InputSphereTypeA, typename InputSphereTypeB>
bool sphere_intersects_sphere(const InputSphereTypeA& a, const InputSphereTypeB& b)
{
	return less(distance_from_point_to_point(a, b), (a.r+b.r));
}

template<typename InputSphereTypeA, typename InputSphereTypeB>
bool sphere_intersects_sphere_with_expansion(const InputSphereTypeA& a, const InputSphereTypeB& b, const double expansion)
{
	return less(distance_from_point_to_point(a, b), (a.r+b.r+expansion));
}

template<typename InputSphereTypeA, typename InputSphereTypeB>
bool sphere_touches_sphere(const InputSphereTypeA& a, const InputSphereTypeB& b)
{
	return equal(distance_from_point_to_point(a, b), (a.r+b.r));
}

template<typename InputSphereTypeA, typename InputSphereTypeB>
bool sphere_contains_sphere(const InputSphereTypeA& a, const InputSphereTypeB& b)
{
	return (greater_or_equal(a.r, b.r) && less_or_equal(distance_from_point_to_point(a, b), (a.r-b.r)));
}

template<typename InputPointTypeA, typename InputPointTypeB, typename InputSphereTypeC>
int halfspace_of_sphere(const InputPointTypeA& plane_point, const InputPointTypeB& plane_normal, const InputSphereTypeC& x)
{
	const double dc=signed_distance_from_point_to_plane(plane_point, plane_normal, x);
	if(dc>0 && (dc-x.r>0))
	{
		return 1;
	}
	else if(dc<0 && (dc+x.r<0))
	{
		return -1;
	}
	return 0;
}

template<typename OutputSphereType, typename InputSphereTypeA, typename InputSphereTypeB>
OutputSphereType intersection_circle_of_two_spheres(const InputSphereTypeA& a, const InputSphereTypeB& b)
{
	const SimplePoint ap=custom_point_from_object<SimplePoint>(a);
	const SimplePoint bp=custom_point_from_object<SimplePoint>(b);
	const SimplePoint cv=bp-ap;
	const double cm=cv.module();
	const double cos_g=(a.r*a.r+cm*cm-b.r*b.r)/(2*a.r*cm);
	const double sin_g=sqrt(1-cos_g*cos_g);
	return custom_sphere_from_point<OutputSphereType>(ap+(cv.unit()*(a.r*cos_g)), a.r*sin_g);
}

template<typename InputSphereType>
bool intersect_line_segment_with_sphere(const InputSphereType& sphere, const SimplePoint& a, const SimplePoint& b, SimplePoint& c)
{
	if(sphere.r<=0.0)
	{
		return false;
	}
	const double d_oa=distance_from_point_to_point(a, sphere);
	const double d_ob=distance_from_point_to_point(b, sphere);
	if((d_oa<sphere.r && d_ob<sphere.r) || (d_oa>sphere.r && d_ob>sphere.r))
	{
		return false;
	}
	else if(d_oa>sphere.r && d_ob<sphere.r)
	{
		return intersect_line_segment_with_sphere(sphere, b, a, c);
	}
	else
	{
		const double angle_oac=min_angle(a, sphere, b);
		if(equal(angle_oac, 0.0) || equal(angle_oac, pi_value()))
		{
			c=SimplePoint(sphere)+((b-SimplePoint(sphere)).unit()*d_ob);
		}
		else
		{
			const double k=sin(angle_oac)/sphere.r;
			double sin_aio=k*d_oa;
			if(sin_aio<-1.0)
			{
				sin_aio=-1.0;
			}
			else if(sin_aio>1.0)
			{
				sin_aio=1.0;
			}
			const double angle_aco=asin(sin_aio);
			const double angle_aoc=pi_value()-(angle_oac+angle_aco);
			const double d_ai=sin(angle_aoc)/k;
			c=a+((b-a).unit()*d_ai);
		}
		return true;
	}
}

struct SimpleSphere
{
	double x;
	double y;
	double z;
	double r;

	SimpleSphere() : x(0), y(0), z(0), r(0)
	{
	}

	SimpleSphere(const double x, const double y, const double z, const double r) : x(x), y(y), z(z), r(r)
	{
	}

	template<typename InputSphereType>
	explicit SimpleSphere(const InputSphereType& input_sphere) : x(input_sphere.x), y(input_sphere.y), z(input_sphere.z), r(input_sphere.r)
	{
	}

	template<typename InputPointType>
	SimpleSphere(const InputPointType& input_point, const double r) : x(input_point.x), y(input_point.y), z(input_point.z), r(r)
	{
	}

	bool operator==(const SimpleSphere& b) const
	{
		return spheres_equal(*this, b, default_comparison_epsilon());
	}

	bool operator<(const SimpleSphere& b) const
	{
		return spheres_less(*this, b);
	}
};

template<typename T>
inline T& operator<<(T& output, const SimpleSphere& sphere)
{
	output << sphere.x << " " << sphere.y << " " << sphere.z << " " << sphere.r;
	return output;
}

template<typename T>
inline T& operator>>(T& input, SimpleSphere& sphere)
{
	input >> sphere.x >> sphere.y >> sphere.z >> sphere.r;
	return input;
}

}

}

#endif /* APOLLOTA_BASIC_OPERATIONS_ON_SPHERES_H_ */
