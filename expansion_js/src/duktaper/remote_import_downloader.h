#ifndef DUKTAPER_REMOTE_IMPORT_DOWNLOADER_H_
#define DUKTAPER_REMOTE_IMPORT_DOWNLOADER_H_

#include <list>

#include "../dependencies/tinf/tinf_wrapper.h"

#include "call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

template<class ImportManyOperatorType>
struct RemoteImportRequest
{
	typedef ImportManyOperatorType ImportManyOperatorTypeUsed;

	std::string url;
	ImportManyOperatorType import_many_operator;
	std::string downloaded_data;
	bool download_started;
	bool download_finished;
	bool download_successful;
	bool fully_processed;

	RemoteImportRequest() : download_started(false), download_finished(false), download_successful(false), fully_processed(false)
	{
	}

	RemoteImportRequest(const std::string& url, const ImportManyOperatorType& import_many_operator) : url(url), import_many_operator(import_many_operator), download_started(false), download_finished(false), download_successful(false), fully_processed(false)
	{
	}

	typename ImportManyOperatorType::Result import_downloaded_data(scripting::CongregationOfDataManagers& congregation_of_data_managers)
	{
		fully_processed=true;

		typename ImportManyOperatorType::Result result;

		if(download_successful && !downloaded_data.empty())
		{
			if(TinfWrapper::check_if_string_gzipped(downloaded_data))
			{
				std::string uncompressed_data;
				if(!TinfWrapper::uncompress_gzipped_string(downloaded_data, uncompressed_data))
				{
					throw std::runtime_error(std::string("Failed to uncompress downloaded file."));
				}
				downloaded_data.swap(uncompressed_data);
			}

			const std::string url_basename=scripting::OperatorsUtilities::remove_suffix(scripting::OperatorsUtilities::get_basename_from_path(url), ".gz");

			scripting::VirtualFileStorage::TemporaryFile tmpfile(url_basename);
			scripting::VirtualFileStorage::set_file(tmpfile.filename(), downloaded_data);

			ImportManyOperatorType import_many_operator_to_use=import_many_operator;

			import_many_operator_to_use.files.push_back(tmpfile.filename());

			if(import_many_operator_to_use.title.empty())
			{
				import_many_operator_to_use.title=url_basename;
			}

			result=import_many_operator_to_use.run(congregation_of_data_managers);
		}

		return result;
	}
};

template<class RemoteImportRequestType>
class RemoteImportDownloader
{
public:
	typedef RemoteImportRequestType RemoteImportRequestTypeUsed;

	class ScopeCleaner
	{
	public:
		ScopeCleaner(RemoteImportDownloader& rid) : rid_(rid)
		{
		}

		~ScopeCleaner()
		{
			rid_.delete_inactive_requests();
		}
	private:
		RemoteImportDownloader& rid_;
	};

	bool is_synchronous() const
	{
		return synchronous();
	}

	RemoteImportRequestType& add_request_and_start_download(const RemoteImportRequestType& request)
	{
		requests_.push_back(request);
		start_download(requests_.back());
		return requests_.back();
	}

	bool check_if_any_request_not_downloaded() const
	{
		for(typename std::list<RemoteImportRequestType>::const_iterator it=requests_.begin();it!=requests_.end();++it)
		{
			if(!it->download_finished)
			{
				return true;
			}
		}
		return false;
	}

	bool check_if_any_request_downloaded_and_not_fully_processed() const
	{
		for(typename std::list<RemoteImportRequestType>::const_iterator it=requests_.begin();it!=requests_.end();++it)
		{
			if(it->download_finished && !(it->fully_processed))
			{
				return true;
			}
		}
		return false;
	}

	RemoteImportRequestType* get_first_request_downloaded_and_not_fully_processed()
	{
		for(typename std::list<RemoteImportRequestType>::iterator it=requests_.begin();it!=requests_.end();++it)
		{
			if(it->download_finished && !(it->fully_processed))
			{
				return (&(*it));
			}
		}
		return 0;
	}

	void delete_inactive_requests()
	{
		for(typename std::list<RemoteImportRequestType>::iterator it=requests_.begin();it!=requests_.end();)
		{
			if(it->fully_processed || !(it->download_started))
			{
				it=requests_.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

protected:
	RemoteImportDownloader()
	{
	}

	virtual ~RemoteImportDownloader()
	{
	}

	virtual bool synchronous() const = 0;

	virtual void start_download(RemoteImportRequestType&) = 0;

private:
	std::list<RemoteImportRequestType> requests_;
};

template<class RemoteImportRequestType>
class RemoteImportDownloaderSimple : public RemoteImportDownloader<RemoteImportRequestType>
{
public:
	static RemoteImportDownloaderSimple& instance()
	{
		static RemoteImportDownloaderSimple rids;
		return rids;
	}

private:
	RemoteImportDownloaderSimple()
	{
	}

	~RemoteImportDownloaderSimple()
	{
	}

	bool synchronous() const
	{
		return true;
	}

	void start_download(RemoteImportRequestType& request)
	{
		const bool success=download_file(request.url, request.downloaded_data);
		request.download_started=true;
		request.download_finished=true;
		request.download_successful=success;
	}

	static bool download_file(const std::string& url, std::string& result_data)
	{
		if(url.empty())
		{
			throw std::runtime_error(std::string("Missing URL."));
		}

		{
			if(!CallShellUtilities::test_if_shell_command_available("curl"))
			{
				throw std::runtime_error(std::string("'curl' command not available."));
			}

			std::ostringstream command_output;
			command_output << "curl '" << url << "'";
			operators::CallShell::Result download_result=operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0);
			if(download_result.exit_status!=0 || download_result.stdout_str.empty())
			{
				return false;
			}
			else
			{
				result_data.swap(download_result.stdout_str);
			}
		}

		return true;
	}
};

}

}

#endif /* DUKTAPER_REMOTE_IMPORT_DOWNLOADER_H_ */
