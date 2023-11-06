#include <iostream>
#include <sstream>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "voronotalt/tessellation_full_construction.h"

int main(const int argc, const char** argv)
{
	unsigned int max_number_of_processors=40;
	double probe=1.4;
	bool output_csa=false;
	bool output_csa_with_graphics=false;
	bool output_sasa=false;

	{
		int i=1;
		while(i<argc)
		{
			const std::string name(argv[i]);
			if(name=="-processors")
			{
				bool success=false;
				if(i+1<argc)
				{
					i++;
					const std::string value(argv[i]);
					if(!value.empty())
					{
						std::istringstream input(value);
						input >> max_number_of_processors;
						if(!input.fail() && max_number_of_processors>=1 && max_number_of_processors<=1000)
						{
							success=true;
						}
					}
				}
				if(!success)
				{
					std::cerr << "Error: invalid command line argument for the maximum number of processors (-processors), must be an integer from 1 to 1000.\n";
					return 1;
				}
			}
			else if(name=="-probe")
			{
				bool success=false;
				if(i+1<argc)
				{
					i++;
					const std::string value(argv[i]);
					if(!value.empty())
					{
						std::istringstream input(value);
						input >> probe;
						if(!input.fail() && probe>0.01 && probe<=30.0)
						{
							success=true;
						}
					}
				}
				if(!success)
				{
					std::cerr << "Error: invalid command line argument for the rolling probe radius (-probe), must be a value from 0.01 to 30.0.\n";
					return 1;
				}
			}
			else if(name=="-output-csa")
			{
				output_csa=true;
			}
			else if(name=="-output-csa-with-graphics")
			{
				output_csa=true;
				output_csa_with_graphics=true;
			}
			else if(name=="-output-sasa")
			{
				output_sasa=true;
			}
			else
			{
				std::cerr << "Error: invalid command line argument '" << name << "'\n";
				return 1;
			}
			i++;
		}
	}

	std::vector<voronotalt::SimpleSphere> spheres;
	{
		voronotalt::SimpleSphere sphere;
		std::cin >> std::ws;
		while(std::cin.good())
		{
			std::cin >> sphere.p.x;
			if(std::cin.fail())
			{
				std::cerr << "Error reading sphere.x at line " << spheres.size() << "\n";
				return 1;
			}
			std::cin >> sphere.p.y;
			if(std::cin.fail())
			{
				std::cerr << "Error reading sphere.y at line " << spheres.size() << "\n";
				return 1;
			}
			std::cin >> sphere.p.z;
			if(std::cin.fail())
			{
				std::cerr << "Error reading sphere.z at line " << spheres.size() << "\n";
				return 1;
			}
			std::cin >> sphere.r;
			if(std::cin.fail())
			{
				std::cerr << "Error reading sphere.r at line " << spheres.size() << "\n";
				return 1;
			}
			std::cin >> std::ws;
			sphere.r+=probe;
			spheres.push_back(sphere);
		}
	}

#ifdef _OPENMP
omp_set_num_threads(max_number_of_processors);
#endif

	voronotalt::TessellationFullConstructionResult result;

	voronotalt::construct_full_tessellation(max_number_of_processors, spheres, output_csa_with_graphics, result);

	std::cout << "total_balls: " << spheres.size() << "\n";
	std::cout << "total_collisions: " << result.total_collisions << "\n";
	std::cout << "total_contacts_count: " << result.total_contacts_summary.count << "\n";
	std::cout << "total_contacts_area: " << result.total_contacts_summary.area << "\n";
	std::cout << "total_contacts_complexity: " << result.total_contacts_summary.complexity << "\n";
	std::cout << "total_cells_count: " << result.total_cells_summary.count << "\n";
	std::cout << "total_cells_sas_area: " << result.total_cells_summary.sas_area << "\n";
	std::cout << "total_cells_sas_inside_volume: " << result.total_cells_summary.sas_inside_volume << "\n";

	if(output_csa)
	{
		for(std::size_t i=0;i<result.contacts_summaries.size();i++)
		{
			const voronotalt::ContactDescriptorSummary& pair_summary=result.contacts_summaries[i];
			std::cout << "csa " << pair_summary.id_a << " " <<  pair_summary.id_b << " " << pair_summary.area << " " << pair_summary.solid_angle_a << " " << pair_summary.solid_angle_b;
			if(output_csa_with_graphics)
			{
				const voronotalt::ContactDescriptorGraphics& pair_graphics=result.contacts_graphics[i];
				std::cout << " BEGIN,TRIANGLE_FAN";
				if(pair_graphics.valid && !pair_graphics.outer_points.empty())
				{
					std::cout << ",NORMAL," << pair_graphics.normal.x << "," << pair_graphics.normal.y << "," << pair_graphics.normal.z;
					std::cout << ",VERTEX," << pair_graphics.barycenter.x << "," << pair_graphics.barycenter.y << "," << pair_graphics.barycenter.z;
					for(std::size_t j=0;j<pair_graphics.outer_points.size();j++)
					{
						std::cout << ",NORMAL," << pair_graphics.normal.x << "," << pair_graphics.normal.y << "," << pair_graphics.normal.z;
						std::cout << ",VERTEX," << pair_graphics.outer_points[j].x << "," << pair_graphics.outer_points[j].y << "," << pair_graphics.outer_points[j].z;
					}
					std::cout << ",NORMAL," << pair_graphics.normal.x << "," << pair_graphics.normal.y << "," << pair_graphics.normal.z;
					std::cout << ",VERTEX," << pair_graphics.outer_points[0].x << "," << pair_graphics.outer_points[0].y << "," << pair_graphics.outer_points[0].z;
				}
				std::cout << ",END";
			}
			std::cout << "\n";
		}
	}

	if(output_sasa)
	{
		for(std::size_t i=0;i<result.cells_summaries.size();i++)
		{
			const voronotalt::CellContactDescriptorsSummary& cell_summary=result.cells_summaries[i];
			if(cell_summary.sas_computed)
			{
				std::cout << "sasa " << cell_summary.id << " " << cell_summary.sas_area << " " << cell_summary.sas_inside_volume << "\n";
			}
		}
	}

	return 1;
}

