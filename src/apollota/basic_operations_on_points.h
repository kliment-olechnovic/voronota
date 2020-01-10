#ifndef APOLLOTA_BASIC_OPERATIONS_ON_POINTS_H_
#define APOLLOTA_BASIC_OPERATIONS_ON_POINTS_H_

#include <cmath>

#include "safer_comparison_of_numbers.h"

namespace voronota
{

namespace apollota
{

struct PODPoint
{
	double x;
	double y;
	double z;
};

template<typename OutputPointType>
OutputPointType custom_point(const double x, const double y, const double z)
{
	OutputPointType result;
	result.x=x;
	result.y=y;
	result.z=z;
	return result;
}

template<typename OutputPointType, typename InputObjectType>
OutputPointType custom_point_from_object(const InputObjectType& o)
{
	return custom_point<OutputPointType>(o.x, o.y, o.z);
}

template<typename InputPointTypeA, typename InputPointTypeB>
bool points_equal(const InputPointTypeA& a, const InputPointTypeB& b)
{
	return (equal(a.x, b.x) && equal(a.y, b.y) && equal(a.z, b.z));
}

template<typename InputPointTypeA, typename InputPointTypeB>
bool points_less(const InputPointTypeA& a, const InputPointTypeB& b)
{
	if(less(a.x, b.x)) { return true; }
	else if(greater(a.x, b.x)) { return false; }

	if(less(a.y, b.y)) { return true; }
	else if(greater(a.y, b.y)) { return false; }

	if(less(a.z, b.z)) { return true; }
	else if(greater(a.z, b.z)) { return false; }

	return false;
}

template<typename OutputPointType, typename InputPointType>
OutputPointType inverted_point(const InputPointType& a)
{
	return custom_point<OutputPointType>(0-a.x, 0-a.y, 0-a.z);
}

template<typename InputPointTypeA, typename InputPointTypeB>
double squared_distance_from_point_to_point(const InputPointTypeA& a, const InputPointTypeB& b)
{
	const double dx=(a.x-b.x);
	const double dy=(a.y-b.y);
	const double dz=(a.z-b.z);
	return (dx*dx+dy*dy+dz*dz);
}

template<typename InputPointTypeA, typename InputPointTypeB>
double distance_from_point_to_point(const InputPointTypeA& a, const InputPointTypeB& b)
{
	return sqrt(squared_distance_from_point_to_point(a, b));
}

template<typename InputPointType>
double squared_point_module(const InputPointType& a)
{
	return (a.x*a.x+a.y*a.y+a.z*a.z);
}

template<typename InputPointType>
double point_module(const InputPointType& a)
{
	return sqrt(squared_point_module(a));
}

template<typename InputPointTypeA, typename InputPointTypeB>
double dot_product(const InputPointTypeA& a, const InputPointTypeB& b)
{
	return (a.x*b.x+a.y*b.y+a.z*b.z);
}

template<typename OutputPointType, typename InputPointTypeA, typename InputPointTypeB>
OutputPointType cross_product(const InputPointTypeA& a, const InputPointTypeB& b)
{
	return custom_point<OutputPointType>(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
}

template<typename OutputPointType, typename InputPointType>
OutputPointType point_and_number_product(const InputPointType& a, const double k)
{
	return custom_point<OutputPointType>(a.x*k, a.y*k, a.z*k);
}

template<typename OutputPointType, typename InputPointType>
OutputPointType unit_point(const InputPointType& a)
{
	return point_and_number_product<OutputPointType>(a, 1/point_module(a));
}

template<typename OutputPointType, typename InputPointTypeA, typename InputPointTypeB>
OutputPointType sum_of_points(const InputPointTypeA& a, const InputPointTypeB& b)
{
	return custom_point<OutputPointType>(a.x+b.x, a.y+b.y, a.z+b.z);
}

template<typename OutputPointType, typename InputPointTypeA, typename InputPointTypeB>
OutputPointType sub_of_points(const InputPointTypeA& a, const InputPointTypeB& b)
{
	return custom_point<OutputPointType>(a.x-b.x, a.y-b.y, a.z-b.z);
}

template<typename OutputPointType, typename InputPointTypeA, typename InputPointTypeB, typename InputPointTypeC>
OutputPointType plane_normal_from_three_points(const InputPointTypeA& a, const InputPointTypeB& b, const InputPointTypeC& c)
{
	return unit_point<OutputPointType>(cross_product<PODPoint>(sub_of_points<PODPoint>(b, a), sub_of_points<PODPoint>(c, a)));
}

template<typename InputPointTypeA, typename InputPointTypeB, typename InputPointTypeC>
double signed_distance_from_point_to_plane(const InputPointTypeA& plane_point, const InputPointTypeB& plane_normal, const InputPointTypeC& x)
{
	return dot_product(unit_point<PODPoint>(plane_normal), sub_of_points<PODPoint>(x, plane_point));
}

template<typename InputPointTypeA, typename InputPointTypeB, typename InputPointTypeC>
int halfspace_of_point(const InputPointTypeA& plane_point, const InputPointTypeB& plane_normal, const InputPointTypeC& x)
{
	const double sd=signed_distance_from_point_to_plane(plane_point, plane_normal, x);
	if(sd>0)
	{
		return 1;
	}
	else if(sd<0)
	{
		return -1;
	}
	return 0;
}

template<typename OutputPointType, typename InputPointTypeA, typename InputPointTypeB, typename InputPointTypeC, typename InputPointTypeD>
OutputPointType intersection_of_plane_and_segment(const InputPointTypeA& plane_point, const InputPointTypeB& plane_normal, const InputPointTypeC& a, const InputPointTypeD& b)
{
	const double da=signed_distance_from_point_to_plane(plane_point, plane_normal, a);
	const double db=signed_distance_from_point_to_plane(plane_point, plane_normal, b);
	if((da-db)==0)
	{
		return custom_point_from_object<OutputPointType>(a);
	}
	else
	{
		const double t=da/(da-db);
		return sum_of_points<OutputPointType>(a, point_and_number_product<PODPoint>(sub_of_points<PODPoint>(b, a), t));
	}
}

template<typename InputPointTypeA, typename InputPointTypeB, typename InputPointTypeC, typename InputPointTypeD>
double signed_volume_of_tetrahedron(const InputPointTypeA& a, const InputPointTypeB& b, const InputPointTypeC& c, const InputPointTypeD& d)
{
	return (dot_product(sub_of_points<PODPoint>(a, d), cross_product<PODPoint>(sub_of_points<PODPoint>(b, d), sub_of_points<PODPoint>(c, d)))/6);
}

template<typename InputPointTypeA, typename InputPointTypeB, typename InputPointTypeC>
double triangle_area(const InputPointTypeA& a, const InputPointTypeB& b, const InputPointTypeC& c)
{
	return (point_module(cross_product<PODPoint>(sub_of_points<PODPoint>(b, a), sub_of_points<PODPoint>(c, a)))/2.0);
}

template<typename InputPointO, typename InputPointA, typename InputPointB>
static double min_angle(const InputPointO& o, const InputPointA& a, const InputPointB& b)
{
	double cos_val=dot_product(unit_point<PODPoint>(sub_of_points<PODPoint>(a, o)), unit_point<PODPoint>(sub_of_points<PODPoint>(b, o)));
	if(cos_val<-1.0)
	{
		cos_val=-1.0;
	}
	else if(cos_val>1.0)
	{
		cos_val=1.0;
	}
	return acos(cos_val);
}

inline double pi_value()
{
	static const double pi=acos(-1.0);
	return pi;
}

template<typename InputPointO, typename InputPointA, typename InputPointB, typename InputPointC>
static double directed_angle(const InputPointO& o, const InputPointA& a, const InputPointB& b, const InputPointC& c)
{
	const double angle=min_angle(o, a, b);
	const PODPoint n=cross_product<PODPoint>(unit_point<PODPoint>(sub_of_points<PODPoint>(a, o)), unit_point<PODPoint>(sub_of_points<PODPoint>(b, o)));
	if(dot_product(sub_of_points<PODPoint>(c, o), n)>=0)
	{
		return angle;
	}
	else
	{
		return (pi_value()*2-angle);
	}
}

template<typename InputPointA0, typename InputPointA1, typename InputPointB0, typename InputPointB1>
static double min_dihedral_angle(const InputPointA0& a0, const InputPointA1& a1, const InputPointB0& b0, const InputPointB1& b1)
{
	if(points_equal(a0, b0) || points_equal(a0, a1) || points_equal(b0, b1))
	{
		return 0.0;
	}
	const PODPoint c=unit_point<PODPoint>(sub_of_points<PODPoint>(b0, a0));
	const PODPoint a2=cross_product<PODPoint>(c, unit_point<PODPoint>(sub_of_points<PODPoint>(a1, a0)));
	if(equal(squared_point_module(a2), 0.0))
	{
		return 0.0;
	}
	const PODPoint b2=cross_product<PODPoint>(c, unit_point<PODPoint>(sub_of_points<PODPoint>(b1, b0)));
	if(equal(squared_point_module(b2), 0.0))
	{
		return 0.0;
	}
	return min_angle(custom_point<PODPoint>(0.0, 0.0, 0.0), a2, b2);
}

template<typename InputSphereType, typename InputPointTypeA, typename InputPointTypeB, typename InputPointTypeC>
double spherical_triangle_area(const InputSphereType& s, const InputPointTypeA& a, const InputPointTypeB& b, const InputPointTypeC& c)
{
	const double va=min_angle(a, b, c);
	const double vb=min_angle(b, a, c);
	const double vc=min_angle(c, a, b);
	const PODPoint sa=sub_of_points<PODPoint>(a, s);
	const PODPoint sb=sub_of_points<PODPoint>(b, s);
	const PODPoint sc=sub_of_points<PODPoint>(c, s);
	const PODPoint o=custom_point<PODPoint>(0.0, 0.0, 0.0);
	const PODPoint nab=cross_product<PODPoint>(sa, sb);
	const PODPoint nac=cross_product<PODPoint>(sa, sc);
	const PODPoint nbc=cross_product<PODPoint>(sb, sc);
	const double vpa=min_angle(o, nab, nac);
	const double vpb=min_angle(o, inverted_point<PODPoint>(nab), nbc);
	const double vpc=min_angle(o, inverted_point<PODPoint>(nac), inverted_point<PODPoint>(nbc));
	const double e=(vpa+vpb+vpc)-(va+vb+vc);
	return (e*s.r*s.r);
}

template<typename OutputPointType, typename InputPointType>
OutputPointType any_normal_of_vector(const InputPointType& a)
{
	PODPoint b=custom_point_from_object<PODPoint>(a);
	if(!equal(b.x, 0.0) && (!equal(b.y, 0.0) || !equal(b.z, 0.0)))
	{
		b.x=0.0-b.x;
		return unit_point<OutputPointType>(cross_product<OutputPointType>(a, b));
	}
	else if(!equal(b.y, 0.0) && (!equal(b.x, 0.0) || !equal(b.z, 0.0)))
	{
		b.y=0.0-b.y;
		return unit_point<OutputPointType>(cross_product<OutputPointType>(a, b));
	}
	else if(!equal(b.x, 0.0))
	{
		return custom_point<OutputPointType>(0, 1, 0);
	}
	else
	{
		return custom_point<OutputPointType>(1, 0, 0);
	}
}

template<typename InputPointTypeA, typename InputPointTypeB, typename InputPointTypeC>
double distance_from_point_to_line(const InputPointTypeA& p, const InputPointTypeB& start, const InputPointTypeC& end)
{
	const PODPoint line_vector=unit_point<PODPoint>(sub_of_points<PODPoint>(end, start));
	const PODPoint translated_p=sub_of_points<PODPoint>(p, start);
	const double distance_on_line=dot_product(translated_p, line_vector);
	return sqrt(squared_point_module(translated_p)-(distance_on_line*distance_on_line));
}

template<typename OutputPointType, typename InputIterator>
static OutputPointType mass_center(const InputIterator begin, const InputIterator end)
{
	PODPoint mc=custom_point<PODPoint>(0.0, 0.0, 0.0);
	unsigned int count=0;
	InputIterator it=begin;
	while(it!=end)
	{
		mc=sum_of_points<PODPoint>(mc, *it);
		count++;
		++it;
	}
	if(count>0)
	{
		mc=point_and_number_product<PODPoint>(mc, 1.0/static_cast<double>(count));
	}
	return custom_point_from_object<OutputPointType>(mc);
}

struct SimplePoint
{
	double x;
	double y;
	double z;

	SimplePoint() : x(0), y(0), z(0)
	{
	}

	SimplePoint(const double x, const double y, const double z) : x(x), y(y), z(z)
	{
	}

	template<typename InputPointType>
	explicit SimplePoint(const InputPointType& input_point) : x(input_point.x), y(input_point.y), z(input_point.z)
	{
	}

	bool operator==(const SimplePoint& b) const
	{
		return points_equal(*this, b);
	}

	bool operator<(const SimplePoint& b) const
	{
		return points_less(*this, b);
	}

	SimplePoint operator+(const SimplePoint& b) const
	{
		return sum_of_points<SimplePoint>(*this, b);
	}

	SimplePoint operator-(const SimplePoint& b) const
	{
		return sub_of_points<SimplePoint>(*this, b);
	}

	double operator*(const SimplePoint& b) const
	{
		return dot_product(*this, b);
	}

	SimplePoint operator*(const double k) const
	{
		return point_and_number_product<SimplePoint>(*this, k);
	}

	SimplePoint operator&(const SimplePoint& b) const
	{
		return cross_product<SimplePoint>(*this, b);
	}

	double module() const
	{
		return point_module(*this);
	}

	SimplePoint unit() const
	{
		return unit_point<SimplePoint>(*this);
	}

	SimplePoint inverted() const
	{
		return inverted_point<SimplePoint>(*this);
	}
};

template<typename T>
inline T& operator<<(T& output, const SimplePoint& p)
{
	output << p.x << " " << p.y << " " << p.z;
	return output;
}

template<typename T>
inline T& operator>>(T& input, SimplePoint& p)
{
	input >> p.x >> p.y >> p.z;
	return input;
}

}

}

#endif /* APOLLOTA_BASIC_OPERATIONS_ON_POINTS_H_ */
