#ifndef VIEWER_OPERATORS_VOROMQA_DARK_GLOBAL_H_
#define VIEWER_OPERATORS_VOROMQA_DARK_GLOBAL_H_

#include "../../../../src/scripting/virtual_file_storage.h"

#include "../stocked_data_resources.h"

#include "nnport_predict.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class VoroMQADarkGlobal : public scripting::OperatorBase<VoroMQADarkGlobal>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		double global_score;
		int number_of_residues;

		Result() : global_score(0.0), number_of_residues(0)
		{
		}

		void add(const Result& r)
		{
			const double sum_of_quality_scores=(global_score*number_of_residues+r.global_score*r.number_of_residues);
			number_of_residues+=r.number_of_residues;
			global_score=(number_of_residues>0.0 ? (sum_of_quality_scores/number_of_residues) : 0.0);
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			write_to_variant_object(heterostorage.variant_object);
		}

		void write_to_variant_object(scripting::VariantObject& variant_object) const
		{
			variant_object.value("global_score")=global_score;
			variant_object.value("number_of_residues")=number_of_residues;
		}
	};

	std::string global_adj_prefix;

	VoroMQADarkGlobal()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		global_adj_prefix=input.get_value_or_default<std::string>("global-adj-prefix", "voromqa_dark_global");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts", "voromqa_dark_global"));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		scripting::VirtualFileStorage::TemporaryFile tmp_profile;
		scripting::VirtualFileStorage::TemporaryFile tmp_scores;

		scripting::operators::GenerateResidueVoroMQAEnergyProfile().init(CMDIN().set("file", tmp_profile.filename())).run(data_manager);

		{
			const std::string filename_for_voromqa_dark_nnport_input_header=scripting::VirtualFileStorage::validate_filename("voromqa_dark_nnport_input_header");
			if(!scripting::VirtualFileStorage::file_exists(filename_for_voromqa_dark_nnport_input_header))
			{
				voronota::scripting::VirtualFileStorage::set_file(filename_for_voromqa_dark_nnport_input_header, resources::data_voromqa_dark_nnport_input_header(), true);
			}

			const std::string filename_for_voromqa_dark_nnport_input_statistics=scripting::VirtualFileStorage::validate_filename("voromqa_dark_nnport_input_statistics");
			if(!scripting::VirtualFileStorage::file_exists(filename_for_voromqa_dark_nnport_input_statistics))
			{
				voronota::scripting::VirtualFileStorage::set_file(filename_for_voromqa_dark_nnport_input_statistics, resources::data_voromqa_dark_nnport_input_statistics(), true);
			}

			const std::string filename_for_voromqa_dark_nnport_input_fdeep_model_json=scripting::VirtualFileStorage::validate_filename("voromqa_dark_nnport_input_fdeep_model_json");
			if(!scripting::VirtualFileStorage::file_exists(filename_for_voromqa_dark_nnport_input_fdeep_model_json))
			{
				voronota::scripting::VirtualFileStorage::set_file(filename_for_voromqa_dark_nnport_input_fdeep_model_json, resources::data_voromqa_dark_nnport_input_fdeep_model_json(), true);
			}

			operators::NNPortPredict().init(CMDIN()
					.set("input-value-column-names-file", filename_for_voromqa_dark_nnport_input_header)
					.set("input-statistics-file", filename_for_voromqa_dark_nnport_input_statistics)
					.set("input-model-files", filename_for_voromqa_dark_nnport_input_fdeep_model_json)
					.set("input-data-file", tmp_profile.filename())
					.set("output-data-file", tmp_scores.filename())
					.add("output-value-column-names", "vd1")
					.add("output-value-column-names", "vd2")
					.add("output-value-column-names", "vd3")
					.add("output-value-column-names", "vd4")
					.add("output-value-column-names", "vd5")
					.add("output-value-column-names", "vd6")).run(0);
		}

		scripting::operators::ImportAdjunctsOfAtoms().init(CMDIN().set("file", tmp_scores.filename())).run(data_manager);

		Result result;
		{
			double sum_of_values=0.0;
			int number_of_residues=0;
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				const scripting::Atom& atom=data_manager.atoms()[i];
				if(atom.crad.name=="CA")
				{
					std::map<std::string, double>::const_iterator it=atom.value.props.adjuncts.find("vd1");
					if(it!=atom.value.props.adjuncts.end())
					{
						sum_of_values+=it->second;
						number_of_residues++;
					}
				}
			}
			result.global_score=(sum_of_values/static_cast<double>(number_of_residues));
			result.number_of_residues=number_of_residues;
		}

		if(!global_adj_prefix.empty())
		{
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_quality_score"]=result.global_score;
		}

		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_VOROMQA_DARK_GLOBAL_H_ */
