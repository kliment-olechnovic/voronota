#ifndef VIEWER_ENVIRONMENT_H_
#define VIEWER_ENVIRONMENT_H_

#include "../../../src/scripting/json_writer.h"

#ifdef FOR_WEB
#include <emscripten.h>
#else
//
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
		emscripten_run_script(script.c_str());
#else
		std::cerr << "Not running '" << script << "'." << std::endl;
#endif
	}

	static void print(const std::string& text)
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

	static void print(const scripting::VariantObject& obj)
	{
#ifdef FOR_WEB
		print(scripting::JSONWriter::write(scripting::JSONWriter::Configuration(6), obj));
#else
		print(scripting::JSONWriter::write(scripting::JSONWriter::Configuration(2), obj));
#endif
	}
};

}

}

#endif /* VIEWER_ENVIRONMENT_H_ */
