#include "duktaper/standalone_run.h"

int main(const int argc, const char** argv)
{
	try
	{
		std::vector<std::string> command_args;
		for(int i=1;i<argc;i++)
		{
			command_args.push_back(argv[i]);
		}
		if(command_args.empty())
		{
			command_args.push_back("-");
		}

		voronota::duktaper::StandaloneRun::run(command_args);

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

