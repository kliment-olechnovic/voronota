#ifndef APOLLOTA_TANGENT_SPHERE_OF_THREE_SPHERES_H_
#define APOLLOTA_TANGENT_SPHERE_OF_THREE_SPHERES_H_

#include <vector>

#include "basic_operations_on_spheres.h"
#include "rotation.h"
#include "safer_quadratic_equation_root.h"

namespace voronota
{

namespace apollota
{

class TangentSphereOfThreeSpheres
{
public:
	template<typename InputSphereTypeA, typename InputSphereTypeB, typename InputSphereTypeC>
	static inline std::vector<SimpleSphere> calculate(const InputSphereTypeA& sm, const InputSphereTypeB& s1, const InputSphereTypeC& s2)
	{
		{
			const double min_r=std::min(sm.r, std::min(s1.r, s2.r));
			if(sm.r!=min_r)
			{
				if(s1.r==min_r) return calculate(s1, sm, s2);
				if(s2.r==min_r) return calculate(s2, sm, s1);
			}
		}

		const double x1=distance_from_point_to_point(sm, s1);
		const double y1=0;
		const double r1=s1.r-sm.r;

		const double x2=dot_product(sub_of_points<SimplePoint>(s1, sm).unit(), sub_of_points<PODPoint>(s2, sm));
		const double y2=sqrt(squared_point_module(sub_of_points<PODPoint>(s2, sm))-x2*x2);
		const double r2=s2.r-sm.r;

		const double a1=2*x1;
		const double b1=2*y1;
		const double d1=2*r1;
		const double o1=(r1*r1-x1*x1-y1*y1);

		const double a2=2*x2;
		const double b2=2*y2;
		const double d2=2*r2;
		const double o2=(r2*r2-x2*x2-y2*y2);

		const double w  = a2*b1-a1*b2;

		const double u1 = (b2*d1-b1*d2) / w;
		const double v1 = (b2*o1-b1*o2) / w;

		const double u2 = -(a2*d1-a1*d2) / w;
		const double v2 = -(a2*o1-a1*o2) / w;

		const double a = u1*u1+u2*u2-1;
		const double b = 2*(u1*v1+u2*v2);

		std::vector<SimpleSphere> results;

		if(check_if_quadratic_equation_is_solvable(a, b))
		{
			const double c = v1*v1+v2*v2;
			std::vector<double> radiuses;
			if(solve_quadratic_equation(a, b, c, radiuses))
			{
				results.reserve(radiuses.size());
				for(std::size_t i=0;i<radiuses.size();i++)
				{
					const double r=radiuses[i];
					if(r>0)
					{
						const double virtual_x=u1*r+v1;
						const double virtual_y=u2*r+v2;
						const double real_l1_offset=virtual_y*x2/y2;
						const double real_l1=virtual_x-real_l1_offset;
						const double real_l2=sqrt(real_l1_offset*real_l1_offset+virtual_y*virtual_y);
						for(int sign_id=0;sign_id<2;sign_id++)
						{
							const double signed_real_l2=real_l2*(sign_id==0 ? 1.0 : -1.0);
							SimpleSphere candidate(SimplePoint(sm)+(sub_of_points<SimplePoint>(s1, sm).unit()*real_l1)+(sub_of_points<SimplePoint>(s2, sm).unit()*signed_real_l2), (r-sm.r));
							if((results.empty() || !(candidate==results.back())) && less(fabs(signed_volume_of_tetrahedron(sm, s1, s2, candidate)), tangent_spheres_max_allowed_error()))
							{
								std::pair<double, double> error_estimate=calculate_tangent_sphere_radius_error_estimate(sm, s1, s2, candidate);
								if(error_estimate.first<0.0)
								{
									candidate.r+=error_estimate.first;
									error_estimate=calculate_tangent_sphere_radius_error_estimate(sm, s1, s2, candidate);
								}
								if(std::max(fabs(error_estimate.first), fabs(error_estimate.second))<tangent_spheres_max_allowed_error())
								{
									results.push_back(candidate);
								}
							}
						}
					}
				}
			}
		}

		return results;
	}

