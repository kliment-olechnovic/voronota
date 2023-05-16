#ifndef DEPENDENCIES_UTILITIES_EIGEN_PCA_H_
#define DEPENDENCIES_UTILITIES_EIGEN_PCA_H_

#include <vector>

namespace eigen_pca
{

void compute_pca_and_project_on_basis(const int components, const bool no_zscores, std::vector< std::vector<double> >& io_data);

std::vector< std::vector<double> > compute_pca_3d_directions(std::vector< std::vector<double> >& input_data);

}

#endif /* DEPENDENCIES_UTILITIES_EIGEN_PCA_H_ */
