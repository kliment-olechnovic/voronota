#ifndef VIEWER_DUKTAPE_WRAPPER_H_
#define VIEWER_DUKTAPE_WRAPPER_H_

#include "../duktape/duktape.h"

#include "../../../src/scripting/script_execution_manager_with_variant_output.h"

namespace voronota
{

namespace duktaper
{

class DuktapeContextWrapper
{
public:
	static bool eval(scripting::ScriptExecutionManagerWithVariantOutput& sem, const std::string& script, std::ostream& stdout, std::ostream& stderr, const bool print_results)
	{
		if(script.empty())
		{
			return false;
		}
		set_script_execution_manager(sem);
		set_stdout(stdout);
		set_stderr(stderr);
		const bool success=(duk_peval_string(get_context(), script.c_str())==0);
		if(!success)
		{
			stderr << "Eval error: " << duk_safe_to_string(get_context(), -1) << std::endl;
		}
		else if(print_results)
		{
			stderr << "Eval result: " << duk_safe_to_string(get_context(), -1) << std::endl;
		}
		duk_pop(get_context());
		return success;
	}

private:
	DuktapeContextWrapper() : context_(0), sem_(0), stdout_(0), stderr_(0)
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

	static duk_ret_t native_print(duk_context *ctx)
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

	static int native_raw_voronota(duk_context *ctx)
	{
		const int N=duk_get_top(ctx);
		std::string script;
		for(int i=0;i<N;i++)
		{
			std::string token=duk_safe_to_string(ctx, i);
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
						script+=token;
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

			duk_push_c_function(context_, native_print, DUK_VARARGS);
			duk_put_global_string(context_, "print");

			duk_push_c_function(context_, native_raw_voronota, DUK_VARARGS);
			duk_put_global_string(context_, "raw_voronota");

			std::string init_script=""
					"raw_voronota_named=function(name, args)"
					"{"
					"  var fullargs=new Array(args.length+1);"
					"  fullargs[0]=name;"
					"  for(var i=0;i<args.length;++i)"
					"  {"
					"    fullargs[i+1]=args[i];"
					"  }"
					"  return raw_voronota.apply(null, fullargs);"
					"}\n";

			duk_peval_string_noresult(context_, init_script.c_str());
		}
	}

	duk_context* context_;
	scripting::ScriptExecutionManagerWithVariantOutput* sem_;
	std::ostream* stdout_;
	std::ostream* stderr_;
};

}

}

#endif /* VIEWER_DUKTAPE_WRAPPER_H_ */
