#ifndef SCRIPTING_OPERATORS_CAT_FILES_H_
#define SCRIPTING_OPERATORS_CAT_FILES_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class CatFiles : public OperatorBase<CatFiles>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::vector<std::string> files;

	CatFiles()
	{
	}

	void initialize(CommandInput& input)
	{
		files=input.get_value_vector_or_all_unused_unnamed_values("files");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("files", CDOD::DATATYPE_STRING_ARRAY, "paths to real or virtual files, last is output"));
	}

	Result run(void*) const
	{
		if(files.size()<2)
		{
			throw std::runtime_error(std::string("Less than two file paths provided."));
		}

		const std::string& output_file=files.back();
		OutputSelector output_selector(output_file);
		std::ostream& output=output_selector.stream();
		assert_io_stream(output_file, output);

		for(std::size_t i=0;(i+1)<files.size();i++)
		{
			InputSelector input_selector(files[i]);
			std::istream& input=input_selector.stream();
			assert_io_stream(files[i], input);
			std::istreambuf_iterator<char> eos;
			std::string data(std::istreambuf_iterator<char>(input), eos);
			output << data;
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_CAT_FILES_H_ */
