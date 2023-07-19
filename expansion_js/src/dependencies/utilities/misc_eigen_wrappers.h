#ifndef DEPENDENCIES_UTILITIES_MISC_EIGEN_WRAPPERS_H_
#define DEPENDENCIES_UTILITIES_MISC_EIGEN_WRAPPERS_H_

#include <vector>

namespace misc_eigen_wrappers
{

std::vector<double> summarize_two_state_motion_as_state(std::vector<double>& state_x, std::vector<double>& state_y);

}

#endif /* DEPENDENCIES_UTILITIES_MISC_EIGEN_WRAPPERS_H_ */
