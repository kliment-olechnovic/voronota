#ifndef DUKTAPER_OPERATORS_FETCH_AFDB_H_
#define DUKTAPER_OPERATORS_FETCH_AFDB_H_

#include "../remote_import_downloader.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class FetchAFDB : public scripting::OperatorBase<FetchAFDB>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		bool asynchronous;
		std::string id;
		std::string url;
		scripting::operators::ImportMany::Result import_result;

		Result() : asynchronous(false)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("asynchronous")=asynchronous;
			heterostorage.variant_object.value("id")=id;
			heterostorage.variant_object.value("url")=url;
			if(!asynchronous)
			{
				import_result.store(heterostorage);
			}
		}
	};

	typedef RemoteImportRequest<scripting::operators::ImportMany> RemoteImportRequestType;
	typedef RemoteImportDownloader<RemoteImportRequestType> RemoteImportDownloaderType;

	RemoteImportDownloaderType* downloader_ptr;
	std::string id;

	FetchAFDB(RemoteImportDownloaderType& downloader) : downloader_ptr(&downloader)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		id=input.get_value_or_first_unused_unnamed_value("id");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("id", CDOD::DATATYPE_STRING, "Protein ID"));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(downloader_ptr==0)
		{
			throw std::runtime_error(std::string("Missing downloader."));
		}

		if(id.empty() || id.size()<5)
		{
			throw std::runtime_error(std::string("Invalid protein ID '")+id+"'.");
		}

		for(std::size_t i=0;i<id.size();i++)
		{
			if(!((id[i]>='a' && id[i]<='z') || (id[i]>='A' && id[i]<='Z') || (id[i]>='0' && id[i]<='9')))
			{
				throw std::runtime_error(std::string("Invalid protein ID '")+id+"'.");
			}
		}

		RemoteImportDownloaderType& downloader=(*downloader_ptr);
		RemoteImportDownloaderType::ScopeCleaner scope_cleaner(downloader);

		scripting::operators::ImportMany import_many_operator;
		import_many_operator.import_operator.title=id;
		import_many_operator.import_operator.loading_parameters.format="mmcif";
		import_many_operator.import_operator.loading_parameters.format_fallback="mmcif";

		std::ostringstream url_output;
		url_output << "https://alphafold.ebi.ac.uk/files/AF-" << id << "-F1-model_v4.cif";

		RemoteImportRequestType& request=downloader.add_request_and_start_download(RemoteImportRequestType(url_output.str(), import_many_operator));

		Result result;
		result.asynchronous=!downloader.is_synchronous();
		result.id=id;
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

#endif /* DUKTAPER_OPERATORS_FETCH_AFDB_H_ */

