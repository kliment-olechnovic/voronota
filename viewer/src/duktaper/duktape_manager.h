#ifndef DUKTAPER_DUKTAPE_MANAGER_H_
#define DUKTAPER_DUKTAPE_MANAGER_H_

#include "../dependencies/duktape/duktape.h"
#include "../dependencies/redi/pstream.h"

#include "script_execution_manager.h"

namespace voronota
{

namespace duktaper
{

class DuktapeManager
{
public:
	class OutputDirector
	{
	public:
		OutputDirector()
		{
		}

		virtual ~OutputDirector()
		{
		}

		static const OutputDirector& instance()
		{
			static OutputDirector output_director;
			return output_director;
		}

		virtual void write_text(const std::string& str) const
		{
			std::cout << str;
			std::cout.flush();
		}

		virtual void write_error(const std::string& str) const
		{
			std::cerr << str;
			std::cerr.flush();
		}

		virtual void write_log(const std::string& str) const
		{
			std::cerr << str;
			std::cerr.flush();
		}
	};

	static bool eval(const std::string& script)
	{
		if(script.empty())
		{
			return false;
		}
		const bool success=(duk_peval_string(get_context(), script.c_str())==0);
		if(!success)
		{
			instance().write_error(std::string("error= ")+std::string(duk_safe_to_string(get_context(), -1))+std::string("\n"));
		}
		else if(flag_to_print_result_on_eval())
		{
			instance().write_log(std::string("= ")+std::string(duk_safe_to_string(get_context(), -1))+std::string("\n"));
		}
		duk_pop(get_context());
		return success;
	}

	static bool& flag_to_print_result_on_eval()
	{
		static bool flag=false;
		return flag;
	}

	static void set_script_execution_manager(scripting::ScriptExecutionManagerWithVariantOutput& sem)
	{
		instance().sem_=&sem;
	}

	static void set_output_director(const OutputDirector& output_director)
	{
		instance().output_director_=&output_director;
	}

private:
	DuktapeManager() : context_(0), sem_(0), output_director_(&OutputDirector::instance())
	{
	}

	~DuktapeManager()
	{
		if(context_!=0)
		{
			duk_destroy_heap(context_);
		}
	}

	static DuktapeManager& instance()
	{
		static DuktapeManager duktape_manager;
		return duktape_manager;
	}

	static duk_context* get_context()
	{
		instance().ensure();
		return instance().context_;
	}

	static duk_ret_t native_raw_write(duk_context *ctx)
	{
		const std::string text=duk_safe_to_string(ctx, -1);
		instance().write_text(text);
		return 0;
	}

	static duk_ret_t native_raw_writeln(duk_context *ctx)
	{
		const std::string text=duk_safe_to_string(ctx, -1);
		instance().write_text(text+"\n");
		return 0;
	}

	static duk_ret_t native_raw_log(duk_context *ctx)
	{
		const std::string text=duk_safe_to_string(ctx, -1);
		if(text.size()>0 && text[text.size()-1]=='\n')
		{
			instance().write_error(text);
		}
		else
		{
			instance().write_error(text+"\n");
		}
		return 0;
	}

	static duk_ret_t native_raw_read(duk_context *ctx)
	{
		if(!std::cin.good())
		{
			duk_push_string(ctx, "Invalid standard input stream");
			return duk_throw(ctx);
		}

		std::istreambuf_iterator<char> eos;
		std::string output(std::istreambuf_iterator<char>(std::cin), eos);

		duk_push_string(ctx, output.c_str());

		return 1;
	}

	static duk_ret_t native_raw_fwrite(duk_context *ctx)
	{
		const std::string filename=duk_require_string(ctx, -2);
		const std::string content=duk_safe_to_string(ctx, -1);

		if(filename.empty())
		{
			duk_push_string(ctx, "Missing file name");
			return duk_throw(ctx);
		}

		scripting::OutputSelector output_selector(filename);
		std::ostream& output=output_selector.stream();

		if(!output.good())
		{
			std::string error_message=std::string("Failed to write file '")+filename+"'";
			duk_push_string(ctx, error_message.c_str());
			return duk_throw(ctx);
		}

		output << content;

		return 0;
	}

