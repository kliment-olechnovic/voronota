#ifndef SCRIPTING_OPERATORS_UPLOAD_VIRTUAL_FILE_H_
#define SCRIPTING_OPERATORS_UPLOAD_VIRTUAL_FILE_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class UploadVirtualFile : public OperatorBase<UploadVirtualFile>
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

	UploadVirtualFile()
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

}

#endif /* SCRIPTING_OPERATORS_UPLOAD_VIRTUAL_FILE_H_ */
