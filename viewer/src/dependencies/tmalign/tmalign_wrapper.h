#ifndef DEPENDENCIES_TMALIGN_TMALIGN_WRAPPER_H_
#define DEPENDENCIES_TMALIGN_TMALIGN_WRAPPER_H_

#include <iostream>
#include <sstream>

int main_of_tmalign(int argc, const char** argv);

class TMAlignWrapper
{
public:
	struct ResultBundle
	{
		int exit_code;
		std::string stdout;
		std::string stderr;
	};

	static ResultBundle run_tmalign(const std::string& input_target_file, const std::string& input_model_file, const std::string& output_matrix_file)
	{
		StandardOutputRedirector sor;

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

		ResultBundle result;
		result.exit_code=main_of_tmalign(static_cast<int>(argv.size()), argv.data());
		result.stdout=sor.cout_output();
		result.stderr=sor.cerr_output();

		return result;
	}

	static ResultBundle run_tmalign(const std::string& input_target_file, const std::string& input_model_file)
	{
		return run_tmalign(input_target_file, input_model_file, std::string());
	}

private:
	class StandardOutputRedirector
	{
	public:
		StandardOutputRedirector() :
			cout_buf_(std::cout.rdbuf()),
			cerr_buf_(std::cerr.rdbuf())
		{
			std::cout.rdbuf(cout_out_.rdbuf());
			std::cerr.rdbuf(cerr_out_.rdbuf());
		}

		~StandardOutputRedirector()
		{
			std::cout.rdbuf(cout_buf_);
			std::cerr.rdbuf(cerr_buf_);
		}

		std::string cout_output() const
		{
			return cout_out_.str();
		}

		std::string cerr_output() const
		{
			return cerr_out_.str();
		}

	private:
		std::streambuf* cout_buf_;
		std::streambuf* cerr_buf_;
		std::ostringstream cout_out_;
		std::ostringstream cerr_out_;
	};
};

#endif /* DEPENDENCIES_TMALIGN_TMALIGN_WRAPPER_H_ */
