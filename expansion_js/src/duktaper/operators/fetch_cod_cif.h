#ifndef DUKTAPER_OPERATORS_FETCH_COD_CIF_H_
#define DUKTAPER_OPERATORS_FETCH_COD_CIF_H_

#include "../../../../src/scripting/operators/import.h"

#include "../call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class FetchCODCIF : public scripting::OperatorBase<FetchCODCIF>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		scripting::operators::Import::Result import_result;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			import_result.store(heterostorage);
		}
	};

	std::string cod_id;
	bool same_radius_for_all_provided;
	double same_radius_for_all;
	bool include_hydrogens;

	FetchCODCIF() : same_radius_for_all_provided(false), same_radius_for_all(1.0), include_hydrogens(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		cod_id=input.get_value_or_first_unused_unnamed_value("cod-id");
		same_radius_for_all_provided=input.is_option("same-radius-for-all");
		same_radius_for_all=input.get_value_or_default<double>("same-radius-for-all", 1.0);
		include_hydrogens=input.get_flag("include-hydrogens");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("cod-id", CDOD::DATATYPE_STRING, "COD ID"));
		doc.set_option_decription(CDOD("same-radius-for-all", CDOD::DATATYPE_FLOAT, "radius to use for all atoms", ""));
		doc.set_option_decription(CDOD("include-hydrogens", CDOD::DATATYPE_BOOL, "flag to include hydrogens"));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(cod_id.empty())
		{
			throw std::runtime_error(std::string("Invalid COD ID '")+cod_id+"'.");
		}

		if(!CallShellUtilities::test_if_shell_command_available("curl"))
		{
			throw std::runtime_error(std::string("'curl' command not available."));
		}

		if(!CallShellUtilities::test_if_shell_command_available("cif_fillcell"))
		{
			throw std::runtime_error(std::string("'cif_fillcell' command not available."));
		}

		if(!CallShellUtilities::test_if_shell_command_available("cif2xyz"))
		{
			throw std::runtime_error(std::string("'cif2xyz' command not available."));
		}

		scripting::VirtualFileStorage::TemporaryFile tmpfile;

		{
			std::ostringstream command_output;
			command_output << "curl --silent 'https://www.crystallography.net/cod/" << cod_id << ".cif' | cif_fillcell --supercell | cif2xyz --print-radii";
			operators::CallShell::Result download_result=operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0);
			if(download_result.exit_status!=0 || download_result.stdout_str.empty())
			{
				throw std::runtime_error(std::string("No data read, stderr output: '")+download_result.stderr_str+"'.");
			}
			else
			{
				scripting::VirtualFileStorage::set_file(tmpfile.filename(), download_result.stdout_str);
			}
		}

		scripting::CommandInput import_operator_params;
		import_operator_params.set("file", tmpfile.filename());
		import_operator_params.set("format", "xyzr");
		import_operator_params.set("title", cod_id);
		import_operator_params.set("include-hydrogens", include_hydrogens);
		if(same_radius_for_all_provided)
		{
			import_operator_params.set("same-radius-for-all", same_radius_for_all);
		}

		Result result;

		result.import_result=scripting::operators::Import().init(import_operator_params).run(congregation_of_data_managers);

		scripting::DataManager* new_object=congregation_of_data_managers.get_object(result.import_result.object_name);

		if(new_object==0)
		{
			throw std::runtime_error(std::string("Failed to add new data object."));
		}

		scripting::operators::SetAdjunctOfAtomsByExpression().init(CMDIN()
				.set("expression", "_modulo")
				.set("input-adjuncts", "resSeq")
				.set("parameters", 27)
				.set("output-adjunct", "cifcell")).run(*new_object);

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_FETCH_COD_CIF_H_ */
