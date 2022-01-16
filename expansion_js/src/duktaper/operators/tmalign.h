#ifndef DUKTAPER_OPERATORS_TMALIGN_H_
#define DUKTAPER_OPERATORS_TMALIGN_H_

#include "../../dependencies/tmalign/tmalign_wrapper.h"

#include "../../../../src/scripting/operators/export_atoms.h"
#include "../../../../src/scripting/operators/move_atoms.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class TMalign : public scripting::OperatorBase<TMalign>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::string target_name;
		std::string model_name;
		int tmalign_exit_code;
		std::string tmalign_stdout;
		std::string tmalign_stderr;
		std::string tmalign_matrix_file;
		double tmalign_score;

		Result() : tmalign_exit_code(0), tmalign_score(0.0)
		{
		}

		void store(scripting::VariantObject& variant_object) const
		{
			variant_object.value("target_name")=target_name;
			variant_object.value("model_name")=model_name;
			variant_object.value("tmalign_exit_code")=tmalign_exit_code;
			variant_object.value("tmalign_stdout")=tmalign_stdout;
			variant_object.value("tmalign_stderr")=tmalign_stderr;
			variant_object.value("tmalign_matrix_file")=tmalign_matrix_file;
			variant_object.value("tmalign_score")=tmalign_score;
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			store(heterostorage.variant_object);
		}
	};

	std::string target_name;
	std::string model_name;
	std::string target_selection;
	std::string model_selection;

	TMalign()
	{
	}

	void initialize(scripting::CommandInput& input, const bool managed)
	{
		if(managed)
		{
			target_name=input.get_value_or_first_unused_unnamed_value_or_default("target", "");
		}
		else
		{
			target_name=input.get_value_or_first_unused_unnamed_value("target");
			model_name=input.get_value_or_first_unused_unnamed_value("model");
		}
		target_selection=input.get_value_or_default<std::string>("target-sel", "");
		model_selection=input.get_value_or_default<std::string>("model-sel", "");
	}

	void initialize(scripting::CommandInput& input)
	{
		initialize(input, false);
	}

	void document(scripting::CommandDocumentation& doc, const bool managed) const
	{
		if(managed)
		{
			doc.set_option_decription(CDOD("target", CDOD::DATATYPE_STRING, "name of target object", ""));
		}
		else
		{
			doc.set_option_decription(CDOD("target", CDOD::DATATYPE_STRING, "name of target object"));
			doc.set_option_decription(CDOD("model", CDOD::DATATYPE_STRING, "name of model object"));
		}
		doc.set_option_decription(CDOD("target-sel", CDOD::DATATYPE_STRING, "selection of atoms for target object", ""));
		doc.set_option_decription(CDOD("model-sel", CDOD::DATATYPE_STRING, "selection of atoms for model object", ""));
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		document(doc, false);
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(target_name.empty())
		{
			throw std::runtime_error(std::string("No target object name provided."));
		}

		if(model_name.empty())
		{
			throw std::runtime_error(std::string("No model object name provided."));
		}

		if(target_name==model_name)
		{
			throw std::runtime_error(std::string("Equal object names provided."));
		}

		congregation_of_data_managers.assert_object_availability(target_name);
		congregation_of_data_managers.assert_object_availability(model_name);

		scripting::DataManager* target_object=congregation_of_data_managers.get_object(target_name);
		scripting::DataManager* model_object=congregation_of_data_managers.get_object(model_name);

		const std::string target_use=target_selection.empty() ? std::string("([--aname CA] or [--aname C3'])") : (std::string("((")+target_selection+") and ([--aname CA] or [--aname C3']))");
		const std::string model_use=model_selection.empty() ? std::string("([--aname CA] or [--aname C3'])") : (std::string("((")+model_selection+") and ([--aname CA] or [--aname C3']))");

		scripting::VirtualFileStorage::TemporaryFile tmp_target_pdb;
		scripting::VirtualFileStorage::TemporaryFile tmp_model_pdb;
		scripting::VirtualFileStorage::TemporaryFile tmp_matrix;

		scripting::operators::ExportAtoms().init(CMDIN()
				.set("use", target_use)
				.set("as-pdb", true)
				.set("for-tmalign", true)
				.set("file", tmp_target_pdb.filename())).run(*target_object);

		scripting::operators::ExportAtoms().init(CMDIN()
				.set("use", model_use)
				.set("as-pdb", true)
				.set("for-tmalign", true)
				.set("file", tmp_model_pdb.filename())).run(*model_object);

		const TMAlignWrapper::ResultBundle tmalign_result=TMAlignWrapper().run_tmalign(tmp_target_pdb.filename(), tmp_model_pdb.filename(), tmp_matrix.filename());

		Result result;
		result.target_name=target_name;
		result.model_name=model_name;
		result.tmalign_exit_code=tmalign_result.exit_code;
		result.tmalign_stdout=tmalign_result.stdout_str;
		result.tmalign_stderr=tmalign_result.stderr_str;
		result.tmalign_matrix_file=scripting::VirtualFileStorage::get_file(tmp_matrix.filename());

		if(!result.tmalign_matrix_file.empty())
		{
			std::vector<std::string> translation_output;
			std::vector<std::string> rotation_output;
			{
				std::istringstream file_input(result.tmalign_matrix_file);
				for(int i=0;i<5 && file_input.good();i++)
				{
					std::string line;
					std::getline(file_input, line);
					if(i>1)
					{
						std::istringstream line_input(line);
						for(int j=0;j<5 && line_input.good();j++)
						{
							std::string token;
							line_input >> token;
							if(j==1)
							{
								translation_output.push_back(token);
							}
							else if(j>1 && j<5)
							{
								rotation_output.push_back(token);
							}
						}
					}
				}
			}
			scripting::operators::MoveAtoms().init(CMDIN()
					.setv("rotate-by-matrix", rotation_output)
					.setv("translate", translation_output)).run(*model_object);
		}

		if(!result.tmalign_stdout.empty())
		{
			const std::string score_line_start="TM-score=";
			const std::string score_line_middle="(if normalized by length of Chain_2";
			std::istringstream file_input(result.tmalign_stdout);
			bool finish=false;
			while(file_input.good() && !finish)
			{
				std::string line;
				std::getline(file_input, line);
				if(line.find(score_line_start)==0)
				{
					const std::size_t middle_pos=line.find(score_line_middle);
					if(middle_pos!=std::string::npos)
					{
						std::istringstream score_input(line.substr(score_line_start.size(), score_line_middle.size()-score_line_start.size()));
						score_input >> result.tmalign_score;
						finish=true;
					}
				}
			}
		}

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_TMALIGN_H_ */
