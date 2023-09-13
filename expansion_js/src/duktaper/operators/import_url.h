#ifndef DUKTAPER_OPERATORS_IMPORT_URL_H_
#define DUKTAPER_OPERATORS_IMPORT_URL_H_

#include "../remote_import_downloader.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

template<class RemoteImportDownloaderType>
class ImportUrl : public scripting::OperatorBase< ImportUrl<RemoteImportDownloaderType> >
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		bool asynchronous;
		std::string url;
		typename RemoteImportDownloaderType::RemoteImportRequestTypeUsed::ImportManyOperatorTypeUsed::Result import_result;

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

	std::string url;
	typename RemoteImportDownloaderType::RemoteImportRequestTypeUsed::ImportManyOperatorTypeUsed import_many_operator;

	ImportUrl()
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
		if(url.empty())
		{
			throw std::runtime_error(std::string("Missing URL."));
		}

		RemoteImportDownloaderType& downloader=RemoteImportDownloaderType::instance();
		typename RemoteImportDownloaderType::ScopeCleaner scope_cleaner(downloader);

		Result result;
		result.asynchronous=!downloader.is_synchronous();
		result.url=url;

		typename RemoteImportDownloaderType::RemoteImportRequestTypeUsed& request=downloader.add_request_and_start_download(typename RemoteImportDownloaderType::RemoteImportRequestTypeUsed(url, import_many_operator));

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
