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

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("tmalign_exit_code")=tmalign_exit_code;
			heterostorage.variant_object.value("tmalign_stdout")=tmalign_stdout;
			heterostorage.variant_object.value("tmalign_stderr")=tmalign_stderr;
			heterostorage.variant_object.value("tmalign_matrix_file")=tmalign_matrix_file;
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
		doc.set_option_decription(scripting::CDOD("target-sel", scripting::CDOD::DATATYPE_STRING, "selection of atoms for target object"));
		doc.set_option_decription(scripting::CDOD("model-sel", scripting::CDOD::DATATYPE_STRING, "selection of atoms for model object"));
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
			args << "-on-objects " << target_name << " -use " << target_use << " -as-pdb -file " << tmp_target_pdb.filename();
			scripting::operators::ExportAtoms().init(args.str()).run(*target_object);
		}

		{
			std::ostringstream args;
			args << "-on-objects " << model_name << " -use " << model_use << " -as-pdb -file " << tmp_model_pdb.filename();
			scripting::operators::ExportAtoms().init(args.str()).run(*model_object);
		}

		const TMAlignWrapper::ResultBundle tmalign_result=TMAlignWrapper().run_tmalign(tmp_target_pdb.filename(), tmp_model_pdb.filename(), tmp_matrix.filename());

		Result result;
		result.tmalign_exit_code=tmalign_result.exit_code;
		result.tmalign_stdout=tmalign_result.stdout;
		result.tmalign_stderr=tmalign_result.stderr;
		result.tmalign_matrix_file=scripting::VirtualFileStorage::get_file(tmp_matrix.filename());

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_TMALIGN_H_ */