	static duk_ret_t native_raw_fread(duk_context *ctx)
	{
		const std::string filename=duk_require_string(ctx, -1);

		if(filename.empty())
		{
			duk_push_string(ctx, "Missing file name");
			return duk_throw(ctx);
		}

		scripting::InputSelector input_selector(filename);
		std::istream& input=input_selector.stream();

		if(!input.good())
		{
			std::string error_message=std::string("Failed to read file '")+filename+"'";
			duk_push_string(ctx, error_message.c_str());
			return duk_throw(ctx);
		}

		std::istreambuf_iterator<char> eos;
		std::string output(std::istreambuf_iterator<char>(input), eos);

		duk_push_string(ctx, output.c_str());

		return 1;
	}

	static duk_ret_t native_raw_shell(duk_context *ctx)
	{
		const std::string command_raw=duk_require_string(ctx, -1);

		if(command_raw.empty())
		{
			duk_push_string(ctx, "Missing shell command");
			return duk_throw(ctx);
		}

		const std::string command=std::string("#!/bin/bash\n")+command_raw;

		redi::ipstream proc(command, redi::pstreams::pstdout|redi::pstreams::pstderr);

		scripting::VariantObject result_object;

		if(proc.out().good())
		{
			std::istreambuf_iterator<char> eos;
			result_object.value("stdout")=std::string(std::istreambuf_iterator<char>(proc.out()), eos);
		}

		if(proc.err().good())
		{
			std::istreambuf_iterator<char> eos;
			result_object.value("stderr")=std::string(std::istreambuf_iterator<char>(proc.err()), eos);
		}

		proc.close();

		if(proc.rdbuf()->exited())
		{
			result_object.value("exit_status")=proc.rdbuf()->status();
		}
		else
		{
			result_object.value("exit_status")=0;
		}

		const std::string result=scripting::JSONWriter::write(scripting::JSONWriter::Configuration(0), result_object);

	    duk_push_string(ctx, result.c_str());
	    duk_json_decode(ctx, -1);

		return 1;
	}

	static duk_ret_t native_raw_spipe(duk_context *ctx)
	{
		const std::string input_data=duk_require_string(ctx, -2);
		const std::string command_raw=duk_require_string(ctx, -1);

		if(command_raw.empty())
		{
			duk_push_string(ctx, "Missing shell command");
			return duk_throw(ctx);
		}

		const std::string command=std::string("#!/bin/bash\n")+command_raw;

		redi::pstream proc(command, redi::pstreams::pstdin|redi::pstreams::pstdout|redi::pstreams::pstderr);

		proc << input_data;
		proc.rdbuf()->peof();

		scripting::VariantObject result_object;

		if(proc.out().good())
		{
			std::istreambuf_iterator<char> eos;
			result_object.value("stdout")=std::string(std::istreambuf_iterator<char>(proc.out()), eos);
		}

		if(proc.err().good())
		{
			std::istreambuf_iterator<char> eos;
			result_object.value("stderr")=std::string(std::istreambuf_iterator<char>(proc.err()), eos);
		}

		proc.close();

		if(proc.rdbuf()->exited())
		{
			result_object.value("exit_status")=proc.rdbuf()->status();
		}
		else
		{
			result_object.value("exit_status")=0;
		}

		const std::string result=scripting::JSONWriter::write(scripting::JSONWriter::Configuration(0), result_object);

	    duk_push_string(ctx, result.c_str());
	    duk_json_decode(ctx, -1);

		return 1;
	}

	static int native_raw_voronota(duk_context *ctx)
	{
		const std::string script=duk_require_string(ctx, -1);

		if(script.empty())
		{
			duk_push_string(ctx, "Missing Voronota script string");
			return duk_throw(ctx);
		}

		std::string result;
		if(instance().sem_!=0)
		{
			result=instance().sem_->execute_script_and_return_last_output_string(script, false);
		}
	    duk_push_string(ctx, result.c_str());
	    duk_json_decode(ctx, -1);

	    return 1;
	}

	static int native_raw_voronota_last_output(duk_context *ctx)
	{
		if(instance().sem_==0 || instance().sem_->last_output_string().empty())
		{
			duk_push_string(ctx, "No last output");
			return duk_throw(ctx);
		}

	    duk_push_string(ctx, instance().sem_->last_output_string().c_str());
	    duk_json_decode(ctx, -1);

	    return 1;
	}

	duk_context* context()
	{
		ensure();
		return context_;
	}

