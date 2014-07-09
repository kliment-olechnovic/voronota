#include <iostream>
#include <functional>
#include <exception>
#include <limits>

#include "apollota/safer_comparison_of_numbers.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/clog_redirector.h"

#include "modes_commons.h"

void calculate_contacts(const auxiliaries::ProgramOptionsHandler& poh);
void calculate_contacts_query(const auxiliaries::ProgramOptionsHandler& poh);
void calculate_vertices(const auxiliaries::ProgramOptionsHandler& poh);
void calculate_vertices_in_parallel(const auxiliaries::ProgramOptionsHandler& poh);
void compare_quadruples_sets(const auxiliaries::ProgramOptionsHandler& poh);
void get_balls_from_atoms_file(const auxiliaries::ProgramOptionsHandler& poh);
void print_demo(const auxiliaries::ProgramOptionsHandler& poh);

int main(const int argc, const char** argv)
{
	typedef std::pointer_to_unary_function<const auxiliaries::ProgramOptionsHandler&, void> ModeFunctionPointer;
	typedef std::map<std::string, ModeFunctionPointer> MapOfModes;

	std::cin.exceptions(std::istream::badbit);
	std::cout.exceptions(std::ostream::badbit);
	std::ios_base::sync_with_stdio(false);

	try
	{
		MapOfModes visible_map_of_modes;
		visible_map_of_modes.insert(MapOfModes::value_type("calculate-vertices", ModeFunctionPointer(calculate_vertices)));
		visible_map_of_modes.insert(MapOfModes::value_type("get-balls-from-atoms-file", ModeFunctionPointer(get_balls_from_atoms_file)));
		visible_map_of_modes.insert(MapOfModes::value_type("calculate-contacts", ModeFunctionPointer(calculate_contacts)));
		visible_map_of_modes.insert(MapOfModes::value_type("calculate-contacts-query", ModeFunctionPointer(calculate_contacts_query)));
		visible_map_of_modes.insert(MapOfModes::value_type("calculate-vertices-in-parallel", ModeFunctionPointer(calculate_vertices_in_parallel)));
		MapOfModes full_map_of_modes=visible_map_of_modes;
		full_map_of_modes.insert(MapOfModes::value_type("compare-quadruples-sets", ModeFunctionPointer(compare_quadruples_sets)));
		full_map_of_modes.insert(MapOfModes::value_type("print-demo", ModeFunctionPointer(print_demo)));

		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions map_of_option_descriptions;
		map_of_option_descriptions["--mode"].init("string", "running mode, which has its own options", true);
		map_of_option_descriptions["--help"].init("", "flag to print usage help to stderr and exit");
		map_of_option_descriptions["--clog-file"].init("string", "path to file for log stream redirection");
		map_of_option_descriptions["--version"].init("", "flag to print version number to stderr and exit");

		auxiliaries::ProgramOptionsHandler poh(argc, argv);

		if(poh.contains_option("--version"))
		{
			std::cerr << "Voronota version 1.2\n";
			return 1;
		}

		const std::string mode=(poh.contains_option_with_argument("--mode") ? poh.argument<std::string>("--mode") : std::string());

		const bool mode_present=!mode.empty();
		const bool mode_valid=full_map_of_modes.count(mode);
		const bool help_present=poh.contains_option("--help");

		if(!mode_present && !help_present)
		{
			std::cerr << "Common options:\n";
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions("", map_of_option_descriptions, std::cerr);
			std::cerr << "Available running modes:\n";
			for(MapOfModes::const_iterator it=visible_map_of_modes.begin();it!=visible_map_of_modes.end();++it)
			{
				std::cerr << "--mode " << it->first << "\n";
			}
			return 1;
		}

		if(!mode_present && help_present)
		{
			std::cerr << "Common options:\n";
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions("", map_of_option_descriptions, std::cerr);
			for(MapOfModes::const_iterator it=visible_map_of_modes.begin();it!=visible_map_of_modes.end();++it)
			{
				std::cerr << "\nMode '" << it->first << "' options\n";
				it->second(poh);
			}
			return 1;
		}

		if(mode_present && !mode_valid)
		{
			std::cerr << "Invalid mode, available running modes are:\n";
			for(MapOfModes::const_iterator it=visible_map_of_modes.begin();it!=visible_map_of_modes.end();++it)
			{
				std::cerr << "--mode " << it->first << "\n";
			}
			return 1;
		}

		if(mode_present && mode_valid && help_present)
		{
			if(visible_map_of_modes.count(mode)>0)
			{
				std::cerr << "Mode '" << mode << "' options:\n";
				visible_map_of_modes.find(mode)->second(poh);
			}
			return 1;
		}

		if(mode_valid)
		{
			poh.remove_option("--mode");

			const std::string clog_filename=poh.argument<std::string>("--clog-file", "");
			poh.remove_option("--clog-file");

			auxiliaries::CLogRedirector clog_redirector(clog_filename);

			full_map_of_modes.find(mode)->second(poh);

			return 0;
		}
	}
	catch(const auxiliaries::ProgramOptionsHandler::Exception& e)
	{
		std::cerr << "\nInvalid parameters: " << (e.what()) << "\n";
		std::cerr << std::endl;
	}
	catch(const std::exception& e)
	{
		std::cerr << "\nException caught: " << (e.what()) << "\n";
		std::cerr << std::endl;
	}
	catch(...)
	{
		std::cerr << "\nUnknown exception caught.\n";
		std::cerr << std::endl;
	}

	return 2;
}
