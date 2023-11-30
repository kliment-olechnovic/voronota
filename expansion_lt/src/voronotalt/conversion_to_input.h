#ifndef VORONOTALT_CONVERSION_TO_INPUT_H_
#define VORONOTALT_CONVERSION_TO_INPUT_H_

#include <vector>

#include "basic_types_and_functions.h"

namespace voronotalt
{

template<class BallsContainer>
inline std::vector<SimpleSphere> get_spheres_from_balls(const BallsContainer& balls, const Float probe)
{
	std::vector<SimpleSphere> result;
	result.reserve(balls.size());
	for(typename BallsContainer::const_iterator it=balls.begin();it!=balls.end();++it)
	{
		result.push_back(SimpleSphere(SimplePoint(it->x, it->y, it->z), (it->r)+probe));
	}
	return result;
}

}

#endif /* VORONOTALT_CONVERSION_TO_INPUT_H_ */
