#ifndef DUKTAPER_OPERATORS_SET_ADJUNCTS_OF_ATOMS_BY_PCA_H_
#define DUKTAPER_OPERATORS_SET_ADJUNCTS_OF_ATOMS_BY_PCA_H_

#include "../operators_common.h"

#include "../../dependencies/Eigen/Dense"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class SetAdjunctsOfAtomsByPCA : public scripting::OperatorBase<SetAdjunctsOfAtomsByPCA>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	scripting::SelectionManager::Query parameters_for_selecting;
	std::vector<std::string> input_adjuncts;
	int components;
	std::string output_prefix;

	SetAdjunctsOfAtomsByPCA() : components(0)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		parameters_for_selecting=scripting::OperatorsUtilities::read_generic_selecting_query(input);
		input_adjuncts=input.get_value_vector<std::string>("input-adjuncts");
		components=input.get_value<int>("components");
		output_prefix=input.get_value<std::string>("output-prefix");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		scripting::OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("input-adjuncts", CDOD::DATATYPE_STRING_ARRAY, "input adjunct names"));
		doc.set_option_decription(CDOD("components", CDOD::DATATYPE_INT, "number of transformed coordinates to output"));
		doc.set_option_decription(CDOD("output-prefix", CDOD::DATATYPE_STRING, "prefix string for names of output adjuncts"));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(input_adjuncts.size()<2)
		{
			throw std::runtime_error(std::string("Too few input adjuncts"));
		}

		if(components<1)
		{
			throw std::runtime_error(std::string("Invalid number of components to use"));
		}

		if(components>static_cast<int>(input_adjuncts.size()))
		{
			throw std::runtime_error(std::string("Number of components to use is greater than number of input adjuncts"));
		}

		scripting::assert_adjunct_name_input(output_prefix+"1", false);

		const std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			const std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
			for(std::size_t i=0;i<input_adjuncts.size();i++)
			{
				if(input_adjuncts[i]!="center_x" && input_adjuncts[i]!="center_y" && input_adjuncts[i]!="center_z" && input_adjuncts[i]!="radius" && atom_adjuncts.count(input_adjuncts[i])==0)
				{
					throw std::runtime_error(std::string("Input adjuncts not present everywhere in selection."));
				}
			}
		}

		Eigen::MatrixXd raw_data_matrix(static_cast<int>(atom_ids.size()), static_cast<int>(input_adjuncts.size()));

		{
			int row_i=0;
			for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
			{
				std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
				std::vector<double> input_adjunct_values(input_adjuncts.size(), 0.0);
				for(std::size_t i=0;i<input_adjuncts.size();i++)
				{
					if(input_adjuncts[i]=="center_x")
					{
						input_adjunct_values[i]=data_manager.atoms()[*it].value.x;
					}
					else if(input_adjuncts[i]=="center_y")
					{
						input_adjunct_values[i]=data_manager.atoms()[*it].value.y;
					}
					else if(input_adjuncts[i]=="center_z")
					{
						input_adjunct_values[i]=data_manager.atoms()[*it].value.z;
					}
					else if(input_adjuncts[i]=="radius")
					{
						input_adjunct_values[i]=data_manager.atoms()[*it].value.r;
					}
					else
					{
						input_adjunct_values[i]=atom_adjuncts[input_adjuncts[i]];
					}
				}
				Eigen::VectorXd row_data_vector=Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(input_adjunct_values.data(), static_cast<int>(input_adjunct_values.size()));
				raw_data_matrix.row(row_i)=row_data_vector;
				row_i++;
			}
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

		{
			int row_i=0;
			for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
			{
				std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
				Eigen::VectorXd row_data_vector=projected_data_matrix.row(row_i);
				for(int i=0;i<components;i++)
				{
					std::ostringstream name_output;
					name_output << output_prefix << (i+1);
					atom_adjuncts[name_output.str()]=row_data_vector[i];
				}
				row_i++;
			}
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_SET_ADJUNCTS_OF_ATOMS_BY_PCA_H_ */
