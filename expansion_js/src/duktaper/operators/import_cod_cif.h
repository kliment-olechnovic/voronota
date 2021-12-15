#ifndef DUKTAPER_OPERATORS_IMPORT_COD_CIF_H_
#define DUKTAPER_OPERATORS_IMPORT_COD_CIF_H_

#include "../../../../src/scripting/operators/import.h"

#include "../call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class ImportCODCIF : public scripting::OperatorBase<ImportCODCIF>
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

	std::string file;
	std::string fetch_cod_id;
	std::string title;
	bool same_radius_for_all_provided;
	double same_radius_for_all;
	bool include_hydrogens;

	ImportCODCIF() : same_radius_for_all_provided(false), same_radius_for_all(1.0), include_hydrogens(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		file=input.get_value_or_default<std::string>("file", "");
		fetch_cod_id=input.get_value_or_default<std::string>("fetch-cod-id", "");
		same_radius_for_all_provided=input.is_option("same-radius-for-all");
		same_radius_for_all=input.get_value_or_default<double>("same-radius-for-all", 1.0);
		include_hydrogens=input.get_flag("include-hydrogens");
		title=(input.is_option("title") ? input.get_value<std::string>("title") : (!fetch_cod_id.empty() ? fetch_cod_id : scripting::OperatorsUtilities::get_basename_from_path(file)));
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "input COD CIF file path", ""));
		doc.set_option_decription(CDOD("fetch-cod-id", CDOD::DATATYPE_STRING, "COD ID to fetch from crystallography.net", ""));
		doc.set_option_decription(CDOD("same-radius-for-all", CDOD::DATATYPE_FLOAT, "radius to use for all atoms", ""));
		doc.set_option_decription(CDOD("include-hydrogens", CDOD::DATATYPE_BOOL, "flag to include hydrogens"));
		doc.set_option_decription(CDOD("title", CDOD::DATATYPE_STRING, "new object title", ""));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(file.empty() && fetch_cod_id.empty())
		{
			throw std::runtime_error(std::string("Neither local file nor COD ID for fetching provided."));
		}

		if(!file.empty() && !fetch_cod_id.empty())
		{
			throw std::runtime_error(std::string("Both local file and COD ID fetching specified, only one should be provided."));
		}

		if(!fetch_cod_id.empty() && !CallShellUtilities::test_if_shell_command_available("curl"))
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
			if(!file.empty())
			{
				command_output << "cat '" << file << "'";
			}
			else if(!fetch_cod_id.empty())
			{
				command_output << "curl --silent 'https://www.crystallography.net/cod/" << fetch_cod_id << ".cif'";
			}
			command_output << " | cif_fillcell --supercell --merge-special-positions | cif2xyz --print-radii";
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
		import_operator_params.set("title", title);
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

		scripting::DataManager& data_manager=(*new_object);

		if(data_manager.atoms().size()%27!=0)
		{
			congregation_of_data_managers.delete_object(new_object);
			throw std::runtime_error(std::string("Number of imported atoms is not divisible by 27."));
		}

		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			const scripting::Atom& atom=data_manager.atoms()[i];
			std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(i);
			atom_adjuncts["cifcell"]=static_cast<double>(atom.crad.resSeq%27);
			atom_adjuncts["cifuid"]=static_cast<double>(atom.crad.resSeq/27);
		}

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_IMPORT_COD_CIF_H_ */
