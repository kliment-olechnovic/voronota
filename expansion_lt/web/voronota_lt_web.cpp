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
		results.push_back(std::string("failed to read input without errors: ")+log_stream.str());
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
		std::ostringstream output;
		output << "log_total_input_balls\t" << result.total_spheres << "\n";
		output << "log_total_collisions\t" << result.total_collisions << "\n";
		output << "log_total_relevant_collisions\t" << result.total_relevant_collisions << "\n";
		output << "log_total_contacts_count\t" << result.total_contacts_summary.count << "\n";
		output << "log_total_contacts_area\t" << result.total_contacts_summary.area << "\n";
		output << "log_total_residue_level_contacts_count\t" << result_grouped_by_residue.grouped_contacts_summaries.size() << "\n";
		output << "log_total_chain_level_contacts_count\t" << result_grouped_by_chain.grouped_contacts_summaries.size() << "\n";
		output << "log_total_cells_count\t" << result.total_cells_summary.count << "\n";
		output << "log_total_cells_sas_area\t" << result.total_cells_summary.sas_area << "\n";
		output << "log_total_cells_sas_inside_volume\t" << result.total_cells_summary.sas_inside_volume << "\n";
		output << "log_total_residue_level_cells_count\t" << result_grouped_by_residue.grouped_cells_summaries.size() << "\n";
		output << "log_total_chain_level_cells_count\t" << result_grouped_by_chain.grouped_cells_summaries.size() << "\n";
		results.push_back(std::string("log"));
		results.push_back(output.str());
	}

	{
		std::ostringstream output;
		voronotalt::PrintingCustomTypes::print_contacts_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, true, output);
		results.push_back(std::string("atom-atom-contacts"));
		results.push_back(output.str());
	}

	{
		std::ostringstream output;
		voronotalt::PrintingCustomTypes::print_contacts_residue_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_contacts_representative_ids, result_grouped_by_residue.grouped_contacts_summaries, output);
		results.push_back(std::string("residue-residue-contacts"));
		results.push_back(output.str());
	}

	{
		std::ostringstream output;
		voronotalt::PrintingCustomTypes::print_contacts_chain_level_to_stream(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_contacts_representative_ids, result_grouped_by_chain.grouped_contacts_summaries, output);
		results.push_back(std::string("chain-chain-contacts"));
		results.push_back(output.str());
	}

	{
		std::ostringstream output;
		voronotalt::PrintingCustomTypes::print_cells_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, true, output);
		results.push_back(std::string("atom-cells"));
		results.push_back(output.str());
	}

	{
		std::ostringstream output;
		voronotalt::PrintingCustomTypes::print_cells_residue_level_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_cells_representative_ids, result_grouped_by_residue.grouped_cells_summaries, output);
		results.push_back(std::string("residue-cell-summaries"));
		results.push_back(output.str());
	}

	{
		std::ostringstream output;
		voronotalt::PrintingCustomTypes::print_cells_chain_level_to_stream(result.cells_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_cells_representative_ids, result_grouped_by_chain.grouped_cells_summaries, output);
		results.push_back(std::string("chain-cell-summaries"));
		results.push_back(output.str());
	}

	return results;
}

EMSCRIPTEN_BINDINGS(voronota_lt_web)
{
	emscripten::register_vector<std::string>("VectorString");
	emscripten::function("generate_results", &generate_results);
}

