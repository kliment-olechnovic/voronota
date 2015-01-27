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
void score_contacts_potential(const auxiliaries::ProgramOptionsHandler&);
void score_contacts_energy(const auxiliaries::ProgramOptionsHandler&);
void score_contacts_quality(const auxiliaries::ProgramOptionsHandler&);
void compare_contacts(const auxiliaries::ProgramOptionsHandler&);
void score_scores(const auxiliaries::ProgramOptionsHandler&);

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
	list_of_modes.push_back(ModeDescriptor("score-contacts-potential", ModeDescriptor::FunctionPtr(score_contacts_potential)));
	list_of_modes.push_back(ModeDescriptor("score-contacts-energy", ModeDescriptor::FunctionPtr(score_contacts_energy)));
	list_of_modes.push_back(ModeDescriptor("score-contacts-quality", ModeDescriptor::FunctionPtr(score_contacts_quality)));
	list_of_modes.push_back(ModeDescriptor("compare-contacts", ModeDescriptor::FunctionPtr(compare_contacts)));
	list_of_modes.push_back(ModeDescriptor("score-scores", ModeDescriptor::FunctionPtr(score_scores)));
	return list_of_modes;
}

std::string version()
{
	static const std::string str="Voronota version 1.9";
	return str;
}

void print_error_message(const std::string& mode, const std::string& message)
{
	std::cerr << version();
	if(!mode.empty())
	{
		std::cerr << " command '" << mode << "'";
	}
	std::cerr << " exit error: " << message;
	std::cerr << std::endl;
}

int main(const int argc, const char** argv)
{
	const std::string mode=(argc>1 ? std::string(argv[1]) : std::string());

	std::cin.exceptions(std::istream::badbit);
	std::cout.exceptions(std::ostream::badbit);
	std::ios_base::sync_with_stdio(false);

	try
	{
		const std::vector<ModeDescriptor> list_of_modes=get_list_of_modes();

		auxiliaries::ProgramOptionsHandler poh(argc, argv);

		{
			const std::string output_precision_option_name="--stdout-precision";
			if(poh.contains_option_with_argument(output_precision_option_name))
			{
				std::cout << std::fixed;
				std::cout.precision(poh.argument<int>(output_precision_option_name));
				poh.remove_option(output_precision_option_name);
			}
		}

		const bool help=poh.contains_option("--help");

		if(!mode.empty() && std::count(list_of_modes.begin(), list_of_modes.end(), mode)>0)
		{
			std::find(list_of_modes.begin(), list_of_modes.end(), mode)->func_ptr(poh);
			if(!help)
			{
				return 0;
			}
		}
		else
		{
			std::cerr << version() << "\n\n";
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
		print_error_message(mode, e.what());
	}
	catch(const std::exception& e)
	{
		print_error_message(mode, e.what());
	}
	catch(...)
	{
		print_error_message(mode, "Unknown exception caught.");
	}

	return 2;
}
