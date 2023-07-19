#include <stdexcept>
#include <Eigen/Dense>

#include "misc_eigen_wrappers.h"

namespace misc_eigen_wrappers
{

std::vector<double> summarize_two_state_motion_as_state(std::vector<double>& state_x, std::vector<double>& state_y)
{
	if(state_x.empty() || state_x.size()!=state_y.size() || state_x.size()%3!=0)
	{
		throw std::runtime_error(std::string("Input vectors do not have the same non-zero lengths divisible by 3"));
	}

	Eigen::VectorXd x=Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(state_x.data(), static_cast<int>(state_x.size()));
	Eigen::VectorXd y=Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(state_y.data(), static_cast<int>(state_y.size()));

	Eigen::VectorXd v=(y-x).normalized();

	Eigen::VectorXd c=x-((x.transpose()*v)(0,0)*v);

	return std::vector<double>(c.data(), c.data()+c.size());
}

}
