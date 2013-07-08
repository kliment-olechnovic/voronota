#include <iostream>
#include <functional>
#include <exception>
#include <limits>

#include "apollota/safe_comparison_of_numbers.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/clog_redirector.h"

void calculate_triangulation(const auxiliaries::ProgramOptionsHandler& poh);
void compare_triangulations(const auxiliaries::ProgramOptionsHandler& poh);
void get_balls_from_pdb_file(const auxiliaries::ProgramOptionsHandler& poh);

int main(const int argc, const char** argv)
{
	typedef std::pointer_to_unary_function<const auxiliaries::ProgramOptionsHandler&, void> ModeFunctionPointer;
	typedef std::map<std::string, ModeFunctionPointer> ModesMap;

	std::cin.exceptions(std::istream::badbit);
	std::cout.exceptions(std::ostream::badbit);
	std::ios_base::sync_with_stdio(false);

	std::string mode;

	try
	{
		ModesMap modes_map;
		modes_map["calculate-triangulation"]=ModeFunctionPointer(calculate_triangulation);
		modes_map["compare-triangulations"]=ModeFunctionPointer(compare_triangulations);
		modes_map["get-balls-from-pdb-file"]=ModeFunctionPointer(get_balls_from_pdb_file);

		auxiliaries::ProgramOptionsHandler poh(argc, argv);

		if(poh.contains_option("--help"))
		{
			auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions map_of_option_descriptions;
			map_of_option_descriptions["--mode"].init("string", "running mode");
			map_of_option_descriptions["--clog-file"].init("string", "path to file for log stream redirection");
			map_of_option_descriptions["--epsilon"].init("number", "threshold for floating-point numbers comparison");

			std::cerr << "\nCommon options\n";
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions(map_of_option_descriptions, std::cerr);
			std::cerr << "\n";
			for(ModesMap::const_iterator it=modes_map.begin();it!=modes_map.end();++it)
			{
				std::cerr << "--mode " << it->first << "\n";
				it->second(poh);
				std::cerr << "\n";
			}

			return 1;
		}
		else
		{
			mode=poh.argument<std::string>("--mode", "");
			poh.remove_option("--mode");

			if(modes_map.count(mode)==1)
			{
				const std::string clog_filename=poh.argument<std::string>("--clog-file", "");
				poh.remove_option("--clog-file");
				auxiliaries::CLogRedirector clog_redirector;
				if(!clog_filename.empty() && !clog_redirector.init(clog_filename))
				{
					std::cerr << "Failed to redirect clog to file: " << clog_filename << "." << std::endl;
					return 1;
				}

				const double epsilon=poh.argument<double>("--epsilon", -1.0);
				poh.remove_option("--epsilon");
				if(epsilon>0.0)
				{
					apollota::comparison_epsilon_reference()=epsilon;
				}

				modes_map.find(mode)->second(poh);
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
