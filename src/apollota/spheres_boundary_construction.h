#ifndef APOLLOTA_SPHERES_BOUNDARY_CONSTRUCTION_H_
#define APOLLOTA_SPHERES_BOUNDARY_CONSTRUCTION_H_

#include <vector>
#include <limits>

#include "basic_operations_on_spheres.h"

namespace voronota
{

namespace apollota
{

inline std::vector<SimpleSphere> construct_artificial_boundary(const std::vector<SimpleSphere>& spheres, const double coordinate_shift)
{
	std::vector<SimpleSphere> result;
	if(!spheres.empty())
	{
		SimpleSphere a(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
		SimpleSphere b((-std::numeric_limits<double>::max()), (-std::numeric_limits<double>::max()), (-std::numeric_limits<double>::max()), (-std::numeric_limits<double>::max()));
		for(std::vector<SimpleSphere>::const_iterator it=spheres.begin();it!=spheres.end();++it)
		{
			a.x=std::min(a.x, it->x);
			a.y=std::min(a.y, it->y);
			a.z=std::min(a.z, it->z);
			a.r=std::min(a.r, it->r);
			b.x=std::max(b.x, it->x);
			b.y=std::max(b.y, it->y);
			b.z=std::max(b.z, it->z);
			b.r=std::max(b.r, it->r);
		}
		const double r=std::max(b.r, 0.0);
		const double shift=std::max(coordinate_shift, 0.0)+(r*2.0)+1.0;
		const double shift_more=shift+1.0;
		result.reserve(8);
		result.push_back(SimpleSphere(a.x-shift_more, a.y-shift_more, a.z-shift_more, r));
		result.push_back(SimpleSphere(a.x-shift, a.y-shift, b.z+shift, r));
		result.push_back(SimpleSphere(a.x-shift, b.y+shift, a.z-shift, r));
		result.push_back(SimpleSphere(a.x-shift, b.y+shift, b.z+shift, r));
		result.push_back(SimpleSphere(b.x+shift, a.y-shift, a.z-shift, r));
		result.push_back(SimpleSphere(b.x+shift, a.y-shift, b.z+shift, r));
		result.push_back(SimpleSphere(b.x+shift, b.y+shift, a.z-shift, r));
		result.push_back(SimpleSphere(b.x+shift_more, b.y+shift_more, b.z+shift_more, r));
	}
	return result;
}

}

}

#endif /* APOLLOTA_SPHERES_BOUNDARY_CONSTRUCTION_H_ */
