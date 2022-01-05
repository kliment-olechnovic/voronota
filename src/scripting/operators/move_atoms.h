#ifndef SCRIPTING_OPERATORS_MOVE_ATOMS_H_
#define SCRIPTING_OPERATORS_MOVE_ATOMS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class MoveAtoms : public OperatorBase<MoveAtoms>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
		}
	};

	SelectionManager::Query parameters_for_selecting;
	DataManager::TransformationOfCoordinates transformation;
	bool rotate_randomly;

	MoveAtoms() : rotate_randomly(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		transformation.pre_translation_vector=input.get_value_vector_or_default<double>("translate-before", std::vector<double>());
		transformation.pre_translation_scale=input.get_value_or_default<double>("scale-for-translate-before", 1.0);
		transformation.rotation_matrix=input.get_value_vector_or_default<double>("rotate-by-matrix", std::vector<double>());
		transformation.rotation_axis_and_angle=input.get_value_vector_or_default<double>("rotate-by-axis-and-angle", std::vector<double>());
		transformation.rotation_three_angles=input.get_value_vector_or_default<double>("rotate-by-three-angles", std::vector<double>());
		transformation.rotation_ztwist_theta_phi=input.get_value_vector_or_default<double>("rotate-by-ztwist-theta-phi", std::vector<double>());
		rotate_randomly=input.get_flag("rotate-randomly");
		transformation.post_translation_vector=input.get_value_vector_or_default<double>("translate", std::vector<double>());
		transformation.post_translation_scale=input.get_value_or_default<double>("scale-for-translate", 1.0);
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("translate-before", CDOD::DATATYPE_FLOAT_ARRAY, "translation vector to apply before rotation", ""));
		doc.set_option_decription(CDOD("scale-for-translate-before", CDOD::DATATYPE_FLOAT, "scale for translation vector to apply before rotation", 1.0));
		doc.set_option_decription(CDOD("rotate-by-matrix", CDOD::DATATYPE_FLOAT_ARRAY, "rotation matrix", ""));
		doc.set_option_decription(CDOD("rotate-by-axis-and-angle", CDOD::DATATYPE_FLOAT_ARRAY, "vector with rotation axis and angle", ""));
		doc.set_option_decription(CDOD("rotate-by-three-angles", CDOD::DATATYPE_FLOAT_ARRAY, "vector with rotation three angles", ""));
		doc.set_option_decription(CDOD("rotate-by-ztwist-theta-phi", CDOD::DATATYPE_FLOAT_ARRAY, "vector with values of z-twist, theta, phi", ""));
		doc.set_option_decription(CDOD("rotate-randomly", CDOD::DATATYPE_BOOL, "flag to rotate randomly"));
		doc.set_option_decription(CDOD("translate", CDOD::DATATYPE_FLOAT_ARRAY, "translation vector to apply after rotation", ""));
		doc.set_option_decription(CDOD("scale-for-translate", CDOD::DATATYPE_FLOAT, "scale for translation vector to apply after rotation", 1.0));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		const std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		DataManager::TransformationOfCoordinates transformation_to_use=transformation;

		if(rotate_randomly)
		{
			transformation_to_use.rotation_three_angles.resize(3, 0.0);
			for(int i=0;i<3;i++)
			{
				transformation_to_use.rotation_three_angles[i]=std::rand()%360;
			}
		}

		data_manager.transform_coordinates_of_atoms(ids, transformation_to_use);

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_MOVE_ATOMS_H_ */

