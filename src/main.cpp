#include <iostream>
#include <functional>
#include <exception>
#include <limits>
#include <vector>
#include <algorithm>

#include "apollota/safer_comparison_of_numbers.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/clog_redirector.h"

#include "modes_commons.h"

void get_balls_from_atoms_file(const auxiliaries::ProgramOptionsHandler& poh);
void calculate_vertices(const auxiliaries::ProgramOptionsHandler& poh);
void calculate_vertices_in_parallel(const auxiliaries::ProgramOptionsHandler& poh);
void calculate_contacts(const auxiliaries::ProgramOptionsHandler& poh);
void calculate_contacts_query(const auxiliaries::ProgramOptionsHandler& poh);

struct ModeDescriptor
{
	typedef std::pointer_to_unary_function<const auxiliaries::ProgramOptionsHandler&, void> FunctionPtr;

	std::string name;
	FunctionPtr func_ptr;

	ModeDescriptor(const std::string& name, const FunctionPtr& func_ptr) : name(name), func_ptr(func_ptr)
	{
	}

	bool operator==(const std::string& check_name) const
	{
		return (check_name==name);
	}
};

int main(const int argc, const char** argv)
{
	std::cin.exceptions(std::istream::badbit);
	std::cout.exceptions(std::ostream::badbit);
	std::ios_base::sync_with_stdio(false);

	try
	{
		std::vector<ModeDescriptor> list_of_modes;
		{
			list_of_modes.push_back(ModeDescriptor("get-balls-from-atoms-file", ModeDescriptor::FunctionPtr(get_balls_from_atoms_file)));
			list_of_modes.push_back(ModeDescriptor("calculate-vertices", ModeDescriptor::FunctionPtr(calculate_vertices)));
			list_of_modes.push_back(ModeDescriptor("calculate-vertices-in-parallel", ModeDescriptor::FunctionPtr(calculate_vertices_in_parallel)));
			list_of_modes.push_back(ModeDescriptor("calculate-contacts", ModeDescriptor::FunctionPtr(calculate_contacts)));
			list_of_modes.push_back(ModeDescriptor("calculate-contacts-query", ModeDescriptor::FunctionPtr(calculate_contacts_query)));
		}

		std::vector<auxiliaries::ProgramOptionsHandler::OptionDescription> list_of_option_descriptions;
		{
			typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
			list_of_option_descriptions.push_back(OD("--mode", "string", "running mode, which has its own options", true));
			list_of_option_descriptions.push_back(OD("--help", "", "flag to print usage help to stderr and exit"));
			list_of_option_descriptions.push_back(OD("--clog-file", "string", "path to file for log stream redirection"));
			list_of_option_descriptions.push_back(OD("--version", "", "flag to print version number to stderr and exit"));
		}

		auxiliaries::ProgramOptionsHandler poh(argc, argv);

		if(poh.contains_option("--version"))
		{
			std::cerr << "Voronota version 1.3\n";
			return 1;
		}

		const std::string mode=(poh.contains_option_with_argument("--mode") ? poh.argument<std::string>("--mode") : std::string());
		const bool help_present=poh.contains_option("--help");

		if(!mode.empty())
		{
			if(std::count(list_of_modes.begin(), list_of_modes.end(), mode)>0)
			{
				if(!help_present)
				{
					poh.remove_option("--mode");

					const std::string clog_filename=poh.argument<std::string>("--clog-file", "");
					poh.remove_option("--clog-file");

					auxiliaries::CLogRedirector clog_redirector(clog_filename);

					std::find(list_of_modes.begin(), list_of_modes.end(), mode)->func_ptr(poh);

					return 0;
				}
				else
				{
					std::cerr << "Mode '" << mode << "' options:\n";
					std::find(list_of_modes.begin(), list_of_modes.end(), mode)->func_ptr(poh);
				}
			}
			else
			{
				std::cerr << "Invalid mode, available running modes are:\n";
				for(std::vector<ModeDescriptor>::const_iterator it=list_of_modes.begin();it!=list_of_modes.end();++it)
				{
					std::cerr << "--mode " << it->name << "\n";
				}
			}
		}
		else
		{
			std::cerr << "Common options:\n";
			auxiliaries::ProgramOptionsHandler::print_list_of_option_descriptions("", list_of_option_descriptions, std::cerr);
			if(!help_present)
			{
				std::cerr << "\nAvailable running modes:\n";
				for(std::vector<ModeDescriptor>::const_iterator it=list_of_modes.begin();it!=list_of_modes.end();++it)
				{
					std::cerr << "--mode " << it->name << "\n";
				}
			}
			else
			{
				for(std::vector<ModeDescriptor>::const_iterator it=list_of_modes.begin();it!=list_of_modes.end();++it)
				{
					std::cerr << "\nMode '" << it->name << "' options:\n";
					it->func_ptr(poh);
				}
			}
		}

		return 1;
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
