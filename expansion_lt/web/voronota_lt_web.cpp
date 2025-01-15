#include <vector>
#include <string>
#include <sstream>

#include <emscripten/bind.h>

#include "../src/voronotalt/voronotalt.h"
#include "../src/voronotalt_cli/voronotalt_cli.h"

std::vector<std::string> generate_results(const std::string& input_data)
{
	std::vector<std::string> results;

	if(input_data.empty())
	{
		results.push_back(std::string("error"));
		results.push_back(std::string("no input provided"));
		return results;
	}

	std::ostringstream log_stream;
	voronotalt::TimeRecorder time_recorder;

	const voronotalt::MolecularFileReading::Parameters molecular_file_reading_parameters(true, false, true);

	voronotalt::SpheresInput::Result spheres_input_result;

	if(!voronotalt::SpheresInput::read_labeled_or_unlabeled_spheres_from_string(input_data, molecular_file_reading_parameters, 1.4, spheres_input_result, log_stream, time_recorder))
	{
		results.push_back(std::string("error"));
		results.push_back(std::string("Failed to read input without errors.\n")+log_stream.str());
		return results;
	}

	const voronotalt::PeriodicBox periodic_box;

	voronotalt::SpheresContainer spheres_container;

	spheres_container.init(spheres_input_result.spheres, periodic_box, time_recorder);

	voronotalt::RadicalTessellation::Result result;
	voronotalt::RadicalTessellation::ResultGraphics result_graphics;

	voronotalt::RadicalTessellation::construct_full_tessellation(spheres_container, std::vector<int>(), false, true, result, result_graphics, time_recorder);

	voronotalt::RadicalTessellation::GroupedResult result_grouped_by_residue;
	voronotalt::RadicalTessellation::group_results(result, spheres_input_result.grouping_by_residue, result_grouped_by_residue, time_recorder);

	voronotalt::RadicalTessellation::GroupedResult result_grouped_by_chain;
	voronotalt::RadicalTessellation::group_results(result, spheres_input_result.grouping_by_chain, result_grouped_by_chain, time_recorder);

	{
		results.push_back(std::string("log_total_input_balls"));
		results.push_back(std::to_string(result.total_spheres));

		results.push_back(std::string("log_total_contacts_count"));
		results.push_back(std::to_string(result.total_contacts_summary.count));

		results.push_back(std::string("log_total_contacts_area"));
		results.push_back(std::to_string(result.total_contacts_summary.area));

		results.push_back(std::string("log_total_residue_level_contacts_count"));
		results.push_back(std::to_string(result_grouped_by_residue.grouped_contacts_summaries.size()));

		results.push_back(std::string("log_total_chain_level_contacts_count"));
		results.push_back(std::to_string(result_grouped_by_chain.grouped_contacts_summaries.size()));

		results.push_back(std::string("log_total_cells_count"));
		results.push_back(std::to_string(result.total_cells_summary.count));

		results.push_back(std::string("log_total_cells_sas_area"));
		results.push_back(std::to_string(result.total_cells_summary.sas_area));

		results.push_back(std::string("log_total_cells_sas_inside_volume"));
		results.push_back(std::to_string(result.total_cells_summary.sas_inside_volume));

		results.push_back(std::string("log_total_residue_level_cells_count"));
		results.push_back(std::to_string(result_grouped_by_residue.grouped_cells_summaries.size()));

		results.push_back(std::string("log_total_chain_level_cells_count"));
		results.push_back(std::to_string(result_grouped_by_chain.grouped_cells_summaries.size()));
	}

	if(!result.contacts_summaries.empty())
	{
		std::ostringstream output;
		voronotalt::PrintingCustomTypes::print_contacts_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, true, output);
		results.push_back(std::string("atom-level-contacts"));
		results.push_back(output.str());
	}

	if(!result_grouped_by_residue.grouped_contacts_summaries.empty())
	{
		std::ostringstream output;
		voronotalt::PrintingCustomTypes::print_contacts_residue_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_contacts_representative_ids, result_grouped_by_residue.grouped_contacts_summaries, output);
		results.push_back(std::string("residue-level-contacts"));
		results.push_back(output.str());
	}

	if(!result_grouped_by_chain.grouped_contacts_summaries.empty())
	{
		std::ostringstream output;
		voronotalt::PrintingCustomTypes::print_contacts_chain_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_contacts_representative_ids, result_grouped_by_chain.grouped_contacts_summaries, output);
		results.push_back(std::string("chain-level-contacts"));
		results.push_back(output.str());
	}

	if(!result.cells_summaries.empty())
	{
		std::ostringstream output;
		voronotalt::PrintingCustomTypes::print_cells_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, true, output);
		results.push_back(std::string("atom-level-cell-summaries"));
		results.push_back(output.str());
	}

	if(!result_grouped_by_residue.grouped_cells_summaries.empty())
	{
		std::ostringstream output;
		voronotalt::PrintingCustomTypes::print_cells_residue_level_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_cells_representative_ids, result_grouped_by_residue.grouped_cells_summaries, output);
		results.push_back(std::string("residue-level-cell-summaries"));
		results.push_back(output.str());
	}

	if(!result_grouped_by_chain.grouped_cells_summaries.empty())
	{
		std::ostringstream output;
		voronotalt::PrintingCustomTypes::print_cells_chain_level_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_cells_representative_ids, result_grouped_by_chain.grouped_cells_summaries, output);
		results.push_back(std::string("chain-level-cell-summaries"));
		results.push_back(output.str());
	}

	return results;
}

EMSCRIPTEN_BINDINGS(voronota_lt_web)
{
	emscripten::register_vector<std::string>("VectorString");
	emscripten::function("generate_results", &generate_results);
}

