#ifndef VIEWER_OPERATORS_IMPORT_DOWNLOADED_H_
#define VIEWER_OPERATORS_IMPORT_DOWNLOADED_H_

#include "../operators_common.h"
#include "../remote_import_downloader_adaptive.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

template<class RemoteImportDownloaderType>
class ImportDownloaded : public scripting::OperatorBase<ImportDownloaded<RemoteImportDownloaderType> >
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
			import_result.store(heterostorage);
		}
	};

	ImportDownloaded()
	{
	}

	void initialize(scripting::CommandInput&)
	{
	}

	void document(scripting::CommandDocumentation&) const
	{
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		RemoteImportDownloaderType& downloader=RemoteImportDownloaderType::instance();

		typename RemoteImportDownloaderType::RemoteImportRequestTypeUsed* request_ptr=downloader.get_first_request_downloaded_and_not_fully_processed();

		if(request_ptr==0)
		{
			throw std::runtime_error(std::string("No finished downloads to process."));
		}

		typename RemoteImportDownloaderType::RemoteImportRequestTypeUsed& request=(*request_ptr);
		request.fully_processed=true;

		typename RemoteImportDownloaderType::ScopeCleaner scope_cleaner(downloader);

		if(!request.download_successful)
		{
			throw std::runtime_error(std::string("No data downloaded for URL '")+request.url+"'.");
		}

		Result result;
		result.asynchronous=!downloader.is_synchronous();
		result.url=request.url;
		result.import_result=request.import_downloaded_data(congregation_of_data_managers);

		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_IMPORT_DOWNLOADED_H_ */
