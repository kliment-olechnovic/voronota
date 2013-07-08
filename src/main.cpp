#include <iostream>
#include <functional>
#include <exception>
#include <limits>

#include "apollota/safe_comparison_of_numbers.h"

#include "auxiliaries/command_line_options_handler.h"
#include "auxiliaries/clog_redirector.h"

void calculate_triangulation(const auxiliaries::CommandLineOptionsHandler& clo, const bool print_help);
void compare_triangulations(const auxiliaries::CommandLineOptionsHandler& clo, const bool print_help);
void get_balls_from_pdb_file(const auxiliaries::CommandLineOptionsHandler& clo, const bool print_help);

int main(const int argc, const char** argv)
{
	std::cin.exceptions(std::istream::badbit);
	std::cout.exceptions(std::ostream::badbit);
	std::ios_base::sync_with_stdio(false);

	std::string mode;

	try
	{
		typedef std::pointer_to_binary_function<const auxiliaries::CommandLineOptionsHandler&, const bool, void> ModeFunctionPointer;
		typedef std::map<std::string, ModeFunctionPointer> ModesMap;
		ModesMap modes_map;
		modes_map["calculate-triangulation"]=ModeFunctionPointer(calculate_triangulation);
		modes_map["compare-triangulations"]=ModeFunctionPointer(compare_triangulations);
		modes_map["get-balls-from-pdb-file"]=ModeFunctionPointer(get_balls_from_pdb_file);

		auxiliaries::CommandLineOptionsHandler clo(argc, argv);

		const bool help=clo.contains_option("--help");
		clo.remove_option("--help");

		if(help)
		{
			for(ModesMap::const_iterator it=modes_map.begin();it!=modes_map.end();++it)
			{
				std::cerr << "  --mode " << it->first << "\n";
				it->second(clo, true);
			}
		}
		else
		{
			mode=clo.argument<std::string>("--mode", "");
			clo.remove_option("--mode");

			if(modes_map.count(mode)==1)
			{
				const std::string clog_filename=clo.argument<std::string>("--clog-file", "");
				clo.remove_option("--clog-file");
				auxiliaries::CLogRedirector clog_redirector;
				if(!clog_filename.empty() && !clog_redirector.init(clog_filename))
				{
					std::cerr << "Failed to redirect clog to file: " << clog_filename << "." << std::endl;
					return 1;
				}

				const double epsilon=clo.argument<double>("--epsilon", -1.0);
				clo.remove_option("--epsilon");
				if(epsilon>0.0)
				{
					apollota::comparison_epsilon_reference()=epsilon;
				}

				modes_map.find(mode)->second(clo, false);
			}
			else
			{
				std::cerr << "Unspecified running mode. Available modes are:" << std::endl;
				for(ModesMap::const_iterator it=modes_map.begin();it!=modes_map.end();++it)
				{
					std::cerr << "  --mode " << it->first << std::endl;
				}
				return 1;
			}
		}
	}
	catch(const std::exception& e)
	{
		if(mode.empty())
		{
			std::cerr << "Exception was caught: ";
		}
		else
		{
			std::cerr << "Operation '" << mode << "' was not successful because exception was caught: ";
		}
		std::cerr << "[" << (e.what()) << "]." << std::endl;
		return 1;
	}
	catch(...)
	{
		std::cerr << "Unknown exception caught." << std::endl;
		return 1;
	}

	std::cin.ignore(std::numeric_limits<std::streamsize>::max());

	return 0;
}
