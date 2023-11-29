#include <iostream>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "voronotalt/radical_tessellation_full_construction.h"
#include "voronotalt/simplified_aw_tessellation_full_construction.h"

#include "voronotalt_cli/clo_parser.h"
#include "voronotalt_cli/spheres_input.h"
#include "voronotalt_cli/printing_custom_types.h"
#include "voronotalt_cli/graphics_writer.h"
#include "voronotalt_cli/time_recorder_chrono.h"

int main(const int argc, const char** argv)
{
	std::ios_base::sync_with_stdio(false);

	unsigned int max_number_of_processors=1;
	voronotalt::Float probe=1.4;
	bool compute_only_inter_residue_contacts=false;
	bool compute_only_inter_chain_contacts=false;
	bool run_in_simplified_aw_diagram_regime=false;
	bool measure_running_time=false;
	bool print_contacts=false;
	bool print_contacts_residue_level=false;
	bool print_contacts_chain_level=false;
	bool print_sas_and_volumes=false;
	bool print_sas_and_volumes_residue_level=false;
	bool print_sas_and_volumes_chain_level=false;
	bool print_everything=false;
	std::string write_contacts_graphics_to_file;
	std::string graphics_title;

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
			else if(opt.name=="compute-only-inter-residue-contacts" && opt.is_flag())
			{
				compute_only_inter_residue_contacts=opt.is_flag_and_true();
			}
			else if(opt.name=="compute-only-inter-chain-contacts" && opt.is_flag())
			{
				compute_only_inter_chain_contacts=opt.is_flag_and_true();
			}
			else if(opt.name=="run-in-aw-diagram-regime" && opt.is_flag())
			{
				run_in_simplified_aw_diagram_regime=opt.is_flag_and_true();
			}
			else if(opt.name=="measure-running-time" && opt.is_flag())
			{
				measure_running_time=opt.is_flag_and_true();
			}
			else if(opt.name=="print-contacts" && opt.is_flag())
			{
				print_contacts=opt.is_flag_and_true();
			}
			else if(opt.name=="print-contacts-residue-level" && opt.is_flag())
			{
				print_contacts_residue_level=opt.is_flag_and_true();
			}
			else if(opt.name=="print-contacts-chain-level" && opt.is_flag())
			{
				print_contacts_chain_level=opt.is_flag_and_true();
			}
			else if(opt.name=="print-sas-and-volumes" && opt.is_flag())
			{
				print_sas_and_volumes=opt.is_flag_and_true();
			}
			else if(opt.name=="print-sas-and-volumes-residue-level" && opt.is_flag())
			{
				print_sas_and_volumes_residue_level=opt.is_flag_and_true();
			}
			else if(opt.name=="print-sas-and-volumes-chain-level" && opt.is_flag())
			{
				print_sas_and_volumes_chain_level=opt.is_flag_and_true();
			}
			else if(opt.name=="print-everything" && opt.is_flag())
			{
				print_everything=opt.is_flag_and_true();
			}
			else if(opt.name=="write-contacts-graphics-to-file" && opt.args_strings.size()==1)
			{
				write_contacts_graphics_to_file=opt.args_strings.front();
			}
			else if(opt.name=="graphics-title" && opt.args_strings.size()==1)
			{
				graphics_title=opt.args_strings.front();
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

	if(print_everything)
	{
		print_contacts=true;
		print_contacts_residue_level=true;
		print_contacts_chain_level=true;
		print_sas_and_volumes=true;
		print_sas_and_volumes_residue_level=true;
		print_sas_and_volumes_chain_level=true;
	}

	const bool need_summaries_on_residue_level=(print_contacts_residue_level || print_sas_and_volumes_residue_level);
	const bool need_summaries_on_chain_level=(print_contacts_chain_level || print_sas_and_volumes_chain_level);

#ifdef _OPENMP
	omp_set_num_threads(max_number_of_processors);
#endif

	voronotalt::TimeRecorderChrono time_recoder_for_all(measure_running_time);
	voronotalt::TimeRecorderChrono time_recoder_for_input(measure_running_time);
	voronotalt::TimeRecorderChrono time_recoder_for_tessellation(measure_running_time);
	voronotalt::TimeRecorderChrono time_recoder_for_output(measure_running_time);

	voronotalt::SpheresInput::Result spheres_input_result;

	if(!voronotalt::SpheresInput::read_labeled_or_unlabeled_spheres_from_stream(std::cin, probe, spheres_input_result, std::cerr, time_recoder_for_input))
	{
		std::cerr << "Error: failed to read input without errors\n";
		return 1;
	}

	if((compute_only_inter_chain_contacts || need_summaries_on_chain_level) && spheres_input_result.number_of_chain_groups<2)
	{
		std::cerr << "Error: inter-chain contact selection not possible - not enough distinct chains derived from labels\n";
		return 1;
	}

	if((compute_only_inter_residue_contacts || need_summaries_on_residue_level) && spheres_input_result.number_of_residue_groups<2)
	{
		std::cerr << "Error: inter-residue contact selection not possible - not enough distinct residues derived from labels\n";
		return 1;
	}

	const std::vector<int> null_grouping;
	const std::vector<int>& grouping_for_filtering=(compute_only_inter_chain_contacts ? spheres_input_result.grouping_by_chain : (compute_only_inter_residue_contacts ? spheres_input_result.grouping_by_residue : null_grouping));

	voronotalt::GraphicsWriter graphics_writer(!write_contacts_graphics_to_file.empty());

	if(!run_in_simplified_aw_diagram_regime)
	{
		time_recoder_for_tessellation.reset();

		const bool summarize_cells=grouping_for_filtering.empty();

		voronotalt::RadicalTessellationFullConstruction::Result result;
		voronotalt::RadicalTessellationFullConstruction::construct_full_tessellation(spheres_input_result.spheres, grouping_for_filtering, graphics_writer.enabled(), summarize_cells, result, time_recoder_for_tessellation);

		voronotalt::RadicalTessellationFullConstruction::GroupedResult result_grouped_by_residue;
		if(need_summaries_on_residue_level)
		{
			voronotalt::RadicalTessellationFullConstruction::group_results(result, spheres_input_result.grouping_by_residue, result_grouped_by_residue, time_recoder_for_tessellation);
		}

		voronotalt::RadicalTessellationFullConstruction::GroupedResult result_grouped_by_chain;
		if(need_summaries_on_chain_level)
		{
			voronotalt::RadicalTessellationFullConstruction::group_results(result, spheres_input_result.grouping_by_chain, result_grouped_by_chain, time_recoder_for_tessellation);
		}

		time_recoder_for_output.reset();

		voronotalt::PrintingCustomTypes::print_tessellation_full_construction_result_log_basic(result, result_grouped_by_residue, result_grouped_by_chain, std::cerr);
		voronotalt::PrintingCustomTypes::print_tessellation_full_construction_result_log_about_cells(result, result_grouped_by_residue, result_grouped_by_chain, std::cerr);

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("print total numbers");

		if(print_contacts)
		{
			voronotalt::PrintingCustomTypes::print_contacts_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, true, std::cout);
		}

		if(print_contacts_residue_level)
		{
			voronotalt::PrintingCustomTypes::print_contacts_residue_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_contacts_representative_ids, result_grouped_by_residue.grouped_contacts_summaries, std::cout);
		}

		if(print_contacts_chain_level)
		{
			voronotalt::PrintingCustomTypes::print_contacts_chain_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_contacts_representative_ids, result_grouped_by_chain.grouped_contacts_summaries, std::cout);
		}

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("print result contacts to stdout");

		if(print_sas_and_volumes)
		{
			voronotalt::PrintingCustomTypes::print_sas_and_volumes_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, true, std::cout);
		}

		if(print_sas_and_volumes_residue_level)
		{
			voronotalt::PrintingCustomTypes::print_sas_and_volumes_residue_level_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_cells_representative_ids, result_grouped_by_residue.grouped_cells_summaries, std::cout);
		}

		if(print_sas_and_volumes_chain_level)
		{
			voronotalt::PrintingCustomTypes::print_sas_and_volumes_chain_level_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_cells_representative_ids, result_grouped_by_chain.grouped_cells_summaries, std::cout);
		}

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("print result sas and volumes to stdout");

		if(graphics_writer.enabled())
		{
			graphics_writer.add_color(0.0, 1.0, 1.0);
			graphics_writer.add_spheres(spheres_input_result.spheres, probe);
			graphics_writer.add_color(1.0, 1.0, 0.0);
			for(std::size_t i=0;i<result.contacts_graphics.size();i++)
			{
				const voronotalt::RadicalTessellationFullConstruction::ContactDescriptorSummary& pair_summary=result.contacts_summaries[i];
				const voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics& pair_graphics=result.contacts_graphics[i];
				graphics_writer.add_triangle_fan(pair_graphics.outer_points, pair_graphics.barycenter, spheres_input_result.spheres[pair_summary.id_a].p, spheres_input_result.spheres[pair_summary.id_b].p);
			}
			time_recoder_for_output.record_elapsed_miliseconds_and_reset("print graphics");
		}
	}
	else
	{
		time_recoder_for_tessellation.reset();

		voronotalt::SimplifiedAWTessellationFullConstruction::Result result;
		voronotalt::SimplifiedAWTessellationFullConstruction::construct_full_tessellation(spheres_input_result.spheres, grouping_for_filtering, graphics_writer.enabled(), result, time_recoder_for_tessellation);

		voronotalt::SimplifiedAWTessellationFullConstruction::GroupedResult result_grouped_by_residue;
		if(need_summaries_on_residue_level)
		{
			voronotalt::SimplifiedAWTessellationFullConstruction::group_results(result, spheres_input_result.grouping_by_residue, result_grouped_by_residue, time_recoder_for_tessellation);
		}

		voronotalt::SimplifiedAWTessellationFullConstruction::GroupedResult result_grouped_by_chain;
		if(need_summaries_on_chain_level)
		{
			voronotalt::SimplifiedAWTessellationFullConstruction::group_results(result, spheres_input_result.grouping_by_chain, result_grouped_by_chain, time_recoder_for_tessellation);
		}

		time_recoder_for_output.reset();

		voronotalt::PrintingCustomTypes::print_tessellation_full_construction_result_log_basic(result, result_grouped_by_residue, result_grouped_by_chain, std::cerr);

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("print total numbers");

		if(print_contacts)
		{
			voronotalt::PrintingCustomTypes::print_contacts_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, true, std::cout);
		}

		if(print_contacts_residue_level)
		{
			voronotalt::PrintingCustomTypes::print_contacts_residue_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_contacts_representative_ids, result_grouped_by_residue.grouped_contacts_summaries, std::cout);
		}

		if(print_contacts_chain_level)
		{
			voronotalt::PrintingCustomTypes::print_contacts_chain_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_contacts_representative_ids, result_grouped_by_chain.grouped_contacts_summaries, std::cout);
		}

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("print result contacts to stdout");

		if(graphics_writer.enabled())
		{
			graphics_writer.add_color(0.0, 1.0, 1.0);
			graphics_writer.add_spheres(spheres_input_result.spheres, probe);
			graphics_writer.add_color(1.0, 1.0, 0.0);
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

	if(graphics_writer.enabled())
	{
		time_recoder_for_output.reset();
		if(!graphics_writer.write_to_file(graphics_title, write_contacts_graphics_to_file))
		{
			std::cerr << "Error (non-terminating): failed to write graphics to file '" << write_contacts_graphics_to_file << "'\n";
		}
		time_recoder_for_output.record_elapsed_miliseconds_and_reset("write printed graphics to file");
	}

	if(measure_running_time)
	{
#ifdef _OPENMP
		std::cerr << "log_openmp_threads\t" << omp_get_max_threads() << "\n";
#endif
		time_recoder_for_input.print_recordings(std::cerr, "log time input stage", true);
		time_recoder_for_tessellation.print_recordings(std::cerr, "log time tessellation stage", true);
		time_recoder_for_output.print_recordings(std::cerr, "log time output stage", true);
		time_recoder_for_all.print_elapsed_time(std::cerr, "log time full program");
	}

	return 0;
}

