#ifndef VIEWER_DUKTAPE_MANAGER_H_
#define VIEWER_DUKTAPE_MANAGER_H_

#include "../duktape/duktape.h"
#include "../redi/pstream.h"

#include "../../../src/scripting/script_execution_manager_with_variant_output.h"

namespace voronota
{

namespace viewer
{

class DuktapeManager
{
public:
	static bool eval(const std::string& script)
	{
		if(script.empty())
		{
			return false;
		}
		const bool success=(duk_peval_string(get_context(), script.c_str())==0);
		if(!success)
		{
			std::cerr << "error= " << duk_safe_to_string(get_context(), -1) << std::endl;
		}
		duk_pop(get_context());
		return success;
	}

	static void set_script_execution_manager(scripting::ScriptExecutionManagerWithVariantOutput& sem)
	{
		instance().sem_=&sem;
	}

private:
	DuktapeManager() : context_(0), sem_(0)
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
		std::cout << text;
		std::cout.flush();
		return 0;
	}

	static duk_ret_t native_raw_writeln(duk_context *ctx)
	{
		const std::string text=duk_safe_to_string(ctx, -1);
		std::cout << text << std::endl;
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
		const std::string command=duk_require_string(ctx, -1);

		if(command.empty())
		{
			duk_push_string(ctx, "Missing shell command");
			return duk_throw(ctx);
		}

		redi::ipstream proc(command, redi::pstreams::pstdout|redi::pstreams::pstderr);

		std::ostringstream output;
		{
			std::string line;
			while(std::getline(proc.out(), line))
			{
				output << line << "\n";
			}
		}

		std::ostringstream errors;
		{
			std::string line;
			while(std::getline(proc.err(), line))
			{
				errors << line << "\n";
			}
		}

		std::cerr << errors.str() << std::endl;

		duk_push_string(ctx, output.str().c_str());

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

			duk_push_c_function(context_, native_raw_read, 0);
			duk_put_global_string(context_, "raw_read");

			duk_push_c_function(context_, native_raw_fwrite, 2);
			duk_put_global_string(context_, "raw_fwrite");

			duk_push_c_function(context_, native_raw_fread, 1);
			duk_put_global_string(context_, "raw_fread");

			duk_push_c_function(context_, native_raw_shell, 1);
			duk_put_global_string(context_, "raw_shell");

			duk_push_c_function(context_, native_raw_voronota, 1);
			duk_put_global_string(context_, "raw_voronota");

			{
				const std::string script=""
						"write=function(obj)"
						"{"
						"  var obj_type=Object.prototype.toString.call(obj);"
						"  if(obj_type==='[object Object]')"
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
						"  if(obj_type==='[object Object]')"
						"  {"
						"    raw_writeln(JSON.stringify(obj));"
						"  }"
						"  else"
						"  {"
						"    raw_writeln(obj);"
						"  }"
						"}"
						"\n"
						"fwrite=function(filename, obj)"
						"{"
						"  var obj_type=Object.prototype.toString.call(obj);"
						"  if(obj_type==='[object Object]')"
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
						"\n";

				const bool success=(duk_peval_string(context_, script.c_str())==0);
				if(!success)
				{
					std::cerr << duk_safe_to_string(context_, -1) << std::endl;
				}
				duk_pop(context_);
			}
		}
	}

	duk_context* context_;
	scripting::ScriptExecutionManagerWithVariantOutput* sem_;
};

}

}

#endif /* VIEWER_DUKTAPE_MANAGER_H_ */
