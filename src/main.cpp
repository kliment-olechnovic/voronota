#include <iostream>
#include <functional>
#include <exception>
#include <limits>
#include <vector>
#include <algorithm>

#include "apollota/safer_comparison_of_numbers.h"

#include "auxiliaries/program_options_handler.h"

void get_balls_from_atoms_file(const auxiliaries::ProgramOptionsHandler&);
void calculate_vertices(const auxiliaries::ProgramOptionsHandler&);
void calculate_vertices_in_parallel(const auxiliaries::ProgramOptionsHandler&);
void calculate_contacts(const auxiliaries::ProgramOptionsHandler&);
void query_balls(const auxiliaries::ProgramOptionsHandler&);
void query_contacts(const auxiliaries::ProgramOptionsHandler&);

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

std::vector<ModeDescriptor> get_list_of_modes()
{
	std::vector<ModeDescriptor> list_of_modes;
	list_of_modes.push_back(ModeDescriptor("get-balls-from-atoms-file", ModeDescriptor::FunctionPtr(get_balls_from_atoms_file)));
	list_of_modes.push_back(ModeDescriptor("calculate-vertices", ModeDescriptor::FunctionPtr(calculate_vertices)));
	list_of_modes.push_back(ModeDescriptor("calculate-vertices-in-parallel", ModeDescriptor::FunctionPtr(calculate_vertices_in_parallel)));
	list_of_modes.push_back(ModeDescriptor("calculate-contacts", ModeDescriptor::FunctionPtr(calculate_contacts)));
	list_of_modes.push_back(ModeDescriptor("query-balls", ModeDescriptor::FunctionPtr(query_balls)));
	list_of_modes.push_back(ModeDescriptor("query-contacts", ModeDescriptor::FunctionPtr(query_contacts)));
	return list_of_modes;
}

int main(const int argc, const char** argv)
{
	std::cin.exceptions(std::istream::badbit);
	std::cout.exceptions(std::ostream::badbit);
	std::ios_base::sync_with_stdio(false);

	try
	{
		const std::vector<ModeDescriptor> list_of_modes=get_list_of_modes();

		const auxiliaries::ProgramOptionsHandler poh(argc, argv);
		const std::string mode=(poh.original_arg(1));
		const bool help=poh.contains_option("--help");

		if(!mode.empty() && std::count(list_of_modes.begin(), list_of_modes.end(), mode)>0)
		{
			if(!help)
			{
				std::find(list_of_modes.begin(), list_of_modes.end(), mode)->func_ptr(poh);
				return 0;
			}
			else
			{
				std::find(list_of_modes.begin(), list_of_modes.end(), mode)->func_ptr(poh);
			}
		}
		else
		{
			std::cerr << "Voronota version 1.6\n\n";
			std::cerr << "Commands:\n\n";
			for(std::vector<ModeDescriptor>::const_iterator it=list_of_modes.begin();it!=list_of_modes.end();++it)
			{
				std::cerr << it->name << "\n";
			}
			std::cerr << "\n";
			if(help)
			{
				for(std::vector<ModeDescriptor>::const_iterator it=list_of_modes.begin();it!=list_of_modes.end();++it)
				{
					std::cerr << "Command '" << it->name << "' options:\n";
					it->func_ptr(poh);
					std::cerr << "\n";
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
