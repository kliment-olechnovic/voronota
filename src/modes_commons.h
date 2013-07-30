#ifndef MODES_COMMONS_H_
#define MODES_COMMONS_H_

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

namespace modes_commons
{

template<typename SphereType>
inline void add_sphere_from_stream_to_vector(std::istream& input, std::vector<SphereType>& spheres)
{
	SphereType sphere;
	input >> sphere.x >> sphere.y >> sphere.z >> sphere.r;
	if(!input.fail())
	{
		spheres.push_back(sphere);
	}
}

}

#endif /* MODES_COMMONS_H_ */
