#ifndef COMMON_SCRIPTING_TABLE_PRINTING_H_
#define COMMON_SCRIPTING_TABLE_PRINTING_H_

#include "basic_types.h"

namespace common
{

namespace scripting
{

class TablePrinting
{
public:
	struct ParametersForGenericTablePrinting
	{
		bool reversed_sorting;
		bool expanded_descriptors;
		std::size_t limit;
		std::string sort_column;

		ParametersForGenericTablePrinting() :
			reversed_sorting(false),
			expanded_descriptors(false),
			limit(std::numeric_limits<std::size_t>::max())
		{
		}
	};

	struct ParametersForContactsTablePrinting : public ParametersForGenericTablePrinting
	{
		bool inter_residue;

		ParametersForContactsTablePrinting() : inter_residue(false)
		{
		}
	};

	static void print_expanded_descriptor(const ChainResidueAtomDescriptor& crad, const bool with_atom_details, std::ostream& output)
	{
		output << (crad.chainID.empty() ? std::string(".") : crad.chainID) << " ";

		if(crad.resSeq==ChainResidueAtomDescriptor::null_num())
		{
			output << "." << " ";
		}
		else
		{
			output << crad.resSeq << " ";
		}

		output << (crad.iCode.empty() ? std::string(".") : crad.iCode) << " ";

		if(with_atom_details)
		{
			if(crad.serial==ChainResidueAtomDescriptor::null_num())
			{
				output << "." << " ";
			}
			else
			{
				output << crad.serial << " ";
			}

			output << (crad.altLoc.empty() ? std::string(".") : crad.altLoc) << " ";
		}

		output << (crad.resName.empty() ? std::string(".") : crad.resName) << " ";

		if(with_atom_details)
		{
			output << (crad.name.empty() ? std::string(".") : crad.name);
		}
	}

	static void print_atoms(
			const std::vector<Atom>& atoms,
			const std::set<std::size_t>& ids,
			const ParametersForGenericTablePrinting& params,
			std::ostream& output)
	{
		std::ostringstream tmp_output;

		if(params.expanded_descriptors)
		{
			tmp_output << "chn resi ic atmi al resn atmn x y z r tags adjuncts\n";
		}
		else
		{
			tmp_output << "atom x y z r tags adjuncts\n";
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::size_t id=(*it);
			if(id<atoms.size())
			{
				const Atom& atom=atoms[id];
				if(params.expanded_descriptors)
				{
					print_expanded_descriptor(atom.crad, true, tmp_output);
					tmp_output << " " << atom.value << "\n";
				}
				else
				{
					tmp_output << atom << "\n";
				}
			}
		}

		std::istringstream tmp_input(tmp_output.str());
		print_nice_columns(tmp_input, output, true, params.sort_column, params.reversed_sorting, params.limit);
	}

	static void print_contacts(
			const std::vector<Atom>& atoms,
			const std::vector<Contact>& contacts,
			const std::set<std::size_t>& ids,
			const ParametersForContactsTablePrinting& params,
			std::ostream& output)
	{
		std::map<ChainResidueAtomDescriptorsPair, ContactValue> map_for_output;
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::size_t id=(*it);
			if(id<contacts.size())
			{
				const Contact& contact=contacts[id];
				if(contact.ids[0]<atoms.size() && contact.ids[1]<atoms.size())
				{
					if(params.inter_residue)
					{
						if(contact.solvent())
						{
							map_for_output[ChainResidueAtomDescriptorsPair(atoms[contact.ids[0]].crad.without_atom(), ChainResidueAtomDescriptor::solvent())].add(contact.value);
						}
						else
						{
							map_for_output[ChainResidueAtomDescriptorsPair(atoms[contact.ids[0]].crad.without_atom(), atoms[contact.ids[1]].crad.without_atom())].add(contact.value);
						}
					}
					else
					{
						if(contact.solvent())
						{
							map_for_output[ChainResidueAtomDescriptorsPair(atoms[contact.ids[0]].crad, ChainResidueAtomDescriptor::solvent())]=contact.value;
						}
						else
						{
							map_for_output[ChainResidueAtomDescriptorsPair(atoms[contact.ids[0]].crad, atoms[contact.ids[1]].crad)]=contact.value;
						}
					}
				}
			}
		}
		std::ostringstream tmp_output;
		enabled_output_of_ContactValue_graphics()=false;
		if(params.expanded_descriptors)
		{
			if(params.inter_residue)
			{
				tmp_output << "chn1 resi1 ic1 resn1 chn2 resi2 ic2 resn2 area dist tags adjuncts\n";
			}
			else
			{
				tmp_output << "chn1 resi1 ic1 atmi1 al1 resn1 atmn1 chn2 resi2 ic2 atmi2 al2 resn2 atmn2 area dist tags adjuncts\n";
			}
		}
		else
		{
			if(params.inter_residue)
			{
				tmp_output << "residue1 residue2 area dist tags adjuncts\n";
			}
			else
			{
				tmp_output << "atom1 atom2 area dist tags adjuncts\n";
			}
		}
		for(std::map<ChainResidueAtomDescriptorsPair, ContactValue>::const_iterator it=map_for_output.begin();it!=map_for_output.end();++it)
		{
			if(params.expanded_descriptors)
			{
				print_expanded_descriptor(it->first.a, !params.inter_residue, tmp_output);
				tmp_output << " ";
				print_expanded_descriptor(it->first.b, !params.inter_residue, tmp_output);
				tmp_output << " " << it->second << "\n";
			}
			else
			{
				tmp_output << it->first << " " << it->second << "\n";
			}
		}
		enabled_output_of_ContactValue_graphics()=true;

