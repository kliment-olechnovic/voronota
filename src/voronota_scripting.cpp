#include "common/scripting/script_execution_manager_with_variant_output.h"
#include "common/scripting/json_writer.h"

#include "voronota_scripting.h"

class ScriptingControllerImplementation
{
public:
	ScriptingControllerImplementation()
	{
	}

	virtual ~ScriptingControllerImplementation()
	{
	}

	std::string& run(const std::string& script)
	{
		output_.clear();
		std::ostringstream output;
		common::scripting::JSONWriter::write(
				common::scripting::JSONWriter::Configuration(0),
				manager_.execute_script_and_return_last_output(script, false),
				output);
		output_=output.str();
		return output_;
	}

private:
	common::scripting::ScriptExecutionManagerWithVariantOutput manager_;
	std::string output_;
};

ScriptingController::ScriptingController() : implementation_(new ScriptingControllerImplementation())
{
}

ScriptingController::~ScriptingController()
{
	ScriptingControllerImplementation* ptr=static_cast<ScriptingControllerImplementation*>(implementation_);
	delete ptr;
}

const char* ScriptingController::run(const char* script)
{
	ScriptingControllerImplementation* ptr=static_cast<ScriptingControllerImplementation*>(implementation_);
	return ptr->run(script).c_str();
}

