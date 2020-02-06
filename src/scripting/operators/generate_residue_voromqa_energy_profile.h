#ifndef SCRIPTING_OPERATORS_GENERATE_RESIDUE_VOROMQA_ENERGY_PROFILE_H_
#define SCRIPTING_OPERATORS_GENERATE_RESIDUE_VOROMQA_ENERGY_PROFILE_H_

#include "../../common/construction_of_contact_effect_groupings.h"

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class GenerateResidueVoroMQAEnergyProfile : public OperatorBase<GenerateResidueVoroMQAEnergyProfile>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string file;
		std::string dump;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			if(!dump.empty())
			{
				heterostorage.variant_object.value("dump")=dump;
			}
		}
	};

	std::string adjunct_inter_atom_energy_scores_raw;
	std::string adjunct_atom_quality_scores;
	std::string adjunct_atom_volumes;
	std::string file;

	GenerateResidueVoroMQAEnergyProfile()
	{
	}

	void initialize(CommandInput& input)
	{
		adjunct_inter_atom_energy_scores_raw=input.get_value_or_default<std::string>("adj-contact-energy", "voromqa_energy");
		adjunct_atom_quality_scores=input.get_value_or_default<std::string>("adj-atom-quality", "voromqa_score_a");
		adjunct_atom_volumes=input.get_value_or_default<std::string>("adj-atom-volume", "volume");
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("adj-contact-energy", CDOD::DATATYPE_STRING, "name of input adjunct with contact energy values", "voromqa_energy"));
		doc.set_option_decription(CDOD("adj-atom-quality", CDOD::DATATYPE_STRING, "name of input adjunct with atom quality scores", "voromqa_score_a"));
		doc.set_option_decription(CDOD("adj-atom-volume", CDOD::DATATYPE_STRING, "name of input adjunct with atom volume value", "volume"));
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_file_name_input(file, false);
		assert_adjunct_name_input(adjunct_inter_atom_energy_scores_raw, false);
		assert_adjunct_name_input(adjunct_atom_quality_scores, false);
		assert_adjunct_name_input(adjunct_atom_volumes, false);

		typedef common::ConstructionOfContactEffectGroupings CCEG;

		std::map<CCEG::CRAD, CCEG::ContactEffectGroupingEnergyProfile> map_of_residue_energy_profiles=
				CCEG::ContactEffectGroupingEnergyProfile::construct_map_of_contact_effect_grouping_energy_profiles(
						data_manager.atoms(), data_manager.contacts(), 3, adjunct_atom_volumes, adjunct_atom_quality_scores, adjunct_inter_atom_energy_scores_raw);

		OutputSelector output_selector(file);

		{
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);

			for(std::map<CCEG::CRAD, CCEG::ContactEffectGroupingEnergyProfile>::const_iterator it=map_of_residue_energy_profiles.begin();it!=map_of_residue_energy_profiles.end();++it)
			{
				const CCEG::CRAD& crad=it->first;
				const CCEG::ContactEffectGroupingEnergyProfile& cegep=it->second;
				output << crad << " " << crad.resName;
				for(std::size_t i=0;i<cegep.layered_residue_attributes.size();i++)
				{
					output << " " << cegep.layered_residue_attributes[i].atom_count_sum;
					output << " " << cegep.layered_residue_attributes[i].atom_volume_sum;
					output << " " << cegep.layered_residue_attributes[i].atom_quality_score_sum;
					output << " " << cegep.layered_residue_attributes[i].solvent_contact_area_sum;
					output << " " << cegep.layered_residue_attributes[i].solvent_contact_energy_sum;
				}
				for(std::size_t i=0;i<cegep.layered_inter_residue_attributes.size();i++)
				{
					output << " " << cegep.layered_inter_residue_attributes[i].inter_atom_contact_area_sum;
					output << " " << cegep.layered_inter_residue_attributes[i].inter_atom_contact_energy_sum;
				}
				for(std::size_t i=0;i<cegep.layered_inter_residue_attributes.size();i++)
				{
					const std::size_t num_of_splits=std::min(
							cegep.layered_inter_residue_attributes[i].inter_atom_contact_area_split_sum.size(),
							cegep.layered_inter_residue_attributes[i].inter_atom_contact_energy_split_sum.size());
					for(std::size_t j=0;j<num_of_splits;j++)
					{
						output << " " << cegep.layered_inter_residue_attributes[i].inter_atom_contact_area_split_sum[j];
						output << " " << cegep.layered_inter_residue_attributes[i].inter_atom_contact_energy_split_sum[j];
					}
				}
				output << "\n";
			}
		}

		Result result;
		result.file=file;
		if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
		{
			result.dump=output_selector.str();
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_GENERATE_RESIDUE_VOROMQA_ENERGY_PROFILE_H_ */
