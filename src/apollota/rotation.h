#ifndef APOLLOTA_ROTATION_H_
#define APOLLOTA_ROTATION_H_

#include "basic_operations_on_points.h"

namespace apollota
{

class Rotation
{
public:
	SimplePoint axis;
	double angle;

	template<typename InputPointType>
	Rotation(const InputPointType& axis, double angle) : axis(axis), angle(angle)
	{
	}

	static double pi()
	{
		return 3.14159265358979323846;
	}

	template<typename OutputPointType, typename InputPointType>
	OutputPointType rotate(const InputPointType& p) const
	{
		if(axis.module()>0)
		{
			const double radians_angle_half=angle*pi()/360.0;
			const Quaternion q1=quaternion_from_value_and_point(cos(radians_angle_half), axis.unit()*sin(radians_angle_half));
			const Quaternion q2=quaternion_from_value_and_point(0, p);
			const Quaternion q3=((q1*q2)*(!q1));
			return custom_point<OutputPointType>(q3.b, q3.c, q3.d);
		}
		else
		{
			return p;
		}
	}

private:
	struct Quaternion
	{
		double a;
		double b;
		double c;
		double d;

		Quaternion(const double a, const double b, const double c, const double d) : a(a), b(b), c(c), d(d)
		{
		}

		Quaternion operator*(const Quaternion& q) const
		{
			return Quaternion(
					a*q.a - b*q.b - c*q.c - d*q.d,
					a*q.b + b*q.a + c*q.d - d*q.c,
					a*q.c - b*q.d + c*q.a + d*q.b,
					a*q.d + b*q.c - c*q.b + d*q.a);
		}

		Quaternion operator!() const
		{
			return Quaternion(a, 0-b, 0-c, 0-d);
		}
	};

	template<typename InputPointType>
	static Quaternion quaternion_from_value_and_point(const double a, const InputPointType& p)
	{
		return Quaternion(a, p.x, p.y, p.z);
	}
};

}

#endif /* APOLLOTA_ROTATION_H_ */
