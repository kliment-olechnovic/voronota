#ifndef APOLLOTA_TANGENT_SPHERE_OF_FOUR_SPHERES_H_
#define APOLLOTA_TANGENT_SPHERE_OF_FOUR_SPHERES_H_

#include <vector>

#include "basic_operations_on_spheres.h"
#include "rotation.h"
#include "safer_quadratic_equation_root.h"
#include "safer_summation.h"

namespace voronota
{

namespace apollota
{

class TangentSphereOfFourSpheres
{
public:
	template<typename InputSphereTypeA, typename InputSphereTypeB, typename InputSphereTypeC, typename InputSphereTypeD>
	static inline std::vector<SimpleSphere> calculate(const InputSphereTypeA& sm, const InputSphereTypeB& s1, const InputSphereTypeC& s2, const InputSphereTypeD& s3)
	{
		{
			const double min_r=std::min(sm.r, std::min(s1.r, std::min(s2.r, s3.r)));
			if(sm.r!=min_r)
			{
				if(s1.r==min_r) return calculate(s1, sm, s2, s3);
				if(s2.r==min_r) return calculate(s2, sm, s1, s3);
				if(s3.r==min_r) return calculate(s3, sm, s1, s2);
			}
		}

		SimpleSphere usm(sm);
		for(int i=0;i<3 && equal(fabs(signed_volume_of_tetrahedron(usm, s1, s2, s3)), 0.0);i++)
		{
			const double allowed_shift=std::max(default_comparison_epsilon(), 0.00001);
			usm=SimpleSphere(sm);
			if(i%3==0) usm.x+=allowed_shift;
			else if(i%3==1) usm.y+=allowed_shift;
			else usm.z+=allowed_shift;
		}

		const unsigned int rotation_steps=2;
		const SimplePoint rotation_axis(1.0, 1.0, 1.0);
		const double rotation_step_angle=30.0;

		for(unsigned int rotation_step=0;rotation_step<=rotation_steps;rotation_step++)
		{
			SimpleSphere ts1(s1.x-usm.x, s1.y-usm.y, s1.z-usm.z, s1.r-usm.r);
			SimpleSphere ts2(s2.x-usm.x, s2.y-usm.y, s2.z-usm.z, s2.r-usm.r);
			SimpleSphere ts3(s3.x-usm.x, s3.y-usm.y, s3.z-usm.z, s3.r-usm.r);

			if(rotation_step>0)
			{
				const Rotation rotation(rotation_axis, rotation_step_angle*static_cast<double>(rotation_step));
				ts1=SimpleSphere(rotation.rotate<SimplePoint>(ts1), ts1.r);
				ts2=SimpleSphere(rotation.rotate<SimplePoint>(ts2), ts2.r);
				ts3=SimpleSphere(rotation.rotate<SimplePoint>(ts3), ts3.r);
			}

			const double x1=ts1.x;
			const double y1=ts1.y;
			const double z1=ts1.z;
			const double r1=ts1.r;

			const double x2=ts2.x;
			const double y2=ts2.y;
			const double z2=ts2.z;
			const double r2=ts2.r;

			const double x3=ts3.x;
			const double y3=ts3.y;
			const double z3=ts3.z;
			const double r3=ts3.r;

			const double a1=2*x1;
			const double b1=2*y1;
			const double c1=2*z1;
			const double d1=2*r1;
			const double o1=safer_sum(r1*r1, -x1*x1, -y1*y1, -z1*z1);

			const double a2=2*x2;
			const double b2=2*y2;
			const double c2=2*z2;
			const double d2=2*r2;
			const double o2=safer_sum(r2*r2, -x2*x2, -y2*y2, -z2*z2);

			const double a3=2*x3;
			const double b3=2*y3;
			const double c3=2*z3;
			const double d3=2*r3;
			const double o3=safer_sum(r3*r3, -x3*x3, -y3*y3, -z3*z3);

			const double w = safer_sum(a1*b3*c2, -a1*b2*c3, b1*a2*c3, -b1*a3*c2, c1*a3*b2, -c1*a2*b3);

			if(w>0.0 || w<0.0)
			{
				const double u1 = -safer_sum( b1*c3*d2, -b1*c2*d3, c1*b2*d3, -c1*b3*d2, d1*b3*c2, -d1*b2*c3 ) / w;
				const double v1 = -safer_sum( b1*c3*o2, -b1*c2*o3, c1*b2*o3, -c1*b3*o2, o1*b3*c2, -o1*b2*c3 ) / w;

				const double u2 =  safer_sum( a1*c3*d2, -a1*c2*d3, c1*a2*d3, -c1*a3*d2, d1*a3*c2, -d1*a2*c3 ) / w;
				const double v2 =  safer_sum( a1*c3*o2, -a1*c2*o3, c1*a2*o3, -c1*a3*o2, o1*a3*c2, -o1*a2*c3 ) / w;

				const double u3 = -safer_sum( a1*b3*d2, -a1*b2*d3, b1*a2*d3, -b1*a3*d2, d1*a3*b2, -d1*a2*b3 ) / w;
				const double v3 = -safer_sum( a1*b3*o2, -a1*b2*o3, b1*a2*o3, -b1*a3*o2, o1*a3*b2, -o1*a2*b3 ) / w;

				const double a = safer_sum(u1*u1, u2*u2, u3*u3, -1.0);
				const double b = safer_sum(2*u1*v1, 2*u2*v2, 2*u3*v3);

				if(check_if_quadratic_equation_is_solvable(a, b))
				{
					const double c = safer_sum(v1*v1, v2*v2, v3*v3);
					std::vector<SimpleSphere> results;
					std::vector<double> radiuses;
					if(solve_quadratic_equation(a, b, c, radiuses))
					{
						results.reserve(radiuses.size());
						for(std::size_t i=0;i<radiuses.size();i++)
						{
							const double r=radiuses[i];
							if(r>=0.0)
							{
								SimpleSphere candidate(safer_sum(u1*r, v1), safer_sum(u2*r, v2), safer_sum(u3*r, v3), r);
								if(rotation_step>0)
								{
									const Rotation rotation(rotation_axis, (0.0-rotation_step_angle)*static_cast<double>(rotation_step));
									candidate=SimpleSphere(rotation.rotate<SimplePoint>(candidate), candidate.r);
								}
								candidate.x+=usm.x;
								candidate.y+=usm.y;
								candidate.z+=usm.z;
								candidate.r-=usm.r;
								std::pair<double, double> error_estimate=calculate_tangent_sphere_radius_error_estimate(sm, s1, s2, s3, candidate);
								if(error_estimate.first<0.0)
								{
									candidate.r+=error_estimate.first;
									error_estimate=calculate_tangent_sphere_radius_error_estimate(sm, s1, s2, s3, candidate);
								}
								if(std::max(fabs(error_estimate.first), fabs(error_estimate.second))<tangent_spheres_max_allowed_error())
								{
									results.push_back(candidate);
								}
							}
						}
					}
					return results;
				}
			}
		}

		return std::vector<SimpleSphere>();
	}

private:
	template<typename InputSphereTypeA, typename InputSphereTypeB, typename InputSphereTypeC, typename InputSphereTypeD, typename InputSphereTypeE>
	static inline std::pair<double, double> calculate_tangent_sphere_radius_error_estimate(const InputSphereTypeA& s1, const InputSphereTypeB& s2, const InputSphereTypeC& s3, const InputSphereTypeD& s4, const InputSphereTypeE& tangent)
	{
		const double d1=minimal_distance_from_sphere_to_sphere(tangent, s1);
		const double d2=minimal_distance_from_sphere_to_sphere(tangent, s2);
		const double d3=minimal_distance_from_sphere_to_sphere(tangent, s3);
		const double d4=minimal_distance_from_sphere_to_sphere(tangent, s4);
		return std::make_pair(std::min(std::min(d1, d2), std::min(d3, d4)), std::max(std::max(d1, d2), std::max(d3, d4)));
	}

	inline static double tangent_spheres_max_allowed_error()
	{
		return std::max(default_comparison_epsilon(), 0.001);
	}
};

}

}

#endif /* APOLLOTA_TANGENT_SPHERE_OF_FOUR_SPHERES_H_ */
