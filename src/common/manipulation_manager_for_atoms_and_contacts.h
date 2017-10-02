#ifndef COMMON_MANIPULATION_MANAGER_FOR_ATOMS_AND_CONTACTS_H_
#define COMMON_MANIPULATION_MANAGER_FOR_ATOMS_AND_CONTACTS_H_

#include "selection_manager_for_atoms_and_contacts.h"

namespace common
{

class ManipulationManagerForAtomsAndContacts
{
public:
	typedef SelectionManagerForAtomsAndContacts::Atom Atom;
	typedef SelectionManagerForAtomsAndContacts::Contact Contact;

	struct DisplayState
	{
		bool drawable;
		bool visible;
		bool marked;
		unsigned int color;

		DisplayState() : drawable(false), visible(false), marked(false), color(0x777777)
		{
		}
	};

	struct CommandHistory
	{
		std::string command;
		std::string output_log;
		std::string output_error;
		bool successful;

		CommandHistory(
				const std::string& command,
				const std::string& output_log,
				const std::string& output_error,
				const bool successful) :
					command(command),
					output_log(output_log),
					output_error(output_error),
					successful(successful)
		{
		}
	};

	ManipulationManagerForAtomsAndContacts() :
		need_sync_atoms_selections_with_dispaly_states_(false),
		need_sync_contacts_selections_with_dispaly_states_(false)
	{
	}

	const std::vector<Atom>& atoms() const
	{
		return atoms_;
	}

	const std::vector<Contact>& contacts() const
	{
		return contacts_;
	}

	const std::vector<DisplayState>& atoms_display_states() const
	{
		return atoms_display_states_;
	}

	const std::vector<DisplayState>& contacts_display_states() const
	{
		return contacts_display_states_;
	}

	const std::vector<CommandHistory>& history() const
	{
		return commands_history_;
	}

	void set_atom_visible(const std::size_t id, const bool visible)
	{
		if(id<atoms_display_states_.size())
		{
			atoms_display_states_[id].visible=visible;
			need_sync_atoms_selections_with_dispaly_states_=true;
		}
	}

	void set_atom_marked(const std::size_t id, const bool marked)
	{
		if(id<atoms_display_states_.size())
		{
			atoms_display_states_[id].marked=marked;
			need_sync_atoms_selections_with_dispaly_states_=true;
		}
	}

	void set_atom_color(const std::size_t id, const unsigned int color)
	{
		if(id<atoms_display_states_.size())
		{
			atoms_display_states_[id].color=(color & 0xFFFFFF);
			need_sync_atoms_selections_with_dispaly_states_=true;
		}
	}

	void set_contact_visible(const std::size_t id, const bool visible)
	{
		if(id<contacts_display_states_.size())
		{
			contacts_display_states_[id].visible=visible;
			need_sync_contacts_selections_with_dispaly_states_=true;
		}
	}

	void set_contact_marked(const std::size_t id, const bool marked)
	{
		if(id<contacts_display_states_.size())
		{
			contacts_display_states_[id].marked=marked;
			need_sync_contacts_selections_with_dispaly_states_=true;
		}
	}

	void set_contact_color(const std::size_t id, const unsigned int color)
	{
		if(id<contacts_display_states_.size())
		{
			contacts_display_states_[id].color=(color & 0xFFFFFF);
			need_sync_contacts_selections_with_dispaly_states_=true;
		}
	}

	const CommandHistory& execute(const std::string& command, std::ostream& output_for_content)
	{
		sync_selections_with_display_states();
		bool successful=false;
		std::ostringstream output_for_log;
		std::ostringstream output_for_errors;
		try
		{
			if(command.empty())
			{
				throw std::runtime_error(std::string("Empty command."));
			}
			std::istringstream input(command);
			std::string token;
			input >> token;
			input >> std::ws;
			if(token=="load-atoms")
			{
				command_load_atoms(input, output_for_log);
			}
			else if(token=="restrict-atoms")
			{
				command_restrict_atoms(input, output_for_log);
			}
			else if(token=="save-atoms")
			{
				command_save_atoms(input, output_for_log);
			}
			else if(token=="query-atoms")
			{
				command_query_atoms(input, output_for_log, output_for_content);
			}
			else if(token=="list-selections-of-atoms")
			{
				command_list_selections_of_atoms(input, output_for_log);
			}
			else if(token=="delete-all-selections-of-atoms")
			{
				command_delete_all_selections_of_atoms(input, output_for_log);
			}
			else if(token=="delete-selections-of-atoms")
			{
				command_delete_selections_of_atoms(input, output_for_log);
			}
			else if(token=="rename-selection-of-atoms")
			{
				command_rename_selection_of_atoms(input, output_for_log);
			}
			else if(token=="construct-contacts")
			{
				command_construct_contacts(input, output_for_log);
			}
			else if(token=="save-contacts")
			{
				command_save_contacts(input, output_for_log);
			}
			else if(token=="load-contacts")
			{
				command_load_contacts(input, output_for_log);
			}
			else if(token=="query-contacts")
			{
				command_query_contacts(input, output_for_log, output_for_content);
			}
			else if(token=="list-selections-of-contacts")
			{
				command_list_selections_of_contacts(input, output_for_log);
			}
			else if(token=="delete-all-selections-of-contacts")
			{
				command_delete_all_selections_of_contacts(input, output_for_log);
			}
			else if(token=="delete-selections-of-contacts")
			{
				command_delete_selections_of_contacts(input, output_for_log);
			}
			else if(token=="rename-selection-of-contacts")
			{
				command_rename_selection_of_contacts(input, output_for_log);
			}
			else
			{
				throw std::runtime_error(std::string("Unrecognized command."));
			}
			successful=true;
		}
		catch(const std::exception& e)
		{
			output_for_errors << e.what();
		}
		commands_history_.push_back(CommandHistory(command, output_for_log.str(), output_for_errors.str(), successful));
		return commands_history_.back();
	}

