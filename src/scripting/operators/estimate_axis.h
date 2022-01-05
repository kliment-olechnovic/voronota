#ifndef SCRIPTING_OPERATORS_ESTIMATE_AXIS_H_
#define SCRIPTING_OPERATORS_ESTIMATE_AXIS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class EstimateAxis : public OperatorBase<EstimateAxis>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		apollota::SimplePoint p1;
		apollota::SimplePoint p2;
		apollota::SimplePoint axis;
		apollota::SimplePoint z_rotation_axis;
		double z_rotation_angle;

		Result() : z_rotation_angle(0.0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.values_array("p1").push_back(VariantValue(p1.x));
			heterostorage.variant_object.values_array("p1").push_back(VariantValue(p1.y));
			heterostorage.variant_object.values_array("p1").push_back(VariantValue(p1.z));

			heterostorage.variant_object.values_array("p2").push_back(VariantValue(p2.x));
			heterostorage.variant_object.values_array("p2").push_back(VariantValue(p2.y));
			heterostorage.variant_object.values_array("p2").push_back(VariantValue(p2.z));

			heterostorage.variant_object.values_array("axis").push_back(VariantValue(axis.x));
			heterostorage.variant_object.values_array("axis").push_back(VariantValue(axis.y));
			heterostorage.variant_object.values_array("axis").push_back(VariantValue(axis.z));

			heterostorage.variant_object.values_array("z_rotation_axis").push_back(VariantValue(z_rotation_axis.x));
			heterostorage.variant_object.values_array("z_rotation_axis").push_back(VariantValue(z_rotation_axis.y));
			heterostorage.variant_object.values_array("z_rotation_axis").push_back(VariantValue(z_rotation_axis.z));

			heterostorage.variant_object.value("z_rotation_angle")=z_rotation_angle;
		}
	};

	std::string selection_expresion_for_atoms_a;
	std::string selection_expresion_for_atoms_b;
	bool align_along_z;
	bool move_to_origin;
	bool move_on_z;
	bool use_min_angle;

	EstimateAxis() : align_along_z(false), move_to_origin(false), move_on_z(false), use_min_angle(false)
	{
	}

	void initialize(CommandInput& input)
	{
		selection_expresion_for_atoms_a=input.get_value<std::string>("atoms-first");
		selection_expresion_for_atoms_b=input.get_value<std::string>("atoms-second");
		align_along_z=input.get_flag("align-along-z");
		move_to_origin=input.get_flag("move-to-origin");
		move_on_z=input.get_flag("move-on-z");
		use_min_angle=input.get_flag("use-min-angle");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("atoms-first", CDOD::DATATYPE_STRING, "selection expression for the first group of atoms"));
		doc.set_option_decription(CDOD("atoms-second", CDOD::DATATYPE_STRING, "selection expression for the second group of atoms"));
		doc.set_option_decription(CDOD("align-along-z", CDOD::DATATYPE_BOOL, "flag to align along Z axis"));
		doc.set_option_decription(CDOD("move-to-origin", CDOD::DATATYPE_BOOL, "flag to move to origin after aligning along Z axis"));
		doc.set_option_decription(CDOD("move-on-z", CDOD::DATATYPE_BOOL, "flag to move on Z axis after aligning along Z axis"));
		doc.set_option_decription(CDOD("use-min-angle", CDOD::DATATYPE_BOOL, "flag to report and use minimal angle"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		const std::set<std::size_t> atom_ids_a=data_manager.selection_manager().select_atoms(SelectionManager::Query(selection_expresion_for_atoms_a, false));

		if(atom_ids_a.empty())
		{
			throw std::runtime_error(std::string("No first atoms selected."));
		}

		const std::set<std::size_t> atom_ids_b=data_manager.selection_manager().select_atoms(SelectionManager::Query(selection_expresion_for_atoms_b, false));

		if(atom_ids_b.empty())
		{
			throw std::runtime_error(std::string("No second atoms selected."));
		}

		Result result;

		for(std::set<std::size_t>::const_iterator it_a=atom_ids_a.begin();it_a!=atom_ids_a.end();++it_a)
		{
			result.p1=result.p1+apollota::SimplePoint(data_manager.atoms()[*it_a].value);
		}
		result.p1=result.p1*(1.0/static_cast<double>(atom_ids_a.size()));

		for(std::set<std::size_t>::const_iterator it_b=atom_ids_b.begin();it_b!=atom_ids_b.end();++it_b)
		{
			result.p2=result.p2+apollota::SimplePoint(data_manager.atoms()[*it_b].value);
		}
		result.p2=result.p2*(1.0/static_cast<double>(atom_ids_b.size()));

		result.axis=(result.p2-result.p1).unit();
		result.z_rotation_axis=(result.axis & apollota::SimplePoint(0.0, 0.0, 1.0)).unit();
		result.z_rotation_angle=apollota::directed_angle(apollota::SimplePoint(0.0, 0.0, 0.0), result.axis, apollota::SimplePoint(0.0, 0.0, 1.0), result.z_rotation_axis)*180.0/apollota::pi_value();

		if(use_min_angle && result.z_rotation_angle>90.0)
		{
			result.z_rotation_angle-=180.0;
		}

		if(align_along_z)
		{
			DataManager::TransformationOfCoordinates transformation;

			transformation.pre_translation_vector.resize(3, 0.0);
			transformation.pre_translation_vector[0]=(0.0-result.p1.x);
			transformation.pre_translation_vector[1]=(0.0-result.p1.y);
			transformation.pre_translation_vector[2]=(0.0-result.p1.z);

			transformation.rotation_axis_and_angle.resize(4, 0.0);
			transformation.rotation_axis_and_angle[0]=result.z_rotation_axis.x;
			transformation.rotation_axis_and_angle[1]=result.z_rotation_axis.y;
			transformation.rotation_axis_and_angle[2]=result.z_rotation_axis.z;
			transformation.rotation_axis_and_angle[3]=result.z_rotation_angle;

			std::vector<double> post_translation_vector;
			if(!move_to_origin)
			{
				transformation.post_translation_vector.resize(3, 0.0);
				if(move_on_z)
				{
					transformation.post_translation_vector[0]=0.0;
					transformation.post_translation_vector[1]=0.0;
					transformation.post_translation_vector[2]=result.p1.z;
				}
				else
				{
					transformation.post_translation_vector[0]=result.p1.x;
					transformation.post_translation_vector[1]=result.p1.y;
					transformation.post_translation_vector[2]=result.p1.z;
				}
			}

			std::set<std::size_t> ids;
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				ids.insert(ids.end(), i);
			}

			data_manager.transform_coordinates_of_atoms(ids, transformation);
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_ESTIMATE_AXIS_H_ */
