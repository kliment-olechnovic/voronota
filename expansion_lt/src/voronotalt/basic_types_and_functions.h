#ifndef VORONOTALT_BASIC_TYPES_AND_FUNCTIONS_H_
#define VORONOTALT_BASIC_TYPES_AND_FUNCTIONS_H_

#include <cmath>

namespace voronotalt
{

struct SimplePoint
{
	double x;
	double y;
	double z;

	SimplePoint() : x(0.0), y(0.0), z(0.0)
	{
	}

	SimplePoint(const double x, const double y, const double z) : x(x), y(y), z(z)
	{
	}
};

struct SimpleSphere
{
	SimplePoint p;
	double r;

	SimpleSphere() : r(0.0)
	{
	}

	SimpleSphere(const SimplePoint& p, const double r) : p(p), r(r)
	{
	}
};

struct SimpleQuaternion
{
	double a;
	double b;
	double c;
	double d;

	SimpleQuaternion(const double a, const double b, const double c, const double d) : a(a), b(b), c(c), d(d)
	{
	}

	SimpleQuaternion(const double a, const SimplePoint& p) : a(a), b(p.x), c(p.y), d(p.z)
	{
	}
};

inline double pi_value()
{
	static const double pi=std::acos(-1.0);
	return pi;
}

inline double pi2_value()
{
	static const double pi2=std::acos(-1.0)*2.0;
	return pi2;
}

inline double default_comparison_epsilon()
{
	static const double e=0.0000000001;
	return e;
}

inline bool equal(const double a, const double b, const double e)
{
	return (((a-b)<=e) && ((b-a)<=e));
}

inline bool equal(const double a, const double b)
{
	return equal(a, b, default_comparison_epsilon());
}

inline bool less(const double a, const double b)
{
	return ((a+default_comparison_epsilon())<b);
}

inline bool greater(const double a, const double b)
{
	return ((a-default_comparison_epsilon())>b);
}

inline bool less_or_equal(const double a, const double b)
{
	return (less(a, b) || equal(a, b));
}

inline bool greater_or_equal(const double a, const double b)
{
	return (greater(a, b) || equal(a, b));
}

inline double squared_distance_from_point_to_point(const SimplePoint& a, const SimplePoint& b)
{
	const double dx=(a.x-b.x);
	const double dy=(a.y-b.y);
	const double dz=(a.z-b.z);
	return (dx*dx+dy*dy+dz*dz);
}

inline double distance_from_point_to_point(const SimplePoint& a, const SimplePoint& b)
{
	return sqrt(squared_distance_from_point_to_point(a, b));
}

inline double squared_point_module(const SimplePoint& a)
{
	return (a.x*a.x+a.y*a.y+a.z*a.z);
}

inline double point_module(const SimplePoint& a)
{
	return sqrt(squared_point_module(a));
}

inline double dot_product(const SimplePoint& a, const SimplePoint& b)
{
	return (a.x*b.x+a.y*b.y+a.z*b.z);
}

inline SimplePoint cross_product(const SimplePoint& a, const SimplePoint& b)
{
	return SimplePoint(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
}

inline SimplePoint point_and_number_product(const SimplePoint& a, const double k)
{
	return SimplePoint(a.x*k, a.y*k, a.z*k);
}

inline SimplePoint unit_point(const SimplePoint& a)
{
	return ((equal(squared_point_module(a), 1.0)) ? a : point_and_number_product(a, 1.0/point_module(a)));
}

inline SimplePoint sum_of_points(const SimplePoint& a, const SimplePoint& b)
{
	return SimplePoint(a.x+b.x, a.y+b.y, a.z+b.z);
}

inline SimplePoint sub_of_points(const SimplePoint& a, const SimplePoint& b)
{
	return SimplePoint(a.x-b.x, a.y-b.y, a.z-b.z);
}

inline double signed_distance_from_point_to_plane(const SimplePoint& plane_point, const SimplePoint& plane_normal, const SimplePoint& x)
{
	return dot_product(unit_point(plane_normal), sub_of_points(x, plane_point));
}

inline int halfspace_of_point(const SimplePoint& plane_point, const SimplePoint& plane_normal, const SimplePoint& x)
{
	const double sd=signed_distance_from_point_to_plane(plane_point, plane_normal, x);
	if(greater(sd, 0.0))
	{
		return 1;
	}
	else if(less(sd, 0.0))
	{
		return -1;
	}
	return 0;
}

inline SimplePoint intersection_of_plane_and_segment(const SimplePoint& plane_point, const SimplePoint& plane_normal, const SimplePoint& a, const SimplePoint& b)
{
	const double da=signed_distance_from_point_to_plane(plane_point, plane_normal, a);
	const double db=signed_distance_from_point_to_plane(plane_point, plane_normal, b);
	if((da-db)==0)
	{
		return a;
	}
	else
	{
		const double t=da/(da-db);
		return sum_of_points(a, point_and_number_product(sub_of_points(b, a), t));
	}
}

inline double triangle_area(const SimplePoint& a, const SimplePoint& b, const SimplePoint& c)
{
	return (point_module(cross_product(sub_of_points(b, a), sub_of_points(c, a)))/2.0);
}

inline double min_angle(const SimplePoint& o, const SimplePoint& a, const SimplePoint& b)
{
	double cos_val=dot_product(unit_point(sub_of_points(a, o)), unit_point(sub_of_points(b, o)));
	if(cos_val<-1.0)
	{
		cos_val=-1.0;
	}
	else if(cos_val>1.0)
	{
		cos_val=1.0;
	}
	return std::acos(cos_val);
}

inline double directed_angle(const SimplePoint& o, const SimplePoint& a, const SimplePoint& b, const SimplePoint& c)
{
	const double angle=min_angle(o, a, b);
	const SimplePoint n=cross_product(unit_point(sub_of_points(a, o)), unit_point(sub_of_points(b, o)));
	if(dot_product(sub_of_points(c, o), n)>=0)
	{
		return angle;
	}
	else
	{
		return (pi_value()*2-angle);
	}
}

SimplePoint any_normal_of_vector(const SimplePoint& a)
{
	SimplePoint b=a;
	if(!equal(b.x, 0.0) && (!equal(b.y, 0.0) || !equal(b.z, 0.0)))
	{
		b.x=0.0-b.x;
		return unit_point(cross_product(a, b));
	}
	else if(!equal(b.y, 0.0) && (!equal(b.x, 0.0) || !equal(b.z, 0.0)))
	{
		b.y=0.0-b.y;
		return unit_point(cross_product(a, b));
	}
	else if(!equal(b.x, 0.0))
	{
		return SimplePoint(0.0, 1.0, 0.0);
	}
	else
	{
		return SimplePoint(1.0, 0.0, 0.0);
	}
}

inline bool sphere_intersects_sphere(const SimpleSphere& a, const SimpleSphere& b)
{
	return less(squared_distance_from_point_to_point(a.p, b.p), (a.r+b.r)*(a.r+b.r));
}

inline bool sphere_contains_sphere(const SimpleSphere& a, const SimpleSphere& b)
{
	return (greater_or_equal(a.r, b.r) && less_or_equal(squared_distance_from_point_to_point(a.p, b.p), (a.r-b.r)*(a.r-b.r)));
}

inline SimpleSphere intersection_circle_of_two_spheres(const SimpleSphere& a, const SimpleSphere& b)
{
	const SimplePoint cv=sub_of_points(b.p, a.p);
	const double cm=point_module(cv);
	const double cos_g=(a.r*a.r+cm*cm-b.r*b.r)/(2*a.r*cm);
	const double sin_g=std::sqrt(1-cos_g*cos_g);
	return SimpleSphere(sum_of_points(a.p, point_and_number_product(cv, a.r*cos_g/cm)), a.r*sin_g);
}

inline bool project_point_inside_line(const SimplePoint& o, const SimplePoint& a, const SimplePoint& b, SimplePoint& result)
{
	const SimplePoint v=unit_point(sub_of_points(b, a));
	const double l=dot_product(v, sub_of_points(o, a));
	if(l>0.0 && (l*l)<=squared_distance_from_point_to_point(a, b))
	{
		result=sum_of_points(a, point_and_number_product(v, l));
		return true;
	}
	return false;
}

inline bool intersect_segment_with_circle(const SimpleSphere& circle, const SimplePoint& p_in, const SimplePoint& p_out, SimplePoint& result)
{
	const double dist_in_to_out=distance_from_point_to_point(p_in, p_out);
	if(dist_in_to_out>0.0)
	{
		const SimplePoint v=point_and_number_product(sub_of_points(p_in, p_out), 1.0/dist_in_to_out);
		const SimplePoint u=sub_of_points(circle.p, p_out);
		const SimplePoint s=sum_of_points(p_out, point_and_number_product(v, dot_product(v, u)));
		const double ll=(circle.r*circle.r)-squared_distance_from_point_to_point(circle.p, s);
		if(ll>=0.0)
		{
			result=sum_of_points(s, point_and_number_product(v, 0.0-std::sqrt(ll)));
			return true;
		}
	}
	return false;
}

inline double min_dihedral_angle(const SimplePoint& o, const SimplePoint& a, const SimplePoint& b1, const SimplePoint& b2)
{
	const SimplePoint oa=unit_point(sub_of_points(a, o));
	const SimplePoint d1=sub_of_points(b1, sum_of_points(o, point_and_number_product(oa, dot_product(oa, sub_of_points(b1, o)))));
	const SimplePoint d2=sub_of_points(b2, sum_of_points(o, point_and_number_product(oa, dot_product(oa, sub_of_points(b2, o)))));
	const double cos_val=dot_product(unit_point(d1), unit_point(d2));
	return std::acos(std::max(-1.0, std::min(cos_val, 1.0)));
}

inline SimpleQuaternion quaternion_product(const SimpleQuaternion& q1, const SimpleQuaternion& q2)
{
	return SimpleQuaternion(
			q1.a*q2.a - q1.b*q2.b - q1.c*q2.c - q1.d*q2.d,
			q1.a*q2.b + q1.b*q2.a + q1.c*q2.d - q1.d*q2.c,
			q1.a*q2.c - q1.b*q2.d + q1.c*q2.a + q1.d*q2.b,
			q1.a*q2.d + q1.b*q2.c - q1.c*q2.b + q1.d*q2.a);
}

inline SimpleQuaternion inverted_quaternion(const SimpleQuaternion& q)
{
	return SimpleQuaternion(q.a, 0.0-q.b, 0.0-q.c, 0.0-q.d);
}

inline SimplePoint rotate_point_around_axis(const SimplePoint axis, const double angle, const SimplePoint& p)
{
	if(squared_point_module(axis)>0)
	{
		const double radians_angle_half=(angle*0.5);
		const SimpleQuaternion q1=SimpleQuaternion(std::cos(radians_angle_half), point_and_number_product(unit_point(axis), std::sin(radians_angle_half)));
		const SimpleQuaternion q2=SimpleQuaternion(0.0, p);
		const SimpleQuaternion q3=quaternion_product(quaternion_product(q1, q2), inverted_quaternion(q1));
		return SimplePoint(q3.b, q3.c, q3.d);
	}
	else
	{
		return p;
	}
}

}

#endif /* VORONOTALT_BASIC_TYPES_AND_FUNCTIONS_H_ */
