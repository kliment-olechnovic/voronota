#include <cmath>
#include <Eigen/Dense>

#include "congruence_coefficient.h"

namespace congruence_coefficient
{

double calculate_congruence_coefficient_of_two_square_symmetric_matrices(const AdjacencyMap& amap_X, const AdjacencyMap& amap_Y)
{
	std::size_t max_index=0;
	for(AdjacencyMap::const_iterator it=amap_X.begin();it!=amap_X.end();++it)
	{
		max_index=std::max(max_index, it->first.first);
		max_index=std::max(max_index, it->first.second);
	}
	for(AdjacencyMap::const_iterator it=amap_Y.begin();it!=amap_Y.end();++it)
	{
		max_index=std::max(max_index, it->first.first);
		max_index=std::max(max_index, it->first.second);
	}

	const std::size_t N=max_index+1;

	Eigen::MatrixXd matrix_X=Eigen::MatrixXd::Zero(N, N);
	for(AdjacencyMap::const_iterator it=amap_X.begin();it!=amap_X.end();++it)
	{
		matrix_X(it->first.first, it->first.second)=it->second;
		matrix_X(it->first.second, it->first.first)=it->second;
	}

	Eigen::MatrixXd matrix_Y=Eigen::MatrixXd::Zero(N, N);
	for(AdjacencyMap::const_iterator it=amap_Y.begin();it!=amap_Y.end();++it)
	{
		matrix_Y(it->first.first, it->first.second)=it->second;
		matrix_Y(it->first.second, it->first.first)=it->second;
	}

	Eigen::MatrixXd product_matrix_XY=matrix_X*matrix_Y;
	Eigen::MatrixXd product_matrix_XX=matrix_X*matrix_X;
	Eigen::MatrixXd product_matrix_YY=matrix_Y*matrix_Y;

	const double result=product_matrix_XY.trace()/(sqrt(product_matrix_XX.trace())*sqrt(product_matrix_YY.trace()));
	return result;
}

}

