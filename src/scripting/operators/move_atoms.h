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
	std::vector<double> pre_translation_vector;
	std::vector<double> rotation_matrix;
	std::vector<double> rotation_axis_and_angle;
	std::vector<double> post_translation_vector;

	MoveAtoms()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		pre_translation_vector=input.get_value_vector_or_default<double>("translate-before", std::vector<double>());
		rotation_matrix=input.get_value_vector_or_default<double>("rotate-by-matrix", std::vector<double>());
		rotation_axis_and_angle=input.get_value_vector_or_default<double>("rotate-by-axis-and-angle", std::vector<double>());
		post_translation_vector=input.get_value_vector_or_default<double>("translate", std::vector<double>());
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("translate-before", CDOD::DATATYPE_FLOAT_ARRAY, "translation vector to apply before rotation", ""));
		doc.set_option_decription(CDOD("rotate-by-matrix", CDOD::DATATYPE_FLOAT_ARRAY, "rotation matrix", ""));
		doc.set_option_decription(CDOD("rotate-by-axis-and-angle", CDOD::DATATYPE_FLOAT_ARRAY, "vector with rotation axis and angle", ""));
		doc.set_option_decription(CDOD("translate", CDOD::DATATYPE_FLOAT_ARRAY, "translation vector to apply after rotation", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		const std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		data_manager.transform_coordinates_of_atoms(ids, pre_translation_vector, rotation_matrix, rotation_axis_and_angle, post_translation_vector);

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_MOVE_ATOMS_H_ */

