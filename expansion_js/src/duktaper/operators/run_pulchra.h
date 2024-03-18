#ifndef DUKTAPER_OPERATORS_RUN_PULCHRA_H_
#define DUKTAPER_OPERATORS_RUN_PULCHRA_H_

#include "../../../../src/scripting/operators/export_atoms.h"
#include "../../../../src/scripting/operators/import.h"

#include "../call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class RunPULCHRA : public scripting::OperatorBase<RunPULCHRA>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		scripting::operators::Import::Result import_result;

		Result()
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			import_result.store(heterostorage);
		}
	};

	RunPULCHRA()
	{
	}

	void initialize(scripting::CommandInput&)
	{
	}

	void document(scripting::CommandDocumentation&) const
	{
	}

	Result run(scripting::DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(!CallShellUtilities::test_if_shell_command_available("pulchra"))
		{
			throw std::runtime_error(std::string("pulchra application not available."));
		}

		const CallShellUtilities::TemporaryDirectory tmpdir;

		scripting::operators::ExportAtoms().init(CMDIN()
				.set("as-pdb", true)
				.set("file", (tmpdir.dir_path()+"/struct.pdb"))).run(data_manager);

		{
			std::ostringstream command_output;
			command_output << "cd '" << tmpdir.dir_path() << "'\n";
			command_output << "pulchra ./struct.pdb";
			operators::CallShell::Result pulchra_result=operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0);
			if(pulchra_result.exit_status!=0)
			{
				throw std::runtime_error(std::string("pulchra call failed."));
			}
		}

		Result result;

		result.import_result=scripting::operators::Import().init(CMDIN()
				.set("file", (tmpdir.dir_path()+"/struct.rebuilt.pdb"))
				.set("format", "pdb")).run(data_manager);

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_RUN_PULCHRA_H_ */