	void execute_plainly(const std::string& command, std::ostream& output)
	{
		std::ostringstream output_for_content;
		if(!command.empty())
		{
			output << "> " << command << std::endl;
			const CommandHistory& ch=execute(command, output_for_content);
			output << output_for_content.str();
			output << ch.output_log;
			if(!ch.output_error.empty())
			{
				output << "Error: " << ch.output_error << "\n";
			}
			output << std::endl;
		}
	}

private:
	struct SummaryOfAtoms
	{
		std::size_t number_total;
		double volume;

		SummaryOfAtoms() : number_total(0), volume(0.0)
		{
		}

		static SummaryOfAtoms collect_summary(const std::vector<Atom>& atoms)
		{
			SummaryOfAtoms summary;
			for(std::vector<Atom>::const_iterator it=atoms.begin();it!=atoms.end();++it)
			{
				summary.feed(*it);
			}
			return summary;
		}

		static SummaryOfAtoms collect_summary(const std::vector<Atom>& atoms, const std::set<std::size_t>& ids)
		{
			SummaryOfAtoms summary;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				if((*it)<atoms.size())
				{
					summary.feed(atoms[*it]);
				}
				else
				{
					throw std::runtime_error(std::string("Invalid atom id encountered when summarizing atoms."));
				}
			}
			return summary;
		}

		void feed(const Atom& atom)
		{
			number_total++;
			if(atom.value.props.adjuncts.count("volume")>0)
			{
				volume+=atom.value.props.adjuncts.find("volume")->second;
			}
		}

		void print(std::ostream& output) const
		{
			output << "count=" << number_total;
			if(volume>0.0)
			{
				output << " volume=" << volume;
			}
		}
	};

	struct SummaryOfContacts
	{
		std::size_t number_total;
		std::size_t number_drawable;
		double area;

		SummaryOfContacts() : number_total(0), number_drawable(0), area(0.0)
		{
		}

		static SummaryOfContacts collect_summary(const std::vector<Contact>& contacts)
		{
			SummaryOfContacts summary;
			for(std::vector<Contact>::const_iterator it=contacts.begin();it!=contacts.end();++it)
			{
				summary.feed(*it);
			}
			return summary;
		}

		static SummaryOfContacts collect_summary(const std::vector<Contact>& contacts, const std::set<std::size_t>& ids)
		{
			SummaryOfContacts summary;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				if((*it)<contacts.size())
				{
					summary.feed(contacts[*it]);
				}
				else
				{
					throw std::runtime_error(std::string("Invalid contact id encountered when summarizing contacts."));
				}
			}
			return summary;
		}

		void feed(const Contact& contact)
		{
			number_total++;
			area+=contact.value.area;
			if(!contact.value.graphics.empty())
			{
				number_drawable++;
			}
		}

