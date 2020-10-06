#ifndef DUKTAPER_OPERATORS_RUN_NOLB_H_
#define DUKTAPER_OPERATORS_RUN_NOLB_H_

#include "../../../../src/scripting/operators/import_many.h"

#include "../call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class RunNolb : public scripting::OperatorBase<RunNolb>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::string nolb_stdout;
		std::string nolb_stderr;
		scripting::operators::ImportMany::Result import_result;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("nolb_stdout")=nolb_stdout;
			heterostorage.variant_object.value("nolb_stderr")=nolb_stderr;
			import_result.store(heterostorage);
		}
	};

	scripting::CongregationOfDataManagers::ObjectQuery query;
	int mode_number;
	int frames;
	bool minimize;

	RunNolb() : mode_number(1), frames(10), minimize(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		query=scripting::OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		mode_number=input.get_value_or_default<int>("mode-number", 1);
		frames=input.get_value_or_default<int>("frames", 10);
		minimize=input.get_flag("minimize");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		scripting::OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		doc.set_option_decription(CDOD("mode-number", CDOD::DATATYPE_INT, "mode number", 1));
		doc.set_option_decription(CDOD("frames", CDOD::DATATYPE_INT, "number of output frames", 10));
		doc.set_option_decription(CDOD("minimize", CDOD::DATATYPE_BOOL, "flag to minimize"));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		const std::vector<scripting::DataManager*> objects=congregation_of_data_managers.get_objects(query);

		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		if(objects.size()>1)
		{
			throw std::runtime_error(std::string("More than one object selected."));
		}

		if(!CallShellUtilities::test_if_shell_command_available("NOLB"))
		{
			throw std::runtime_error(std::string("NOLB application not available."));
		}

		Result result;

		for(std::size_t i=0;i<objects.size();i++)
		{
			scripting::DataManager* object=objects[i];
			const std::string object_name=congregation_of_data_managers.get_object_attributes(object).name;

			CallShellUtilities::TemporaryDirectory tmpdir;

			scripting::operators::ExportAtoms().init(CMDIN()
					.set("as-pdb", true)
					.set("file", (tmpdir.dir_path()+"/"+object_name))).run(*object);

			{
				std::ostringstream command_output;
				command_output << "cd '" << tmpdir.dir_path() << "'\n";
				command_output << "NOLB " << object_name << " -n " << mode_number << " -s " << frames;
				if(minimize)
				{
					command_output << " -m";
				}
				command_output << "\n";
				operators::CallShell::Result nolb_result=operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0);
				if(nolb_result.exit_status!=0)
				{
					throw std::runtime_error(std::string("NOLB call failed."));
				}
				result.nolb_stdout=nolb_result.stdout_str;
				result.nolb_stderr=nolb_result.stderr_str;
			}

			{
				std::string object_name_without_suffix=object_name;
				object_name_without_suffix=scripting::OperatorsUtilities::remove_suffix(object_name_without_suffix, ".pdb");
				object_name_without_suffix=scripting::OperatorsUtilities::remove_suffix(object_name_without_suffix, ".PDB");

				std::ostringstream result_file_path;
				result_file_path << tmpdir.dir_path() << "/" << object_name_without_suffix << "_nlb_" << mode_number << ".pdb";

				result.import_result=scripting::operators::ImportMany().init(CMDIN()
						.set("files", result_file_path.str())
						.set("format", "pdb")
						.set("split-pdb-files", true)
						.set("include-heteroatoms", true)).run(congregation_of_data_managers);
			}
		}

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_RUN_NOLB_H_ */
