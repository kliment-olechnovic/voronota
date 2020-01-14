#include <iostream>

#include "duktaper/duktape_wrapper.h"

int main(const int /*argc*/, const char** /*argv*/)
{
	voronota::scripting::ScriptExecutionManagerWithVariantOutput execution_manager;
	std::istreambuf_iterator<char> eos;
	std::string script(std::istreambuf_iterator<char>(std::cin), eos);
	voronota::duktaper::DuktapeContextWrapper::eval(execution_manager, script, std::cout, std::cerr, false);
	return 0;
}