		void print(std::ostream& output) const
		{
			output << "count=" << number_total;
			output << " drawable=" << number_drawable;
			output << " area=" << area;
		}
	};

	struct CommandInputParsingUtilities
	{
		static void assert_absence_of_input(std::istream& input)
		{
			if((input >> std::ws).good())
			{
				throw std::runtime_error(std::string("No additional parameters allowed."));
			}
		}

		static void read_string_considering_quotes(std::istream& input, std::string& output, const bool allow_empty_value_in_quotes=false)
		{
			input >> std::ws;
			const int c=input.peek();
			if(c==std::char_traits<char>::to_int_type('"') || c==std::char_traits<char>::to_int_type('\''))
			{
				input.get();
				output.clear();
				std::getline(input, output, std::char_traits<char>::to_char_type(c));
				if(!allow_empty_value_in_quotes && output.empty())
				{
					throw std::runtime_error(std::string("Empty string in quotes."));
				}
			}
			else
			{
				input >> output;
			}
		}

		static void read_all_strings_considering_quotes(std::istream& input, std::vector<std::string>& output, const bool allow_empty_value_in_quotes=false)
		{
			std::vector<std::string> result;
			while((input >> std::ws).good())
			{
				std::string str;
				read_string_considering_quotes(input, str, allow_empty_value_in_quotes);
				if(!input.fail())
				{
					result.push_back(str);
				}
			}
			output.swap(result);
		}

		static unsigned int read_color_integer_from_string(const std::string& color_str)
		{
			unsigned int color_int=0;
			if(!color_str.empty())
			{
				std::istringstream color_input(color_str);
				color_input >> std::hex >> color_int;
				if(color_input.fail() || color_int>0xFFFFFF)
				{
					throw std::runtime_error(std::string("Invalid hex color string '")+color_str+"'.");
				}
			}
			return color_int;
		}
	};

	struct CommandInputParsingGuard
	{
		std::string token;
		bool token_validated;

		CommandInputParsingGuard() : token_validated(false)
		{
		}

		void on_iteration_start(std::istream& input)
		{
			input >> std::ws;
			input >> token;
			if(input.fail() || token.empty())
			{
				throw std::runtime_error(std::string("Missing command parameters."));
			}
		}

		void on_token_processed(std::istream& input)
		{
			if(input.fail())
			{
				if(!token.empty())
				{
					throw std::runtime_error(std::string("Invalid value for the command parameter '")+token+"'.");
				}
				else
				{
					throw std::runtime_error(std::string("Invalid command."));
				}
			}
			else
			{
				token_validated=true;
			}
		}

		void on_iteration_end(std::istream& input) const
		{
			if(!token_validated)
			{
				if(!token.empty())
				{
					throw std::runtime_error(std::string("Invalid command parameter '")+token+"'.");
				}
				else
				{
					throw std::runtime_error(std::string("Invalid command."));
				}
			}
			input >> std::ws;
		}
	};

	struct CommandParametersForGenericSelecting
	{
		std::string type_for_expression;
		std::string type_for_full_residues;
		std::string expression;
		bool full_residues;

		CommandParametersForGenericSelecting() :
			type_for_expression("use"),
			type_for_full_residues("full-residues"),
			expression("{}"),
			full_residues(false)
		{
		}

		bool read(const std::string& type, std::istream& input)
		{
			if(type==type_for_expression)
			{
				CommandInputParsingUtilities::read_string_considering_quotes(input, expression);
			}
			else if(type==type_for_full_residues)
			{
				full_residues=true;
			}
			else
			{
				return false;
			}
			return true;
		}
	};

	struct CommandParametersForGenericQueryProcessing
	{
		bool show;
		bool hide;
		bool mark;
		bool unmark;
		int color_int;
		std::string name;
		std::string color;

		CommandParametersForGenericQueryProcessing() :
			show(false),
			hide(false),
			mark(false),
			unmark(false),
			color_int(0)
		{
		}

		bool read(const std::string& type, std::istream& input)
		{
			if(type=="name")
			{
				input >> name;
				if(name.empty())
				{
					throw std::runtime_error(std::string("Selection name cannot be empty."));
				}
				else if(name[0]=='_')
				{
					throw std::runtime_error(std::string("Explicitly specified selection name cannot start with '_'."));
				}
			}
			else if(type=="show")
			{
				if(hide)
				{
					throw std::runtime_error(std::string("Cannot show and hide at the same time."));
				}
				show=true;
			}
			else if(type=="hide")
			{
				if(show)
				{
					throw std::runtime_error(std::string("Cannot show and hide at the same time."));
				}
				hide=true;
			}
			else if(type=="mark")
			{
				if(unmark)
				{
					throw std::runtime_error(std::string("Cannot mark and unmark at the same time."));
				}
				mark=true;
			}
			else if(type=="unmark")
			{
				if(mark)
				{
					throw std::runtime_error(std::string("Cannot mark and unmark at the same time."));
				}
				unmark=true;
			}
			else if(type=="color")
			{
				input >> color;
				color_int=CommandInputParsingUtilities::read_color_integer_from_string(color);
			}
			else
			{
				return false;
			}
			return true;
		}

		bool apply_to_display_states(const std::set<std::size_t>& ids, std::vector<DisplayState>& display_states) const
		{
			if(show || hide || mark || unmark || !color.empty())
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					if((*it)<display_states.size())
					{
						DisplayState& ds=display_states[*it];
						if(show || hide)
						{
							ds.visible=(show && ds.drawable);
						}
						if(mark || unmark)
						{
							ds.marked=mark;
						}
						if(!color.empty())
						{
							ds.color=color_int;
						}
					}
				}
				return true;
			}
			return false;
		}
	};

	struct CommandParametersForGenericTablePrinting
	{
		bool print;
		bool reversed_sorting;
		bool expanded_descriptors;
		std::size_t sort_column;
		std::size_t limit;

		CommandParametersForGenericTablePrinting() :
			print(false),
			reversed_sorting(false),
			expanded_descriptors(false),
			sort_column(std::numeric_limits<std::size_t>::max()),
			limit(std::numeric_limits<std::size_t>::max())
		{
		}

		bool read(const std::string& type, std::istream& input)
		{
			if(type=="print")
			{
				print=true;
			}
			else if(type=="print-sorted")
			{
				print=true;
				input >> sort_column;
			}
			else if(type=="print-sorted-reversed")
			{
				print=true;
				reversed_sorting=true;
				input >> sort_column;
			}
			else if(type=="print-expanded")
			{
				print=true;
				expanded_descriptors=true;
			}
			else if(type=="print-limit")
			{
				print=true;
				input >> limit;
			}
			else
			{
				return false;
			}
			return true;
		}
	};

	struct CommandParametersForContactsTablePrinting : public CommandParametersForGenericTablePrinting
	{
		bool inter_residue;

		CommandParametersForContactsTablePrinting() : inter_residue(false)
		{
		}

		bool read(const std::string& type, std::istream& input)
		{
			const bool parent_read=CommandParametersForGenericTablePrinting::read(type, input);
			if(!parent_read)
			{
				if(type=="print-inter-residue")
				{
					print=true;
					inter_residue=true;
				}
				else
				{
					return false;
				}
			}
			return true;
		}
	};

	class TablePrinting
	{
	public:
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
				const CommandParametersForGenericTablePrinting& params,
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
				const CommandParametersForContactsTablePrinting& params,
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
				const bool first_row_is_title=false,
				const std::size_t sort_column=std::numeric_limits<std::size_t>::max(),
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

	void assert_atoms_availability() const
	{
		if(atoms_.empty())
		{
			throw std::runtime_error(std::string("No atoms available."));
		}
	}

	void assert_atoms_selections_availability() const
	{
		if(selection_manager_.map_of_atoms_selections().empty())
		{
			throw std::runtime_error(std::string("No atoms selections available."));
		}
	}

	void assert_atoms_selections_availability(const std::vector<std::string>& names) const
	{
		for(std::size_t i=0;i<names.size();i++)
		{
			if(selection_manager_.map_of_atoms_selections().count(names[i])==0)
			{
				throw std::runtime_error(std::string("Invalid atoms selection name '")+names[i]+"'.");
			}
		}
	}

	void assert_atoms_selection_availability(const std::string& name) const
	{
		assert_atoms_selections_availability(std::vector<std::string>(1, name));
	}

	void assert_contacts_availability() const
	{
		if(contacts_.empty())
		{
			throw std::runtime_error(std::string("No contacts available."));
		}
	}

	void assert_contacts_selections_availability() const
	{
		if(selection_manager_.map_of_contacts_selections().empty())
		{
			throw std::runtime_error(std::string("No contacts selections available."));
		}
	}

	void assert_contacts_selections_availability(const std::vector<std::string>& names) const
	{
		for(std::size_t i=0;i<names.size();i++)
		{
			if(selection_manager_.map_of_contacts_selections().count(names[i])==0)
			{
				throw std::runtime_error(std::string("Invalid contacts selection name '")+names[i]+"'.");
			}
		}
	}

	void assert_contacts_selection_availability(const std::string& name) const
	{
		assert_contacts_selections_availability(std::vector<std::string>(1, name));
	}

	void reset_atoms(std::vector<Atom>& atoms)
	{
		if(atoms.empty())
		{
			throw std::runtime_error(std::string("No atoms to set."));
		}
		atoms_.swap(atoms);
		atoms_display_states_.clear();
		atoms_display_states_.resize(atoms_.size());
		contacts_.clear();
		contacts_display_states_.clear();
		selection_manager_=SelectionManagerForAtomsAndContacts(&atoms_, 0);
	}

	void reset_contacts(std::vector<Contact>& contacts)
	{
		if(contacts.empty())
		{
			throw std::runtime_error(std::string("No contacts to set."));
		}
		assert_atoms_availability();
		if(!SelectionManagerForAtomsAndContacts::check_contacts_compatibility_with_atoms(atoms_, contacts))
		{
			throw std::runtime_error(std::string("Contacts are not compatible with atoms."));
		}
		contacts_.swap(contacts);
		contacts_display_states_.clear();
		contacts_display_states_.resize(contacts_.size());
		for(std::size_t i=0;i<contacts_.size();i++)
		{
			DisplayState& ds=contacts_display_states_[i];
			ds.drawable=(!contacts_[i].value.graphics.empty());
		}
		selection_manager_.set_contacts(&contacts_);
	}

	void sync_atoms_selections_with_display_states(const bool force=false)
	{
		if((need_sync_atoms_selections_with_dispaly_states_ || force) && !atoms_display_states_.empty())
		{
			std::set<std::size_t> ids_visible;
			std::set<std::size_t> ids_marked;
			for(std::size_t i=0;i<atoms_display_states_.size();i++)
			{
				const DisplayState& ds=atoms_display_states_[i];
				if(ds.visible)
				{
					ids_visible.insert(i);
				}
				if(ds.marked)
				{
					ids_marked.insert(i);
				}
			}

			if(ids_visible.empty())
			{
				selection_manager_.delete_atoms_selection("_visible");
			}
			else
			{
				selection_manager_.set_atoms_selection("_visible", ids_visible);
			}

			if(ids_marked.empty())
			{
				selection_manager_.delete_atoms_selection("_marked");
			}
			else
			{
				selection_manager_.set_atoms_selection("_marked", ids_marked);
			}
		}
		need_sync_atoms_selections_with_dispaly_states_=false;
	}

	void sync_contacts_selections_with_display_states(const bool force=false)
	{
		if((need_sync_contacts_selections_with_dispaly_states_ || force) && !contacts_display_states_.empty())
		{
			std::set<std::size_t> ids_visible;
			std::set<std::size_t> ids_marked;
			for(std::size_t i=0;i<contacts_display_states_.size();i++)
			{
				const DisplayState& ds=contacts_display_states_[i];
				if(ds.visible)
				{
					ids_visible.insert(i);
				}
				if(ds.marked)
				{
					ids_marked.insert(i);
				}
			}

			if(ids_visible.empty())
			{
				selection_manager_.delete_contacts_selection("_visible");
			}
			else
			{
				selection_manager_.set_contacts_selection("_visible", ids_visible);
			}

			if(ids_marked.empty())
			{
				selection_manager_.delete_contacts_selection("_marked");
			}
			else
			{
				selection_manager_.set_contacts_selection("_marked", ids_marked);
			}
		}
		need_sync_contacts_selections_with_dispaly_states_=false;
	}

	void sync_selections_with_display_states()
	{
		sync_atoms_selections_with_display_states();
		sync_contacts_selections_with_display_states();
	}

	void command_load_atoms(std::istringstream& input, std::ostream& output)
	{
		ConstructionOfAtomicBalls::collect_atomic_balls_from_file collect_atomic_balls_from_file;
		std::string atoms_file;
		std::string radii_file;
		double default_radius=ConstructionOfAtomicBalls::collect_atomic_balls_from_file::default_default_radius();
		bool only_default_radius=false;
		std::string format="pdb";

		while(input.good())
		{
			CommandInputParsingGuard guard;
			guard.on_iteration_start(input);
			if(guard.token=="file")
			{
				CommandInputParsingUtilities::read_string_considering_quotes(input, atoms_file);
				guard.on_token_processed(input);
			}
			else if(guard.token=="radii-file")
			{
				CommandInputParsingUtilities::read_string_considering_quotes(input, radii_file);
				guard.on_token_processed(input);
			}
			else if(guard.token=="default-radius")
			{
				input >> default_radius;
				guard.on_token_processed(input);
			}
			else if(guard.token=="same-radius-for-all")
			{
				only_default_radius=true;
				guard.on_token_processed(input);
			}
			else if(guard.token=="format")
			{
				CommandInputParsingUtilities::read_string_considering_quotes(input, format);
				guard.on_token_processed(input);
			}
			else if(guard.token=="include-heteroatoms")
			{
				collect_atomic_balls_from_file.include_heteroatoms=true;
				guard.on_token_processed(input);
			}
			else if(guard.token=="include-hydrogens")
			{
				collect_atomic_balls_from_file.include_hydrogens=true;
				guard.on_token_processed(input);
			}
			else if(guard.token=="as-assembly")
			{
				collect_atomic_balls_from_file.multimodel_chains=true;
				guard.on_token_processed(input);
			}
			guard.on_iteration_end(input);
		}

		if(atoms_file.empty())
		{
			throw std::runtime_error(std::string("Missing input atoms file."));
		}

		if(format!="pdb" && format!="mmcif" && format!="plain")
		{
			throw std::runtime_error(std::string("Unrecognized format '")+format+"', allowed formats are 'pdb', 'mmcif' or 'plain'.");
		}

		if(!radii_file.empty() || only_default_radius || default_radius!=ConstructionOfAtomicBalls::collect_atomic_balls_from_file::default_default_radius())
		{
			collect_atomic_balls_from_file.set_atom_radius_assigner(default_radius, only_default_radius, radii_file);
		}

		std::vector<Atom> atoms;
		bool success=false;

		if(format=="pdb" || format=="mmcif")
		{
			if(format=="mmcif")
			{
				collect_atomic_balls_from_file.mmcif=true;
			}
			success=collect_atomic_balls_from_file(atoms_file, atoms);
		}
		else if(format=="plain")
		{
			auxiliaries::IOUtilities().read_file_lines_to_set(atoms_file, atoms);
			if(!atoms.empty())
			{
				if(!radii_file.empty() || only_default_radius)
				{
					for(std::size_t i=0;i<atoms.size();i++)
					{
						Atom& atom=atoms[i];
						atom.value.r=collect_atomic_balls_from_file.atom_radius_assigner.get_atom_radius(atom.crad.resName, atom.crad.name);
					}
				}
				success=true;
			}
		}

		if(success)
		{
			if(atoms.size()<4)
			{
				throw std::runtime_error(std::string("Less than 4 atoms read."));
			}
			else
			{
				reset_atoms(atoms);
				output << "Read atoms from file '" << atoms_file << "' (";
				SummaryOfAtoms::collect_summary(atoms_).print(output);
				output << ")\n";
			}
		}
		else
		{
			throw std::runtime_error(std::string("Failed to read atoms from file '")+atoms_file+"' in '"+format+"' format.");
		}
	}

	void command_restrict_atoms(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;

		while(input.good())
		{
			CommandInputParsingGuard guard;
			guard.on_iteration_start(input);
			if(parameters_for_selecting.read(guard.token, input))
			{
				guard.on_token_processed(input);
			}
			guard.on_iteration_end(input);
		}

		const std::set<std::size_t> ids=selection_manager_.select_atoms(parameters_for_selecting.expression, parameters_for_selecting.full_residues);
		if(ids.size()<4)
		{
			throw std::runtime_error(std::string("Less than 4 atoms selected."));
		}

		std::vector<Atom> atoms;
		atoms.reserve(ids.size());
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			atoms.push_back(atoms_.at(*it));
		}

		const SummaryOfAtoms old_summary=SummaryOfAtoms::collect_summary(atoms_);

		reset_atoms(atoms);

		output << "Restricted atoms from (";
		old_summary.print(output);
		output << ") to (";
		SummaryOfAtoms::collect_summary(atoms_).print(output);
		output << ")\n";
	}

	void command_save_atoms(std::istringstream& input, std::ostream& output) const
	{
		assert_atoms_availability();

		std::string file;

		while(input.good())
		{
			CommandInputParsingGuard guard;
			guard.on_iteration_start(input);
			if(guard.token=="file")
			{
				CommandInputParsingUtilities::read_string_considering_quotes(input, file);
				guard.on_token_processed(input);
			}
			guard.on_iteration_end(input);
		}

		if(file.empty())
		{
			throw std::runtime_error(std::string("Missing output file."));
		}

		std::ofstream foutput(file.c_str(), std::ios::out);
		if(foutput.good())
		{
			auxiliaries::IOUtilities().write_set(atoms_, foutput);
			output << "Wrote atoms to file '" << file << "' (";
			SummaryOfAtoms::collect_summary(atoms_).print(output);
			output << ")\n";
		}
		else
		{
			throw std::runtime_error(std::string("Failed to open file '")+file+"' for writing.");
		}
	}

	void command_query_atoms(std::istringstream& input, std::ostream& output_for_log, std::ostream& output_for_content)
	{
		assert_atoms_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		CommandParametersForGenericQueryProcessing parameters_for_processing;
		CommandParametersForGenericTablePrinting parameters_for_printing;

		while(input.good())
		{
			CommandInputParsingGuard guard;
			guard.on_iteration_start(input);
			if(parameters_for_selecting.read(guard.token, input))
			{
				guard.on_token_processed(input);
			}
			else if(parameters_for_processing.read(guard.token, input))
			{
				guard.on_token_processed(input);
			}
			else if(parameters_for_printing.read(guard.token, input))
			{
				guard.on_token_processed(input);
			}
			guard.on_iteration_end(input);
		}

		const std::set<std::size_t> ids=selection_manager_.select_atoms(parameters_for_selecting.expression, parameters_for_selecting.full_residues);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		if(parameters_for_processing.apply_to_display_states(ids, atoms_display_states_))
		{
			sync_atoms_selections_with_display_states(true);
		}

		if(parameters_for_printing.print)
		{
			TablePrinting::print_atoms(atoms_, ids, parameters_for_printing, output_for_content);
		}

		{
			output_for_log << "Summary of atoms: ";
			SummaryOfAtoms::collect_summary(atoms_, ids).print(output_for_log);
			output_for_log << "\n";
		}

		if(!parameters_for_processing.name.empty())
		{
			selection_manager_.set_atoms_selection(parameters_for_processing.name, ids);
			output_for_log << "Set selection of atoms named '" << parameters_for_processing.name << "'\n";
		}
	}

	void command_list_selections_of_atoms(std::istringstream& input, std::ostream& output)
	{
		CommandInputParsingUtilities::assert_absence_of_input(input);
		assert_atoms_selections_availability();
		const std::map< std::string, std::set<std::size_t> >& map_of_selections=selection_manager_.map_of_atoms_selections();
		output << "Selections of atoms:\n";
		for(std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_selections.begin();it!=map_of_selections.end();++it)
		{
			output << "  name='" << (it->first) << "' ";
			SummaryOfAtoms::collect_summary(atoms_, it->second).print(output);
			output << "\n";
		}
	}

	void command_delete_all_selections_of_atoms(std::istringstream& input, std::ostream& output)
	{
		CommandInputParsingUtilities::assert_absence_of_input(input);
		assert_atoms_selections_availability();
		selection_manager_.delete_atoms_selections();
		output << "Removed all selections of atoms\n";
	}

	void command_delete_selections_of_atoms(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_selections_availability();

		std::vector<std::string> names;
		CommandInputParsingUtilities::read_all_strings_considering_quotes(input, names);

		if(names.empty())
		{
			throw std::runtime_error(std::string("No atoms selections names provided."));
		}

		assert_atoms_selections_availability(names);

		for(std::size_t i=0;i<names.size();i++)
		{
			selection_manager_.delete_atoms_selection(names[i]);
		}

		output << "Removed selections of atoms:";
		for(std::size_t i=0;i<names.size();i++)
		{
			output << " " << names[i];
		}
		output << "\n";
	}

	void command_rename_selection_of_atoms(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_selections_availability();

		std::vector<std::string> names;
		CommandInputParsingUtilities::read_all_strings_considering_quotes(input, names);

		if(names.size()!=2)
		{
			throw std::runtime_error(std::string("Not exactly two names provided for renaming."));
		}

		const std::set<std::size_t> ids=selection_manager_.get_atoms_selection(names[0]);
		selection_manager_.set_atoms_selection(names[1], ids);
		selection_manager_.delete_atoms_selection(names[0]);
		output << "Renamed selection of atoms from '" << names[0] << "' to '" << names[1] << "'\n";
	}

	void command_construct_contacts(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_availability();

		ConstructionOfContacts::construct_bundle_of_contact_information construct_bundle_of_contact_information;
		ConstructionOfContacts::enhance_contacts enhance_contacts;

		bool render=false;
		CommandParametersForGenericSelecting render_parameters_for_selecting;
		render_parameters_for_selecting.type_for_expression="render-use";
		render_parameters_for_selecting.type_for_full_residues="render-full-residues";
		render_parameters_for_selecting.expression="{min-seq-sep 1}";
		render_parameters_for_selecting.full_residues=false;

		while(input.good())
		{
			CommandInputParsingGuard guard;
			guard.on_iteration_start(input);
			if(guard.token=="probe")
			{
				input >> construct_bundle_of_contact_information.probe;
				enhance_contacts.probe=construct_bundle_of_contact_information.probe;
				guard.on_token_processed(input);
			}
			else if(guard.token=="render-default")
			{
				render=true;
				guard.on_token_processed(input);
			}
			else if(guard.token=="calculate-volumes")
			{
				construct_bundle_of_contact_information.calculate_volumes=true;
				guard.on_token_processed(input);
			}
			else if(guard.token=="tag-centrality")
			{
				enhance_contacts.tag_centrality=true;
				guard.on_token_processed(input);
			}
			else if(guard.token=="tag-peripherial")
			{
				enhance_contacts.tag_peripherial=true;
				guard.on_token_processed(input);
			}
			else if(render_parameters_for_selecting.read(guard.token, input))
			{
				render=true;
				guard.on_token_processed(input);
			}
			guard.on_iteration_end(input);
		}

		ConstructionOfContacts::BundleOfTriangulationInformation bundle_of_triangulation_information;
		ConstructionOfContacts::BundleOfContactInformation bundle_of_contact_information;

		if(construct_bundle_of_contact_information(common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(atoms_), bundle_of_triangulation_information, bundle_of_contact_information))
		{
			reset_contacts(bundle_of_contact_information.contacts);

			if(construct_bundle_of_contact_information.calculate_volumes)
			{
				for(std::size_t i=0;i<bundle_of_contact_information.volumes.size() && i<atoms_.size();i++)
				{
					atoms_[i].value.props.adjuncts["volume"]=bundle_of_contact_information.volumes[i];
				}
			}

			std::set<std::size_t> draw_ids;
			if(render)
			{
				draw_ids=selection_manager_.select_contacts(render_parameters_for_selecting.expression, render_parameters_for_selecting.full_residues);
			}

			enhance_contacts(bundle_of_triangulation_information, draw_ids, contacts_);

			output << "Constructed contacts (";
			SummaryOfContacts::collect_summary(contacts_).print(output);
			output << ")\n";
		}
		else
		{
			throw std::runtime_error(std::string("Failed to construct contacts."));
		}
	}

	void command_save_contacts(std::istringstream& input, std::ostream& output) const
	{
		assert_contacts_availability();

		std::string file;

		while(input.good())
		{
			CommandInputParsingGuard guard;
			guard.on_iteration_start(input);
			if(guard.token=="file")
			{
				CommandInputParsingUtilities::read_string_considering_quotes(input, file);
				guard.on_token_processed(input);
			}
			guard.on_iteration_end(input);
		}

		if(file.empty())
		{
			throw std::runtime_error(std::string("Missing output file."));
		}

		std::ofstream foutput(file.c_str(), std::ios::out);
		if(foutput.good())
		{
			enabled_output_of_ContactValue_graphics()=true;
			auxiliaries::IOUtilities().write_set(contacts_, foutput);
			output << "Wrote contacts to file '" << file << "' (";
			SummaryOfContacts::collect_summary(contacts_).print(output);
			output << ")\n";
		}
		else
		{
			throw std::runtime_error(std::string("Failed to open file '")+file+"' for writing.");
		}
	}

	void command_load_contacts(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_availability();

		std::string file;

		while(input.good())
		{
			CommandInputParsingGuard guard;
			guard.on_iteration_start(input);
			if(guard.token=="file")
			{
				CommandInputParsingUtilities::read_string_considering_quotes(input, file);
				guard.on_token_processed(input);
			}
			guard.on_iteration_end(input);
		}

		if(file.empty())
		{
			throw std::runtime_error(std::string("Missing input file."));
		}

		std::vector<Contact> contacts;

		auxiliaries::IOUtilities().read_file_lines_to_set(file, contacts);

		if(!contacts.empty())
		{
			reset_contacts(contacts);

			output << "Read contacts from file '" << file << "' (";
			SummaryOfContacts::collect_summary(contacts_).print(output);
			output << ")\n";
		}
		else
		{
			throw std::runtime_error(std::string("Failed to read contacts from file '")+file+"'.");
		}
	}

	void command_query_contacts(std::istringstream& input, std::ostream& output_for_log, std::ostream& output_for_content)
	{
		assert_contacts_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		CommandParametersForGenericQueryProcessing parameters_for_processing;
		CommandParametersForContactsTablePrinting parameters_for_printing;

		while(input.good())
		{
			CommandInputParsingGuard guard;
			guard.on_iteration_start(input);
			if(parameters_for_selecting.read(guard.token, input))
			{
				guard.on_token_processed(input);
			}
			else if(parameters_for_processing.read(guard.token, input))
			{
				guard.on_token_processed(input);
			}
			else if(parameters_for_printing.read(guard.token, input))
			{
				guard.on_token_processed(input);
			}
			guard.on_iteration_end(input);
		}

		const std::set<std::size_t> ids=selection_manager_.select_contacts(parameters_for_selecting.expression, parameters_for_selecting.full_residues);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		if(parameters_for_processing.apply_to_display_states(ids, contacts_display_states_))
		{
			sync_contacts_selections_with_display_states(true);
		}

		if(parameters_for_printing.print)
		{
			TablePrinting::print_contacts(atoms_, contacts_, ids, parameters_for_printing, output_for_content);
		}

		{
			output_for_log << "Summary of contacts: ";
			SummaryOfContacts::collect_summary(contacts_, ids).print(output_for_log);
			output_for_log << "\n";
		}

		if(!parameters_for_processing.name.empty())
		{
			selection_manager_.set_contacts_selection(parameters_for_processing.name, ids);
			output_for_log << "Set selection of contacts named '" << parameters_for_processing.name << "'\n";
		}
	}

	void command_list_selections_of_contacts(std::istringstream& input, std::ostream& output)
	{
		CommandInputParsingUtilities::assert_absence_of_input(input);
		assert_contacts_selections_availability();
		const std::map< std::string, std::set<std::size_t> >& map_of_selections=selection_manager_.map_of_contacts_selections();
		output << "Selections of contacts:\n";
		for(std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_selections.begin();it!=map_of_selections.end();++it)
		{
			output << "  name='" << (it->first) << "' ";
			SummaryOfContacts::collect_summary(contacts_, it->second).print(output);
			output << "\n";
		}
	}

	void command_delete_all_selections_of_contacts(std::istringstream& input, std::ostream& output)
	{
		CommandInputParsingUtilities::assert_absence_of_input(input);
		assert_contacts_selections_availability();
		selection_manager_.delete_contacts_selections();
		output << "Removed all selections of contacts\n";
	}

	void command_delete_selections_of_contacts(std::istringstream& input, std::ostream& output)
	{
		assert_contacts_selections_availability();

		std::vector<std::string> names;
		CommandInputParsingUtilities::read_all_strings_considering_quotes(input, names);

		if(names.empty())
		{
			throw std::runtime_error(std::string("No contacts selections names provided."));
		}

		assert_contacts_selections_availability(names);

		for(std::size_t i=0;i<names.size();i++)
		{
			selection_manager_.delete_contacts_selection(names[i]);
		}

		output << "Removed selections of contacts:";
		for(std::size_t i=0;i<names.size();i++)
		{
			output << " " << names[i];
		}
		output << "\n";
	}

	void command_rename_selection_of_contacts(std::istringstream& input, std::ostream& output)
	{
		assert_contacts_selections_availability();

		std::vector<std::string> names;
		CommandInputParsingUtilities::read_all_strings_considering_quotes(input, names);

		if(names.size()!=2)
		{
			throw std::runtime_error(std::string("Not exactly two names provided for renaming."));
		}

		const std::set<std::size_t> ids=selection_manager_.get_contacts_selection(names[0]);
		selection_manager_.set_contacts_selection(names[1], ids);
		selection_manager_.delete_contacts_selection(names[0]);
		output << "Renamed selection of contacts from '" << names[0] << "' to '" << names[1] << "'\n";
	}

	std::vector<Atom> atoms_;
	std::vector<Contact> contacts_;
	std::vector<DisplayState> atoms_display_states_;
	std::vector<DisplayState> contacts_display_states_;
	SelectionManagerForAtomsAndContacts selection_manager_;
	std::vector<CommandHistory> commands_history_;
	bool need_sync_atoms_selections_with_dispaly_states_;
	bool need_sync_contacts_selections_with_dispaly_states_;
};

}

#endif /* COMMON_MANIPULATION_MANAGER_FOR_ATOMS_AND_CONTACTS_H_ */
