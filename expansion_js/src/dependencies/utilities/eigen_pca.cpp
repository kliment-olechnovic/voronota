#include <cmath>
#include <stdexcept>
#include <Eigen/Dense>

#include "utilities/eigen_pca.h"

namespace eigen_pca
{

void compute_pca_and_project_on_basis(const int components, const bool no_zscores, std::vector< std::vector<double> >& io_data)
{
	if(io_data.empty() || io_data.front().empty())
	{
		throw std::runtime_error(std::string("Empty input data"));
	}

	if(components<1)
	{
		throw std::runtime_error(std::string("Invalid number of components to use"));
	}

	if(components>static_cast<int>(io_data.front().size()))
	{
		throw std::runtime_error(std::string("Number of components to use is greater than size of input data vector"));
	}

	if(!no_zscores)
	{
		std::vector<double> sum(io_data.at(0).size(), 0.0);
		std::vector<double> sum2(sum.size(), 0.0);
		for(std::size_t i=0;i<io_data.size();i++)
		{
			for(std::size_t j=0;j<io_data[i].size();j++)
			{
				sum[j]+=io_data[i][j];
				sum2[j]+=io_data[i][j]*io_data[i][j];
			}
		}
		for(std::size_t j=0;j<sum.size();j++)
		{
			const double mean=sum[j]/static_cast<double>(io_data.size());
			const double mean2=sum2[j]/static_cast<double>(io_data.size());
			const double sd=std::sqrt(mean2-(mean*mean));
			for(std::size_t i=0;i<io_data.size();i++)
			{
				io_data[i][j]=(sd>0.0 ? ((io_data[i][j]-mean)/sd) : 0.0);
			}
		}
	}

	Eigen::MatrixXd raw_data_matrix(static_cast<int>(io_data.size()), static_cast<int>(io_data.front().size()));
	for(std::size_t i=0;i<io_data.size();i++)
	{
		Eigen::VectorXd row_data_vector=Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(io_data[i].data(), static_cast<int>(io_data[i].size()));
		raw_data_matrix.row(static_cast<int>(i))=row_data_vector;
	}

	Eigen::MatrixXd centered_data_matrix=raw_data_matrix.rowwise()-raw_data_matrix.colwise().mean();

	Eigen::MatrixXd covariance_matrix=centered_data_matrix.adjoint()*centered_data_matrix;

	Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigensolver(covariance_matrix);

	if(eigensolver.info()!=Eigen::Success)
	{
		throw std::runtime_error(std::string("Failed to perform eigendecomposition."));
	}

	Eigen::MatrixXd pca_basis=eigensolver.eigenvectors().rightCols(components);

	Eigen::MatrixXd projected_data_matrix=(centered_data_matrix*pca_basis);

	for(std::size_t i=0;i<io_data.size();i++)
	{
		Eigen::VectorXd row_data_vector=projected_data_matrix.row(static_cast<int>(i));
		io_data[i].resize(components);
		for(int j=0;j<components;j++)
		{
			io_data[i][j]=row_data_vector[j];
		}
	}
}

std::vector< std::vector<double> > compute_pca_3d_directions(std::vector< std::vector<double> >& input_data)
{
	if(input_data.empty() || input_data.front().empty())
	{
		throw std::runtime_error(std::string("Empty input data"));
	}

	if(input_data.front().size()!=3)
	{
		throw std::runtime_error(std::string("Input data element size is not 3"));
	}

	Eigen::MatrixXd raw_data_matrix(static_cast<int>(input_data.size()), static_cast<int>(input_data.front().size()));
	for(std::size_t i=0;i<input_data.size();i++)
	{
		Eigen::VectorXd row_data_vector=Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(input_data[i].data(), static_cast<int>(input_data[i].size()));
		raw_data_matrix.row(static_cast<int>(i))=row_data_vector;
	}

	Eigen::MatrixXd centered_data_matrix=raw_data_matrix.rowwise()-raw_data_matrix.colwise().mean();

	Eigen::MatrixXd covariance_matrix=centered_data_matrix.adjoint()*centered_data_matrix;

	Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigensolver(covariance_matrix);

	if(eigensolver.info()!=Eigen::Success)
	{
		throw std::runtime_error(std::string("Failed to perform eigendecomposition."));
	}

	Eigen::MatrixXd pca_basis=eigensolver.eigenvectors().rightCols(3);

	std::vector< std::vector<double> > ds(3, std::vector<double>(3, 0.0));

	for(int col=0;col<3;col++)
	{
		for(int row=0;row<3;row++)
		{
			ds[col][row]=pca_basis.coeff(row, col);
		}
		const double length=sqrt(ds[col][0]*ds[col][0]+ds[col][1]*ds[col][1]+ds[col][2]*ds[col][2]);
		for(int row=0;row<3;row++)
		{
			ds[col][row]/=length;
		}
	}

	return ds;
}

}
