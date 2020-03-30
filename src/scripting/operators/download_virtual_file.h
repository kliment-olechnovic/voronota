#ifndef SCRIPTING_OPERATORS_DOWNLOAD_VIRTUAL_FILE_H_
#define SCRIPTING_OPERATORS_DOWNLOAD_VIRTUAL_FILE_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class DownloadVirtualFile : public OperatorBase<DownloadVirtualFile>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string name;
	std::string file;

	DownloadVirtualFile()
	{
	}

	void initialize(CommandInput& input)
	{
		name=input.get_value<std::string>("name");
		file=input.get_value<std::string>("file");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "name of virtual file"));
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to real file"));
	}

	Result run(void*) const
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

}

#endif /* SCRIPTING_OPERATORS_DOWNLOAD_VIRTUAL_FILE_H_ */
