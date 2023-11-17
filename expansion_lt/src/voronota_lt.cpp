#include <iostream>
#include <sstream>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "voronotalt/clo_parser.h"
#include "voronotalt/io_utilities.h"
#include "voronotalt/tessellation_full_construction.h"
#include "voronotalt/simplified_aw_tessellation_full_construction.h"
#include "voronotalt/graphics_output.h"

namespace
{

struct SphereLabel
{
	std::string chain_id;
	std::string residue_id;
	std::string atom_name;
};

}

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
		const std::vector<voronotalt::CLOParser::Option> cloptions=voronotalt::CLOParser::read_options(argc, argv);

		for(std::size_t i=0;i<cloptions.size();i++)
		{
			const voronotalt::CLOParser::Option& opt=cloptions[i];
			if(opt.name=="processors" && opt.args_ints.size()==1)
			{
				max_number_of_processors=static_cast<unsigned int>(opt.args_ints.front());
				if(!(max_number_of_processors>=1 && max_number_of_processors<=1000))
				{
					std::cerr << "Error: invalid command line argument for the maximum number of processors, must be an integer from 1 to 1000.\n";
					return 1;
				}
			}
			else if(opt.name=="probe" && opt.args_doubles.size()==1)
			{
				probe=static_cast<voronotalt::Float>(opt.args_doubles.front());
				if(!(probe>0.01 && probe<=30.0))
				{
					std::cerr << "Error: invalid command line argument for the rolling probe radius, must be a value from 0.01 to 30.0.\n";
					return 1;
				}
			}
			else if(opt.name=="output-csa" && opt.is_flag())
			{
				output_csa=opt.is_flag_and_true();
			}
			else if(opt.name=="output-csa-with-graphics" && opt.is_flag())
			{
				output_csa_with_graphics=opt.is_flag_and_true();
				if(output_csa_with_graphics)
				{
					output_csa=true;
				}
			}
			else if(opt.name=="output-sasa" && opt.is_flag())
			{
				output_sasa=opt.is_flag_and_true();
			}
			else if(opt.name=="measure-time" && opt.is_flag())
			{
				measure_time=opt.is_flag_and_true();
			}
			else if(opt.name=="old-regime" && opt.is_flag())
			{
				old_regime=opt.is_flag_and_true();
			}
			else if(opt.name.empty())
			{
				std::cerr << "Error: unnamed command line arguments detected.\n";
				return 1;
			}
			else
			{
				std::cerr << "Error: invalid command line option '" << opt.name << "'.\n";
				return 1;
			}
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
	std::vector<SphereLabel> sphere_labels;

	{
		std::vector<std::string> string_ids;
		std::vector<double> values;
		if(voronotalt::read_string_ids_and_double_values_from_text_stream(4, std::cin, string_ids, values))
		{
			const std::size_t N=(values.size()/4);
			const std::size_t label_size=(string_ids.size()/N);
			if(label_size>3 || string_ids.size()!=N*label_size)
			{
				std::cerr << "Invalid label size, must be exactly 0, 1, 2, or 3 string IDs per line\n";
				return 1;
			}
			spheres.resize(N);
			for(std::size_t i=0;i<N;i++)
			{
				voronotalt::SimpleSphere& sphere=spheres[i];
				sphere.p.x=static_cast<voronotalt::Float>(values[i*4+0]);
				sphere.p.y=static_cast<voronotalt::Float>(values[i*4+1]);
				sphere.p.z=static_cast<voronotalt::Float>(values[i*4+2]);
				sphere.r=static_cast<voronotalt::Float>(values[i*4+3])+probe;
			}
			if(label_size>0)
			{
				sphere_labels.resize(N);
				for(std::size_t i=0;i<N;i++)
				{
					SphereLabel& sphere_label=sphere_labels[i];
					if(label_size==1)
					{
						sphere_label.atom_name=string_ids[i];
					}
					else if(label_size==2)
					{
						sphere_label.residue_id=string_ids[i*label_size+0];
						sphere_label.atom_name=string_ids[i*label_size+1];
					}
					else if(label_size==3)
					{
						sphere_label.chain_id=string_ids[i*label_size+0];
						sphere_label.residue_id=string_ids[i*label_size+1];
						sphere_label.atom_name=string_ids[i*label_size+2];
					}
				}
			}
		}
		else
		{
			std::cerr << "Invalid data in stdin, must be a text table with exactly 0, 1, 2, or 3 string IDs and exactly 4 floating point values (x, y, z, r) per line\n";
			return 1;
		}
	}

	time_recoder_for_input.record_elapsed_miliseconds_and_reset("read balls from stdin");

	time_recoder_for_tessellation.reset();

	voronotalt::PreparationForTessellation::Result preparation_result;
	voronotalt::PreparationForTessellation::prepare_for_tessellation(spheres, preparation_result, time_recoder_for_tessellation);

	if(old_regime)
	{
		voronotalt::SimplifiedAWTessellationFullConstruction::Result result;
		voronotalt::SimplifiedAWTessellationFullConstruction::construct_full_tessellation(spheres, preparation_result, output_csa_with_graphics, result, time_recoder_for_tessellation);

		time_recoder_for_output.reset();

		std::cout << "total_balls: " << preparation_result.total_spheres << "\n";
		std::cout << "total_collisions: " << preparation_result.total_collisions << "\n";
		std::cout << "total_contacts_count: " << result.total_contacts_summary.count << "\n";
		std::cout << "total_contacts_area: " << result.total_contacts_summary.area << "\n";

		if(output_csa)
		{
			for(std::size_t i=0;i<result.contacts_summaries.size();i++)
			{
				const voronotalt::SimplifiedAWTessellationFullConstruction::ContactDescriptorSummary& pair_summary=result.contacts_summaries[i];
				std::cout << "csa " << pair_summary.id_a << " " <<  pair_summary.id_b << " " << pair_summary.area;
				if(output_csa_with_graphics)
				{
					for(std::size_t j=0;j<pair_summary.graphics.size();j++)
					{
						voronotalt::print_triangle_fan_for_pymol(pair_summary.graphics[j].outer_points, pair_summary.graphics[j].barycenter, voronotalt::unit_point(voronotalt::sub_of_points(spheres[pair_summary.id_b].p, spheres[pair_summary.id_a].p)), (j==0 ? " " : ","), std::cout);
					}
				}
				std::cout << "\n";
			}
		}

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("write results to stdout");
	}
	else
	{
		voronotalt::TessellationFullConstruction::Result result;
		voronotalt::TessellationFullConstruction::construct_full_tessellation(spheres, preparation_result, output_csa_with_graphics, result, time_recoder_for_tessellation);

		time_recoder_for_output.reset();

		std::cout << "total_balls: " << preparation_result.total_spheres << "\n";
		std::cout << "total_collisions: " << preparation_result.total_collisions << "\n";
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
					voronotalt::print_triangle_fan_for_pymol(pair_graphics.outer_points, pair_graphics.barycenter, voronotalt::unit_point(voronotalt::sub_of_points(spheres[pair_summary.id_b].p, spheres[pair_summary.id_a].p)), " ", std::cout);
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

