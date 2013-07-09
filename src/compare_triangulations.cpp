#include <iostream>
#include <fstream>

#include "apollota/comparison_of_triangulations.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/program_options_handler.h"

namespace
{

void add_sphere_from_stream_to_vector(std::istream& input, std::vector<apollota::SimpleSphere>& spheres)
{
	apollota::SimpleSphere sphere;
	input >> sphere.x >> sphere.y >> sphere.z >> sphere.r;
	if(!input.fail())
	{
		spheres.push_back(sphere);
	}
}

void add_quadruple_from_stream_to_vector(std::istream& input, std::vector<apollota::Quadruple>& quadruples)
{
	std::vector<std::size_t> numbers(4, 0);
	for(std::size_t i=0;i<numbers.size() && !input.fail();i++)
	{
		input >> numbers[i];
	}
	if(!input.fail())
	{
		quadruples.push_back(apollota::Quadruple(numbers));
	}

}

}

void compare_triangulations(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--first-triangulation-file"].init("string", "path to the first triangulation file", true);
		basic_map_of_option_descriptions["--second-triangulation-file"].init("string", "path to the second triangulation file", true);
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions full_map_of_option_descriptions=basic_map_of_option_descriptions;
		full_map_of_option_descriptions["--init-radius-for-BSH"].init("string", "initial radius for bounding sphere hierarchy");
		if(poh.contains_option("--help"))
		{
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions(basic_map_of_option_descriptions, std::cerr);
			std::cerr << "\n";
			std::cerr << "  stdin   <-  list of balls (line format: 'x y z r')\n";
			std::cerr << "  stdout  ->  summary of differences\n";
			return;
		}
		else
		{
			poh.compare_with_map_of_option_descriptions(full_map_of_option_descriptions);
		}
	}

	const double init_radius_fo_BSH=poh.argument<double>("--init-radius-for-BSH", 3.5);
	if(init_radius_fo_BSH<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy initial radius should be greater than 1.");
	}

	const std::string first_triangulation_file=poh.argument<std::string>("--first-triangulation-file");
	const std::string second_triangulation_file=poh.argument<std::string>("--second-triangulation-file");

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", add_sphere_from_stream_to_vector, spheres);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided.");
	}

	std::ifstream first_triangulation_stream(first_triangulation_file.c_str(), std::ifstream::in);
	std::ifstream second_triangulation_stream(second_triangulation_file.c_str(), std::ifstream::in);

	std::vector<apollota::Quadruple> first_triangulation_quadruples;
	auxiliaries::read_lines_to_container(std::cin, "#", add_quadruple_from_stream_to_vector, first_triangulation_quadruples);
	std::vector<apollota::Quadruple> second_triangulation_quadruples;
	auxiliaries::read_lines_to_container(std::cin, "#", add_quadruple_from_stream_to_vector, first_triangulation_quadruples);

	const apollota::ComparisonOfTriangulations::Result differences=apollota::ComparisonOfTriangulations::calculate_directional_difference_between_two_sets_of_quadruples(spheres, init_radius_fo_BSH, first_triangulation_quadruples, second_triangulation_quadruples);

	std::cout << "all_differences " << differences.all_differences.size() << "\n";
	std::cout << "confirmed_differences " << differences.confirmed_differences.size() << "\n";
}
