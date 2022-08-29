#ifndef SCRIPTING_OPERATORS_IMPORT_DOCKING_RESULT_H_
#define SCRIPTING_OPERATORS_IMPORT_DOCKING_RESULT_H_

#include "import.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ImportDockingResult : public OperatorBase<ImportDockingResult>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;
		std::string object_name;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.summaries_of_atoms["loaded"]=atoms_summary;
			heterostorage.variant_object.value("object_name")=object_name;
		}
	};

	std::string static_file;
	std::string mobile_file;
	std::vector<double> ftdock_transform;
	std::vector<double> matrix_transform;
	std::vector<double> generic_transform;
	Import import_operator;

	ImportDockingResult()
	{
	}

	void initialize(CommandInput& input)
	{
		static_file=input.get_value<std::string>("static-file");
		mobile_file=input.get_value<std::string>("mobile-file");
		ftdock_transform=input.get_value_vector_or_default<double>("ftdock-transform", std::vector<double>());
		matrix_transform=input.get_value_vector_or_default<double>("matrix-transform", std::vector<double>());
		generic_transform=input.get_value_vector_or_default<double>("generic-transform", std::vector<double>());
		import_operator.initialize(input, true);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("static-file", CDOD::DATATYPE_STRING, "paths to static structure file"));
		doc.set_option_decription(CDOD("mobile-file", CDOD::DATATYPE_STRING, "paths to mobile structure file"));
		doc.set_option_decription(CDOD("ftdock-transform", CDOD::DATATYPE_FLOAT_ARRAY, "ftdock transformation (cell_span, x, y, z, a1, a2, a3)"));
		doc.set_option_decription(CDOD("matrix-transform", CDOD::DATATYPE_FLOAT_ARRAY, "matrix transformation (x, y, z, a1, a2, a3, a4, a5, a6, a7, a8, a9)"));
		doc.set_option_decription(CDOD("generic-transform", CDOD::DATATYPE_FLOAT_ARRAY, "generic transformation (cell_span|0, x, y, z, a1, a2, a3, 0|a4, 0|a5, 0|a6, 0|a7, 0|a8, 0|a9)"));
		import_operator.document(doc, true);
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(static_file.empty())
		{
			throw std::runtime_error(std::string("Missing static file path."));
		}

		if(mobile_file.empty())
		{
			throw std::runtime_error(std::string("Missing mobile file path."));
		}

		if(ftdock_transform.empty() && matrix_transform.empty() && generic_transform.empty())
		{
			throw std::runtime_error(std::string("No transformation provided."));
		}

		if(((ftdock_transform.empty() ? 0 : 1)+(matrix_transform.empty() ? 0 : 1)+(generic_transform.empty() ? 0 : 1))!=1)
		{
			throw std::runtime_error(std::string("Not exactly one transformation provided."));
		}

		if(!ftdock_transform.empty() && ftdock_transform.size()!=7)
		{
			throw std::runtime_error(std::string("Invalid number of ftdock transform values, must be 7."));
		}

		if(!matrix_transform.empty() && matrix_transform.size()!=12)
		{
			throw std::runtime_error(std::string("Invalid number of matrix transform values, must be 12."));
		}

		if(!generic_transform.empty() && generic_transform.size()!=13)
		{
			throw std::runtime_error(std::string("Invalid number of generic transform values, must be 13."));
		}

		LoadingOfData::Result loading_result_static;

		{
			LoadingOfData::Parameters loading_parameters=import_operator.loading_parameters;
			loading_parameters.file=static_file;
			LoadingOfData::construct_result(loading_parameters, loading_result_static);
			if(loading_result_static.atoms.empty())
			{
				throw std::runtime_error(std::string("Failed to load static structure file."));
			}
		}

		LoadingOfData::Result loading_result_mobile;

		{
			LoadingOfData::Parameters loading_parameters=import_operator.loading_parameters;
			loading_parameters.file=mobile_file;
			LoadingOfData::construct_result(loading_parameters, loading_result_mobile);
			if(loading_result_mobile.atoms.empty())
			{
				throw std::runtime_error(std::string("Failed to load mobile structure file."));
			}
		}

		if((loading_result_static.atoms.size()+loading_result_mobile.atoms.size())<2)
		{
			throw std::runtime_error(std::string("Less than 2 atoms in total read."));
		}

		std::vector<double> ftdock_transform_to_use;
		std::vector<double> matrix_transform_to_use;

		if(!generic_transform.empty())
		{
			if(generic_transform[0]>0.0)
			{
				ftdock_transform_to_use.resize(7, 0.0);
				for(std::size_t i=0;i<7;i++)
				{
					ftdock_transform_to_use[i]=generic_transform[i];
				}
			}
			else
			{
				matrix_transform_to_use.resize(12, 0.0);
				for(std::size_t i=0;i<12;i++)
				{
					matrix_transform_to_use[i]=generic_transform[i+1];
				}
			}
		}
		else if(!ftdock_transform.empty())
		{
			ftdock_transform_to_use=ftdock_transform;
		}
		else if(!matrix_transform.empty())
		{
			matrix_transform_to_use=matrix_transform;
		}

		if(!ftdock_transform_to_use.empty())
		{
			DataManager::TransformationOfCoordinates transformation;

			transformation.post_translation_scale=ftdock_transform_to_use[0];

			transformation.post_translation_vector.resize(3, 0.0);
			transformation.post_translation_vector[0]=ftdock_transform_to_use[1];
			transformation.post_translation_vector[1]=ftdock_transform_to_use[2];
			transformation.post_translation_vector[2]=ftdock_transform_to_use[3];

			transformation.rotation_ztwist_theta_phi.resize(3, 0.0);
			transformation.rotation_ztwist_theta_phi[0]=ftdock_transform_to_use[4];
			transformation.rotation_ztwist_theta_phi[1]=ftdock_transform_to_use[5];
			transformation.rotation_ztwist_theta_phi[2]=ftdock_transform_to_use[6];

			transformation.transform_coordinates_of_atoms(loading_result_mobile.atoms);
		}

		if(!matrix_transform_to_use.empty())
		{
			DataManager::TransformationOfCoordinates transformation;

			transformation.post_translation_vector.resize(3, 0.0);
			transformation.post_translation_vector[0]=matrix_transform_to_use[0];
			transformation.post_translation_vector[1]=matrix_transform_to_use[1];
			transformation.post_translation_vector[2]=matrix_transform_to_use[2];

			transformation.rotation_matrix.resize(9, 0.0);
			for(std::size_t i=0;i<9;i++)
			{
				transformation.rotation_matrix[i]=matrix_transform_to_use[3+i];
			}

			transformation.transform_coordinates_of_atoms(loading_result_mobile.atoms);
		}

		loading_result_static.atoms.insert(loading_result_static.atoms.end(), loading_result_mobile.atoms.begin(), loading_result_mobile.atoms.end());

		DataManager* object_new=congregation_of_data_managers.add_object(DataManager(), import_operator.title);
		DataManager& data_manager=*object_new;

		data_manager.reset_atoms_by_swapping(loading_result_static.atoms);

		congregation_of_data_managers.set_all_objects_picked(false);
		congregation_of_data_managers.set_object_picked(object_new, true);

		Result result;

		result.atoms_summary=SummaryOfAtoms(data_manager.atoms());
		result.object_name=congregation_of_data_managers.get_object_attributes(object_new).name;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_IMPORT_DOCKING_RESULT_H_ */
