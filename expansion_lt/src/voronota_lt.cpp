#include <iostream>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "voronotalt/clo_parser.h"
#include "voronotalt/io_utilities.h"
#include "voronotalt/spheres_input.h"
#include "voronotalt/tessellation_full_construction.h"
#include "voronotalt/simplified_aw_tessellation_full_construction.h"
#include "voronotalt/graphics_writer.h"

int main(const int argc, const char** argv)
{
	std::ios_base::sync_with_stdio(false);

	unsigned int max_number_of_processors=1;
	voronotalt::Float probe=1.4;
	std::string write_graphics_file;
	bool output_csa=false;
	bool output_sasa=false;
	bool measure_time=false;
	bool old_regime=false;
	bool inter_residue=false;
	bool inter_chain=false;
	bool residue_level=false;
	bool output_with_labels=false;

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
			else if(opt.name=="write-graphics-file" && opt.args_strings.size()==1)
			{
				write_graphics_file=opt.args_strings.front();
			}
			else if(opt.name=="output-csa" && opt.is_flag())
			{
				output_csa=opt.is_flag_and_true();
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
			else if(opt.name=="inter-residue" && opt.is_flag())
			{
				inter_residue=opt.is_flag_and_true();
			}
			else if(opt.name=="inter-chain" && opt.is_flag())
			{
				inter_chain=opt.is_flag_and_true();
			}
			else if(opt.name=="residue-level" && opt.is_flag())
			{
				residue_level=opt.is_flag_and_true();
			}
			else if(opt.name=="output-with-labels" && opt.is_flag())
			{
				output_with_labels=opt.is_flag_and_true();
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

	voronotalt::SpheresInput::Result spheres_input_result;

	if(!voronotalt::SpheresInput::read_labeled_or_unlabeled_spheres_from_stream(std::cin, probe, spheres_input_result, std::cerr, time_recoder_for_input))
	{
		return 1;
	}

	if(inter_chain && spheres_input_result.number_of_chain_groups<2)
	{
		std::cerr << "Inter-chain contact filtering not possible - not enough distinct chains derived from labels\n";
		return 1;
	}

	if((inter_residue || residue_level) && spheres_input_result.number_of_residue_groups<2)
	{
		std::cerr << "Inter-residue contact filtering not possible - not enough distinct residues derived from labels\n";
		return 1;
	}

	std::vector<int> grouping_none;

	const std::vector<int>& grouping=(inter_chain ? spheres_input_result.grouping_by_chain : (inter_residue ? spheres_input_result.grouping_by_residue : grouping_none));

	voronotalt::GraphicsWriter graphics_writer(!write_graphics_file.empty());

	if(old_regime)
	{
		time_recoder_for_tessellation.reset();

		voronotalt::SimplifiedAWTessellationFullConstruction::Result result;
		voronotalt::SimplifiedAWTessellationFullConstruction::construct_full_tessellation(spheres_input_result.spheres, grouping, graphics_writer.enabled(), result, time_recoder_for_tessellation);

		voronotalt::SimplifiedAWTessellationFullConstruction::GroupedResult result_grouped_by_residue;

		if(residue_level)
		{
			voronotalt::SimplifiedAWTessellationFullConstruction::group_results(result, spheres_input_result.grouping_by_residue, result_grouped_by_residue, time_recoder_for_tessellation);
		}

		time_recoder_for_output.reset();

		std::cout << "total_balls: " << result.total_spheres << "\n";
		std::cout << "total_collisions: " << result.total_collisions << "\n";
		std::cout << "total_relevant_collisions: " << result.total_relevant_collisions << "\n";
		std::cout << "total_contacts_count: " << result.total_contacts_summary.count << "\n";
		std::cout << "total_contacts_area: " << result.total_contacts_summary.area << "\n";

		if(residue_level)
		{
			std::cout << "total_residue_level_contacts_count: " << result_grouped_by_residue.grouped_contacts_summaries.size() << "\n";
		}

		if(output_csa)
		{
			for(std::size_t i=0;i<result.contacts_summaries.size();i++)
			{
				const voronotalt::SimplifiedAWTessellationFullConstruction::ContactDescriptorSummary& pair_summary=result.contacts_summaries[i];
				std::cout << "csa " << pair_summary.id_a << " " <<  pair_summary.id_b << " " << pair_summary.area << "\n";
			}
		}

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("write results to stdout");

		if(graphics_writer.enabled())
		{
			for(std::size_t i=0;i<result.contacts_graphics.size();i++)
			{
				const voronotalt::SimplifiedAWTessellationFullConstruction::ContactDescriptorSummary& pair_summary=result.contacts_summaries[i];
				const voronotalt::SimplifiedAWTessellationContactConstruction::ContactDescriptorGraphics& pair_graphics=result.contacts_graphics[i];
				for(std::size_t j=0;j<pair_graphics.contours_graphics.size();j++)
				{
					graphics_writer.add_triangle_fan(pair_graphics.contours_graphics[j].outer_points, pair_graphics.contours_graphics[j].barycenter, spheres_input_result.spheres[pair_summary.id_a].p, spheres_input_result.spheres[pair_summary.id_b].p);
				}
			}

			time_recoder_for_output.record_elapsed_miliseconds_and_reset("print graphics");
		}
	}
	else
	{
		time_recoder_for_tessellation.reset();

		const bool summarize_cells=grouping.empty();

		voronotalt::TessellationFullConstruction::Result result;
		voronotalt::TessellationFullConstruction::construct_full_tessellation(spheres_input_result.spheres, grouping, graphics_writer.enabled(), summarize_cells, result, time_recoder_for_tessellation);

		voronotalt::TessellationFullConstruction::GroupedResult result_grouped_by_residue;

		if(residue_level)
		{
			voronotalt::TessellationFullConstruction::group_results(result, spheres_input_result.grouping_by_residue, result_grouped_by_residue, time_recoder_for_tessellation);
		}

		time_recoder_for_output.reset();

		std::cout << "total_balls: " << result.total_spheres << "\n";
		std::cout << "total_collisions: " << result.total_collisions << "\n";
		std::cout << "total_relevant_collisions: " << result.total_relevant_collisions << "\n";
		std::cout << "total_contacts_count: " << result.total_contacts_summary.count << "\n";
		std::cout << "total_contacts_area: " << result.total_contacts_summary.area << "\n";
		std::cout << "total_contacts_complexity: " << result.total_contacts_summary.complexity << "\n";
		std::cout << "total_cells_count: " << result.total_cells_summary.count << "\n";
		std::cout << "total_cells_sas_area: " << result.total_cells_summary.sas_area << "\n";
		std::cout << "total_cells_sas_inside_volume: " << result.total_cells_summary.sas_inside_volume << "\n";

		if(residue_level)
		{
			std::cout << "total_residue_level_contacts_count: " << result_grouped_by_residue.grouped_contacts_summaries.size() << "\n";
			std::cout << "total_residue_level_cells_count: " << result_grouped_by_residue.grouped_cells_summaries.size() << "\n";
		}

		if(output_csa)
		{
			for(std::size_t i=0;i<result.contacts_summaries.size();i++)
			{
				const voronotalt::TessellationFullConstruction::ContactDescriptorSummary& pair_summary=result.contacts_summaries[i];
				std::cout << "csa " << pair_summary.id_a << " " <<  pair_summary.id_b << " " << pair_summary.area << " " << pair_summary.solid_angle_a << " " << pair_summary.solid_angle_b << "\n";
			}
		}

		if(summarize_cells && output_sasa)
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

		if(graphics_writer.enabled())
		{
			for(std::size_t i=0;i<result.contacts_graphics.size();i++)
			{
				const voronotalt::TessellationFullConstruction::ContactDescriptorSummary& pair_summary=result.contacts_summaries[i];
				const voronotalt::TessellationContactConstruction::ContactDescriptorGraphics& pair_graphics=result.contacts_graphics[i];
				graphics_writer.add_triangle_fan(pair_graphics.outer_points, pair_graphics.barycenter, spheres_input_result.spheres[pair_summary.id_a].p, spheres_input_result.spheres[pair_summary.id_b].p);
			}

			time_recoder_for_output.record_elapsed_miliseconds_and_reset("print graphics");
		}
	}

	if(graphics_writer.enabled())
	{
		graphics_writer.write_to_file(write_graphics_file);

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("write printed graphics to file");
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