	void ensure()
	{
		if(context_==0)
		{
			context_=duk_create_heap_default();

			duk_push_c_function(context_, native_raw_write, 1);
			duk_put_global_string(context_, "raw_write");

			duk_push_c_function(context_, native_raw_writeln, 1);
			duk_put_global_string(context_, "raw_writeln");

			duk_push_c_function(context_, native_raw_log, 1);
			duk_put_global_string(context_, "raw_log");

			duk_push_c_function(context_, native_raw_read, 0);
			duk_put_global_string(context_, "raw_read");

			duk_push_c_function(context_, native_raw_fwrite, 2);
			duk_put_global_string(context_, "raw_fwrite");

			duk_push_c_function(context_, native_raw_fread, 1);
			duk_put_global_string(context_, "raw_fread");

			duk_push_c_function(context_, native_raw_shell, 1);
			duk_put_global_string(context_, "raw_shell");

			duk_push_c_function(context_, native_raw_spipe, 2);
			duk_put_global_string(context_, "raw_spipe");

			duk_push_c_function(context_, native_raw_voronota, 1);
			duk_put_global_string(context_, "raw_voronota");

			duk_push_c_function(context_, native_raw_voronota_last_output, 0);
			duk_put_global_string(context_, "raw_voronota_last_output");

			{
				const std::string script=""
						"write=function(obj)"
						"{"
						"  var obj_type=Object.prototype.toString.call(obj);"
						"  if(obj_type==='[object Object]' || obj_type==='[object Array]')"
						"  {"
						"    raw_write(JSON.stringify(obj));"
						"  }"
						"  else"
						"  {"
						"    raw_write(obj);"
						"  }"
						"}"
						"\n"
						"writeln=function(obj)"
						"{"
						"  var obj_type=Object.prototype.toString.call(obj);"
						"  if(obj_type==='[object Object]' || obj_type==='[object Array]')"
						"  {"
						"    raw_writeln(JSON.stringify(obj));"
						"  }"
						"  else"
						"  {"
						"    raw_writeln(obj);"
						"  }"
						"}"
						"\n"
						"log=function(obj)"
						"{"
						"  var obj_type=Object.prototype.toString.call(obj);"
						"  if(obj_type==='[object Object]' || obj_type==='[object Array]')"
						"  {"
						"    raw_log(JSON.stringify(obj));"
						"  }"
						"  else"
						"  {"
						"    raw_log(obj);"
						"  }"
						"}"
						"\n"
						"fwrite=function(filename, obj)"
						"{"
						"  var obj_type=Object.prototype.toString.call(obj);"
						"  if(obj_type==='[object Object]' || obj_type==='[object Array]')"
						"  {"
						"    raw_fwrite(filename, JSON.stringify(obj));"
						"  }"
						"  else"
						"  {"
						"    raw_fwrite(filename, obj);"
						"  }"
						"}"
						"\n"
						"source=function(filename)"
						"{"
						"  var lines=raw_fread(filename).split('\\n');"
						"  if(lines.length>0 && lines[0].indexOf('#')===0)"
						"  {"
						"    lines.shift();"
						"  }"
						"  if(lines.length>0)"
						"  {"
						"    eval(lines.join('\\n'));"
						"  }"
						"}"
						"\n"
						"read=raw_read;"
						"\n"
						"fread=raw_fread;"
						"\n"
						"shell=raw_shell;"
						"\n"
						"spipe=raw_spipe;"
						"\n";

				const bool success=(duk_peval_string(context_, script.c_str())==0);
				if(!success)
				{
					write_error(std::string(duk_safe_to_string(context_, -1))+"\n");
				}
				duk_pop(context_);
			}
		}
	}

	void write_text(const std::string& str)
	{
		if(output_director_!=0)
		{
			output_director_->write_text(str);
		}
	}

	void write_error(const std::string& str)
	{
		if(output_director_!=0)
		{
			output_director_->write_error(str);
		}
	}

	void write_log(const std::string& str)
	{
		if(output_director_!=0)
		{
			output_director_->write_log(str);
		}
	}

	duk_context* context_;
	scripting::ScriptExecutionManagerWithVariantOutput* sem_;
	const OutputDirector* output_director_;
};

}

}

#endif /* DUKTAPER_DUKTAPE_MANAGER_H_ */
