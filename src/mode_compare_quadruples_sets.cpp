#include <iostream>
#include <fstream>

#include "apollota/comparison_of_triangulations.h"

#include "modes_commons.h"

namespace
{

bool add_quadruple_from_stream_to_vector(std::istream& input, std::vector<apollota::Quadruple>& quadruples)
{
	std::vector<std::size_t> numbers(4, 0);
	for(std::size_t i=0;i<numbers.size() && !input.fail();i++)
	{
		input >> numbers[i];
	}
	if(!input.fail())
	{
		quadruples.push_back(apollota::Quadruple(numbers));
		return true;
	}
	return false;
}

}

void compare_quadruples_sets(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--first-quadruples-file", "string", "path to the first quadruples file", true));
		list_of_option_descriptions.push_back(OD("--second-quadruples-file", "string", "path to the second quadruples file", true));
		list_of_option_descriptions.push_back(OD("--init-radius-for-BSH", "string", "initial radius for bounding sphere hierarchy"));
		if(!modes_commons::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of balls (line format: 'x y z r')\n";
			std::cerr << "stdout  ->  summary of differences\n";
			return;
		}
	}

	const std::string first_triangulation_file=poh.argument<std::string>("--first-quadruples-file");

	const std::string second_triangulation_file=poh.argument<std::string>("--second-quadruples-file");

	const double init_radius_for_BSH=poh.argument<double>("--init-radius-for-BSH", 3.5);
	if(init_radius_for_BSH<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy initial radius should be greater than 1.");
	}

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	std::vector<apollota::Quadruple> first_triangulation_quadruples;
	{
		std::ifstream first_triangulation_stream(first_triangulation_file.c_str(), std::ifstream::in);
		if(!first_triangulation_stream.good())
		{
			throw std::runtime_error("Cannot open the first quadruples file.");
		}
		auxiliaries::read_lines_to_container(first_triangulation_stream, "#", add_quadruple_from_stream_to_vector, first_triangulation_quadruples);
		if(first_triangulation_quadruples.empty())
		{
			throw std::runtime_error("No quadruples in the first quadruples file.");
		}
	}

	std::vector<apollota::Quadruple> second_triangulation_quadruples;
	{
		std::ifstream second_triangulation_stream(second_triangulation_file.c_str(), std::ifstream::in);
		if(!second_triangulation_stream.good())
		{
			throw std::runtime_error("Cannot open the second quadruples file.");
		}
		auxiliaries::read_lines_to_container(second_triangulation_stream, "#", add_quadruple_from_stream_to_vector, second_triangulation_quadruples);
		if(second_triangulation_quadruples.empty())
		{
			throw std::runtime_error("No quadruples in the second quadruples file.");
		}
	}

	const apollota::ComparisonOfTriangulations::Result differences=apollota::ComparisonOfTriangulations::calculate_directional_difference_between_two_sets_of_quadruples(spheres, init_radius_for_BSH, first_triangulation_quadruples, second_triangulation_quadruples);

	std::cout << "all_differences " << differences.all_differences.size() << "\n";
	std::cout << "confirmed_differences " << differences.confirmed_differences.size() << "\n";
}
