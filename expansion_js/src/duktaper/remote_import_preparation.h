#ifndef DUKTAPER_REMOTE_IMPORT_PREPARATION_H_
#define DUKTAPER_REMOTE_IMPORT_PREPARATION_H_

#include <list>

#include "../../../src/scripting/operators/import_many.h"

#include "../dependencies/tinf/tinf_wrapper.h"

#include "call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

class RemoteImportPreparation
{
public:
	struct Request
	{
		std::string url;
		scripting::operators::ImportMany import_many_operator;
		std::string downloaded_data;
		bool download_finished;
		bool download_successful;
		bool fully_processed;

		Request() : download_finished(false), download_successful(false), fully_processed(false)
		{
		}

		Request(const std::string& url, const scripting::operators::ImportMany& import_many_operator) : url(url), import_many_operator(import_many_operator), download_finished(false), download_successful(false)
		{
		}

		scripting::operators::ImportMany::Result import_downloaded_data(scripting::CongregationOfDataManagers& congregation_of_data_managers)
		{
			fully_processed=true;

			scripting::operators::ImportMany::Result result;

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

				scripting::operators::ImportMany import_many_operator_to_use=import_many_operator;

				import_many_operator_to_use.files.push_back(tmpfile.filename());

				if(import_many_operator_to_use.import_operator.title.empty())
				{
					import_many_operator_to_use.import_operator.title=url_basename;
				}

				result=import_many_operator_to_use.run(congregation_of_data_managers);
			}

			return result;
		}
	};

	RemoteImportPreparation()
	{
	}

	void add_request(const Request& request)
	{
		requests_.push_back(request);
	}

	Request* download_request_until_first_success()
	{
		bool success=false;
		for(std::list<Request>::iterator it=requests_.begin();it!=requests_.end() && !success;++it)
		{
			Request& r=(*it);
			r.download_successful=download_file(r.url, r.downloaded_data);
			r.download_finished=true;
			success=r.download_successful;
		}
		return get_first_successfully_downloaded_request();
	}

	Request* get_first_successfully_downloaded_request()
	{
		for(std::list<Request>::iterator it=requests_.begin();it!=requests_.end();++it)
		{
			Request& r=(*it);
			if(r.download_successful)
			{
				return &r;
			}
		}
		return 0;
	}

private:
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

	std::list<Request> requests_;
};

}

}

#endif /* DUKTAPER_REMOTE_IMPORT_PREPARATION_H_ */
