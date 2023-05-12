#ifndef DUKTAPER_OPERATORS_SET_ADJUNCTS_OF_ATOMS_BY_PCA_H_
#define DUKTAPER_OPERATORS_SET_ADJUNCTS_OF_ATOMS_BY_PCA_H_

#include "../operators_common.h"

#include "../../dependencies/utilities/eigen_pca.h"

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
	bool no_zscores;
	std::string output_prefix;

	SetAdjunctsOfAtomsByPCA() : components(0), no_zscores(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		parameters_for_selecting=scripting::OperatorsUtilities::read_generic_selecting_query(input);
		input_adjuncts=input.get_value_vector<std::string>("input-adjuncts");
		components=input.get_value<int>("components");
		no_zscores=input.get_flag("no-zscores");
		output_prefix=input.get_value<std::string>("output-prefix");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		scripting::OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("input-adjuncts", CDOD::DATATYPE_STRING_ARRAY, "input adjunct names"));
		doc.set_option_decription(CDOD("components", CDOD::DATATYPE_INT, "number of transformed coordinates to output"));
		doc.set_option_decription(CDOD("no-zscores", CDOD::DATATYPE_BOOL, "flag to not convert input values to z-scores"));
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

		std::vector< std::vector<double> > io_data(atom_ids.size(), std::vector<double>(input_adjuncts.size(), 0.0));

		{
			int atom_i=0;
			for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
			{
				std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
				std::vector<double>& input_adjunct_values=io_data[atom_i];
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
				atom_i++;
			}
		}

		eigen_pca::compute_pca_and_project_on_basis(components, no_zscores, io_data);

		{
			int row_i=0;
			for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
			{
				std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
				for(std::size_t j=0;j<io_data[row_i].size();j++)
				{
					std::ostringstream name_output;
					name_output << output_prefix << (j+1);
					atom_adjuncts[name_output.str()]=io_data[row_i][j];
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
