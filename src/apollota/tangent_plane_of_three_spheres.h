#ifndef APOLLOTA_TANGENT_PLANE_OF_THREE_SPHERES_H_
#define APOLLOTA_TANGENT_PLANE_OF_THREE_SPHERES_H_

#include <vector>
#include <algorithm>

#include "basic_operations_on_spheres.h"
#include "rotation.h"
#include "safer_quadratic_equation_root.h"
#include "safer_summation.h"

namespace voronota
{

namespace apollota
{

class TangentPlaneOfThreeSpheres
{
public:
	template<typename InputSphereTypeA, typename InputSphereTypeB, typename InputSphereTypeC>
	static inline std::vector< std::pair<SimplePoint, SimplePoint> > calculate(const InputSphereTypeA& a, const InputSphereTypeB& b, const InputSphereTypeC& c)
	{
		const std::vector<SimplePoint> normals=calculate_tangent_planes_normals(a, b, c);
		std::vector< std::pair<SimplePoint, SimplePoint> > planes;
		for(std::size_t i=0;i<normals.size();i++)
		{
			planes.push_back(std::make_pair((custom_point_from_object<SimplePoint>(a)+(normals[i]*(a.r+default_comparison_epsilon()))), normals[i]));
		}
		return planes;
	}

private:
	template<typename InputSphereTypeA, typename InputSphereTypeB, typename InputSphereTypeC, typename InputPointType>
	static inline bool check_tangent_plane(const InputSphereTypeA& s1, const InputSphereTypeB& s2, const InputSphereTypeC& s3, const InputPointType& tangent_plane_normal)
	{
		const SimplePoint sp1=custom_point_from_object<SimplePoint>(s1);
		const SimplePoint sp2=custom_point_from_object<SimplePoint>(s2);
		const SimplePoint sp3=custom_point_from_object<SimplePoint>(s3);
		const SimplePoint t=custom_point_from_object<SimplePoint>(tangent_plane_normal);
		return (equal( (((sp2+t*s2.r)-(sp1+t*s1.r)) * t), 0 ) &&
				equal( (((sp3+t*s3.r)-(sp1+t*s1.r)) * t), 0 ));
	}

	template<typename InputSphereTypeA, typename InputSphereTypeB, typename InputSphereTypeC>
	static inline std::vector<SimplePoint> calculate_tangent_planes_normals(const InputSphereTypeA& sm, const InputSphereTypeB& s1, const InputSphereTypeC& s2)
	{
		{
			const double min_r=std::min(sm.r, std::min(s1.r, s2.r));
			if(sm.r!=min_r)
			{
				if(s1.r==min_r) return calculate_tangent_planes_normals(s1, sm, s2);
				if(s2.r==min_r) return calculate_tangent_planes_normals(s2, sm, s1);
			}
		}

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

			const double ad=0-x1;
			if(ad>0.0 || ad<0.0)
			{
				const double a0=r1/ad;
				const double ay=y1/ad;
				const double az=z1/ad;

				const double bd=-safer_sum(y2, ay*x2);
				if(bd>0.0 || bd<0.0)
				{
					const double b0=safer_sum(r2, a0*x2)/bd;
					const double bz=safer_sum(z2, az*x2)/bd;

					const double c0=safer_sum(a0, ay*b0);
					const double cz=safer_sum(ay*bz, az);

					const double a=safer_sum(1, cz*cz, bz*bz);
					const double b=safer_sum(2*c0*cz, 2*b0*bz);

					if(check_if_quadratic_equation_is_solvable(a, b))
					{
						const double c=safer_sum(c0*c0, b0*b0, -1.0);
						std::vector<SimplePoint> results;
						std::vector<double> zs;
						if(solve_quadratic_equation(a, b, c, zs))
						{
							results.reserve(zs.size());
							for(std::size_t i=0;i<zs.size();i++)
							{
								const double z=zs[i];
								SimplePoint candidate(safer_sum(c0, z*cz), safer_sum(b0, z*bz), z);
								if(rotation_step>0)
								{
									const Rotation rotation(rotation_axis, (-rotation_step_angle)*static_cast<double>(rotation_step));
									candidate=rotation.rotate<SimplePoint>(candidate);
								}
								if(check_tangent_plane(sm, s1, s2, candidate))
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

		return std::vector<SimplePoint>();
	}
};

}

}

#endif /* APOLLOTA_TANGENT_PLANE_OF_THREE_SPHERES_H_ */
