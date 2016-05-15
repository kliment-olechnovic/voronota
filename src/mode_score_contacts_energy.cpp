#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

#include "modescommon/contacts_scoring_utilities.h"

void score_contacts_energy(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
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

	const std::map<InteractionName, double> map_of_contacts=auxiliaries::IOUtilities().read_lines_to_map< std::map<InteractionName, double> >(std::cin);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No contacts input.");
	}

	const std::map<InteractionName, double> map_of_potential_values=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<InteractionName, double> >(potential_file);
	if(map_of_potential_values.empty())
	{
		throw std::runtime_error("No potential values input.");
	}

	std::map<CRADsPair, EnergyDescriptor> inter_atom_energy_descriptors;
	{
		for(std::map<InteractionName, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const CRADsPair& crads=it->first.crads;
			EnergyDescriptor& ed=inter_atom_energy_descriptors[crads];
			if(!CRAD::match_with_sequence_separation_interval(crads.a, crads.b, 0, ignorable_max_seq_sep, false) && !check_crads_pair_for_peptide_bond(crads))
			{
				ed.total_area=(it->second);
				ed.contacts_count=1;
				std::map<InteractionName, double>::const_iterator potential_value_it=
						map_of_potential_values.find(InteractionName(generalize_crads_pair(crads), it->first.tag));
				if(potential_value_it!=map_of_potential_values.end())
				{
					ed.energy=ed.total_area*(potential_value_it->second);
				}
				else
				{
					ed.strange_area=ed.total_area;
				}
			}
		}
		auxiliaries::IOUtilities().write_map_to_file(inter_atom_energy_descriptors, inter_atom_scores_file);
	}

	const std::map<CRAD, EnergyDescriptor> atom_energy_descriptors=auxiliaries::ChainResidueAtomDescriptorsGraphOperations::accumulate_mapped_values_by_graph_neighbors(inter_atom_energy_descriptors, depth);
	auxiliaries::IOUtilities().write_map_to_file(atom_energy_descriptors, atom_scores_file);

	{
		EnergyDescriptor global_ed;
		for(std::map< CRADsPair, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
		{
			global_ed.add(it->second);
		}
		std::cout << "global " << global_ed << "\n";
	}
}
