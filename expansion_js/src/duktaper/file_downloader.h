#ifndef DUKTAPER_FILE_DOWNLOADER_H_
#define DUKTAPER_FILE_DOWNLOADER_H_

#include "../dependencies/tinf/tinf_wrapper.h"

#include "call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

class FileDownloader
{
public:
	static bool download_file(const std::string& url, const bool automatically_uncompress, std::string& result_data)
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

		if(automatically_uncompress && TinfWrapper::check_if_string_gzipped(result_data))
		{
			std::string uncompressed_data;
			if(!TinfWrapper::uncompress_gzipped_string(result_data, uncompressed_data))
			{
				throw std::runtime_error(std::string("Failed to uncompress downloaded file."));
			}
			result_data.swap(uncompressed_data);
		}

		return true;
	}
};

}

}

#endif /* DUKTAPER_FILE_DOWNLOADER_H_ */
