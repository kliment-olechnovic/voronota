#ifndef APOLLOTA_BASIC_OPERATIONS_ON_SPHERES_H_
#define APOLLOTA_BASIC_OPERATIONS_ON_SPHERES_H_

#include "basic_operations_on_points.h"

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

template<typename InputSphereTypeA,typename InputSphereTypeB>
double minimal_distance_from_sphere_to_sphere(const InputSphereTypeA& a, const InputSphereTypeB& b)
{
	return (distance_from_point_to_point(a, b)-a.r-b.r);
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
	SimpleSphere(const InputSphereType& input_sphere) : x(input_sphere.x), y(input_sphere.y), z(input_sphere.z), r(input_sphere.r)
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
};

}

#endif /* APOLLOTA_BASIC_OPERATIONS_ON_SPHERES_H_ */
