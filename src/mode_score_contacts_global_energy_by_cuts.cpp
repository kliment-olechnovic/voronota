#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

#include "modescommon/contacts_scoring_utilities.h"
#include "modescommon/sequence_utilities.h"

namespace
{

std::vector<CRAD> collect_sequence_from_contacts(const std::map<InteractionName, double>& map_of_contacts)
{
	std::set<CRAD> set_of_residues;
	for(std::map<InteractionName, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		const CRADsPair& crads=it->first.crads;
		if(crads.a!=CRAD::solvent())
		{
			set_of_residues.insert(crads.a.without_atom());
		}
		if(crads.b!=CRAD::solvent())
		{
			set_of_residues.insert(crads.b.without_atom());
		}
	}
	return std::vector<CRAD>(set_of_residues.begin(), set_of_residues.end());
}

std::set<CRAD> collect_head_and_tail_of_sequence(const std::vector<CRAD>& sequence, const int cut_from_start, const int cut_from_end)
{
	std::set<CRAD> result;
	if(!sequence.empty())
	{
		for(std::size_t i=0;i<sequence.size();i++)
		{
			if(sequence[i].resSeq<(sequence.front().resSeq+cut_from_start) || sequence[i].resSeq>(sequence.back().resSeq-cut_from_end))
			{
				result.insert(sequence[i]);
			}
		}
	}
	return result;
}

std::string get_sequence_string_cut_out(const std::vector<CRAD>& sequence, const int cut_from_start, const int cut_from_end)
{
	std::vector<CRAD> inner_sequence;
	std::set<CRAD> forbidden_residues=collect_head_and_tail_of_sequence(sequence, cut_from_start, cut_from_end);
	for(std::size_t i=0;i<sequence.size();i++)
	{
		if(forbidden_residues.count(sequence[i])==0)
		{
			inner_sequence.push_back(sequence[i]);
		}
	}
	return SequenceUtilities::convert_residue_sequence_container_to_string(inner_sequence);
}

std::map<InteractionName, double> exclude_residues_from_contacts(const std::map<InteractionName, double>& map_of_contacts, const std::set<CRAD>& exclusion_set, const double solvent_expansion)
{
	std::map<InteractionName, double> result;
	for(std::map<InteractionName, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		const CRADsPair& crads=it->first.crads;
		const bool exclude_a=(exclusion_set.count(crads.a.without_atom())>0);
		const bool exclude_b=(exclusion_set.count(crads.b.without_atom())>0);
		if(!exclude_a && !exclude_b)
		{
			result[it->first]=it->second;
		}
		else if(!exclude_a && exclude_b && crads.a!=CRAD::solvent())
		{
			result[InteractionName(CRADsPair(crads.a, CRAD::solvent()), ".")]+=(it->second*solvent_expansion);
		}
		else if(!exclude_b && exclude_a && crads.b!=CRAD::solvent())
		{
			result[InteractionName(CRADsPair(crads.b, CRAD::solvent()), ".")]+=(it->second*solvent_expansion);
		}
	}
	return result;
}

EnergyDescriptor calculate_global_energy(const std::map<InteractionName, double>& map_of_potential_values, const int ignorable_max_seq_sep, const std::map<InteractionName, double>& map_of_contacts)
{
	std::map<CRADsPair, EnergyDescriptor> inter_atom_energy_descriptors;
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
	EnergyDescriptor global_ed;
	for(std::map< CRADsPair, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
	{
		global_ed.add(it->second);
	}
	return global_ed;
}

template<typename Set, typename Element>
void update_limited_set(Set& set, const unsigned long limit, const Element& element)
{
	set.insert(element);
	if(limit>0 && set.size()>limit)
	{
		typename Set::iterator it=set.end();
		--it;
		set.erase(it);
	}
}

}

void score_contacts_global_energy_by_cuts(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 conditions area')");
	pohw.describe_io("stdout", false, true, "global scores");

	const std::string potential_file=poh.argument<std::string>(pohw.describe_option("--potential-file", "string", "file path to input potential values", true), "");
	const int ignorable_max_seq_sep=poh.argument<int>(pohw.describe_option("--ignorable-max-seq-sep", "number", "maximum residue sequence separation for ignorable contacts"), 1);
	const int cut_step=std::max(1, poh.argument<int>(pohw.describe_option("--cut-step", "number", "big cut length increasing step"), 10));
	const unsigned long limit_results=poh.argument<unsigned long>(pohw.describe_option("--limit-results", "number", "maximum number of results to print"), 1);

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

	const std::vector<CRAD> sequence=collect_sequence_from_contacts(map_of_contacts);
	if(sequence.empty())
	{
		throw std::runtime_error("Empty sequence.");
	}

	const int sequence_length=(sequence.back().resSeq-sequence.front().resSeq+1);
	const int min_length_after_cuts=std::max(10, sequence_length/5);

	std::set< std::pair<double, std::pair<int, int> > > result_set;

	for(int cut_from_start=0;cut_from_start<sequence_length;cut_from_start+=cut_step)
	{
		for(int cut_from_end=0;cut_from_end<sequence_length;cut_from_end+=cut_step)
		{
			if((cut_from_start+cut_from_end+min_length_after_cuts)<sequence_length)
			{
				const std::map<InteractionName, double> reduced_map_of_contacts=exclude_residues_from_contacts(map_of_contacts, collect_head_and_tail_of_sequence(sequence, cut_from_start, cut_from_end), 3.0);
				if(!reduced_map_of_contacts.empty())
				{
					const EnergyDescriptor global_ed=calculate_global_energy(map_of_potential_values, ignorable_max_seq_sep, reduced_map_of_contacts);
					if(global_ed.total_area>0)
					{
						update_limited_set(result_set, limit_results, std::make_pair((global_ed.energy/global_ed.total_area), std::make_pair(cut_from_start, cut_from_end)));
					}
				}
			}
		}
	}

	if(cut_step>1 && !result_set.empty())
	{
		int best_so_far_cut_from_start=result_set.begin()->second.first;
		int best_so_far_cut_from_end=result_set.begin()->second.second;
		for(int cut_from_start=std::max(0, best_so_far_cut_from_start-cut_step);cut_from_start<std::min(sequence_length, best_so_far_cut_from_start+cut_step);cut_from_start++)
		{
			for(int cut_from_end=std::max(0, best_so_far_cut_from_end-cut_step);cut_from_end<std::min(sequence_length, best_so_far_cut_from_end+cut_step);cut_from_end++)
			{
				if((cut_from_start+cut_from_end+min_length_after_cuts)<sequence_length)
				{
					const std::map<InteractionName, double> reduced_map_of_contacts=exclude_residues_from_contacts(map_of_contacts, collect_head_and_tail_of_sequence(sequence, cut_from_start, cut_from_end), 3.0);
					if(!reduced_map_of_contacts.empty())
					{
						const EnergyDescriptor global_ed=calculate_global_energy(map_of_potential_values, ignorable_max_seq_sep, reduced_map_of_contacts);
						if(global_ed.total_area>0)
						{
							update_limited_set(result_set, limit_results, std::make_pair((global_ed.energy/global_ed.total_area), std::make_pair(cut_from_start, cut_from_end)));
						}
					}
				}
			}
		}
	}

	for(std::set< std::pair<double, std::pair<int, int> > >::const_iterator it=result_set.begin();it!=result_set.end();++it)
	{
		std::cout << it->second.first << " " << it->second.second << " " << it->first << " ";
		std::cout << get_sequence_string_cut_out(sequence, it->second.first, it->second.second) << "\n";
	}
}
