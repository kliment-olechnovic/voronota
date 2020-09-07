#ifndef DUKTAPER_CALL_SHELL_UTILITIES_H_
#define DUKTAPER_CALL_SHELL_UTILITIES_H_

#include "operators/call_shell.h"

namespace voronota
{

namespace duktaper
{

class CallShellUtilities
{
public:
	class TemporaryDirectory
	{
	public:
		TemporaryDirectory()
		{
			operators::CallShell::Result r=operators::CallShell().init(CMDIN().set("command-string", "mktemp -d")).run(0);
			if(r.exit_status!=0 || r.stdout_str.empty())
			{
				throw std::runtime_error(std::string("Failed to create temporary directory."));
			}
			dir_path_=r.stdout_str;
			while(!dir_path_.empty() && dir_path_[dir_path_.size()-1]<=' ')
			{
				dir_path_.pop_back();
			}
		}

		~TemporaryDirectory()
		{
			if(!dir_path_.empty())
			{
				std::ostringstream command_output;
				command_output << "rm -r '" << dir_path_ << "'";
				operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0);
			}
		}

		const std::string& dir_path() const
		{
			return dir_path_;
		}

	private:
		std::string dir_path_;
	};

	static bool test_if_file_not_empty(const std::string& file_path)
	{
		std::ostringstream command_output;
		command_output << "test -s '" << file_path << "'";
		return (operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0).exit_status==0);
	}

	static bool create_directory(const std::string& dir_path)
	{
		std::ostringstream command_output;
		command_output << "mkdir -p '" << dir_path << "'";
		return (operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0).exit_status==0);
	}

	static bool test_if_shell_command_available(const std::string& shell_command)
	{
		std::ostringstream command_output;
		command_output << "command -v '" << shell_command << "'";
		return (operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0).exit_status==0);
	}
};

}

}


#endif /* DUKTAPER_CALL_SHELL_UTILITIES_H_ */
