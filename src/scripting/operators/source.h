#ifndef SCRIPTING_OPERATORS_SOURCE_H_
#define SCRIPTING_OPERATORS_SOURCE_H_

#include "../operators_common.h"

#include "../script_partitioner.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class Source : public OperatorBase<Source>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string file;

	Source()
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
	}

	Result run(ScriptPartitioner& script_partitioner) const
	{
		if(file.empty())
		{
			throw std::runtime_error(std::string("File not provided."));
		}

		InputSelector finput_selector(file);
		std::istream& finput=finput_selector.stream();

		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
		}

		std::istreambuf_iterator<char> eos;
		std::string script(std::istreambuf_iterator<char>(finput), eos);

		if(script.empty())
		{
			throw std::runtime_error(std::string("No script in file '")+file+"'.");
		}

		script_partitioner.add_pending_sentences_from_string_to_front(script);

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SOURCE_H_ */
