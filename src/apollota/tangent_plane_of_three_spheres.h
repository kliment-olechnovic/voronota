#ifndef APOLLOTA_TANGENT_PLANE_OF_THREE_SPHERES_H_
#define APOLLOTA_TANGENT_PLANE_OF_THREE_SPHERES_H_

#include <vector>
#include <algorithm>

#include "basic_operations_on_spheres.h"

namespace apollota
{

class TangentPlaneOfThreeSpheres
{
public:
	template<typename InputSphereTypeA, typename InputSphereTypeB, typename InputSphereTypeC>
	static inline std::vector< std::pair<SimplePoint, SimplePoint> > calculate(const InputSphereTypeA& a, const InputSphereTypeB& b, const InputSphereTypeC& c)
	{
		const std::vector<SimplePoint> normals=calculate_tangent_planes_normals<SimplePoint>(a, b, c);
		std::vector< std::pair<SimplePoint, SimplePoint> > planes;
		for(std::size_t i=0;i<normals.size();i++)
		{
			planes.push_back(std::make_pair((custom_point_from_object<SimplePoint>(a)+(normals[i]*a.r)), normals[i]));
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

	template<typename OutputPointType, typename InputSphereTypeA, typename InputSphereTypeB, typename InputSphereTypeC>
	static inline std::vector<OutputPointType> calculate_tangent_planes_normals(const InputSphereTypeA& sm, const InputSphereTypeB& s1, const InputSphereTypeC& s2)
	{
		{
			const double min_r=std::min(sm.r, std::min(s1.r, s2.r));
			if(sm.r!=min_r)
			{
				if(s1.r==min_r) return calculate_tangent_planes_normals<OutputPointType>(s1, sm, s2);
				if(s2.r==min_r) return calculate_tangent_planes_normals<OutputPointType>(s2, sm, s1);
			}
		}

		const double substractions1[3]={s1.x-sm.x, s1.y-sm.y, s1.z-sm.z};
		const double substractions2[3]={s2.x-sm.x, s2.y-sm.y, s2.z-sm.z};

		std::size_t permutation[3]={0,1,2};
		while(equal(substractions1[permutation[0]], 0) && std::next_permutation(permutation, permutation+3)) {}
		std::size_t reverse_permutation[3]={0,0,0};
		for(std::size_t i=0;i<3;i++) { reverse_permutation[permutation[i]]=i; }

		const double x1=substractions1[permutation[0]];
		const double y1=substractions1[permutation[1]];
		const double z1=substractions1[permutation[2]];
		const double r1=s1.r-sm.r;

		const double x2=substractions2[permutation[0]];
		const double y2=substractions2[permutation[1]];
		const double z2=substractions2[permutation[2]];
		const double r2=s2.r-sm.r;

		const double ad=0-x1;
		const double a0=r1/ad;
		const double ay=y1/ad;
		const double az=z1/ad;

		const double bd=0-(y2+ay*x2);
		const double b0=(r2+a0*x2)/bd;
		const double bz=(z2+az*x2)/bd;

		const double c0=a0+ay*b0;
		const double cz=(ay*bz+az);

		const double a=(1+cz*cz+bz*bz);
		const double b=2*(c0*cz+b0*bz);
		const double c=(c0*c0+b0*b0-1);
		const double D=b*b-4*a*c;

		std::vector<double> zs;
		if(D>=0.0)
		{
			if(D==0.0)
			{
				zs.push_back((-b)/(2*a));
			}
			else
			{
				zs.push_back((-b-sqrt(D))/(2*a));
				zs.push_back((-b+sqrt(D))/(2*a));
			}
		}

		std::vector<OutputPointType> results;
		results.reserve(zs.size());
		for(std::size_t i=0;i<zs.size();i++)
		{
			const double z=zs[i];
			const double permuted_candidate[3]={c0+z*cz, b0+z*bz, z};
			const OutputPointType candidate=custom_point<OutputPointType>(permuted_candidate[reverse_permutation[0]], permuted_candidate[reverse_permutation[1]], permuted_candidate[reverse_permutation[2]]);
			if(check_tangent_plane(sm, s1, s2, candidate))
			{
				results.push_back(candidate);
			}
		}

		return results;
	}
};

}

#endif /* APOLLOTA_TANGENT_PLANE_OF_THREE_SPHERES_H_ */
