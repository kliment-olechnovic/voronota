#include <stdexcept>
#include <string>

#include "qcprot_wrapper.h"
#include "qcprot.h"

namespace QCProtWrapper
{

bool run_qcprot(std::vector< std::vector<double> >& xyz_3xN_coords_a, std::vector< std::vector<double> >& xyz_3xN_coords_b, QCProtResult& qcprot_result)
{
	if(xyz_3xN_coords_a.size()!=3 || xyz_3xN_coords_b.size()!=3
			|| xyz_3xN_coords_a[0].size()<4 || xyz_3xN_coords_a[0].size()!=xyz_3xN_coords_a[1].size() || xyz_3xN_coords_a[0].size()!=xyz_3xN_coords_a[2].size()
			|| xyz_3xN_coords_b[0].size()<4 || xyz_3xN_coords_b[0].size()!=xyz_3xN_coords_b[1].size() || xyz_3xN_coords_b[0].size()!=xyz_3xN_coords_b[2].size())
	{
		throw std::runtime_error(std::string("Invalid input vectors for running QCProt."));
	}

	if(xyz_3xN_coords_a[0].size()!=xyz_3xN_coords_b[0].size())
	{
		throw std::runtime_error(std::string("Not same size input vectors for running QCProt."));
	}

	std::vector<double*> pointers_a(3, 0);
	std::vector<double*> pointers_b(3, 0);
	for(int i=0;i<3;i++)
	{
		pointers_a[i]=xyz_3xN_coords_a[i].data();
		pointers_b[i]=xyz_3xN_coords_b[i].data();
	}

	const std::size_t N=xyz_3xN_coords_a[0].size();

	for(int i=0;i<3;i++)
	{
		qcprot_result.translation_vector_a[i]=0.0;
		qcprot_result.translation_vector_b[i]=0.0;
		for(std::size_t j=0;j<N;j++)
		{
			qcprot_result.translation_vector_a[i]+=xyz_3xN_coords_a[i][j];
			qcprot_result.translation_vector_b[i]-=xyz_3xN_coords_b[i][j];
		}
		qcprot_result.translation_vector_a[i]/=static_cast<double>(N);
		qcprot_result.translation_vector_b[i]/=static_cast<double>(N);
	}

	std::vector<double> weights(N, 1.0);
	double** frag_a=pointers_a.data();
	double** frag_b=pointers_b.data();

	qcprot_result.rmsd=CalcRMSDRotationalMatrix(frag_a, frag_b, static_cast<int>(N), qcprot_result.rotation_matrix, weights.data());

	return true;
}

}
