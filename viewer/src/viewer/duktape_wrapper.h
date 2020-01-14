#ifndef VIEWER_DUKTAPE_WRAPPER_H_
#define VIEWER_DUKTAPE_WRAPPER_H_

#ifdef FOR_WEB
namespace voronota
{

namespace viewer
{

namespace duktape
{

inline void eval(scripting::ScriptExecutionManagerWithVariantOutput&, const std::string&)
{
}

}

}

}
#else

#include "../duktape/duktape.h"

#include "../../../src/scripting/script_execution_manager_with_variant_output.h"

namespace voronota
{

namespace viewer
{

namespace duktape
{

class ContextWrapper
{
public:
	static void set_script_execution_manager(scripting::ScriptExecutionManagerWithVariantOutput& sem)
	{
		instance().sem_=&sem;
	}

	static duk_context* get_context()
	{
		instance().ensure();
		return instance().context_;
	}

private:
	ContextWrapper() : context_(0), sem_(0)
	{
	}

	~ContextWrapper()
	{
		if(context_!=0)
		{
			duk_destroy_heap(context_);
		}
	}

	static ContextWrapper& instance()
	{
		static ContextWrapper context_wrapper;
		return context_wrapper;
	}

	static duk_ret_t native_print(duk_context *ctx)
	{
		duk_push_string(ctx, " ");
		duk_insert(ctx, 0);
		duk_join(ctx, duk_get_top(ctx) - 1);
		std::cout << duk_safe_to_string(ctx, -1) << std::endl;
		return 0;
	}

	static int native_v_do(duk_context *ctx)
	{
		duk_push_string(ctx, " ");
		duk_insert(ctx, 0);
		duk_join(ctx, duk_get_top(ctx) - 1);
		const std::string script=duk_safe_to_string(ctx, -1);
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

			duk_push_c_function(context_, native_v_do, DUK_VARARGS);
			duk_put_global_string(context_, "v_do");
		}
	}

	duk_context* context_;
	scripting::ScriptExecutionManagerWithVariantOutput* sem_;
};

inline void eval(scripting::ScriptExecutionManagerWithVariantOutput& sem, const std::string& script)
{
	ContextWrapper::set_script_execution_manager(sem);
	if(duk_peval_string(ContextWrapper::get_context(), script.c_str())!=0)
	{
		std::cerr << "JS error: " << duk_safe_to_string(ContextWrapper::get_context(), -1) << std::endl;
	}
	else
	{
		std::cerr << "JS: " << duk_safe_to_string(ContextWrapper::get_context(), -1) << std::endl;
	}
	duk_pop(ContextWrapper::get_context());
}

}

}

}

#endif /* FOR_WEB */

#endif /* VIEWER_DUKTAPE_WRAPPER_H_ */
