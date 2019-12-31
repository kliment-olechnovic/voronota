#ifndef SCRIPTING_OPERATORS_UPLOAD_VIRTUAL_FILE_H_
#define SCRIPTING_OPERATORS_UPLOAD_VIRTUAL_FILE_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class UploadVirtualFile
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

	UploadVirtualFile()
	{
	}

	UploadVirtualFile& init(CommandInput& input)
	{
		name=input.get_value<std::string>("name");
		file=input.get_value<std::string>("file");
		return (*this);
	}

	Result run() const
	{
		VirtualFileStorage::assert_filename_is_valid(name);

		if(file.empty())
		{
			throw std::runtime_error(std::string("Empty input file name."));
		}

		std::ifstream finput(file.c_str(), std::ios::in);

		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
		}

		std::istreambuf_iterator<char> eos;
		std::string data(std::istreambuf_iterator<char>(finput), eos);

		VirtualFileStorage::set_file(name, data);

		Result result;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_UPLOAD_VIRTUAL_FILE_H_ */
