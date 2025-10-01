#ifndef DUKTAPER_OPERATORS_FETCH_MMCIF_H_
#define DUKTAPER_OPERATORS_FETCH_MMCIF_H_

#include "import_mmcif.h"

#include "../remote_import_downloader.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class FetchMMCIF : public scripting::OperatorBase<FetchMMCIF>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		bool asynchronous;
		std::vector<std::string> pdb_ids;
		std::vector<std::string> urls;
		ImportMMCIF::Result import_result;

		Result() : asynchronous(false)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("asynchronous")=asynchronous;
			std::vector<scripting::VariantValue>& pdb_ids_array=heterostorage.variant_object.values_array("pdb_ids");
			for(std::size_t i=0;i<pdb_ids.size();i++)
			{
				pdb_ids_array.push_back(scripting::VariantValue(pdb_ids[i]));
			}
			std::vector<scripting::VariantValue>& urls_array=heterostorage.variant_object.values_array("urls");
			for(std::size_t i=0;i<urls.size();i++)
			{
				urls_array.push_back(scripting::VariantValue(urls[i]));
			}
			if(!asynchronous)
			{
				import_result.store(heterostorage);
			}
		}
	};

	typedef RemoteImportRequest<ImportMMCIF> RemoteImportRequestType;
	typedef RemoteImportDownloader<RemoteImportRequestType> RemoteImportDownloaderType;

	RemoteImportDownloaderType* downloader_ptr;
	std::vector<std::string> pdb_ids;
	bool assembly_provided;
	int assembly;
	bool no_heteroatoms;
	bool all_states;
	bool use_label_ids;

	FetchMMCIF(RemoteImportDownloaderType& downloader) :
		downloader_ptr(&downloader),
		assembly_provided(false),
		assembly(1),
		no_heteroatoms(false),
		all_states(false),
		use_label_ids(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		pdb_ids=input.get_value_vector_or_all_unused_unnamed_values("pdb-id");
		assembly_provided=input.is_option("assembly");
		assembly=input.get_value_or_default<int>("assembly", 1);
		no_heteroatoms=input.get_flag("no-heteroatoms");
		all_states=input.get_flag("all-states");
		use_label_ids=input.get_flag("use-label-ids");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("pdb-id", CDOD::DATATYPE_STRING_ARRAY, "PDB ID, or multiple PDB IDs"));
		doc.set_option_decription(CDOD("assembly", CDOD::DATATYPE_INT, "assembly number, 0 for asymmetric unit", "1"));
		doc.set_option_decription(CDOD("no-heteroatoms", CDOD::DATATYPE_BOOL, "flag to not include heteroatoms"));
		doc.set_option_decription(CDOD("all-states", CDOD::DATATYPE_BOOL, "flag to import all NMR states"));
		doc.set_option_decription(CDOD("use-label-ids", CDOD::DATATYPE_BOOL, "flag to use label_ IDs instead of auth_ IDs"));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(downloader_ptr==0)
		{
			throw std::runtime_error(std::string("Missing downloader."));
		}

		if(pdb_ids.empty())
		{
			throw std::runtime_error(std::string("No PDB IDs provided."));
		}

		if(assembly<0)
		{
			throw std::runtime_error(std::string("Invalid assembly number."));
		}

		if(assembly_provided && assembly!=0 && all_states)
		{
			throw std::runtime_error(std::string("Incompatible usage of options 'assembly' and 'all-states' together."));
		}

		for(std::size_t j=0;j<pdb_ids.size();j++)
		{
			const std::string& pdb_id=pdb_ids[j];

			if(pdb_id.empty() || !((pdb_id.size()==4 && pdb_id[0]>='1' && pdb_id[0]<='9') || (pdb_id.size()==12 && pdb_id.rfind("pdb_", 0)==0)))
			{
				throw std::runtime_error(std::string("Invalid PDB ID '")+pdb_id+"'.");
			}

			if(pdb_id.size()==4)
			{
				for(std::size_t i=0;i<pdb_id.size();i++)
				{
					if(!((pdb_id[i]>='a' && pdb_id[i]<='z') || (pdb_id[i]>='A' && pdb_id[i]<='Z') || (pdb_id[i]>='0' && pdb_id[i]<='9')))
					{
						throw std::runtime_error(std::string("Invalid PDB ID '")+pdb_id+"'.");
					}
				}
			}
		}

		Result result;

		for(std::size_t j=0;j<pdb_ids.size();j++)
		{
			const std::string& pdb_id=pdb_ids[j];

			ImportMMCIF import_many_operator;
			import_many_operator.title=pdb_id;
			import_many_operator.max_models_per_file=(all_states ? 99999 : 1);
			import_many_operator.forced_include_heteroatoms=true;
			import_many_operator.include_heteroatoms=!no_heteroatoms;
			import_many_operator.use_label_ids=use_label_ids;

			RemoteImportDownloaderType& downloader=(*downloader_ptr);
			RemoteImportDownloaderType::ScopeCleaner scope_cleaner(downloader);

			std::ostringstream title_output;
			std::ostringstream url_output;

			title_output << pdb_id;

			if((assembly_provided && assembly!=0) || (!assembly_provided && !all_states))
			{
				url_output << "https://files.rcsb.org/download/" << pdb_id << "-assembly" << assembly << ".cif.gz";
				title_output << "_as_" << assembly;
			}
			else
			{
				if(all_states)
				{
					title_output << "_state_${model}";
				}
				url_output << "https://files.rcsb.org/download/" << pdb_id << ".cif.gz";
			}

			import_many_operator.title=title_output.str();

			RemoteImportRequestType& request=downloader.add_request_and_start_download(RemoteImportRequestType(url_output.str(), import_many_operator));

			result.asynchronous=!downloader.is_synchronous();
			result.pdb_ids.push_back(pdb_id);
			result.urls.push_back(request.url);

			if(downloader.is_synchronous())
			{
				if(!request.download_successful)
				{
					throw std::runtime_error(std::string("No data downloaded."));
				}

				ImportMMCIF::Result sub_import_result=request.import_downloaded_data(congregation_of_data_managers);
				result.import_result.atoms_summary.feed(sub_import_result.atoms_summary);
				result.import_result.object_names.insert(result.import_result.object_names.end(), sub_import_result.object_names.begin(), sub_import_result.object_names.end());
			}
		}

		return result;
	}

};

}

}

}

#endif /* DUKTAPER_OPERATORS_FETCH_MMCIF_H_ */

