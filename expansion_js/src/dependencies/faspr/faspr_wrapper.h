#ifndef DEPENDENCIES_FASPR_FASPR_WRAPPER_H_
#define DEPENDENCIES_FASPR_FASPR_WRAPPER_H_

#include "faspr_config.h"

#include "../../../../src/scripting/io_selectors.h"

namespace FASPR
{
	int main_of_faspr(int argc, const char** argv, const FASPRConfig& faspr_config, voronota::scripting::StandardOutputMockup& som);
}

class FASPRWrapper
{
public:
	struct ResultBundle
	{
		int exit_code;
		std::string stdout_str;
		std::string stderr_str;
	};

	static ResultBundle run_faspr(const FASPRConfig& faspr_config, const std::string& input_file, const std::string& output_file)
	{
		const std::string program_name="FASPR";
		const std::string input_option="-i";
		const std::string output_option="-o";

		std::vector<const char*> argv;
		argv.push_back(program_name.c_str());
		argv.push_back(input_option.c_str());
		argv.push_back(input_file.c_str());
		argv.push_back(output_option.c_str());
		argv.push_back(output_file.c_str());

		voronota::scripting::StandardOutputMockup som;
		ResultBundle result;
		result.exit_code=FASPR::main_of_faspr(static_cast<int>(argv.size()), argv.data(), faspr_config, som);
		result.stdout_str=som.cout_output();
		result.stderr_str=som.cerr_output();

		return result;
	}
};

#endif /* DEPENDENCIES_FASPR_FASPR_WRAPPER_H_ */

