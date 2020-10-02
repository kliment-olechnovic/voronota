#ifndef SCRIPTING_OPERATORS_VOROMQA_MEMBRANE_PLACE_H_
#define SCRIPTING_OPERATORS_VOROMQA_MEMBRANE_PLACE_H_

#include "../operators_common.h"
#include "../membrane_placement_for_data_manager_using_voromqa.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class VoroMQAMembranePlace : public OperatorBase<VoroMQAMembranePlace>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		MembranePlacementForDataManagerUsingVoroMQA::OrientationScore best_score;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantObject& info=heterostorage.variant_object;
			info.value("number_of_checks")=best_score.number_of_checks;
			info.value("best_score")=best_score.value();
			std::vector<VariantValue>& direction=info.values_array("direction");
			direction.resize(3);
			direction[0]=best_score.direction.x;
			direction[1]=best_score.direction.y;
			direction[2]=best_score.direction.z;
			info.value("projection_center")=best_score.projection_center;
		}
	};

	std::string adjunct_contact_frustration_value;
	std::string adjunct_atom_exposure_value;
	std::string adjunct_atom_membrane_place_value;
	std::string adjunct_atom_weighted_membrane_place_value;
	std::string adjunct_atom_shift_from_membrane_center;
	std::string global_adj_prefix;
	double membrane_width;
	double membrane_width_extended;

	VoroMQAMembranePlace() : membrane_width(30.0), membrane_width_extended(30.0)
	{
	}

	void initialize(CommandInput& input)
	{
		adjunct_contact_frustration_value=input.get_value_or_default<std::string>("adj-contact-frustration-value", "frustration_energy_mean");
		adjunct_atom_exposure_value=input.get_value_or_default<std::string>("adj-atom-exposure-value", "exposure_value");
		adjunct_atom_membrane_place_value=input.get_value_or_default<std::string>("adj-atom-membrane-place-value", "membrane_place_value");
		adjunct_atom_weighted_membrane_place_value=input.get_value_or_default<std::string>("adj-atom-weighted-membrane-place-value", "weighted_membrane_place_value");
		adjunct_atom_shift_from_membrane_center=input.get_value_or_default<std::string>("adj-atom-shift-from-membrane-center", "");
		global_adj_prefix=input.get_value_or_default<std::string>("global-adj-prefix", "");
		membrane_width=input.get_value<double>("membrane-width");
		membrane_width_extended=input.get_value_or_default<double>("membrane-width-extended", membrane_width);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("adj-contact-frustration-value", CDOD::DATATYPE_STRING, "name of input adjunct with frustration energy mean values", "frustration_energy_mean"));
		doc.set_option_decription(CDOD("adj-atom-exposure-value", CDOD::DATATYPE_STRING, "name of input adjunct with exposure values", "exposure_value"));
		doc.set_option_decription(CDOD("adj-atom-membrane-place-value", CDOD::DATATYPE_STRING, "name of output adjunct for membrane place values", "membrane_place_value"));
		doc.set_option_decription(CDOD("adj-atom-weighted-membrane-place-value", CDOD::DATATYPE_STRING, "name of output adjunct for weighted membrane place values", "weighted_membrane_place_value"));
		doc.set_option_decription(CDOD("adj-atom-shift-from-membrane-center", CDOD::DATATYPE_STRING, "name of output adjunct for shift from membrane center", ""));
		doc.set_option_decription(CDOD("global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts", ""));
		doc.set_option_decription(CDOD("membrane-width", CDOD::DATATYPE_FLOAT, "membrane width"));
		doc.set_option_decription(CDOD("membrane-width-extended", CDOD::DATATYPE_FLOAT, "membrane width extended", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(adjunct_contact_frustration_value, false);
		assert_adjunct_name_input(adjunct_atom_exposure_value, true);
		assert_adjunct_name_input(adjunct_atom_membrane_place_value, true);

		if(membrane_width<6.0)
		{
			throw std::runtime_error(std::string("Invalid membrane width."));
		}

		if(membrane_width_extended<membrane_width)
		{
			throw std::runtime_error(std::string("Invalid extended membrane width."));
		}

		std::vector<MembranePlacementForDataManagerUsingVoroMQA::AtomDescriptor> atom_descriptors=
				MembranePlacementForDataManagerUsingVoroMQA::init_atom_descriptors(data_manager, adjunct_contact_frustration_value, adjunct_atom_exposure_value);

		const MembranePlacementForDataManagerUsingVoroMQA::OrientationScore best_score=
				MembranePlacementForDataManagerUsingVoroMQA::score_orientation(atom_descriptors, membrane_width, membrane_width_extended);

		if(!adjunct_atom_membrane_place_value.empty())
		{
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				data_manager.atom_adjuncts_mutable(i).erase(adjunct_atom_membrane_place_value);
			}

			for(std::size_t i=0;i<atom_descriptors.size();i++)
			{
				const MembranePlacementForDataManagerUsingVoroMQA::AtomDescriptor& ad=atom_descriptors[i];
				data_manager.atom_adjuncts_mutable(ad.atom_id)[adjunct_atom_membrane_place_value]=ad.membrane_place_value;
			}
		}

		if(!adjunct_atom_weighted_membrane_place_value.empty())
		{
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				data_manager.atom_adjuncts_mutable(i).erase(adjunct_atom_weighted_membrane_place_value);
			}

			for(std::size_t i=0;i<atom_descriptors.size();i++)
			{
				const MembranePlacementForDataManagerUsingVoroMQA::AtomDescriptor& ad=atom_descriptors[i];
				if(ad.exposure_recorded)
				{
					data_manager.atom_adjuncts_mutable(ad.atom_id)[adjunct_atom_weighted_membrane_place_value]=ad.membrane_place_value*(1.0-std::max(0.0, std::min(ad.exposure, 1.0)));
				}
			}
		}

		if(!adjunct_atom_shift_from_membrane_center.empty())
		{
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				data_manager.atom_adjuncts_mutable(i).erase(adjunct_atom_shift_from_membrane_center);
			}

			for(std::size_t i=0;i<atom_descriptors.size();i++)
			{
				const MembranePlacementForDataManagerUsingVoroMQA::AtomDescriptor& ad=atom_descriptors[i];
				data_manager.atom_adjuncts_mutable(ad.atom_id)[adjunct_atom_shift_from_membrane_center]=(ad.projection-best_score.projection_center);
			}
		}

		if(!global_adj_prefix.empty())
		{
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_best_score"]=best_score.value();
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_checks"]=best_score.number_of_checks;
		}

		Result result;
		result.best_score=best_score;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_VOROMQA_MEMBRANE_PLACE_H_ */
