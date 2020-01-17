#ifndef VIEWER_DUKTAPE_WRAPPER_H_
#define VIEWER_DUKTAPE_WRAPPER_H_

#include "../duktape/duktape.h"
#include "../redi/pstream.h"

#include "../../../src/scripting/script_execution_manager_with_variant_output.h"

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

		script << ""
				"raw_voronota_parameters_object_to_array=function(obj)"
				"{"
				"  var result=[];"
				"  var obj_type=Object.prototype.toString.call(obj);"
				"  if(obj_type==='[object Object]')"
				"  {"
				"    for(var key in obj)"
				"    {"
				"      result.push('-'+key);"
				"      var value=obj[key];"
				"      var value_type=Object.prototype.toString.call(value);"
				"      if(value_type==='[object Array]')"
				"      {"
				"        for(var i=0;i<value.length;i++)"
				"        {"
				"          result.push(value[i]);"
				"        }"
				"      }"
				"      else"
				"      {"
				"        result.push(value);"
				"      }"
				"    }"
				"  }"
				"  else if(obj_type==='[object Array]')"
				"  {"
				"    for(var i=0;i<obj.length;i++)"
				"    {"
				"      result.push(obj[i]);"
				"    }"
				"  }"
				"  else"
				"  {"
				"    result.push(obj);"
				"  }"
				"  return result;"
				"}"
				"\n"
				"raw_voronota_named=function(name, args)"
				"{"
				"  var all_args=[name];"
				"  for(var i=0;i<args.length;++i)"
				"  {"
				"    var values=raw_voronota_parameters_object_to_array(args[i]);"
				"    for(var j=0;j<values.length;j++)"
				"    {"
				"      all_args.push(values[j]);"
				"    }"
				"  }"
				"  return raw_voronota.apply(null, all_args);"
				"}"
				"\n"
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
				"\n";

		const std::string namespace_name="voronota";

		script << "var " << namespace_name << "={}\n";

		script << namespace_name << ".do=function(){return raw_voronota.apply(null, arguments);}\n";

		const std::vector<std::string> command_names=sem.collection_of_command_documentations().get_all_names();

		for(std::size_t i=0;i<command_names.size();i++)
		{
			const std::string command_name=command_names[i];
			if(command_name=="exit")
			{
				script << "exit=function(){return raw_voronota('exit');}\n";
			}
			std::string function_name=command_name;
			for(std::size_t j=0;j<function_name.size();j++)
			{
				if(function_name[j]=='-')
				{
					function_name[j]='_';
				}
			}
			script << namespace_name << "." << function_name << "=function(){return raw_voronota_named('" << command_name << "', arguments);}\n";
		}

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

	static duk_ret_t native_raw_shell(duk_context *ctx)
	{
		const std::string command=duk_require_string(ctx, -1);
		if(command.empty())
		{
			return 0;
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
		std::vector<std::string> tokens;
		{
			const int N=duk_get_top(ctx);
			for(int i=0;i<N;i++)
			{
				if(duk_is_array(ctx, i)==1)
				{
					const int M=duk_get_length(ctx, i);
					for(int j=0;j<M;j++)
					{
						duk_get_prop_index(ctx, i, j);
						const std::string token=duk_safe_to_string(ctx, -1);
						duk_pop(ctx);
						tokens.push_back(token);
					}
				}
				else
				{
					const std::string token=duk_safe_to_string(ctx, i);
					tokens.push_back(token);
				}
			}
		}

		std::string script;
		for(std::size_t i=0;i<tokens.size();i++)
		{
			const std::string& token=tokens[i];
			if(i==0)
			{
				script+=token;
			}
			else
			{
				script+=" ";
				if(token.empty())
				{
					script+="'";
					script+=token;
					script+="'";
				}
				else
				{
					if(token[0]=='-')
					{
						std::string option=token;
						for(std::size_t j=0;j<option.size();j++)
						{
							if(option[j]=='_')
							{
								option[j]='-';
							}
						}
						script+=option;
					}
					else
					{
						if(token.find('\'')==std::string::npos)
						{
							script+="'";
							script+=token;
							script+="'";
						}
						else if(token.find('"')==std::string::npos)
						{
							script+="\"";
							script+=token;
							script+="\"";
						}
						else
						{
							script+=token;
						}
					}
				}
			}
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

			duk_push_c_function(context_, native_raw_shell, DUK_VARARGS);
			duk_put_global_string(context_, "raw_shell");

			duk_push_c_function(context_, native_raw_voronota, DUK_VARARGS);
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
