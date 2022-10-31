#ifndef DUKTAPER_OPERATORS_FETCH_H_
#define DUKTAPER_OPERATORS_FETCH_H_

#include "../remote_import_downloader.h"

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
		bool asynchronous;
		std::string pdb_id;
		std::string url;
		scripting::operators::ImportMany::Result import_result;

		Result() : asynchronous(false)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("asynchronous")=asynchronous;
			heterostorage.variant_object.value("pdb_id")=pdb_id;
			heterostorage.variant_object.value("url")=url;
			if(!asynchronous)
			{
				import_result.store(heterostorage);
			}
		}
	};

	RemoteImportDownloader* downloader_ptr;
	std::string pdb_id;
	bool assembly_provided;
	int assembly;
	bool no_heteroatoms;
	bool all_states;

	Fetch(RemoteImportDownloader& downloader) : downloader_ptr(&downloader), assembly_provided(false), assembly(1), no_heteroatoms(false), all_states(false)
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
		if(downloader_ptr==0)
		{
			throw std::runtime_error(std::string("Missing downloader."));
		}

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

		if(assembly_provided && assembly!=0 && all_states)
		{
			throw std::runtime_error(std::string("Incompatible usage of options 'assembly' and 'all-states' together."));
		}

		scripting::operators::ImportMany import_many_operator;
		import_many_operator.import_operator.title=pdb_id;
		import_many_operator.split_pdb_files=all_states;
		import_many_operator.import_operator.loading_parameters.forced_include_heteroatoms=true;
		import_many_operator.import_operator.loading_parameters.include_heteroatoms=!no_heteroatoms;
		import_many_operator.import_operator.loading_parameters.forced_multimodel_chains=true;
		import_many_operator.import_operator.loading_parameters.multimodel_chains=(assembly!=0);
		import_many_operator.import_operator.loading_parameters.format="pdb";
		import_many_operator.import_operator.loading_parameters.format_fallback="pdb";

		RemoteImportDownloader& downloader=(*downloader_ptr);
		RemoteImportDownloader::ScopeCleaner scope_cleaner(downloader);

		std::ostringstream url_output;
		scripting::operators::ImportMany import_many_operator_to_use=import_many_operator;

		if((assembly_provided && assembly!=0) || (!assembly_provided && !all_states))
		{
			url_output << "https://files.rcsb.org/download/" << pdb_id << ".pdb" << assembly << ".gz";
			std::ostringstream title_output;
			title_output << pdb_id << "_as_" << assembly;
			import_many_operator_to_use.import_operator.title=title_output.str();
		}
		else
		{
			url_output << "https://files.rcsb.org/download/" << pdb_id << ".pdb.gz";
			import_many_operator_to_use.import_operator.loading_parameters.multimodel_chains=false;
		}

		RemoteImportRequest& request=downloader.add_request_and_start_download(RemoteImportRequest(url_output.str(), import_many_operator_to_use));

		Result result;
		result.asynchronous=!downloader.is_synchronous();
		result.pdb_id=pdb_id;
		result.url=request.url;

		if(downloader.is_synchronous())
		{
			if(!request.download_successful)
			{
				throw std::runtime_error(std::string("No data downloaded."));
			}

			result.import_result=request.import_downloaded_data(congregation_of_data_managers);
		}

		return result;
	}

};

}

}

}

#endif /* DUKTAPER_OPERATORS_FETCH_H_ */

