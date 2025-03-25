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
	bool pair_special;

	CenterAtoms() : mean_for_origin(false), pair_special(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		mean_for_origin=input.get_flag("mean-for-origin");
		pair_special=input.get_flag("pair-special");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("mean-for-origin", CDOD::DATATYPE_BOOL, "flag to use mean of coordinates for origin"));
	}

	Result run(DataManager& data_manager) const
	{
		if(pair_special && mean_for_origin)
		{
			throw std::runtime_error(std::string("The option to use mean of coordinates for origin is incompatible with pair-focused centering."));
		}

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

		if(pair_special)
		{
			data_manager.assert_primary_structure_info_valid();

			if(focused_ids.size()!=2)
			{
				throw std::runtime_error(std::string("Not exactly two atoms selected for the pair-focused centering."));
			}

			std::size_t id1=(*focused_ids.begin());
			std::size_t id2=(*focused_ids.rbegin());

			if(data_manager.atoms()[id2].crad.without_numbering()<data_manager.atoms()[id1].crad.without_numbering())
			{
				std::swap(id1, id2);
			}

			{
				const apollota::SimplePoint p1(data_manager.atoms()[id1].value);
				const apollota::SimplePoint p2(data_manager.atoms()[id2].value);
				const apollota::SimplePoint pm=(p1+p2)*0.5;
				const apollota::SimplePoint original_direction=(p2-pm).unit();
				const apollota::SimplePoint new_direction(0.0, 1.0, 0.0);
				const apollota::SimplePoint rotation_axis=apollota::cross_product<apollota::SimplePoint>(original_direction, new_direction);
				double rotation_angle=std::asin(rotation_axis.module())/apollota::pi_value()*180.0;
				if(apollota::dot_product(original_direction, new_direction)<0.0)
				{
					rotation_angle=180.0-rotation_angle;
				}
				const apollota::SimplePoint unit_rotation_axis=rotation_axis.unit();

				DataManager::TransformationOfCoordinates transformation;
				transformation.pre_translation_vector.resize(3, 0.0);
				transformation.pre_translation_vector[0]=-pm.x;
				transformation.pre_translation_vector[1]=-pm.y;
				transformation.pre_translation_vector[2]=-pm.z;
				transformation.rotation_axis_and_angle.resize(4, 0.0);
				transformation.rotation_axis_and_angle[0]=unit_rotation_axis.x;
				transformation.rotation_axis_and_angle[1]=unit_rotation_axis.y;
				transformation.rotation_axis_and_angle[2]=unit_rotation_axis.z;
				transformation.rotation_axis_and_angle[3]=rotation_angle;

				data_manager.transform_coordinates_of_atoms(all_ids, transformation, false);
			}

			{
				apollota::SimplePoint residue_direction;
				int residue_direction_counter=0;
				const std::vector<std::size_t>& residue_atom_ids1=data_manager.primary_structure_info().residues[data_manager.primary_structure_info().map_of_atoms_to_residues[id1]].atom_ids;
				const std::vector<std::size_t>& residue_atom_ids2=data_manager.primary_structure_info().residues[data_manager.primary_structure_info().map_of_atoms_to_residues[id2]].atom_ids;
				for(std::vector<std::size_t>::const_iterator it=residue_atom_ids1.begin();it!=residue_atom_ids1.end();++it)
				{
					if((*it)!=id1)
					{
						const apollota::SimplePoint residue_atom_p(data_manager.atoms()[*it].value);
						residue_direction=residue_direction+residue_atom_p;
						residue_direction_counter++;
					}
				}
				for(std::vector<std::size_t>::const_iterator it=residue_atom_ids2.begin();it!=residue_atom_ids2.end();++it)
				{
					if((*it)!=id2)
					{
						const apollota::SimplePoint residue_atom_p(data_manager.atoms()[*it].value);
						residue_direction=residue_direction+residue_atom_p;
						residue_direction_counter++;
					}
				}
				if(residue_direction_counter>0)
				{
					residue_direction=residue_direction*(1.0/static_cast<double>(residue_direction_counter));
					const apollota::SimplePoint vertical_axis(0.0, 1.0, 0.0);
					const double residue_direction_shift=apollota::dot_product(vertical_axis, residue_direction);
					const apollota::SimplePoint original_residue_direction=(residue_direction-(vertical_axis*residue_direction_shift)).unit();
					const apollota::SimplePoint new_residue_direction(1.0, 0.0, 0.0);
					const apollota::SimplePoint rotation_axis=apollota::cross_product<apollota::SimplePoint>(original_residue_direction, new_residue_direction);
					double rotation_angle=std::asin(rotation_axis.module())/apollota::pi_value()*180.0;
					if(apollota::dot_product(original_residue_direction, new_residue_direction)<0.0)
					{
						rotation_angle=180.0-rotation_angle;
					}
					const apollota::SimplePoint unit_rotation_axis=rotation_axis.unit();

					DataManager::TransformationOfCoordinates transformation;
					transformation.rotation_axis_and_angle.resize(4, 0.0);
					transformation.rotation_axis_and_angle[0]=unit_rotation_axis.x;
					transformation.rotation_axis_and_angle[1]=unit_rotation_axis.y;
					transformation.rotation_axis_and_angle[2]=unit_rotation_axis.z;
					transformation.rotation_axis_and_angle[3]=rotation_angle;

					data_manager.transform_coordinates_of_atoms(all_ids, transformation, false);
				}
			}
		}
		else
		{
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
		}

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

