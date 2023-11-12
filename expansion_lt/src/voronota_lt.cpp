#include <iostream>
#include <sstream>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "voronotalt/io_utilities.h"

#include "voronotalt/tessellation_full_construction.h"

#include "voronotalt/simplified_aw_tessellation_full_construction.h"

int main(const int argc, const char** argv)
{
	std::ios_base::sync_with_stdio(false);

	unsigned int max_number_of_processors=1;
	voronotalt::Float probe=1.4;
	bool output_csa=false;
	bool output_csa_with_graphics=false;
	bool output_sasa=false;
	bool measure_time=false;
	bool old_regime=false;

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
			else if(name=="-measure-time")
			{
				measure_time=true;
			}
			else if(name=="-old-regime")
			{
				old_regime=true;
			}
			else
			{
				std::cerr << "Error: invalid command line argument '" << name << "'\n";
				return 1;
			}
			i++;
		}
	}

#ifdef _OPENMP
	omp_set_num_threads(max_number_of_processors);
#endif

	voronotalt::TimeRecorder time_recoder_for_all(measure_time);
	voronotalt::TimeRecorder time_recoder_for_input(measure_time);
	voronotalt::TimeRecorder time_recoder_for_tessellation(measure_time);
	voronotalt::TimeRecorder time_recoder_for_output(measure_time);

	time_recoder_for_input.reset();

	std::vector<voronotalt::SimpleSphere> spheres;

	{
		std::vector<double> values;
		voronotalt::read_double_values_from_text_stream(std::cin, values);
		if(values.empty())
		{
			std::cerr << "No data read from stdin\n";
			return 1;
		}
		if(values.size()%4!=0)
		{
			std::cerr << "Invalid data in stdin\n";
			return 1;
		}
		spheres.resize(values.size()/4);
		for(std::size_t i=0;i<spheres.size();i++)
		{
			voronotalt::SimpleSphere& sphere=spheres[i];
			sphere.p.x=static_cast<voronotalt::Float>(values[i*4+0]);
			sphere.p.y=static_cast<voronotalt::Float>(values[i*4+1]);
			sphere.p.z=static_cast<voronotalt::Float>(values[i*4+2]);
			sphere.r=static_cast<voronotalt::Float>(values[i*4+3])+probe;
		}
	}

	time_recoder_for_input.record_elapsed_miliseconds_and_reset("read balls from stdin");

	if(old_regime)
	{
		time_recoder_for_tessellation.reset();

		voronotalt::SimplifiedAWTessellationFullConstruction::Result result;
		voronotalt::SimplifiedAWTessellationFullConstruction::construct_full_tessellation(spheres, output_csa_with_graphics, result, time_recoder_for_tessellation);

		time_recoder_for_output.reset();

		std::cout << "total_balls: " << spheres.size() << "\n";
		std::cout << "total_collisions: " << result.total_collisions << "\n";
		std::cout << "total_contacts_count: " << result.total_contacts_summary.count << "\n";
		std::cout << "total_contacts_area: " << result.total_contacts_summary.area << "\n";

		if(output_csa)
		{
			for(std::size_t i=0;i<result.contacts_summaries.size();i++)
			{
				const voronotalt::SimplifiedAWTessellationFullConstruction::ContactDescriptorSummary& pair_summary=result.contacts_summaries[i];
				std::cout << "csa " << pair_summary.id_a << " " <<  pair_summary.id_b << " " << pair_summary.area;
				std::cout << "\n";
			}
		}

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("write results to stdout");
	}
	else
	{
		time_recoder_for_tessellation.reset();

		voronotalt::TessellationFullConstruction::Result result;
		voronotalt::TessellationFullConstruction::construct_full_tessellation(spheres, output_csa_with_graphics, result, time_recoder_for_tessellation);

		time_recoder_for_output.reset();

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
				const voronotalt::TessellationFullConstruction::ContactDescriptorSummary& pair_summary=result.contacts_summaries[i];
				std::cout << "csa " << pair_summary.id_a << " " <<  pair_summary.id_b << " " << pair_summary.area << " " << pair_summary.solid_angle_a << " " << pair_summary.solid_angle_b;
				if(output_csa_with_graphics)
				{
					const voronotalt::TessellationContactConstruction::ContactDescriptorGraphics& pair_graphics=result.contacts_graphics[i];
					std::cout << " BEGIN,TRIANGLE_FAN";
					if(!pair_graphics.outer_points.empty())
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
				const voronotalt::TessellationFullConstruction::CellContactDescriptorsSummary& cell_summary=result.cells_summaries[i];
				if(cell_summary.stage==2)
				{
					std::cout << "sasa " << cell_summary.id << " " << cell_summary.sas_area << " " << cell_summary.sas_inside_volume << "\n";
				}
			}
		}

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("write results to stdout");
	}

	if(measure_time)
	{
		int number_of_threads=1;
#ifdef _OPENMP
		number_of_threads=omp_get_max_threads();
#endif
		std::cout << "threads_max_number: " << number_of_threads << "\n";
		time_recoder_for_input.print_recordings(std::cout, "time input stage", true);
		time_recoder_for_tessellation.print_recordings(std::cout, "time tessellation stage", true);
		time_recoder_for_output.print_recordings(std::cout, "time output stage", true);
		time_recoder_for_all.print_elapsed_time(std::cout, "time full program");
	}

	return 1;
}

