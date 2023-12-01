#include <iostream>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "voronotalt/voronotalt.h"

#include "voronotalt_cli/voronotalt_cli.h"

namespace
{

void print_help()
{
	const std::string message=R"(
Voronota-LT version 0.9

'voronota-lt' executable constructs a radical Voronoi tessellation (also known as a Laguerre-Voronoi diagram or a power diagram)
of atomic balls of van der Waals radii constrained inside a solvent-accessible surface defined by a rolling probe.
The software computes inter-atom contact areas, per-cell solvent accessible surface areas, per-cell constrained volumes.
'voronota-lt' is very fast when used on molecular data with a not large rolling probe radius (less than 2.0 angstroms, 1.4 is recommended)
and can be made even faster by running it using multiple processors.

Options:
    --probe                                          number     rolling probe radius, default is 1.4
    --processors                                     number     maximum number of OpenMP threads to use, default is 1
    --compute-only-inter-residue-contacts                       flag to only compute inter-residue contacts, turns off per-cell summaries
    --compute-only-inter-chain-contacts                         flag to only compute inter-chain contacts, turns off per-cell summaries
    --run-in-aw-diagram-regime                                  flag to run construct a simplified additively weighted Voronoi diagram, turns off per-cell summaries
    --measure-running-time                                      flag to measure and output running times
    --print-contacts                                            flag to print table of contacts to stdout
    --print-contacts-residue-level                              flag to print residue-level grouped contacts to stdout
    --print-contacts-chain-level                                flag to print chain-level grouped contacts to stdout
    --print-cells                                               flag to print table of per-cell summaries to stdout
    --print-cells-residue-level                                 flag to print residue-level grouped per-cell summaries to stdout
    --print-cells-chain-level                                   flag to print chain-level grouped per-cell summaries to stdout
    --print-everything                                          flag to print everything to stdout, terminate if printing everything is not possible
    --write-contacts-to-file                         string     output file path to write table of contacts
    --write-contacts-residue-level-to-file           string     output file path to write residue-level grouped contacts
    --write-contacts-chain-level-to-file             string     output file path to write chain-level grouped contacts
    --write-cells-to-file                            string     output file path to write of per-cell summaries
    --write-cells-residue-level-to-file              string     output file path to write residue-level grouped per-cell summaries
    --write-cells-chain-level-to-file                string     output file path to write chain-level grouped per-cell summaries
    --write-log-to-file                              string     output file path to write global log, does not turn off printing log to stderr
    --help | -h                                                 flag to print help to stderr and exit

Standard input stream:
    Space-separated or tab-separated header-less table of balls, one of the following line formats possible:
        x y z radius
        chainID x y z radius
        chainID residueID x y z radius
        chainID residueID atomName x y z radius
    Alternatively, output of 'voronota get-balls-from-atoms-file' is acceptable, where line format is:
        x y z radius # atomSerial chainID resSeq resName atomName altLoc iCode

Standard output stream:
    Requested tables with headers, with column values tab-separated

Standard error output stream:
    Log (a name-value pair line), error messages

Usage examples:

    cat ~/2zsk.pdb | voronota get-balls-from-atoms-file | voronota-lt --print-contacts-residue-level --compute-only-inter-residue-contacts

    cat ~/2zsk.pdb | voronota get-balls-from-atoms-file | voronota-lt --processors 8 --write-contacts-to-file ./contacts.tsv --write-cells-to-file ./cells.tsv
)";

