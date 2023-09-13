#ifndef VIEWER_REMOTE_IMPORT_DOWNLOADER_ADAPTIVE_H_
#define VIEWER_REMOTE_IMPORT_DOWNLOADER_ADAPTIVE_H_

#include "../../../expansion_js/src/duktaper/remote_import_downloader.h"

#ifdef FOR_WEB
#include <emscripten.h>
#endif

namespace voronota
{

namespace viewer
{

typedef duktaper::RemoteImportRequest<scripting::operators::ImportMany> RemoteImportRequestForOLD;
typedef duktaper::RemoteImportRequest<duktaper::operators::ImportMMCIF> RemoteImportRequestForMMCIF;

#ifdef FOR_WEB

template<class RemoteImportRequestType>
class RemoteImportDownloaderAdaptive : public duktaper::RemoteImportDownloader<RemoteImportRequestType>
{
public:
	static RemoteImportDownloaderAdaptive& instance()
	{
		static RemoteImportDownloaderAdaptive rida;
		return rida;
	}

private:
	RemoteImportDownloaderAdaptive()
	{
	}

	~RemoteImportDownloaderAdaptive()
	{
	}

	bool synchronous() const
	{
		return false;
	}

	void start_download(RemoteImportRequestType& request)
	{
		emscripten_async_wget_data(request.url.c_str(), reinterpret_cast<void*>(&request), RemoteImportDownloaderAdaptive::on_download_success, RemoteImportDownloaderAdaptive::on_download_error);
		request.download_started=true;
	}

	static void on_download_success(void* arg, void* data, int size)
	{
		RemoteImportRequestType* request_ptr=reinterpret_cast<RemoteImportRequestType*>(arg);
		request_ptr->download_finished=true;
		if(size>0 && data!=0)
		{
			request_ptr->downloaded_data=std::string(reinterpret_cast<const char*>(data), static_cast<std::size_t>(size));
			request_ptr->download_successful=true;
		}
	}

	static void on_download_error(void* arg)
	{
		RemoteImportRequestType* request_ptr=reinterpret_cast<RemoteImportRequestType*>(arg);
		request_ptr->download_finished=true;
		request_ptr->download_successful=false;
	}
};

typedef RemoteImportDownloaderAdaptive<RemoteImportRequestForOLD> RemoteImportDownloaderAdaptiveForOLD;
typedef RemoteImportDownloaderAdaptive<RemoteImportRequestForMMCIF> RemoteImportDownloaderAdaptiveForMMCIF;

#else

typedef duktaper::RemoteImportDownloaderSimple<RemoteImportRequestForOLD> RemoteImportDownloaderAdaptiveForOLD;
typedef duktaper::RemoteImportDownloaderSimple<RemoteImportRequestForMMCIF> RemoteImportDownloaderAdaptiveForMMCIF;

#endif

}

}

#endif /* VIEWER_REMOTE_IMPORT_DOWNLOADER_ADAPTIVE_H_ */

