#ifndef DUKTAPER_OPERATORS_RUN_BFF_H_
#define DUKTAPER_OPERATORS_RUN_BFF_H_

#include "../../../../src/scripting/operators/export_contacts_as_connected_mesh.h"

#include "convert_bff_obj_to_svg.h"

#include "../call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class RunBFF : public scripting::OperatorBase<RunBFF>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::string bff_stdout;
		std::string bff_stderr;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("bff_stdout")=bff_stdout;
			heterostorage.variant_object.value("bff_stderr")=bff_stderr;
		}
	};

	scripting::operators::ExportContactsAsConnectedMesh export_contacts_as_connected_mesh_operator;
	ConvertBFFObjToSVG convert_bff_obj_to_svg;

	RunBFF()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		export_contacts_as_connected_mesh_operator.initialize(input, true);
		convert_bff_obj_to_svg.initialize(input, true);
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		export_contacts_as_connected_mesh_operator.document(doc, true);
		convert_bff_obj_to_svg.document(doc, true);
	}

	Result run(scripting::DataManager& data_manager) const
	{
		if(!CallShellUtilities::test_if_shell_command_available("bff-command-line"))
		{
			throw std::runtime_error(std::string("bff-command-line application not available."));
		}

		const CallShellUtilities::TemporaryDirectory tmpdir;
		const std::string rfs_output_obj_file=tmpdir.dir_path()+"/mesh.obj";
		const std::string rfs_output_bff_obj_file=tmpdir.dir_path()+"/bffmesh.obj";

		const scripting::VirtualFileStorage::TemporaryFile vfs_output_color_file;

		scripting::operators::ExportContactsAsConnectedMesh export_contacts_as_connected_mesh_operator_to_use=export_contacts_as_connected_mesh_operator;
		export_contacts_as_connected_mesh_operator_to_use.output_obj_file=rfs_output_obj_file;
		export_contacts_as_connected_mesh_operator_to_use.output_color_file=vfs_output_color_file.filename();
		export_contacts_as_connected_mesh_operator_to_use.run(data_manager);

		Result result;

		{
			std::ostringstream command_output;
			command_output << "bff-command-line " << rfs_output_obj_file << " " << rfs_output_bff_obj_file;
			operators::CallShell::Result bff_result=operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0);
			if(bff_result.exit_status!=0)
			{
				throw std::runtime_error(std::string("bff-command-line call failed."));
			}
			result.bff_stdout=bff_result.stdout_str;
			result.bff_stderr=bff_result.stderr_str;
		}

		ConvertBFFObjToSVG convert_bff_obj_to_svg_to_use=convert_bff_obj_to_svg;
		convert_bff_obj_to_svg_to_use.input_obj_file=rfs_output_bff_obj_file;
		convert_bff_obj_to_svg_to_use.input_colors_file=vfs_output_color_file.filename();
		convert_bff_obj_to_svg_to_use.run(0);

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_RUN_BFF_H_ */