	std::cerr << message << std::endl;
}

}

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
	bool print_cells=false;
	bool print_cells_residue_level=false;
	bool print_cells_chain_level=false;
	bool print_everything=false;
	std::string write_input_balls_to_file;
	std::string write_contacts_to_file;
	std::string write_contacts_residue_level_to_file;
	std::string write_contacts_chain_level_to_file;
	std::string write_cells_to_file;
	std::string write_cells_residue_level_to_file;
	std::string write_cells_chain_level_to_file;
	std::string write_contacts_graphics_to_file;
	std::string graphics_title;
	std::string write_log_to_file;
	bool help=false;
	std::ostringstream error_log_for_options_parsing;

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
				if(!(probe>=0.0 && probe<=30.0))
				{
					std::cerr << "Error: invalid command line argument for the rolling probe radius, must be a value from 0.0 to 30.0.\n";
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
			else if(opt.name=="print-cells" && opt.is_flag())
			{
				print_cells=opt.is_flag_and_true();
			}
			else if(opt.name=="print-cells-residue-level" && opt.is_flag())
			{
				print_cells_residue_level=opt.is_flag_and_true();
			}
			else if(opt.name=="print-cells-chain-level" && opt.is_flag())
			{
				print_cells_chain_level=opt.is_flag_and_true();
			}
			else if(opt.name=="print-everything" && opt.is_flag())
			{
				print_everything=opt.is_flag_and_true();
			}
			else if(opt.name=="write-input-balls-to-file" && opt.args_strings.size()==1)
			{
				write_input_balls_to_file=opt.args_strings.front();
			}
			else if(opt.name=="write-contacts-to-file" && opt.args_strings.size()==1)
			{
				write_contacts_to_file=opt.args_strings.front();
			}
			else if(opt.name=="write-contacts-residue-level-to-file" && opt.args_strings.size()==1)
			{
				write_contacts_residue_level_to_file=opt.args_strings.front();
			}
			else if(opt.name=="write-contacts-chain-level-to-file" && opt.args_strings.size()==1)
			{
				write_contacts_chain_level_to_file=opt.args_strings.front();
			}
			else if(opt.name=="write-cells-to-file" && opt.args_strings.size()==1)
			{
				write_cells_to_file=opt.args_strings.front();
			}
			else if(opt.name=="write-cells-residue-level-to-file" && opt.args_strings.size()==1)
			{
				write_cells_residue_level_to_file=opt.args_strings.front();
			}
			else if(opt.name=="write-cells-chain-level-to-file" && opt.args_strings.size()==1)
			{
				write_cells_chain_level_to_file=opt.args_strings.front();
			}
			else if(opt.name=="write-contacts-graphics-to-file" && opt.args_strings.size()==1)
			{
				write_contacts_graphics_to_file=opt.args_strings.front();
			}
			else if(opt.name=="graphics-title" && opt.args_strings.size()==1)
			{
				graphics_title=opt.args_strings.front();
			}
			else if(opt.name=="write-log-to-file" && opt.args_strings.size()==1)
			{
				write_log_to_file=opt.args_strings.front();
			}
			else if((opt.name=="help" || opt.name=="h") && opt.is_flag())
			{
				help=opt.is_flag_and_true();
			}
			else if(opt.name.empty())
			{
				error_log_for_options_parsing << "Error: unnamed command line arguments detected.\n";
			}
			else
			{
				error_log_for_options_parsing << "Error: invalid command line option '" << opt.name << "'.\n";
			}
		}
	}

	if(help)
	{
		print_help();
		return 0;
	}

	if(!error_log_for_options_parsing.str().empty())
	{
		std::cerr << error_log_for_options_parsing.str() << "\n";
		return 1;
	}

#ifdef _OPENMP
	omp_set_num_threads(max_number_of_processors);
#else
	if(max_number_of_processors>1)
	{
		std::cerr << "Error: this executable was not compiled to use OpenMP, therefore using more than 1 processor is not supported.\n";
		return 1;
	}
