#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"

#include "../common/construction_of_voromqa_score.h"

namespace
{

typedef voronota::common::ChainResidueAtomDescriptor CRAD;
typedef voronota::common::ChainResidueAtomDescriptorsPair CRADsPair;
typedef voronota::common::InteractionName InteractionName;
typedef voronota::common::EnergyDescriptor EnergyDescriptor;

}

void score_contacts_energy(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 conditions area')");
	pohw.describe_io("stdout", false, true, "global scores");

	const std::string potential_file=poh.argument<std::string>(pohw.describe_option("--potential-file", "string", "file path to input potential values", true), "");
	const int ignorable_max_seq_sep=poh.argument<int>(pohw.describe_option("--ignorable-max-seq-sep", "number", "maximum residue sequence separation for ignorable contacts"), 1);
	const std::string inter_atom_scores_file=poh.argument<std::string>(pohw.describe_option("--inter-atom-scores-file", "string", "file path to output inter-atom scores"), "");
	const std::string atom_scores_file=poh.argument<std::string>(pohw.describe_option("--atom-scores-file", "string", "file path to output atom scores"), "");
	const int depth=poh.argument<int>(pohw.describe_option("--depth", "number", "neighborhood normalization depth"), 2);

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const std::map<InteractionName, double> map_of_contacts=voronota::auxiliaries::IOUtilities().read_lines_to_map< std::map<InteractionName, double> >(std::cin);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No contacts input.");
	}

	const std::map<InteractionName, double> map_of_potential_values=voronota::auxiliaries::IOUtilities().read_file_lines_to_map< std::map<InteractionName, double> >(potential_file);
	if(map_of_potential_values.empty())
	{
		throw std::runtime_error("No potential values input.");
	}

	voronota::common::ConstructionOfVoroMQAScore::ParametersToConstructBundleOfVoroMQAEnergyInformation parameters;
	parameters.ignorable_max_seq_sep=ignorable_max_seq_sep;
	parameters.depth=depth;

	voronota::common::ConstructionOfVoroMQAScore::BundleOfVoroMQAEnergyInformation bundle;

	if(!voronota::common::ConstructionOfVoroMQAScore::construct_bundle_of_voromqa_energy_information(parameters, map_of_potential_values, map_of_contacts, bundle))
	{
		throw std::runtime_error("Failed to calculate energies.");
	}

	voronota::auxiliaries::IOUtilities().write_map_to_file(bundle.inter_atom_energy_descriptors, inter_atom_scores_file);
	voronota::auxiliaries::IOUtilities().write_map_to_file(bundle.atom_energy_descriptors, atom_scores_file);
	std::cout << "global " << bundle.global_energy_descriptor << "\n";
}
