#ifndef DEPENDENCIES_QCPROT_QCPROT_WRAPPER_H_
#define DEPENDENCIES_QCPROT_QCPROT_WRAPPER_H_

#include <vector>

namespace QCProtWrapper
{

struct QCProtResult
{
	double rmsd;
	double translation_vector_a[3];
	double translation_vector_b[3];
	double rotation_matrix[9];
};

bool run_qcprot(std::vector< std::vector<double> >& xyz_3xN_coords_a, std::vector< std::vector<double> >& xyz_3xN_coords_b, QCProtResult& qcprot_result);

}

#endif /* DEPENDENCIES_QCPROT_QCPROT_WRAPPER_H_ */
