#ifndef DEPENDENCIES_TMALIGN_TMALIGN_WRAPPER_H_
#define DEPENDENCIES_TMALIGN_TMALIGN_WRAPPER_H_

#include "../../../../src/scripting/io_selectors.h"

namespace TMalign
{
	int main_of_tmalign(int argc, const char** argv, voronota::scripting::StandardOutputMockup& som);
}

class TMAlignWrapper
{
public:
	struct ResultBundle
	{
		int exit_code;
		std::string stdout_str;
		std::string stderr_str;
	};

	static ResultBundle run_tmalign(const std::string& input_target_file, const std::string& input_model_file, const std::string& output_matrix_file)
	{
		const std::string program_name="TMalign";
		const std::string matrix_option="-m";

		std::vector<const char*> argv;
		argv.push_back(program_name.c_str());
		argv.push_back(input_model_file.c_str());
		argv.push_back(input_target_file.c_str());
		if(!output_matrix_file.empty())
		{
			argv.push_back(matrix_option.c_str());
			argv.push_back(output_matrix_file.c_str());
		}

		voronota::scripting::StandardOutputMockup som;
		ResultBundle result;
		result.exit_code=TMalign::main_of_tmalign(static_cast<int>(argv.size()), argv.data(), som);
		result.stdout_str=som.cout_output();
		result.stderr_str=som.cerr_output();

		return result;
	}

	static ResultBundle run_tmalign(const std::string& input_target_file, const std::string& input_model_file)
	{
		return run_tmalign(input_target_file, input_model_file, std::string());
	}
};

#endif /* DEPENDENCIES_TMALIGN_TMALIGN_WRAPPER_H_ */
