#ifndef DUKTAPER_OPERATORS_CALL_SHELL_H_
#define DUKTAPER_OPERATORS_CALL_SHELL_H_

#include <redi/pstream.h>

#include "../operators_common.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class CallShell : public scripting::OperatorBase<CallShell>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		int exit_status;
		std::string stdout_str;
		std::string stderr_str;

		Result() : exit_status(1)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("exit_status")=exit_status;
			heterostorage.variant_object.value("stdout")=stdout_str;
			heterostorage.variant_object.value("stderr")=stderr_str;
		}
	};

	std::string command_string;
	std::string input_file;
	std::string input_string;
	std::string output_file;

	CallShell()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		command_string=input.get_value_or_first_unused_unnamed_value("command-string");
		input_file=input.get_value_or_default<std::string>("input-file", "");
		input_string=input.get_value_or_default<std::string>("input-string", "");
		output_file=input.get_value_or_default<std::string>("output-file", "");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("command-string", CDOD::DATATYPE_STRING, "command string"));
		doc.set_option_decription(CDOD("input-file", CDOD::DATATYPE_STRING, "path to input data file", ""));
		doc.set_option_decription(CDOD("input-string", CDOD::DATATYPE_STRING, "input data string", ""));
		doc.set_option_decription(CDOD("output-file", CDOD::DATATYPE_STRING, "path to output data file", ""));
	}

	Result run(void*) const
	{
		if(command_string.empty())
		{
			throw std::runtime_error(std::string("Mo command string provided."));
		}

		if(!input_file.empty() && !input_string.empty())
		{
			throw std::runtime_error(std::string("More than one input data source provided."));
		}

		std::string input_data;

		if(!input_file.empty())
		{
			scripting::InputSelector finput_selector(input_file);
			std::istream& finput=finput_selector.stream();
			scripting::assert_io_stream(input_file, finput);

			std::istreambuf_iterator<char> eos;
			std::string data(std::istreambuf_iterator<char>(finput), eos);

			input_data.swap(data);
		}
		else if(!input_string.empty())
		{
			input_data=input_string;
		}

		const std::string command=std::string("#!/bin/bash\n")+command_string;

		redi::pstream proc(command, input_data.empty() ? (redi::pstreams::pstdout|redi::pstreams::pstderr) : (redi::pstreams::pstdin|redi::pstreams::pstdout|redi::pstreams::pstderr));

		if(!input_data.empty())
		{
			proc << input_data;
			proc.rdbuf()->peof();
		}

		Result result;

		if(proc.out().good())
		{
			std::istreambuf_iterator<char> eos;
			result.stdout_str=std::string(std::istreambuf_iterator<char>(proc.out()), eos);
		}

		if(proc.err().good())
		{
			std::istreambuf_iterator<char> eos;
			result.stderr_str=std::string(std::istreambuf_iterator<char>(proc.err()), eos);
		}

		proc.close();

		if(proc.rdbuf()->exited())
		{
			result.exit_status=proc.rdbuf()->status();
		}
		else
		{
			result.exit_status=0;
		}

		if(!output_file.empty())
		{
			scripting::OutputSelector output_selector(output_file);
			std::ostream& foutput=output_selector.stream();
			scripting::assert_io_stream(output_file, foutput);

			foutput << result.stdout_str;
			result.stdout_str.clear();
		}

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_CALL_SHELL_H_ */
