#ifndef MODESCOMMON_READ_SPHERE_H_
#define MODESCOMMON_READ_SPHERE_H_

#include "../auxiliaries/io_utilities.h"

namespace modescommon
{

template<typename SphereType>
inline bool add_sphere_from_stream_to_vector(std::istream& input, std::vector<SphereType>& spheres)
{
	SphereType sphere;
	input >> sphere.x >> sphere.y >> sphere.z >> sphere.r;
	if(!input.fail())
	{
		spheres.push_back(sphere);
		return true;
	}
	return false;
}

}

#endif /* MODESCOMMON_READ_SPHERE_H_ */
