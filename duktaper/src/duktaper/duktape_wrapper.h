#ifndef VIEWER_DUKTAPE_WRAPPER_H_
#define VIEWER_DUKTAPE_WRAPPER_H_

#include "../duktape/duktape.h"
#include "../redi/pstream.h"

#include "../../../src/scripting/script_execution_manager_with_variant_output.h"
#include "../../../src/scripting/binding_javascript.h"

namespace voronota
{

namespace duktaper
{

class DuktapeContextWrapper
{
public:
	static bool eval(const std::string& script)
	{
		if(script.empty())
		{
			return false;
		}
		const bool success=(duk_peval_string(get_context(), script.c_str())==0);
		if(instance().stderr_!=0)
		{
			std::ostream& stderr=(*instance().stderr_);
			if(!success)
			{
				stderr << "error= " << duk_safe_to_string(get_context(), -1) << std::endl;
			}
			else if(instance().eval_autoprint_)
			{
				stderr << "= " << duk_safe_to_string(get_context(), -1) << std::endl;
			}
		}
		duk_pop(get_context());
		return success;
	}

	static bool setup(scripting::ScriptExecutionManagerWithVariantOutput& sem, std::ostream& stdout, std::ostream& stderr, const std::vector<std::string>& script_args)
	{
		set_script_execution_manager(sem);
		set_stdout(stdout);
		set_stderr(stderr);

		std::ostringstream script;

		script << scripting::BindingJavascript::generate_setup_script(sem.collection_of_command_documentations())  << "\n";

		script << ""
				"print=function(obj, json_spacing)"
				"{"
				"  var obj_type=Object.prototype.toString.call(obj);"
				"  if(obj_type==='[object Object]')"
				"  {"
				"    var spaces=0;"
				"    if(json_spacing)"
				"    {"
				"      spaces=json_spacing;"
				"    }"
				"    raw_print(JSON.stringify(obj, null, spaces));"
				"  }"
				"  else"
				"  {"
				"    raw_print(obj);"
				"  }"
				"}"
				"\n"
				"fprint=function(filename, obj, json_spacing)"
				"{"
				"  var obj_type=Object.prototype.toString.call(obj);"
				"  if(obj_type==='[object Object]')"
				"  {"
				"    var spaces=0;"
				"    if(json_spacing)"
				"    {"
				"      spaces=json_spacing;"
				"    }"
				"    raw_fprint(filename, JSON.stringify(obj, null, spaces));"
				"  }"
				"  else"
				"  {"
				"    raw_fprint(filename, obj);"
				"  }"
				"}"
				"\n"
				"read=raw_read;"
				"\n"
				"fread=raw_fread;"
				"\n"
				"shell=raw_shell;"
				"\n";

		{
			script << "var script_args=[";
			for(std::size_t i=0;i<script_args.size();i++)
			{
				script << "'" << script_args[i] << "'";
				if(i+1<script_args.size())
				{
					script << ", ";
				}
			}
			script << "];\n";
		}

		return eval(script.str());
	}

	static void set_eval_autoprint(const bool enabled)
	{
		instance().eval_autoprint_=enabled;
	}

private:
	DuktapeContextWrapper() : context_(0), sem_(0), stdout_(0), stderr_(0), eval_autoprint_(false)
	{
	}

	~DuktapeContextWrapper()
	{
		if(context_!=0)
		{
			duk_destroy_heap(context_);
		}
	}

	static DuktapeContextWrapper& instance()
	{
		static DuktapeContextWrapper context_wrapper;
		return context_wrapper;
	}

	static void set_script_execution_manager(scripting::ScriptExecutionManagerWithVariantOutput& sem)
	{
		instance().sem_=&sem;
	}

	static void set_stdout(std::ostream& stdout)
	{
		instance().stdout_=&stdout;
	}

	static void set_stderr(std::ostream& stderr)
	{
		instance().stderr_=&stderr;
	}

	static duk_context* get_context()
	{
		instance().ensure();
		return instance().context_;
	}

	static duk_ret_t native_raw_print(duk_context *ctx)
	{
		duk_push_string(ctx, " ");
		duk_insert(ctx, 0);
		duk_join(ctx, duk_get_top(ctx) - 1);
		const std::string text=duk_safe_to_string(ctx, -1);
		if(instance().stdout_!=0)
		{
			std::ostream& stdout=(*instance().stdout_);
			stdout << text << std::endl;
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

	static duk_ret_t native_raw_fprint(duk_context *ctx)
	{
		const std::string filename=duk_require_string(ctx, -2);
		const std::string content=duk_safe_to_string(ctx, -1);

		if(filename.empty())
		{
			duk_push_string(ctx, "Missing file name");
			return duk_throw(ctx);
		}

		std::ofstream output(filename.c_str(), std::ios::out);

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

		std::ifstream input(filename.c_str(), std::ios::in);

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

		if(instance().stderr_!=0)
		{
			std::ostream& stderr=(*instance().stderr_);
			stderr << errors.str();
		}

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

			duk_push_c_function(context_, native_raw_print, DUK_VARARGS);
			duk_put_global_string(context_, "raw_print");

			duk_push_c_function(context_, native_raw_read, 0);
			duk_put_global_string(context_, "raw_read");

			duk_push_c_function(context_, native_raw_fprint, 2);
			duk_put_global_string(context_, "raw_fprint");

			duk_push_c_function(context_, native_raw_fread, 1);
			duk_put_global_string(context_, "raw_fread");

			duk_push_c_function(context_, native_raw_shell, 1);
			duk_put_global_string(context_, "raw_shell");

			duk_push_c_function(context_, native_raw_voronota, 1);
			duk_put_global_string(context_, "raw_voronota");
		}
	}

	duk_context* context_;
	scripting::ScriptExecutionManagerWithVariantOutput* sem_;
	std::ostream* stdout_;
	std::ostream* stderr_;
	bool eval_autoprint_;
};

}

}

#endif /* VIEWER_DUKTAPE_WRAPPER_H_ */