		std::istringstream tmp_input(tmp_output.str());
		print_nice_columns(tmp_input, output, true, params.sort_column, params.reversed_sorting, params.limit);
	}

private:
	static void print_nice_columns(
			std::istream& input,
			std::ostream& output,
			const bool first_row_is_title,
			const std::string& sort_column_name,
			const bool reverse_sorted=false,
			const std::size_t limit_rows=std::numeric_limits<std::size_t>::max())
	{
		std::vector< std::vector<std::string> > rows;
		while(input.good())
		{
			std::string line;
			std::getline(input, line);
			rows.push_back(std::vector<std::string>());
			if(!line.empty())
			{
				std::istringstream line_input(line);
				while(line_input.good())
				{
					std::string token;

					{
						line_input >> std::ws;
						const int c=input.peek();
						if(c==std::char_traits<char>::to_int_type('"') || c==std::char_traits<char>::to_int_type('\''))
						{
							line_input.get();
							std::getline(line_input, token, std::char_traits<char>::to_char_type(c));
							const std::string quote_symbol(1, std::char_traits<char>::to_char_type(c));
							token=quote_symbol+token+quote_symbol;
						}
						else
						{
							line_input >> token;
						}
					}

					if(!token.empty())
					{
						rows.back().push_back(token);
					}
				}
			}
		}

		if(rows.empty())
		{
			return;
		}

		std::vector<std::size_t> widths;
		for(std::size_t i=0;i<rows.size();i++)
		{
			for(std::size_t j=0;j<rows[i].size();j++)
			{
				const std::size_t w=rows[i][j].size();
				if(j<widths.size())
				{
					widths[j]=std::max(widths[j], w);
				}
				else
				{
					widths.push_back(w);
				}
			}
		}

		if(widths.empty())
		{
			return;
		}

		std::size_t sort_column=std::numeric_limits<std::size_t>::max();
		if(first_row_is_title && !sort_column_name.empty())
		{
			bool found=false;
			for(std::size_t j=0;j<rows[0].size() && !found;j++)
			{
				if(rows[0][j]==sort_column_name)
				{
					sort_column=j;
					found=true;
				}
			}
		}

		if(sort_column>=widths.size())
		{
			const std::size_t actual_limit_rows=(first_row_is_title && limit_rows<std::numeric_limits<std::size_t>::max()) ? (limit_rows+1) : limit_rows;
			for(std::size_t i=0;i<rows.size() && i<actual_limit_rows;i++)
			{
				for(std::size_t j=0;j<rows[i].size();j++)
				{
					output << std::setw(widths[j]+2) << std::left << rows[i][j];
				}
				output << "\n";
			}
		}
		else
		{
			std::vector< std::pair< std::pair<std::string, double>, std::size_t> > descriptors_to_ids;
			descriptors_to_ids.reserve(rows.size());

			bool all_values_are_numeric=true;
			for(std::size_t i=(first_row_is_title ? 1 : 0);i<rows.size();i++)
			{
				if(sort_column<rows[i].size())
				{
					descriptors_to_ids.push_back(std::make_pair(std::make_pair(rows[i][sort_column], 0.0), i));
					if(all_values_are_numeric)
					{
						std::istringstream value_input(rows[i][sort_column]);
						bool value_is_numeric=false;
						if(value_input.good())
						{
							double value=0.0;
							value_input >> value;
							if(!value_input.fail())
							{
								descriptors_to_ids.back().first.second=value;
								value_is_numeric=true;
							}
						}
						all_values_are_numeric=value_is_numeric;
					}
				}
				else
				{
					descriptors_to_ids.push_back(std::make_pair(std::make_pair(std::string(), 0.0), i));
					all_values_are_numeric=false;
				}
			}


			for(std::size_t i=0;i<descriptors_to_ids.size();i++)
			{
				if(all_values_are_numeric)
				{
					descriptors_to_ids[i].first.first.clear();
				}
				else
				{
					descriptors_to_ids[i].first.second=0.0;
				}
			}

			std::sort(descriptors_to_ids.begin(), descriptors_to_ids.end());
			if(reverse_sorted)
			{
				std::reverse(descriptors_to_ids.begin(), descriptors_to_ids.end());
			}

			if(first_row_is_title)
			{
				for(std::size_t j=0;j<rows[0].size();j++)
				{
					output << std::setw(widths[j]+2) << std::left << rows[0][j];
				}
				output << "\n";
			}

			for(std::size_t i=0;i<descriptors_to_ids.size() && i<limit_rows;i++)
			{
				const std::size_t id=descriptors_to_ids[i].second;
				for(std::size_t j=0;j<rows[id].size();j++)
				{
					output << std::setw(widths[j]+2) << std::left << rows[id][j];
				}
				output << "\n";
			}
		}
	}
};

}

}

#endif /* COMMON_SCRIPTING_TABLE_PRINTING_H_ */
