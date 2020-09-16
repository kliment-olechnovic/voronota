#ifndef DUKTAPER_OPERATORS_SCWRL_H_
#define DUKTAPER_OPERATORS_SCWRL_H_

#include "../../../../src/scripting/operators_all.h"

#include "../call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class Scwrl : public scripting::OperatorBase<Scwrl>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	Scwrl()
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

		if(!CallShellUtilities::test_if_shell_command_available("Scwrl4"))
		{
			throw std::runtime_error(std::string("Scwrl4 application not available."));
		}

		CallShellUtilities::TemporaryDirectory tmpdir;

		scripting::operators::ExportAtoms().init(CMDIN()
				.set("use", "[--aname CA,C,N,O,OXT]")
				.set("as-pdb", true)
				.set("file", tmpdir.dir_path()+"/backbone.pdb")).run(data_manager);

		{
			std::ostringstream command_output;
			command_output << "Scwrl4 -h -i '" << tmpdir.dir_path() << "/backbone.pdb'" << " -o '" << tmpdir.dir_path() << "/rebuilt.pdb'";
			operators::CallShell::Result scwrl_result=operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0);
			if(scwrl_result.exit_status!=0)
			{
				throw std::runtime_error(std::string("Scwrl4 call failed."));
			}
		}

		if(!CallShellUtilities::test_if_file_not_empty(tmpdir.dir_path()+"/rebuilt.pdb"))
		{
			throw std::runtime_error(std::string("Scwrl4 call did not produce output."));
		}

		scripting::operators::Import().init(CMDIN().set("file", tmpdir.dir_path()+"/rebuilt.pdb")).run(data_manager);

		Result result;

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_SCWRL_H_ */
