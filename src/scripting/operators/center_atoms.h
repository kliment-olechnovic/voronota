#ifndef SCRIPTING_OPERATORS_CENTER_ATOMS_H_
#define SCRIPTING_OPERATORS_CENTER_ATOMS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class CenterAtoms : public OperatorBase<CenterAtoms>
{
public:
    struct Result : public OperatorResultBase<Result>
    {
    	SummaryOfAtoms atoms_summary;
    	SummaryOfAtoms focused_atoms_summary;

    	void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			VariantSerialization::write(focused_atoms_summary, heterostorage.variant_object.object("focused_atoms_summary"));
		}
    };

	SelectionManager::Query parameters_for_selecting;
	bool mean_for_origin;

	CenterAtoms() : mean_for_origin(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		mean_for_origin=input.get_flag("mean-for-origin");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("mean-for-origin", CDOD::DATATYPE_BOOL, "flag to use mean of coordinates for origin"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		const std::set<std::size_t> all_ids=data_manager.selection_manager().select_atoms(SelectionManager::Query());
		if(all_ids.size()!=data_manager.atoms().size())
		{
			throw std::runtime_error(std::string("Not all atoms available for transformation."));
		}

		const std::set<std::size_t> focused_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(focused_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		DataManager::TransformationOfCoordinates transformation;
		transformation.pre_translation_vector.resize(3, 0.0);

		if(mean_for_origin)
		{
			apollota::SimplePoint sum_of_centers;
			for(std::set<std::size_t>::const_iterator it=focused_ids.begin();it!=focused_ids.end();++it)
			{
				const common::BallValue& ball=data_manager.atoms()[*it].value;
				sum_of_centers.x+=ball.x;
				sum_of_centers.y+=ball.y;
				sum_of_centers.z+=ball.z;
			}
			transformation.pre_translation_vector[0]=0.0-(sum_of_centers.x/static_cast<double>(focused_ids.size()));
			transformation.pre_translation_vector[1]=0.0-(sum_of_centers.y/static_cast<double>(focused_ids.size()));
			transformation.pre_translation_vector[2]=0.0-(sum_of_centers.z/static_cast<double>(focused_ids.size()));
		}
		else
		{
			const SummaryOfAtoms summary_of_focused_atoms(data_manager.atoms(), focused_ids);
			const apollota::SimplePoint translation=apollota::SimplePoint(0.0, 0.0, 0.0)-((summary_of_focused_atoms.bounding_box.p_min+summary_of_focused_atoms.bounding_box.p_max)*0.5);
			transformation.pre_translation_vector[0]=translation.x;
			transformation.pre_translation_vector[1]=translation.y;
			transformation.pre_translation_vector[2]=translation.z;
		}

		data_manager.transform_coordinates_of_atoms(all_ids, transformation);

        Result result;
        result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), all_ids);
        result.focused_atoms_summary=SummaryOfAtoms(data_manager.atoms(), focused_ids);

        return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_CENTER_ATOMS_H_ */

