#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cmath>
#include <algorithm>

#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"

#include "../common/construction_of_cad_score.h"

void compare_contacts(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	typedef voronota::common::ConstructionOfCADScore::CRADsPair CRADsPair;
	typedef voronota::common::ConstructionOfCADScore::CADDescriptor CADDescriptor;

	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of model contacts (line format: 'annotation1 annotation2 area')");
	pohw.describe_io("stdout", false, true, "global scores (atom-level and residue-level)");

	const std::string target_contacts_file=poh.argument<std::string>(pohw.describe_option("--target-contacts-file", "string", "file path to input target contacts", true), "");
	const std::string inter_atom_scores_file=poh.argument<std::string>(pohw.describe_option("--inter-atom-scores-file", "string", "file path to output inter-atom scores"), "");
	const std::string inter_residue_scores_file=poh.argument<std::string>(pohw.describe_option("--inter-residue-scores-file", "string", "file path to output inter-residue scores"), "");
	const std::string atom_scores_file=poh.argument<std::string>(pohw.describe_option("--atom-scores-file", "string", "file path to output atom scores"), "");
	const std::string residue_scores_file=poh.argument<std::string>(pohw.describe_option("--residue-scores-file", "string", "file path to output residue scores"), "");
	const int depth=poh.argument<int>(pohw.describe_option("--depth", "number", "local neighborhood depth"), 0);
	const unsigned int smoothing_window=poh.argument<unsigned int>(pohw.describe_option("--smoothing-window", "number", "window to smooth residue scores along sequence"), 0);
	const std::string smoothed_scores_file=poh.argument<std::string>(pohw.describe_option("--smoothed-scores-file", "string", "file path to output smoothed residue scores"), "");
	const bool ignore_residue_names=poh.contains_option(pohw.describe_option("--ignore-residue-names", "", "flag to consider just residue numbers and ignore residue names"));
	const bool residue_level_only=poh.contains_option(pohw.describe_option("--residue-level-only", "", "flag to output only residue-level results"));
	CADDescriptor::detailed_output_switch()=poh.contains_option(pohw.describe_option("--detailed-output", "", "flag to enable detailed output"));
	const std::string chains_renaming_file=poh.argument<std::string>(pohw.describe_option("--chains-renaming-file", "string", "file path to input chains renaming"), "");
	const bool remap_chains=poh.contains_option(pohw.describe_option("--remap-chains", "", "flag to calculate optimal chains remapping"));
	const bool remap_chains_log=poh.contains_option(pohw.describe_option("--remap-chains-log", "", "flag output remapping progress to stderr"));
	const std::string remapped_chains_file=poh.argument<std::string>(pohw.describe_option("--remapped-chains-file", "string", "file path to output calculated chains remapping"), "");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::map<CRADsPair, double> map_of_contacts=voronota::auxiliaries::IOUtilities().read_lines_to_map< std::map<CRADsPair, double> >(std::cin);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No contacts input.");
	}

	const std::map<CRADsPair, double> map_of_target_contacts=voronota::auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRADsPair, double> >(target_contacts_file);
	if(map_of_target_contacts.empty())
	{
		throw std::runtime_error("No target contacts input.");
	}

	voronota::common::ConstructionOfCADScore::ParametersToConstructBundleOfCADScoreInformation parameters;

	parameters.ignore_residue_names=ignore_residue_names;
	parameters.remap_chains=remap_chains;
	parameters.remap_chains_logging=remap_chains_log;
	parameters.atom_level=!residue_level_only;
	parameters.residue_level=true;
	parameters.depth=depth;

	if(!chains_renaming_file.empty())
	{
		parameters.map_of_renamings=voronota::auxiliaries::IOUtilities().read_file_lines_to_map< std::map<std::string, std::string> >(chains_renaming_file);
	}

	voronota::common::ConstructionOfCADScore::BundleOfCADScoreInformation bundle;

	if(!voronota::common::ConstructionOfCADScore::construct_bundle_of_cadscore_information(parameters, map_of_target_contacts, map_of_contacts, bundle))
	{
		throw std::runtime_error("Failed to calculate CAD-score.");
	}

	if(parameters.remap_chains)
	{
		voronota::auxiliaries::IOUtilities().write_map_to_file(bundle.map_of_renamings_from_remapping, remapped_chains_file);
	}

	if(parameters.atom_level)
	{
		voronota::auxiliaries::IOUtilities().write_map_to_file(bundle.map_of_inter_atom_cad_descriptors, inter_atom_scores_file);

		voronota::auxiliaries::IOUtilities().write_map_to_file(bundle.map_of_atom_cad_descriptors, atom_scores_file);

		std::cout << "atom_level_global " << bundle.atom_level_global_descriptor << "\n";
		std::cout << "atom_average_local " << bundle.atom_average_local_score << "\n";
		std::cout << "atom_count " << bundle.map_of_atom_cad_descriptors.size() << "\n";
	}

	if(parameters.residue_level)
	{
		voronota::auxiliaries::IOUtilities().write_map_to_file(bundle.map_of_inter_residue_cad_descriptors, inter_residue_scores_file);

		voronota::auxiliaries::IOUtilities().write_map_to_file(bundle.map_of_residue_cad_descriptors, residue_scores_file);

		if(!smoothed_scores_file.empty())
		{
			voronota::auxiliaries::IOUtilities().write_map_to_file(bundle.residue_scores(smoothing_window), smoothed_scores_file);
		}

		std::cout << "residue_level_global " << bundle.residue_level_global_descriptor << "\n";
		std::cout << "residue_average_local " << bundle.residue_average_local_score << "\n";
		std::cout << "residue_count " << bundle.map_of_residue_cad_descriptors.size() << "\n";
	}
}
