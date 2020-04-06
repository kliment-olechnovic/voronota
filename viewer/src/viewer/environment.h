#ifndef VIEWER_ENVIRONMENT_H_
#define VIEWER_ENVIRONMENT_H_

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
};

}

}

#endif /* VIEWER_ENVIRONMENT_H_ */
