#ifndef VIEWER_ENVIRONMENT_H_
#define VIEWER_ENVIRONMENT_H_

#include "../../../src/scripting/script_execution_manager_with_variant_output.h"
#include "../../../src/scripting/binding_javascript.h"

#ifdef FOR_WEB
#include <emscripten.h>
#else
#include "../duktaper/duktape_manager.h"
#endif

namespace voronota
{

namespace viewer
{

class Environment
{
public:
	static void execute_javascript(const std::string& script)
	{
#ifdef FOR_WEB
		std::string wrapped_script;
		wrapped_script+="try {\n var f=new Function(\"";
		wrapped_script+=scripting::JSONWriter::replace_special_characters_with_escape_sequences(script);
		wrapped_script+="\");\n f();\n}\ncatch(err) {\n console.log('Error in delegated JS script: '+err.message);\n}\n";
		emscripten_run_script(wrapped_script.c_str());
#else
		duktaper::DuktapeManager::eval(script);
#endif
	}

	static void setup_javascript_bindings(scripting::ScriptExecutionManagerWithVariantOutput& sem)
	{
#ifdef FOR_WEB
		execute_javascript(scripting::BindingJavascript::generate_setup_script(sem.collection_of_command_documentations()));
#else
		duktaper::DuktapeManager::set_script_execution_manager(sem);
		execute_javascript(scripting::BindingJavascript::generate_setup_script(sem.collection_of_command_documentations()));
#endif
	}

	static void print_log(const std::string& text)
	{
#ifdef FOR_WEB
		std::string script;
		script+="set_output_area_text(\"";
		script+=scripting::JSONWriter::replace_special_characters_with_escape_sequences(text);
		script+="\");";
		execute_javascript(script);
#else
		std::cerr << text << std::endl;
#endif
	}

	static void print_log(const scripting::VariantObject& obj)
	{
#ifdef FOR_WEB
		print_log(scripting::JSONWriter::write(scripting::JSONWriter::Configuration(6), obj));
#else
		print_log(scripting::JSONWriter::write(scripting::JSONWriter::Configuration(2), obj));
#endif
	}
};

}

}

#endif /* VIEWER_ENVIRONMENT_H_ */
