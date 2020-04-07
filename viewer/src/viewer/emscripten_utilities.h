#ifndef VIEWER_EMSCRIPTEN_UTILITIES_H_
#define VIEWER_EMSCRIPTEN_UTILITIES_H_

#ifdef FOR_WEB
#include <emscripten.h>

namespace voronota
{

namespace viewer
{

class EnscriptenUtilities
{
public:
	static void execute_javascript(const std::string& script)
	{

		std::string wrapped_script;
		wrapped_script+="try {\n var f=new Function(\"";
		wrapped_script+=scripting::JSONWriter::replace_special_characters_with_escape_sequences(script);
		wrapped_script+="\");\n f();\n}\ncatch(err) {\n console.log('Error in delegated JS script: '+err.message);\n}\n";
		emscripten_run_script(wrapped_script.c_str());
	}
};

}

}

#endif

#endif /* VIEWER_EMSCRIPTEN_UTILITIES_H_ */
