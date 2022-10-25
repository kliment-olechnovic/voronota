#ifndef DUKTAPER_OPERATORS_FETCH_H_
#define DUKTAPER_OPERATORS_FETCH_H_

#include "../../../../src/scripting/operators/import_many.h"

#include "../../dependencies/tinf/tinf_wrapper.h"

#include "../call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class Fetch : public scripting::OperatorBase<Fetch>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		scripting::operators::ImportMany::Result import_result;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			import_result.store(heterostorage);
		}
	};

	std::string pdb_id;
	bool assembly_provided;
	int assembly;
	bool no_heteroatoms;
	bool all_states;

	Fetch() : assembly_provided(false), assembly(1), no_heteroatoms(false), all_states(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		pdb_id=input.get_value_or_first_unused_unnamed_value("pdb-id");
		assembly_provided=input.is_option("assembly");
		assembly=input.get_value_or_default<int>("assembly", 1);
		no_heteroatoms=input.get_flag("no-heteroatoms");
		all_states=input.get_flag("all-states");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("pdb-id", CDOD::DATATYPE_STRING, "PDB ID"));
		doc.set_option_decription(CDOD("assembly", CDOD::DATATYPE_INT, "assembly number, 0 for asymmetric unit", "1"));
		doc.set_option_decription(CDOD("no-heteroatoms", CDOD::DATATYPE_BOOL, "flag to not include heteroatoms"));
		doc.set_option_decription(CDOD("all-states", CDOD::DATATYPE_BOOL, "flag to import all NMR states"));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(pdb_id.empty() || pdb_id.size()!=4 || pdb_id[0]<'1' || pdb_id[0]>'9')
		{
			throw std::runtime_error(std::string("Invalid PDB ID '")+pdb_id+"'.");
		}

		for(std::size_t i=0;i<pdb_id.size();i++)
		{
			if(!((pdb_id[i]>='a' && pdb_id[i]<='z') || (pdb_id[i]>='A' && pdb_id[i]<='Z') || (pdb_id[i]>='0' && pdb_id[i]<='9')))
			{
				throw std::runtime_error(std::string("Invalid PDB ID '")+pdb_id+"'.");
			}
		}

		if(assembly<0)
		{
			throw std::runtime_error(std::string("Invalid assembly number."));
		}

		if(assembly_provided && all_states)
		{
			throw std::runtime_error(std::string("Incompatible options 'assembly' and 'all-states' used together."));
		}

		if(!CallShellUtilities::test_if_shell_command_available("curl"))
		{
			throw std::runtime_error(std::string("'curl' command not available."));
		}

		scripting::VirtualFileStorage::TemporaryFile tmpfile;

		int used_assembly=assembly;

		{
			bool finished=false;
			bool downloaded=false;

			for(int stage=1;stage<=2 && !finished;stage++)
			{
				std::ostringstream command_output;
				if(used_assembly==0)
				{
					command_output << "curl 'https://files.rcsb.org/download/" << pdb_id << ".pdb.gz'";
				}
				else
				{
					command_output << "curl 'https://files.rcsb.org/download/" << pdb_id << ".pdb" << used_assembly << ".gz'";
				}
				operators::CallShell::Result download_result=operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0);
				if(download_result.exit_status!=0 || download_result.stdout_str.empty())
				{
					if(assembly_provided || used_assembly==0)
					{
						finished=true;
					}
					else
					{
						used_assembly=0;
					}
				}
				else
				{
					if(TinfWrapper::check_if_string_gzipped(download_result.stdout_str))
					{
						std::string uncompressed_data;
						if(!TinfWrapper::uncompress_gzipped_string(download_result.stdout_str, uncompressed_data))
						{
							throw std::runtime_error(std::string("Failed to uncompress downloaded file."));
						}
						scripting::VirtualFileStorage::set_file(tmpfile.filename(), uncompressed_data);
					}
					else
					{
						scripting::VirtualFileStorage::set_file(tmpfile.filename(), download_result.stdout_str);
					}
					finished=true;
					downloaded=true;
				}
			}

			if(!downloaded)
			{
				throw std::runtime_error(std::string("No data downloaded."));
			}
		}

		std::ostringstream title_output;
		title_output << pdb_id;
		if(used_assembly!=0)
		{
			title_output << "_as_" << used_assembly;
		}

		Result result;

		result.import_result=scripting::operators::ImportMany().init(CMDIN()
				.set("files", tmpfile.filename())
				.set("format", "pdb")
				.set("as-assembly", (used_assembly!=0))
				.set("split-pdb-files", all_states)
				.set("include-heteroatoms", !no_heteroatoms)
				.set("title", title_output.str())).run(congregation_of_data_managers);

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_FETCH_H_ */