	template<typename InputSphereTypeA, typename InputSphereTypeB, typename InputSphereTypeC>
	static inline std::vector<SimpleSphere> calculate(const InputSphereTypeA& sm, const InputSphereTypeB& s1, const InputSphereTypeC& s2, const double custom_tangent_sphere_radius)
	{
		{
			const double min_r=std::min(sm.r, std::min(s1.r, s2.r));
			if(sm.r!=min_r)
			{
				if(s1.r==min_r) return calculate(s1, sm, s2, custom_tangent_sphere_radius);
				if(s2.r==min_r) return calculate(s2, sm, s1, custom_tangent_sphere_radius);
			}
		}

		const double r=custom_tangent_sphere_radius+sm.r;

		const unsigned int rotation_steps=2;
		const SimplePoint rotation_axis(1.0, 1.0, 1.0);
		const double rotation_step_angle=30.0;

		for(unsigned int rotation_step=0;rotation_step<=rotation_steps;rotation_step++)
		{
			SimpleSphere ts1(s1.x-sm.x, s1.y-sm.y, s1.z-sm.z, s1.r-sm.r);
			SimpleSphere ts2(s2.x-sm.x, s2.y-sm.y, s2.z-sm.z, s2.r-sm.r);

			if(rotation_step>0)
			{
				const Rotation rotation(rotation_axis, rotation_step_angle*static_cast<double>(rotation_step));
				ts1=SimpleSphere(rotation.rotate<SimplePoint>(ts1), ts1.r);
				ts2=SimpleSphere(rotation.rotate<SimplePoint>(ts2), ts2.r);
			}

			const double x1=ts1.x;
			const double y1=ts1.y;
			const double z1=ts1.z;
			const double r1=ts1.r;

			const double x2=ts2.x;
			const double y2=ts2.y;
			const double z2=ts2.z;
			const double r2=ts2.r;

			const double a1=2*x1;
			const double b1=2*y1;
			const double c1=2*z1;
			const double o1=(r+r1)*(r+r1)-(r*r)-(x1*x1+y1*y1+z1*z1);

			const double a2=2*x2;
			const double b2=2*y2;
			const double c2=2*z2;
			const double o2=(r+r2)*(r+r2)-(r*r)-(x2*x2+y2*y2+z2*z2);

			const double w1=(a1*b2-a2*b1);
			const double w2=(b1*a2-b2*a1);

			if((w1>0.0 || w1<0.0) && (w2>0.0 || w2<0.0))
			{
				const double ux=(c2*b1-c1*b2)/w1;
				const double vx=(o2*b1-o1*b2)/w1;

				const double uy=(c2*a1-c1*a2)/w2;
				const double vy=(o2*a1-o1*a2)/w2;

				const double a=(ux*ux+uy*uy+1);
				const double b=2*(ux*vx+uy*vy);

				if(check_if_quadratic_equation_is_solvable(a, b))
				{
					const double c=(vx*vx+vy*vy-r*r);
					std::vector<SimpleSphere> results;
					std::vector<double> zs;
					if(solve_quadratic_equation(a, b, c, zs))
					{
						results.reserve(zs.size());
						for(std::size_t i=0;i<zs.size();i++)
						{
							const double z=zs[i];
							SimpleSphere candidate((ux*z+vx), (uy*z+vy), z, r);
							if(rotation_step>0)
							{
								const Rotation rotation(rotation_axis, (0.0-rotation_step_angle)*static_cast<double>(rotation_step));
								candidate=SimpleSphere(rotation.rotate<SimplePoint>(candidate), candidate.r);
							}
							candidate.x+=sm.x;
							candidate.y+=sm.y;
							candidate.z+=sm.z;
							candidate.r-=sm.r;
							if(equal(candidate.r, custom_tangent_sphere_radius))
							{
								const std::pair<double, double> error_estimate=calculate_tangent_sphere_radius_error_estimate(sm, s1, s2, candidate);
								if(std::max(fabs(error_estimate.first), fabs(error_estimate.second))<tangent_spheres_max_allowed_error())
								{
									results.push_back(candidate);
								}
							}
						}
						return results;
					}
				}
			}
		}

		return std::vector<SimpleSphere>();
	}

private:
	template<typename InputSphereTypeA, typename InputSphereTypeB, typename InputSphereTypeC, typename InputSphereTypeD>
	static inline std::pair<double, double> calculate_tangent_sphere_radius_error_estimate(const InputSphereTypeA& s1, const InputSphereTypeB& s2, const InputSphereTypeC& s3, const InputSphereTypeD& tangent)
	{
		const double d1=minimal_distance_from_sphere_to_sphere(tangent, s1);
		const double d2=minimal_distance_from_sphere_to_sphere(tangent, s2);
		const double d3=minimal_distance_from_sphere_to_sphere(tangent, s3);
		return std::make_pair(std::min(std::min(d1, d2), d3), std::max(std::max(d1, d2), d3));
	}

	inline static double tangent_spheres_max_allowed_error()
	{
		return std::max(default_comparison_epsilon(), 0.001);
	}
};

}

}

#endif /* APOLLOTA_TANGENT_SPHERE_OF_THREE_SPHERES_H_ */
