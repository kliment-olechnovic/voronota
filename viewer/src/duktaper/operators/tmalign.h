#ifndef DUKTAPER_OPERATORS_TMALIGN_H_
#define DUKTAPER_OPERATORS_TMALIGN_H_

#include "../../dependencies/tmalign/tmalign_wrapper.h"

#include "../../../../src/scripting/operators_all.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class TMalign : public scripting::operators::OperatorBase<TMalign>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		int tmalign_exit_code;
		std::string tmalign_stdout;
		std::string tmalign_stderr;
		std::string tmalign_matrix_file;
		double tmalign_score;

		Result() : tmalign_exit_code(0), tmalign_score(0.0)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("tmalign_exit_code")=tmalign_exit_code;
			heterostorage.variant_object.value("tmalign_stdout")=tmalign_stdout;
			heterostorage.variant_object.value("tmalign_stderr")=tmalign_stderr;
			heterostorage.variant_object.value("tmalign_matrix_file")=tmalign_matrix_file;
			heterostorage.variant_object.value("tmalign_score")=tmalign_score;
		}
	};

	std::string target_name;
	std::string model_name;
	std::string target_selection;
	std::string model_selection;

	TMalign()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		target_name=input.get_value_or_first_unused_unnamed_value("target");
		model_name=input.get_value_or_first_unused_unnamed_value("model");
		target_selection=input.get_value_or_default<std::string>("target-sel", "");
		model_selection=input.get_value_or_default<std::string>("model-sel", "");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(scripting::CDOD("target", scripting::CDOD::DATATYPE_STRING, "name of target object"));
		doc.set_option_decription(scripting::CDOD("model", scripting::CDOD::DATATYPE_STRING, "name of model object"));
		doc.set_option_decription(scripting::CDOD("target-sel", scripting::CDOD::DATATYPE_STRING, "selection of atoms for target object", ""));
		doc.set_option_decription(scripting::CDOD("model-sel", scripting::CDOD::DATATYPE_STRING, "selection of atoms for model object", ""));
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

		const std::string target_use=target_selection.empty() ? std::string("[-aname CA]") : (std::string("((")+target_selection+") and ([-aname CA]))");
		const std::string model_use=model_selection.empty() ? std::string("[-aname CA]") : (std::string("((")+model_selection+") and ([-aname CA]))");

		scripting::VirtualFileStorage::TemporaryFile tmp_target_pdb;
		scripting::VirtualFileStorage::TemporaryFile tmp_model_pdb;
		scripting::VirtualFileStorage::TemporaryFile tmp_matrix;

		{
			std::ostringstream args;
			args << " -use " << target_use << " -as-pdb -file " << tmp_target_pdb.filename();
			scripting::operators::ExportAtoms().init(args.str()).run(*target_object);
		}

		{
			std::ostringstream args;
			args << " -use " << model_use << " -as-pdb -file " << tmp_model_pdb.filename();
			scripting::operators::ExportAtoms().init(args.str()).run(*model_object);
		}

		const TMAlignWrapper::ResultBundle tmalign_result=TMAlignWrapper().run_tmalign(tmp_target_pdb.filename(), tmp_model_pdb.filename(), tmp_matrix.filename());

		Result result;
		result.tmalign_exit_code=tmalign_result.exit_code;
		result.tmalign_stdout=tmalign_result.stdout_str;
		result.tmalign_stderr=tmalign_result.stderr_str;
		result.tmalign_matrix_file=scripting::VirtualFileStorage::get_file(tmp_matrix.filename());

		if(!result.tmalign_matrix_file.empty())
		{
			std::ostringstream translation_output;
			std::ostringstream rotation_output;
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
								translation_output << " " << token;
							}
							else if(j>1 && j<5)
							{
								rotation_output << " " << token;
							}
						}
					}
				}
			}
			{
				std::ostringstream args;
				args << "-rotate-by-matrix " << rotation_output.str() << " -translate " << translation_output.str();
				scripting::operators::MoveAtoms().init(args.str()).run(*model_object);
			}
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
