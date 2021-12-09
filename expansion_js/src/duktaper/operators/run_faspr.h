#ifndef DUKTAPER_OPERATORS_RUN_FASPR_H_
#define DUKTAPER_OPERATORS_RUN_FASPR_H_

#include "../../../../src/scripting/operators/import.h"

#include "../call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class RunFASPR : public scripting::OperatorBase<RunFASPR>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	RunFASPR()
	{
	}

	std::string faspr_binary;

	void initialize(scripting::CommandInput& input)
	{
		faspr_binary=input.get_value<std::string>("faspr-binary");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("faspr-binary", CDOD::DATATYPE_STRING, "path to FASPR binary executable"));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(!CallShellUtilities::test_if_shell_command_available(faspr_binary))
		{
			throw std::runtime_error(std::string("FASPR application not available."));
		}

		CallShellUtilities::TemporaryDirectory tmpdir;

		scripting::operators::ExportAtoms().init(CMDIN()
				.set("use", "[--aname CA,C,N,O,OXT]")
				.set("as-pdb", true)
				.set("file", tmpdir.dir_path()+"/backbone.pdb")).run(data_manager);

		{
			std::ostringstream command_output;
			command_output << faspr_binary << " -i '" << tmpdir.dir_path() << "/backbone.pdb'" << " -o '" << tmpdir.dir_path() << "/rebuilt.pdb'";
			operators::CallShell::Result faspr_result=operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0);
			if(faspr_result.exit_status!=0)
			{
				throw std::runtime_error(std::string("FASPR call failed."));
			}
		}

		if(!CallShellUtilities::test_if_file_not_empty(tmpdir.dir_path()+"/rebuilt.pdb"))
		{
			throw std::runtime_error(std::string("FASPR call did not produce output."));
		}

		scripting::operators::Import().init(CMDIN().set("file", tmpdir.dir_path()+"/rebuilt.pdb")).run(data_manager);

		Result result;

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_RUN_FASPR_H_ */
