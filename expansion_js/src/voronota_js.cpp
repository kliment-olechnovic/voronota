#include "duktaper/standalone_run.h"

#include "../../src/voronota_version.h"

namespace
{

bool check_option_name(const std::string& arg_str, const std::string& option_name)
{
	return (arg_str==(std::string("--")+option_name) || arg_str==(std::string("-")+option_name));
}

}

int main(const int argc, const char** argv)
{
	try
	{
		bool no_setup_defaults=false;
		std::vector<std::string> command_args;
		for(int i=1;i<argc;i++)
		{
			const std::string arg_str(argv[i]);
			if(check_option_name(arg_str, "version"))
			{
				std::cout << "Voronota-JS version " << voronota::version() << std::endl;
				return 0;
			}
			else if(check_option_name(arg_str, "no-setup-defaults"))
			{
				no_setup_defaults=true;
			}
			else
			{
				command_args.push_back(arg_str);
			}
		}
		if(command_args.empty())
		{
			command_args.push_back("-");
		}

		voronota::duktaper::StandaloneRun::run(no_setup_defaults, command_args);

		return 0;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Exception caught: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "Unknown exception caught." << std::endl;
	}

	return 1;
}