#endif

	if(print_everything)
	{
		print_contacts=true;
		print_contacts_residue_level=true;
		print_contacts_chain_level=true;
		print_cells=true;
		print_cells_residue_level=true;
		print_cells_chain_level=true;
	}

	const bool need_summaries_on_residue_level=(print_contacts_residue_level || print_cells_residue_level || !write_contacts_residue_level_to_file.empty() || !write_cells_residue_level_to_file.empty());
	const bool need_summaries_on_chain_level=(print_contacts_chain_level || print_cells_chain_level || !write_contacts_chain_level_to_file.empty() || !write_cells_chain_level_to_file.empty());

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

	if(!write_input_balls_to_file.empty())
	{
		std::ofstream foutput(write_input_balls_to_file.c_str(), std::ios::out);
		if(foutput.good())
		{
			voronotalt::PrintingCustomTypes::print_balls_to_stream(spheres_input_result.spheres, spheres_input_result.sphere_labels, probe, foutput);
		}
		else
		{
			std::cerr << "Error (non-terminating): failed to write input balls to file '" << write_input_balls_to_file << "'\n";
		}
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

	std::ostringstream log_output;

	voronotalt::GraphicsWriter graphics_writer(!write_contacts_graphics_to_file.empty());

	if(!run_in_simplified_aw_diagram_regime)
	{
		time_recoder_for_tessellation.reset();

		const bool summarize_cells=grouping_for_filtering.empty();

		voronotalt::RadicalTessellation::Result result;
		voronotalt::RadicalTessellation::ResultGraphics result_graphics;
		voronotalt::RadicalTessellation::construct_full_tessellation(spheres_input_result.spheres, grouping_for_filtering, graphics_writer.enabled(), summarize_cells, result, result_graphics, time_recoder_for_tessellation);

		voronotalt::RadicalTessellation::GroupedResult result_grouped_by_residue;
		if(need_summaries_on_residue_level)
		{
			voronotalt::RadicalTessellation::group_results(result, spheres_input_result.grouping_by_residue, result_grouped_by_residue, time_recoder_for_tessellation);
		}

		voronotalt::RadicalTessellation::GroupedResult result_grouped_by_chain;
		if(need_summaries_on_chain_level)
		{
			voronotalt::RadicalTessellation::group_results(result, spheres_input_result.grouping_by_chain, result_grouped_by_chain, time_recoder_for_tessellation);
		}

		time_recoder_for_output.reset();

		voronotalt::PrintingCustomTypes::print_tessellation_full_construction_result_log_basic(result, result_grouped_by_residue, result_grouped_by_chain, log_output);
		voronotalt::PrintingCustomTypes::print_tessellation_full_construction_result_log_about_cells(result, result_grouped_by_residue, result_grouped_by_chain, log_output);

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("print total numbers");

		if(print_contacts)
		{
			voronotalt::PrintingCustomTypes::print_contacts_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, true, std::cout);
		}

		if(!write_contacts_to_file.empty())
		{
			std::ofstream foutput(write_contacts_to_file.c_str(), std::ios::out);
			if(foutput.good())
			{
				voronotalt::PrintingCustomTypes::print_contacts_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, true, foutput);
			}
			else
			{
				std::cerr << "Error (non-terminating): failed to write contacts to file '" << write_contacts_to_file << "'\n";
			}
		}

		if(print_contacts_residue_level)
		{
			voronotalt::PrintingCustomTypes::print_contacts_residue_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_contacts_representative_ids, result_grouped_by_residue.grouped_contacts_summaries, std::cout);
		}

		if(!write_contacts_residue_level_to_file.empty())
		{
			std::ofstream foutput(write_contacts_residue_level_to_file.c_str(), std::ios::out);
			if(foutput.good())
			{
				voronotalt::PrintingCustomTypes::print_contacts_residue_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_contacts_representative_ids, result_grouped_by_residue.grouped_contacts_summaries, foutput);
			}
			else
			{
				std::cerr << "Error (non-terminating): failed to write contacts on residue level to file '" << write_contacts_residue_level_to_file << "'\n";
			}
		}

		if(print_contacts_chain_level)
		{
			voronotalt::PrintingCustomTypes::print_contacts_chain_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_contacts_representative_ids, result_grouped_by_chain.grouped_contacts_summaries, std::cout);
		}

		if(!write_contacts_chain_level_to_file.empty())
		{
			std::ofstream foutput(write_contacts_chain_level_to_file.c_str(), std::ios::out);
			if(foutput.good())
			{
				voronotalt::PrintingCustomTypes::print_contacts_chain_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_contacts_representative_ids, result_grouped_by_chain.grouped_contacts_summaries, foutput);
			}
			else
			{
				std::cerr << "Error (non-terminating): failed to write contacts on chain level to file '" << write_contacts_chain_level_to_file << "'\n";
			}
		}

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("print result contacts");

		if(print_cells)
		{
			voronotalt::PrintingCustomTypes::print_cells_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, true, std::cout);
		}

		if(!write_cells_to_file.empty())
		{
			std::ofstream foutput(write_cells_to_file.c_str(), std::ios::out);
			if(foutput.good())
			{
				voronotalt::PrintingCustomTypes::print_cells_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, true, foutput);
			}
			else
			{
				std::cerr << "Error (non-terminating): failed to write cells to file '" << write_cells_to_file << "'\n";
			}
		}

		if(print_cells_residue_level)
		{
			voronotalt::PrintingCustomTypes::print_cells_residue_level_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_cells_representative_ids, result_grouped_by_residue.grouped_cells_summaries, std::cout);
		}

		if(!write_cells_residue_level_to_file.empty())
		{
			std::ofstream foutput(write_cells_residue_level_to_file.c_str(), std::ios::out);
			if(foutput.good())
			{
				voronotalt::PrintingCustomTypes::print_cells_residue_level_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_cells_representative_ids, result_grouped_by_residue.grouped_cells_summaries, foutput);
			}
			else
			{
				std::cerr << "Error (non-terminating): failed to write cells on residue level to file '" << write_cells_residue_level_to_file << "'\n";
			}
		}

		if(print_cells_chain_level)
		{
			voronotalt::PrintingCustomTypes::print_cells_chain_level_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_cells_representative_ids, result_grouped_by_chain.grouped_cells_summaries, std::cout);
		}

		if(!write_cells_chain_level_to_file.empty())
		{
			std::ofstream foutput(write_cells_chain_level_to_file.c_str(), std::ios::out);
			if(foutput.good())
			{
				voronotalt::PrintingCustomTypes::print_cells_chain_level_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_cells_representative_ids, result_grouped_by_chain.grouped_cells_summaries, foutput);
			}
			else
			{
				std::cerr << "Error (non-terminating): failed to write cells on chain level to file '" << write_cells_chain_level_to_file << "'\n";
			}
		}

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("print result sas and volumes");

		if(graphics_writer.enabled())
		{
			graphics_writer.add_color(0.0, 1.0, 1.0);
			graphics_writer.add_spheres(spheres_input_result.spheres, probe);
			graphics_writer.add_color(1.0, 1.0, 0.0);
			for(std::size_t i=0;i<result_graphics.contacts_graphics.size();i++)
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummary& pair_summary=result.contacts_summaries[i];
				const voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics& pair_graphics=result_graphics.contacts_graphics[i];
				graphics_writer.add_triangle_fan(pair_graphics.outer_points, pair_graphics.barycenter, spheres_input_result.spheres[pair_summary.id_a].p, spheres_input_result.spheres[pair_summary.id_b].p);
			}
			time_recoder_for_output.record_elapsed_miliseconds_and_reset("print graphics");
		}
	}
	else
	{
		time_recoder_for_tessellation.reset();

		voronotalt::SimplifiedAWTessellation::Result result;
		voronotalt::SimplifiedAWTessellation::ResultGraphics result_graphics;
		voronotalt::SimplifiedAWTessellation::construct_full_tessellation(spheres_input_result.spheres, grouping_for_filtering, graphics_writer.enabled(), result, result_graphics, time_recoder_for_tessellation);

		voronotalt::SimplifiedAWTessellation::GroupedResult result_grouped_by_residue;
		if(need_summaries_on_residue_level)
		{
			voronotalt::SimplifiedAWTessellation::group_results(result, spheres_input_result.grouping_by_residue, result_grouped_by_residue, time_recoder_for_tessellation);
		}

		voronotalt::SimplifiedAWTessellation::GroupedResult result_grouped_by_chain;
		if(need_summaries_on_chain_level)
		{
			voronotalt::SimplifiedAWTessellation::group_results(result, spheres_input_result.grouping_by_chain, result_grouped_by_chain, time_recoder_for_tessellation);
		}

		time_recoder_for_output.reset();

		voronotalt::PrintingCustomTypes::print_tessellation_full_construction_result_log_basic(result, result_grouped_by_residue, result_grouped_by_chain, log_output);

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("print total numbers");

		if(print_contacts)
		{
			voronotalt::PrintingCustomTypes::print_contacts_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, true, std::cout);
		}

		if(!write_contacts_to_file.empty())
		{
			std::ofstream foutput(write_contacts_to_file.c_str(), std::ios::out);
			if(foutput.good())
			{
				voronotalt::PrintingCustomTypes::print_contacts_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, true, foutput);
			}
			else
			{
				std::cerr << "Error (non-terminating): failed to write contacts to file '" << write_contacts_to_file << "'\n";
			}
		}

		if(print_contacts_residue_level)
		{
			voronotalt::PrintingCustomTypes::print_contacts_residue_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_contacts_representative_ids, result_grouped_by_residue.grouped_contacts_summaries, std::cout);
		}

		if(!write_contacts_residue_level_to_file.empty())
		{
			std::ofstream foutput(write_contacts_residue_level_to_file.c_str(), std::ios::out);
			if(foutput.good())
			{
				voronotalt::PrintingCustomTypes::print_contacts_residue_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_contacts_representative_ids, result_grouped_by_residue.grouped_contacts_summaries, foutput);
			}
			else
			{
				std::cerr << "Error (non-terminating): failed to write contacts on residue level to file '" << write_contacts_residue_level_to_file << "'\n";
			}
		}

		if(print_contacts_chain_level)
		{
			voronotalt::PrintingCustomTypes::print_contacts_chain_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_contacts_representative_ids, result_grouped_by_chain.grouped_contacts_summaries, std::cout);
		}

		if(!write_contacts_chain_level_to_file.empty())
		{
			std::ofstream foutput(write_contacts_chain_level_to_file.c_str(), std::ios::out);
			if(foutput.good())
			{
				voronotalt::PrintingCustomTypes::print_contacts_chain_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_contacts_representative_ids, result_grouped_by_chain.grouped_contacts_summaries, foutput);
			}
			else
			{
				std::cerr << "Error (non-terminating): failed to write contacts on chain level to file '" << write_contacts_chain_level_to_file << "'\n";
			}
		}

		time_recoder_for_output.record_elapsed_miliseconds_and_reset("print result contacts");

		if(graphics_writer.enabled())
		{
			graphics_writer.add_color(0.0, 1.0, 1.0);
			graphics_writer.add_spheres(spheres_input_result.spheres, probe);
			graphics_writer.add_color(1.0, 1.0, 0.0);
			for(std::size_t i=0;i<result_graphics.contacts_graphics.size();i++)
			{
				const voronotalt::SimplifiedAWTessellation::ContactDescriptorSummary& pair_summary=result.contacts_summaries[i];
				const voronotalt::SimplifiedAWTessellationContactConstruction::ContactDescriptorGraphics& pair_graphics=result_graphics.contacts_graphics[i];
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
		log_output << "log_openmp_threads\t" << omp_get_max_threads() << "\n";
#endif
		time_recoder_for_input.print_recordings(log_output, "log time input stage", true);
		time_recoder_for_tessellation.print_recordings(log_output, "log time tessellation stage", true);
		time_recoder_for_output.print_recordings(log_output, "log time output stage", true);
		time_recoder_for_all.print_elapsed_time(log_output, "log time full program");
	}

	std::cerr << log_output.str();

	if(!write_log_to_file.empty())
	{
		std::ofstream foutput(write_log_to_file.c_str(), std::ios::out);
		if(foutput.good())
		{
			foutput << log_output.str();
		}
		else
		{
			std::cerr << "Error (non-terminating): failed to write log to file '" << write_log_to_file << "'\n";
		}
	}

	return 0;
}

