#ifndef DUKTAPER_OPERATORS_IMPORT_URL_H_
#define DUKTAPER_OPERATORS_IMPORT_URL_H_

#include "../remote_import_downloader.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

template<class ImportManyOperatorType>
class ImportUrl : public scripting::OperatorBase< ImportUrl<ImportManyOperatorType> >
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		bool asynchronous;
		std::string url;
		typename ImportManyOperatorType::Result import_result;

		Result() : asynchronous(false)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("asynchronous")=asynchronous;
			heterostorage.variant_object.value("url")=url;
			if(!asynchronous)
			{
				import_result.store(heterostorage);
			}
		}
	};

	typedef RemoteImportRequest<ImportManyOperatorType> RemoteImportRequestType;
	typedef RemoteImportDownloader<RemoteImportRequestType> RemoteImportDownloaderType;

	RemoteImportDownloaderType* downloader_ptr;
	std::string url;
	ImportManyOperatorType import_many_operator;

	ImportUrl(RemoteImportDownloaderType& downloader) : downloader_ptr(&downloader)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		url=input.get_value_or_first_unused_unnamed_value("url");
		import_many_operator.initialize(input, true);
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("url", CDOD::DATATYPE_STRING, "resource URL"));
		import_many_operator.document(doc, true);
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(downloader_ptr==0)
		{
			throw std::runtime_error(std::string("Missing downloader."));
		}

		if(url.empty())
		{
			throw std::runtime_error(std::string("Missing URL."));
		}

		RemoteImportDownloaderType& downloader=(*downloader_ptr);
		typename RemoteImportDownloaderType::ScopeCleaner scope_cleaner(downloader);

		Result result;
		result.asynchronous=!downloader.is_synchronous();
		result.url=url;

		RemoteImportRequestType& request=downloader.add_request_and_start_download(RemoteImportRequestType(url, import_many_operator));

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

#endif /* DUKTAPER_OPERATORS_IMPORT_URL_H_ */
