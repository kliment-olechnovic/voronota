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
		bool visible;
		unsigned int color;

		DisplayState() : visible(true), color(0x777777)
		{
		}

		DisplayState(const bool visible, const unsigned int color) : visible(visible), color(color)
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

	ManipulationManagerForAtomsAndContacts()
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

	const CommandHistory& execute(const std::string& command, std::ostream& output_for_content)
	{
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
			if(token=="read-atoms")
			{
				command_read_atoms(input, output_for_log);
			}
			else if(token=="restrict-atoms")
			{
				command_restrict_atoms(input, output_for_log);
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

		void feed(const Atom& atom)
		{
			number_total++;
			if(atom.value.props.adjuncts.count("volume")>0)
			{
				volume+=atom.value.props.adjuncts.find("volume")->second;
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

		void feed(const Contact& contact)
		{
			number_total++;
			area+=contact.value.area;
			if(!contact.value.graphics.empty())
			{
				number_drawable++;
			}
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
				read_string_considering_quotes(input, expression);
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
		bool print=false;
		bool show=false;
		bool hide=false;
		int color_int;
		std::string name;
		std::string color;

		CommandParametersForGenericQueryProcessing() : print(false), show(false), hide(false), color_int(0)
		{
		}

		bool read(const std::string& type, std::istream& input)
		{
			if(type=="name")
			{
				input >> name;
			}
			else if(type=="print")
			{
				print=true;
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
			else if(type=="color")
			{
				input >> color;
				color_int=read_color_integer_from_string(color);
			}
			else
			{
				return false;
			}
			return true;
		}

		void apply_to_display_states(const std::set<std::size_t>& ids, std::vector<DisplayState>& display_states) const
		{
			if(show || hide || !color.empty())
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					if((*it)<display_states.size())
					{
						DisplayState& ds=display_states[*it];
						if(show || hide)
						{
							ds.visible=show;
						}
						if(!color.empty())
						{
							ds.color=color_int;
						}
					}
				}
			}
		}
	};

	static void print_summary_of_atoms(const SummaryOfAtoms& summary, std::ostream& output)
	{
		output << "count=" << summary.number_total;
		if(summary.volume>0.0)
		{
			output << " volume=" << summary.volume;
		}
	}

	static void print_summary_of_contacts(const SummaryOfContacts& summary, std::ostream& output)
	{
		output << "count=" << summary.number_total;
		output << " drawable=" << summary.number_drawable;
		output << " area=" << summary.area;
	}

	static void read_string_considering_quotes(std::istream& input, std::string& output)
	{
		input >> std::ws;
		const int c=input.peek();
		if(c==std::char_traits<char>::to_int_type('"') || c==std::char_traits<char>::to_int_type('\''))
		{
			input.get();
			output.clear();
			std::getline(input, output, std::char_traits<char>::to_char_type(c));
		}
		else
		{
			input >> output;
		}
	}

	static unsigned int read_color_integer_from_string(const std::string& color_str)
	{
		unsigned int color_int=0;
		if(!color_str.empty())
		{
			std::istringstream color_input(color_str);
			color_input >> std::hex >> color_int;
			if(color_input.fail())
			{
				throw std::runtime_error(std::string("Invalid hex color string '")+color_str+"'.");
			}
		}
		return color_int;
	}

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

	void reset_atoms(std::vector<Atom>& atoms)
	{
		atoms_.swap(atoms);
		atoms_display_states_.clear();
		atoms_display_states_.resize(atoms_.size(), DisplayState(true, 0xFF7700));
		contacts_.clear();
		contacts_display_states_.clear();
		selection_manager_=SelectionManagerForAtomsAndContacts(&atoms_, 0);
	}

	void reset_atoms()
	{
		std::vector<Atom> atoms;
		reset_atoms(atoms);
	}

	void reset_contacts(std::vector<Contact>& contacts)
	{
		if(!SelectionManagerForAtomsAndContacts::check_contacts_compatibility_with_atoms(atoms_, contacts))
		{
			throw std::runtime_error(std::string("Contacts are not compatible with atoms."));
		}
		contacts_.swap(contacts);
		contacts_display_states_.clear();
		contacts_display_states_.resize(contacts_.size(), DisplayState(true, 0x0077FF));
		selection_manager_.set_contacts(&contacts_);
	}

	void reset_contacts()
	{
		std::vector<Contact> contacts;
		reset_contacts(contacts);
	}

	SummaryOfAtoms collect_summary_of_atoms() const
	{
		SummaryOfAtoms summary;
		for(std::vector<Atom>::const_iterator it=atoms_.begin();it!=atoms_.end();++it)
		{
			summary.feed(*it);
		}
		return summary;
	}

	SummaryOfAtoms collect_summary_of_atoms(const std::set<std::size_t>& ids) const
	{
		SummaryOfAtoms summary;
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			if((*it)<atoms_.size())
			{
				summary.feed(atoms_[*it]);
			}
			else
			{
				throw std::runtime_error(std::string("Invalid atom id encountered when summarizing atoms."));
			}
		}
		return summary;
	}

	SummaryOfContacts collect_summary_of_contacts() const
	{
		SummaryOfContacts summary;
		for(std::vector<Contact>::const_iterator it=contacts_.begin();it!=contacts_.end();++it)
		{
			summary.feed(*it);
		}
		return summary;
	}

	SummaryOfContacts collect_summary_of_contacts(const std::set<std::size_t>& ids) const
	{
		SummaryOfContacts summary;
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			if((*it)<contacts_.size())
			{
				summary.feed(contacts_[*it]);
			}
			else
			{
				throw std::runtime_error(std::string("Invalid contact id encountered when summarizing contacts."));
			}
		}
		return summary;
	}

	void command_read_atoms(std::istringstream& input, std::ostream& output)
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
				input >> atoms_file;
				guard.on_token_processed(input);
			}
			else if(guard.token=="radii-file")
			{
				input >> radii_file;
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
				input >> format;
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
			auxiliaries::IOUtilities().read_lines_to_set(std::cin, atoms);
			if(!radii_file.empty() || only_default_radius)
			{
				for(std::size_t i=0;i<atoms.size();i++)
				{
					Atom& atom=atoms[i];
					atom.value.r=collect_atomic_balls_from_file.atom_radius_assigner.get_atom_radius(atom.crad.resName, atom.crad.name);
				}
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
				print_summary_of_atoms(collect_summary_of_atoms(), output);
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

		CommandParametersForGenericSelecting selection_expression;

		while(input.good())
		{
			CommandInputParsingGuard guard;
			guard.on_iteration_start(input);
			if(selection_expression.read(guard.token, input))
			{
				guard.on_token_processed(input);
			}
			guard.on_iteration_end(input);
		}

		const std::set<std::size_t> ids=selection_manager_.select_atoms(selection_expression.expression, selection_expression.full_residues);
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

		const SummaryOfAtoms old_summary=collect_summary_of_atoms();

		reset_atoms(atoms);

		output << "Restricted atoms from (";
		print_summary_of_atoms(old_summary, output);
		output << ") to (";
		print_summary_of_atoms(collect_summary_of_atoms(), output);
		output << ")\n";
	}

	void command_query_atoms(std::istringstream& input, std::ostream& output_for_log, std::ostream& output_for_content)
	{
		assert_atoms_availability();

		CommandParametersForGenericSelecting selection_expression;
		CommandParametersForGenericQueryProcessing common_query_parameters;

		while(input.good())
		{
			CommandInputParsingGuard guard;
			guard.on_iteration_start(input);
			if(selection_expression.read(guard.token, input))
			{
				guard.on_token_processed(input);
			}
			else if(common_query_parameters.read(guard.token, input))
			{
				guard.on_token_processed(input);
			}
			guard.on_iteration_end(input);
		}

		const std::set<std::size_t> ids=selection_manager_.select_atoms(selection_expression.expression, selection_expression.full_residues);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		common_query_parameters.apply_to_display_states(ids, atoms_display_states_);

		if(common_query_parameters.print)
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const Atom& atom=atoms_[*it];
				output_for_content << atom << "\n";
			}
		}

		{
			output_for_log << "Summary of atoms: ";
			print_summary_of_atoms(collect_summary_of_atoms(ids), output_for_log);
			output_for_log << "\n";
		}

		if(!common_query_parameters.name.empty())
		{
			selection_manager_.set_atoms_selection(common_query_parameters.name, ids);
			output_for_log << "Set selection of atoms named '" << common_query_parameters.name << "'\n";
		}
	}

	void command_list_selections_of_atoms(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_selections_availability();

		if((input >> std::ws).good())
		{
			throw std::runtime_error(std::string("No additional parameters allowed."));
		}

		const std::map< std::string, std::set<std::size_t> >& map_of_selections=selection_manager_.map_of_atoms_selections();
		if(map_of_selections.empty())
		{
			output << "No selections of atoms to list\n";
		}
		else
		{
			output << "Selections of atoms:\n";
			for(std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_selections.begin();it!=map_of_selections.end();++it)
			{
				output << "  name='" << (it->first) << "' ";
				print_summary_of_atoms(collect_summary_of_atoms(it->second), output);
				output << "\n";
			}
		}
	}

	void command_delete_all_selections_of_atoms(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_selections_availability();

		if((input >> std::ws).good())
		{
			throw std::runtime_error(std::string("No additional parameters allowed."));
		}

		selection_manager_.delete_atoms_selections();
		output << "Removed all selections of atoms\n";
	}

	void command_delete_selections_of_atoms(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_selections_availability();

		std::vector<std::string> names;

		while(input.good())
		{
			CommandInputParsingGuard guard;
			guard.on_iteration_start(input);
			if(selection_manager_.map_of_atoms_selections().count(guard.token)>0)
			{
				names.push_back(guard.token);
			}
			else
			{
				throw std::runtime_error(std::string("Invalid atoms selection name '")+guard.token+"'.");
			}
			guard.on_iteration_end(input);
		}

		if(names.empty())
		{
			throw std::runtime_error(std::string("No atoms selection names provided."));
		}

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

		std::pair<std::string, std::string> rename;

		input >> rename.first >> rename.second;

		if(input.fail() || rename.first.empty() || rename.second.empty())
		{
			throw std::runtime_error(std::string("Missing a pair of names for renaming."));
		}

		if((input >> std::ws).good())
		{
			throw std::runtime_error(std::string("Too many parameters, only a pair of names for renaming is needed."));
		}

		const std::set<std::size_t> ids=selection_manager_.get_atoms_selection(rename.first);
		selection_manager_.set_atoms_selection(rename.second, ids);
		selection_manager_.delete_atoms_selection(rename.first);
		output << "Renamed selection of atoms from '" << rename.first << "' to '" << rename.second << "'\n";
	}

	void command_construct_contacts(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_availability();

		ConstructionOfContacts::construct_bundle_of_contact_information construct_bundle_of_contact_information;
		construct_bundle_of_contact_information.calculate_volumes=true;

		ConstructionOfContacts::enhance_contacts enhance_contacts;
		enhance_contacts.tag_centrality=true;
		enhance_contacts.tag_peripherial=true;

		bool render=false;
		CommandParametersForGenericSelecting rendering_selection_expression;
		rendering_selection_expression.type_for_expression="render-use";
		rendering_selection_expression.type_for_full_residues="render-full-residues";
		rendering_selection_expression.expression="{min-seq-sep 1}";
		rendering_selection_expression.full_residues=false;

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
			else if(rendering_selection_expression.read(guard.token, input))
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

			for(std::size_t i=0;i<bundle_of_contact_information.volumes.size() && i<atoms_.size();i++)
			{
				atoms_[i].value.props.adjuncts["volume"]=bundle_of_contact_information.volumes[i];
			}

			std::set<std::size_t> draw_ids;
			if(render)
			{
				draw_ids=selection_manager_.select_contacts(rendering_selection_expression.expression, rendering_selection_expression.full_residues);
			}

			enhance_contacts(bundle_of_triangulation_information, draw_ids, contacts_);

			output << "Constructed contacts (";
			print_summary_of_contacts(collect_summary_of_contacts(), output);
			output << ")\n";
		}
		else
		{
			throw std::runtime_error(std::string("Failed to construct contacts."));
		}
	}

	void command_query_contacts(std::istringstream& input, std::ostream& output_for_log, std::ostream& output_for_content)
	{
		assert_contacts_availability();

		CommandParametersForGenericSelecting selection_expression;
		CommandParametersForGenericQueryProcessing common_query_parameters;

		while(input.good())
		{
			CommandInputParsingGuard guard;
			guard.on_iteration_start(input);
			if(selection_expression.read(guard.token, input))
			{
				guard.on_token_processed(input);
			}
			else if(common_query_parameters.read(guard.token, input))
			{
				guard.on_token_processed(input);
			}
			guard.on_iteration_end(input);
		}

		const std::set<std::size_t> ids=selection_manager_.select_contacts(selection_expression.expression, selection_expression.full_residues);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		common_query_parameters.apply_to_display_states(ids, contacts_display_states_);

		if(common_query_parameters.print)
		{
			enabled_output_of_ContactValue_graphics()=false;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const Contact& contact=contacts_[*it];
				if(contact.solvent())
				{
					output_for_content << atoms_[contact.ids[0]].crad << " " << ChainResidueAtomDescriptor::solvent();
				}
				else
				{
					output_for_content << atoms_[contact.ids[0]].crad << " " << atoms_[contact.ids[1]].crad;
				}
				output_for_content  << " " << contact.value << "\n";
			}
			enabled_output_of_ContactValue_graphics()=true;
		}

		{
			output_for_log << "Summary of contacts: ";
			print_summary_of_contacts(collect_summary_of_contacts(ids), output_for_log);
			output_for_log << "\n";
		}

		if(!common_query_parameters.name.empty())
		{
			selection_manager_.set_contacts_selection(common_query_parameters.name, ids);
			output_for_log << "Set selection of contacts named '" << common_query_parameters.name << "'\n";
		}
	}

	void command_list_selections_of_contacts(std::istringstream& input, std::ostream& output)
	{
		assert_contacts_selections_availability();

		if((input >> std::ws).good())
		{
			throw std::runtime_error(std::string("No additional parameters allowed."));
		}

		const std::map< std::string, std::set<std::size_t> >& map_of_selections=selection_manager_.map_of_contacts_selections();
		if(map_of_selections.empty())
		{
			output << "No selections of contacts to list\n";
		}
		else
		{
			output << "Selections of contacts:\n";
			for(std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_selections.begin();it!=map_of_selections.end();++it)
			{
				output << "  name='" << (it->first) << "' ";
				print_summary_of_contacts(collect_summary_of_contacts(it->second), output);
				output << "\n";
			}
		}
	}

	void command_delete_all_selections_of_contacts(std::istringstream& input, std::ostream& output)
	{
		assert_contacts_selections_availability();

		if((input >> std::ws).good())
		{
			throw std::runtime_error(std::string("No additional parameters allowed."));
		}

		selection_manager_.delete_contacts_selections();
		output << "Removed all selections of contacts\n";
	}

	void command_delete_selections_of_contacts(std::istringstream& input, std::ostream& output)
	{
		assert_contacts_selections_availability();

		std::vector<std::string> names;

		while(input.good())
		{
			CommandInputParsingGuard guard;
			guard.on_iteration_start(input);
			if(selection_manager_.map_of_atoms_selections().count(guard.token)>0)
			{
				names.push_back(guard.token);
			}
			else
			{
				throw std::runtime_error(std::string("Invalid atoms selection name '")+guard.token+"'.");
			}
			guard.on_iteration_end(input);
		}

		if(names.empty())
		{
			throw std::runtime_error(std::string("No contacts selection names provided."));
		}

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

		std::pair<std::string, std::string> rename;

		input >> rename.first >> rename.second;

		if(input.fail() || rename.first.empty() || rename.second.empty())
		{
			throw std::runtime_error(std::string("Missing a pair of names for renaming."));
		}

		if((input >> std::ws).good())
		{
			throw std::runtime_error(std::string("Too many parameters, only a pair of names for renaming is needed."));
		}

		const std::set<std::size_t> ids=selection_manager_.get_contacts_selection(rename.first);
		selection_manager_.set_contacts_selection(rename.second, ids);
		selection_manager_.delete_contacts_selection(rename.first);
		output << "Renamed selection of contacts from '" << rename.first << "' to '" << rename.second << "'\n";
	}

	std::vector<Atom> atoms_;
	std::vector<Contact> contacts_;
	std::vector<DisplayState> atoms_display_states_;
	std::vector<DisplayState> contacts_display_states_;
	SelectionManagerForAtomsAndContacts selection_manager_;
	std::vector<CommandHistory> commands_history_;
};

}

#endif /* COMMON_MANIPULATION_MANAGER_FOR_ATOMS_AND_CONTACTS_H_ */
