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
						data_manager.atoms(), data_manager.contacts(), 4, false, adjunct_atom_volumes, adjunct_atom_quality_scores, adjunct_inter_atom_energy_scores_raw);

		OutputSelector output_selector(file);

		{
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);

			if(!map_of_residue_energy_profiles.empty())
			{
				output << "pos ncat";
				for(int i=0;i<20;i++)
				{
					output << " nf" << i;
				}
				const CCEG::ContactEffectGroupingEnergyProfile& cegep=(map_of_residue_energy_profiles.begin()->second);
				for(std::size_t i=0;i<cegep.layered_residue_attributes.size();i++)
				{
					output << " rl" << i << "_ac";
					output << " rl" << i << "_av";
					output << " rl" << i << "_aqs";
					output << " rl" << i << "_sca";
					output << " rl" << i << "_sce";
				}
				for(std::size_t i=0;i<cegep.layered_inter_residue_attributes.size();i++)
				{
					for(std::size_t j=0;j<cegep.layered_inter_residue_attributes[i].length();j++)
					{
						output << " irl" << i << "_ss" << j << "_iacas";
						output << " irl" << i << "_ss" << j << "_iaces";
					}
				}
				output << "\n";
			}

			for(std::map<CCEG::CRAD, CCEG::ContactEffectGroupingEnergyProfile>::const_iterator it=map_of_residue_energy_profiles.begin();it!=map_of_residue_energy_profiles.end();++it)
			{
				{
					const CCEG::CRAD& crad=it->first;

					output << crad.chainID << "_" << crad.resSeq << "_" << crad.resName;
					if(!crad.iCode.empty())
					{
						output << "_i" << crad.iCode;
					}

					const int res_name_number=convert_residue_name_to_number(crad.resName);

					output << " " << res_name_number;

					for(int i=0;i<20;i++)
					{
						output << " " << (i==res_name_number ? 1 : 0);
					}
				}

				const CCEG::ContactEffectGroupingEnergyProfile& cegep=it->second;

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
					for(std::size_t j=0;j<cegep.layered_inter_residue_attributes[i].length();j++)
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

private:
	static int convert_residue_name_to_number(const std::string& name)
	{
		static const std::map<std::string, int> m=create_map_of_residue_names_to_numbers();
		const std::map<std::string, int>::const_iterator it=m.find(name);
		return (it==m.end() ? -1 : it->second);
	}

	static std::map<std::string, int> create_map_of_residue_names_to_numbers()
	{
		std::map<std::string, int> m;

		m["LEU"]=-1;
		m["VAL"]=-1;
		m["ILE"]=-1;
		m["ALA"]=-1;
		m["PHE"]=-1;
		m["TRP"]=-1;
		m["MET"]=-1;
		m["PRO"]=-1;
		m["ASP"]=-1;
		m["GLU"]=-1;
		m["LYS"]=-1;
		m["ARG"]=-1;
		m["HIS"]=-1;
		m["CYS"]=-1;
		m["SER"]=-1;
		m["THR"]=-1;
		m["TYR"]=-1;
		m["ASN"]=-1;
		m["GLN"]=-1;
		m["GLY"]=-1;

		int id=0;
		for(std::map<std::string, int>::iterator it=m.begin();it!=m.end();++it)
		{
			it->second=id;
			id++;
		}

		return m;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_GENERATE_RESIDUE_VOROMQA_ENERGY_PROFILE_H_ */
