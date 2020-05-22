#ifndef DUKTAPER_CACHE_FILE_H_
#define DUKTAPER_CACHE_FILE_H_

#include "operators/checksum.h"
#include "operators/call_shell.h"

namespace voronota
{

namespace duktaper
{

class CacheFile
{
public:
	CacheFile() : file_available_(false)
	{
	}

	CacheFile(
			const std::string& cache_dir,
			const std::string& data_string_for_checksum,
			const std::string& extension) :
				file_available_(false)
	{
		if(cache_dir.empty())
		{
			throw std::runtime_error(std::string("No cache directory provided."));
		}

		if(data_string_for_checksum.empty())
		{
			throw std::runtime_error(std::string("No cache checksum data provided."));
		}

		{
			scripting::VirtualFileStorage::TemporaryFile tmp_file_for_checksum;
			scripting::VirtualFileStorage::set_file(tmp_file_for_checksum.filename(), data_string_for_checksum);

			std::ostringstream args;
			args << " -data-file " << tmp_file_for_checksum.filename();
			checksum_=operators::Checksum().init(args.str()).run(0).checksum;
		}

		if(checksum_.empty())
		{
			throw std::runtime_error(std::string("Failed to compute checksum."));
		}

		file_path_=cache_dir+"/"+checksum_+extension;

		{
			std::ostringstream args;
			args << " -command-string \"test \\-s '" << file_path_ << "'\"";
			file_available_=operators::CallShell().init(args.str()).run(0).exit_status==0;
		}

		if(!file_available_)
		{
			std::ostringstream args;
			args << " -command-string \"mkdir \\-p '" << cache_dir << "'\"";
			const int status=operators::CallShell().init(args.str()).run(0).exit_status;
			if(status!=0)
			{
				throw std::runtime_error(std::string("Failed to find or cache create directory '")+cache_dir+"'");
			}
		}
	}

	const std::string checksum() const
	{
		return checksum_;
	}

	const std::string file_path() const
	{
		return file_path_;
	}

	bool file_available() const
	{
		return file_available_;
	}

private:
	std::string checksum_;
	std::string file_path_;
	bool file_available_;
};

}

}

#endif /* DUKTAPER_CACHE_FILE_H_ */
