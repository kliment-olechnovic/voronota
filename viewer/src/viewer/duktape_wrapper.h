#ifndef VIEWER_DUKTAPE_WRAPPER_H_
#define VIEWER_DUKTAPE_WRAPPER_H_

#ifdef FOR_WEB
namespace voronota
{

namespace viewer
{

namespace duktape
{

inline void eval(const std::string&)
{
}

}

}

}
#else

#include "../duktape/duktape.h"

#include "script_execution_manager.h"

namespace voronota
{

namespace viewer
{

namespace duktape
{

class ContextWrapper
{
public:
	static duk_context* global_context()
	{
		static ContextWrapper context_wrapper;
		return context_wrapper.context();
	}

private:
	ContextWrapper() : context_(0)
	{
	}

	~ContextWrapper()
	{
		if(context_!=0)
		{
			duk_destroy_heap(context_);
		}
	}

	static duk_ret_t native_print(duk_context *ctx)
	{
		duk_push_string(ctx, " ");
		duk_insert(ctx, 0);
		duk_join(ctx, duk_get_top(ctx) - 1);
		std::cout << duk_safe_to_string(ctx, -1) << std::endl;
		return 0;
	}

	static int native_execute_command(duk_context *ctx)
	{
		std::string command(duk_require_string(ctx, 0));
		std::string result;
		{
			ScriptExecutionManager* sem=ScriptExecutionManager::instance();
			if(sem!=0)
			{
				result=sem->execute_command(command);
			}
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

			duk_push_c_function(context_, native_execute_command, 1);
			duk_put_global_string(context_, "v_do");
		}
	}

	duk_context* context_;
};

inline void eval(const std::string& script)
{
	if(duk_peval_string(ContextWrapper::global_context(), script.c_str())!=0)
	{
		std::cerr << "JS error: " << duk_safe_to_string(ContextWrapper::global_context(), -1) << std::endl;
	}
	else
	{
		std::cerr << "JS: " << duk_safe_to_string(ContextWrapper::global_context(), -1) << std::endl;
	}
	duk_pop(ContextWrapper::global_context());
}

}

}

}

#endif /* FOR_WEB */

#endif /* VIEWER_DUKTAPE_WRAPPER_H_ */
