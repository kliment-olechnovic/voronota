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
	Import import_operator;

	ImportDockingResult()
	{
	}

	void initialize(CommandInput& input)
	{
		static_file=input.get_value<std::string>("static-file");
		mobile_file=input.get_value<std::string>("mobile-file");
		ftdock_transform=input.get_value_vector<double>("ftdock-transform");
		import_operator.initialize(input, true);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("static-file", CDOD::DATATYPE_STRING, "paths to static structure file"));
		doc.set_option_decription(CDOD("mobile-file", CDOD::DATATYPE_STRING, "paths to mobile structure file"));
		doc.set_option_decription(CDOD("ftdock-transform", CDOD::DATATYPE_FLOAT_ARRAY, "ftdock transformation (cell_span, x, y, z, a1, a2, a3)"));
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

		if(ftdock_transform.size()!=7)
		{
			throw std::runtime_error(std::string("Invalid number of ftdock transform values, must be 7."));
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

		if((loading_result_static.atoms.size()+loading_result_mobile.atoms.size())<4)
		{
			throw std::runtime_error(std::string("Less than 4 atoms in total read."));
		}

		{
			DataManager::TransformationOfCoordinates transformation;

			transformation.post_translation_scale=ftdock_transform[0];

			transformation.post_translation_vector.resize(3, 0.0);
			transformation.post_translation_vector[0]=ftdock_transform[1];
			transformation.post_translation_vector[1]=ftdock_transform[2];
			transformation.post_translation_vector[2]=ftdock_transform[3];

			transformation.rotation_ztwist_theta_phi.resize(3, 0.0);
			transformation.rotation_ztwist_theta_phi[0]=ftdock_transform[4];
			transformation.rotation_ztwist_theta_phi[1]=ftdock_transform[5];
			transformation.rotation_ztwist_theta_phi[2]=ftdock_transform[6];

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
