#ifndef DUKTAPER_OPERATORS_FETCH_H_
#define DUKTAPER_OPERATORS_FETCH_H_

#include "../../../../src/scripting/operators_all.h"

#include "../call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class Fetch : public scripting::operators::OperatorBase<Fetch>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		scripting::SummaryOfAtoms atoms_summary;
		std::string object_name;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			scripting::VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.summaries_of_atoms["loaded"]=atoms_summary;
			heterostorage.variant_object.value("object_name")=object_name;
		}
	};

	std::string pdb_id;
	int assembly;
	bool no_heteroatoms;

	Fetch() : assembly(1), no_heteroatoms(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		pdb_id=input.get_value_or_first_unused_unnamed_value("pdb-id");
		assembly=input.get_value_or_default<int>("assembly", 1);
		no_heteroatoms=input.get_flag("no-heteroatoms");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("pdb-id", CDOD::DATATYPE_STRING, "PDB ID"));
		doc.set_option_decription(CDOD("assembly", CDOD::DATATYPE_INT, "assembly number, 0 for asymmetric unit", "1"));
		doc.set_option_decription(CDOD("no-heteroatoms", CDOD::DATATYPE_BOOL, "flag to not include heteroatoms"));
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

		if(!CallShellUtilities::test_if_shell_command_available("curl"))
		{
			throw std::runtime_error(std::string("'curl' command not available."));
		}

		if(!CallShellUtilities::test_if_shell_command_available("zcat"))
		{
			throw std::runtime_error(std::string("'zcat' command not available."));
		}

		scripting::VirtualFileStorage::TemporaryFile tmpfile;

		{
			std::ostringstream command_output;
			if(assembly==0)
			{
				command_output << "curl 'https://files.rcsb.org/download/" << pdb_id << ".pdb.gz' | zcat";
			}
			else
			{
				command_output << "curl 'https://files.rcsb.org/download/" << pdb_id << ".pdb" << assembly << ".gz' | zcat";
			}
			operators::CallShell::Result download_result=operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0);
			if(download_result.exit_status!=0 || download_result.stdout.empty())
			{
				throw std::runtime_error(std::string("No data downloaded."));
			}
			scripting::VirtualFileStorage::set_file(tmpfile.filename(), download_result.stdout);
		}

		std::ostringstream title_output;
		title_output << pdb_id;
		if(assembly!=0)
		{
			title_output << "_as_" << assembly;
		}

		scripting::operators::Import::Result import_result=scripting::operators::Import().init(CMDIN()
				.set("file", tmpfile.filename())
				.set("format", "pdb")
				.set("as-assembly", (assembly!=0))
				.set("include-heteroatoms", !no_heteroatoms)
				.set("title", title_output.str())).run(congregation_of_data_managers);

		Result result;
		result.atoms_summary=import_result.atoms_summary;
		result.object_name=import_result.object_name;

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_FETCH_H_ */
