#ifndef SCRIPTING_OPERATORS_DOWNLOAD_VIRTUAL_FILE_H_
#define SCRIPTING_OPERATORS_DOWNLOAD_VIRTUAL_FILE_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class DownloadVirtualFile
{
public:
	struct Result
	{
		const Result& write(HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	std::string name;
	std::string file;

	DownloadVirtualFile()
	{
	}

	DownloadVirtualFile& init(CommandInput& input)
	{
		name=input.get_value<std::string>("name");
		file=input.get_value<std::string>("file");
		return (*this);
	}

	Result run(void*&) const
	{
		VirtualFileStorage::assert_file_exists(name);

		if(file.empty())
		{
			throw std::runtime_error(std::string("Empty output file name."));
		}

		std::ofstream foutput(file.c_str(), std::ios::out);

		if(!foutput.good())
		{
			throw std::runtime_error(std::string("Failed to write to file '")+file+"'.");
		}

		foutput << VirtualFileStorage::get_file(name);

		Result result;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_DOWNLOAD_VIRTUAL_FILE_H_ */
