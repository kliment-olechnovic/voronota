#ifndef COMMON_COMMANDING_MANAGER_FOR_ATOMS_AND_CONTACTS_H_
#define COMMON_COMMANDING_MANAGER_FOR_ATOMS_AND_CONTACTS_H_

#include "../auxiliaries/color_utilities.h"

#include "selection_manager_for_atoms_and_contacts.h"
#include "command_input.h"
#include "construction_of_secondary_structure.h"

namespace common
{

class CommandingManagerForAtomsAndContacts
{
public:
	typedef SelectionManagerForAtomsAndContacts::Atom Atom;
	typedef SelectionManagerForAtomsAndContacts::Contact Contact;

	struct DisplayState
	{
		struct Visual
		{
			bool implemented;
			bool visible;
			unsigned int color;

			Visual() : implemented(false), visible(false), color(0x7F7F7F)
			{
			}
		};

		bool drawable;
		bool marked;
		std::vector<Visual> visuals;

		DisplayState() : drawable(false), marked(false)
		{
		}

		bool visible() const
		{
			bool result=false;
			if(drawable)
			{
				for(std::size_t i=0;i<visuals.size() && !result;i++)
				{
					result=(result || visuals[i].visible);
				}
			}
			return result;
		}

		bool implemented() const
		{
			bool result=false;
			if(drawable)
			{
				for(std::size_t i=0;i<visuals.size() && !result;i++)
				{
					result=(result || visuals[i].implemented);
				}
			}
			return result;
		}
	};

	struct CommandRecord
	{
		std::string command;
		bool successful;
		bool changed_atoms;
		bool changed_contacts;
		bool changed_atoms_tags;
		bool changed_contacts_tags;
		bool changed_atoms_display_states;
		bool changed_contacts_display_states;
		std::string output_log;
		std::string output_error;

		explicit CommandRecord(const std::string& command) :
			command(command),
			successful(false),
			changed_atoms(false),
			changed_contacts(false),
			changed_atoms_tags(false),
			changed_contacts_tags(false),
			changed_atoms_display_states(false),
			changed_contacts_display_states(false)
		{
		}
	};

	struct BoundingBox
	{
		bool filled;
		apollota::SimplePoint p_min;
		apollota::SimplePoint p_max;

		BoundingBox() : filled(false)
		{
		}

		template<typename Point>
		void update(const Point& p)
		{
			if(!filled)
			{
				p_min=apollota::SimplePoint(p);
				p_max=p_min;
			}
			else
			{
				p_min.x=std::min(p_min.x, p.x);
				p_min.y=std::min(p_min.y, p.y);
				p_min.z=std::min(p_min.z, p.z);
				p_max.x=std::max(p_max.x, p.x);
				p_max.y=std::max(p_max.y, p.y);
				p_max.z=std::max(p_max.z, p.z);
			}
			filled=true;
		}
	};

	struct CommandOutputSink
	{
		std::ostringstream output_stream;
		std::set<std::size_t> output_set_of_ids;
		BoundingBox bounding_box;
	};

	CommandingManagerForAtomsAndContacts()
	{
		map_of_command_function_pointers_.insert(std::make_pair("load-atoms", &CommandingManagerForAtomsAndContacts::command_load_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("restrict-atoms", &CommandingManagerForAtomsAndContacts::command_restrict_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("save-atoms", &CommandingManagerForAtomsAndContacts::command_save_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("select-atoms", &CommandingManagerForAtomsAndContacts::command_select_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("tag-atoms", &CommandingManagerForAtomsAndContacts::command_tag_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("untag-atoms", &CommandingManagerForAtomsAndContacts::command_untag_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("mark-atoms", &CommandingManagerForAtomsAndContacts::command_mark_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("unmark-atoms", &CommandingManagerForAtomsAndContacts::command_unmark_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("show-atoms", &CommandingManagerForAtomsAndContacts::command_show_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("hide-atoms", &CommandingManagerForAtomsAndContacts::command_hide_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("color-atoms", &CommandingManagerForAtomsAndContacts::command_color_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("spectrum-atoms", &CommandingManagerForAtomsAndContacts::command_spectrum_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("set-secondary-structure-tags", &CommandingManagerForAtomsAndContacts::command_set_secondary_structure_tags));
		map_of_command_function_pointers_.insert(std::make_pair("print-atoms", &CommandingManagerForAtomsAndContacts::command_print_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("list-selections-of-atoms", &CommandingManagerForAtomsAndContacts::command_list_selections_of_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("delete-all-selections-of-atoms", &CommandingManagerForAtomsAndContacts::command_delete_all_selections_of_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("delete-selections-of-atoms", &CommandingManagerForAtomsAndContacts::command_delete_selections_of_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("rename-selection-of-atoms", &CommandingManagerForAtomsAndContacts::command_rename_selection_of_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("construct-contacts", &CommandingManagerForAtomsAndContacts::command_construct_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("save-contacts", &CommandingManagerForAtomsAndContacts::command_save_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("load-contacts", &CommandingManagerForAtomsAndContacts::command_load_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("select-contacts", &CommandingManagerForAtomsAndContacts::command_select_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("tag-contacts", &CommandingManagerForAtomsAndContacts::command_tag_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("untag-contacts", &CommandingManagerForAtomsAndContacts::command_untag_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("mark-contacts", &CommandingManagerForAtomsAndContacts::command_mark_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("unmark-contacts", &CommandingManagerForAtomsAndContacts::command_unmark_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("show-contacts", &CommandingManagerForAtomsAndContacts::command_show_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("hide-contacts", &CommandingManagerForAtomsAndContacts::command_hide_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("color-contacts", &CommandingManagerForAtomsAndContacts::command_color_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("spectrum-contacts", &CommandingManagerForAtomsAndContacts::command_spectrum_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("print-contacts", &CommandingManagerForAtomsAndContacts::command_print_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("write-contacts-as-pymol-cgo", &CommandingManagerForAtomsAndContacts::command_write_contacts_as_pymol_cgo));
		map_of_command_function_pointers_.insert(std::make_pair("list-selections-of-contacts", &CommandingManagerForAtomsAndContacts::command_list_selections_of_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("delete-all-selections-of-contacts", &CommandingManagerForAtomsAndContacts::command_delete_all_selections_of_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("delete-selections-of-contacts", &CommandingManagerForAtomsAndContacts::command_delete_selections_of_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("rename-selection-of-contacts", &CommandingManagerForAtomsAndContacts::command_rename_selection_of_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("save-atoms-and-contacts", &CommandingManagerForAtomsAndContacts::command_save_atoms_and_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("load-atoms-and-contacts", &CommandingManagerForAtomsAndContacts::command_load_atoms_and_contacts));
		map_of_command_function_pointers_.insert(std::make_pair("zoom-by-atoms", &CommandingManagerForAtomsAndContacts::command_zoom_by_atoms));
		map_of_command_function_pointers_.insert(std::make_pair("zoom-by-contacts", &CommandingManagerForAtomsAndContacts::command_zoom_by_contacts));
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

	const ConstructionOfPrimaryStructure::BundleOfPrimaryStructure& primary_structure_info() const
	{
		return primary_structure_info_;
	}

	const ConstructionOfSecondaryStructure::BundleOfSecondaryStructure& secondary_structure_info() const
	{
		return secondary_structure_info_;
	}

	const std::vector<std::string>& atoms_representation_names() const
	{
		return atoms_representation_names_;
	}

	const std::vector<std::string>& contacts_representation_names() const
	{
		return contacts_representation_names_;
	}

	bool is_any_atom_visible() const
	{
		for(std::size_t i=0;i<atoms_display_states_.size();i++)
		{
			if(atoms_display_states_[i].visible())
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_atom_marked() const
	{
		for(std::size_t i=0;i<atoms_display_states_.size();i++)
		{
			if(atoms_display_states_[i].marked)
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_contact_visible() const
	{
		for(std::size_t i=0;i<contacts_display_states_.size();i++)
		{
			if(contacts_display_states_[i].visible())
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_contact_marked() const
	{
		for(std::size_t i=0;i<contacts_display_states_.size();i++)
		{
			if(contacts_display_states_[i].marked)
			{
				return true;
			}
		}
		return false;
	}

	bool add_representations_of_atoms(const std::vector<std::string>& names)
	{
		if(add_names_to_representations(names, atoms_representation_names_))
		{
			resize_visuals_in_atoms_display_states();
			return true;
		}
		return false;
	}

	bool add_representations_of_contacts(const std::vector<std::string>& names)
	{
		if(add_names_to_representations(names, contacts_representation_names_))
		{
			resize_visuals_in_contacts_display_states();
			return true;
		}
		return false;
	}

	bool set_atoms_representation_implemented_always(const std::size_t representation_id, const bool status)
	{
		return set_representation_implemented_always(atoms_representation_names_, representation_id, status, atoms_representations_implemented_always_);
	}

	bool set_contacts_representation_implemented_always(const std::size_t representation_id, const bool status)
	{
		return set_representation_implemented_always(contacts_representation_names_, representation_id, status, contacts_representations_implemented_always_);
	}

	bool set_atoms_representation_implemented(const std::size_t representation_id, const std::vector<bool>& statuses)
	{
		return set_representation_implemented(atoms_representation_names_, representation_id, statuses, atoms_display_states_);
	}

	bool set_contacts_representation_implemented(const std::size_t representation_id, const std::vector<bool>& statuses)
	{
		return set_representation_implemented(contacts_representation_names_, representation_id, statuses, contacts_display_states_);
	}

	bool executable(const std::string& command) const
	{
		std::string verb;
		std::istringstream input(command);
		input >> verb;
		return (!verb.empty() && map_of_command_function_pointers_.count(verb)>0);
	}

	CommandRecord execute(const std::string& command, CommandOutputSink& sink)
	{
		CommandRecord record(command);

		if(executable(command))
		{
			std::ostringstream output_for_log;
			std::ostringstream output_for_errors;

			CommandInput input;
			CommandArguments cargs(input, output_for_log, sink.output_stream, sink.output_set_of_ids, sink.bounding_box);

			try
			{
				input.init(command);
				sync_selections_with_display_states_if_needed(command);
				CommandFunctionPointer cfp=map_of_command_function_pointers_.find(input.get_command_name())->second;
				(this->*cfp)(cargs);
				record.successful=true;
			}
			catch(const std::exception& e)
			{
				output_for_errors << e.what();
			}

			record.output_log=output_for_log.str();
			record.output_error=output_for_errors.str();

			record.changed_atoms=cargs.changed_atoms;
			record.changed_contacts=(cargs.changed_contacts || record.changed_atoms);
			record.changed_atoms_tags=(cargs.changed_atoms_tags || record.changed_atoms);
			record.changed_contacts_tags=(cargs.changed_contacts_tags || record.changed_contacts);
			record.changed_atoms_display_states=(cargs.changed_atoms_display_states || record.changed_atoms);
			record.changed_contacts_display_states=(cargs.changed_contacts_display_states || record.changed_contacts);

			if(record.changed_atoms && !atoms_representations_implemented_always_.empty())
			{
				for(std::set<std::size_t>::const_iterator it=atoms_representations_implemented_always_.begin();it!=atoms_representations_implemented_always_.end();++it)
				{
					set_atoms_representation_implemented(*it, std::vector<bool>(atoms_.size(), true));
				}
			}

			if(record.changed_contacts && !contacts_representations_implemented_always_.empty())
			{
				for(std::set<std::size_t>::const_iterator it=contacts_representations_implemented_always_.begin();it!=contacts_representations_implemented_always_.end();++it)
				{
					set_contacts_representation_implemented(*it, std::vector<bool>(contacts_.size(), true));
				}
			}
		}

		return record;
	}

	CommandRecord execute(const std::string& command)
	{
		CommandOutputSink sink;
		return execute(command, sink);
	}

private:
	class CommandArguments
	{
	public:
		CommandInput& input;
		std::ostream& output_for_log;
		std::ostream& output_for_data;
		std::set<std::size_t>& output_set_of_ids;
		BoundingBox& bounding_box;
		bool changed_atoms;
		bool changed_contacts;
		bool changed_atoms_tags;
		bool changed_contacts_tags;
		bool changed_atoms_display_states;
		bool changed_contacts_display_states;

		CommandArguments(
				CommandInput& input,
				std::ostream& output_for_log,
				std::ostream& output_for_data,
				std::set<std::size_t>& output_set_of_ids,
				BoundingBox& bounding_box) :
					input(input),
					output_for_log(output_for_log),
					output_for_data(output_for_data),
					output_set_of_ids(output_set_of_ids),
					bounding_box(bounding_box),
					changed_atoms(false),
					changed_contacts(false),
					changed_atoms_tags(false),
					changed_contacts_tags(false),
					changed_atoms_display_states(false),
					changed_contacts_display_states(false)
		{
		}
	};

	typedef void (CommandingManagerForAtomsAndContacts::*CommandFunctionPointer)(CommandArguments&);

	class SummaryOfAtoms
	{
	public:
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

	class SummaryOfContacts
	{
	public:
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

	class CommandParametersForGenericSelecting
	{
	public:
		std::string type_for_expression;
		std::string type_for_full_residues;
		std::string type_for_forced_id;
		std::string expression;
		bool full_residues;
		std::set<std::size_t> forced_ids;

		CommandParametersForGenericSelecting() :
			type_for_expression("use"),
			type_for_full_residues("full-residues"),
			type_for_forced_id("id"),
			expression("{}"),
			full_residues(false)
		{
		}

		CommandParametersForGenericSelecting(const std::string& types_prefix, const std::string& default_expression) :
			type_for_expression(types_prefix+"use"),
			type_for_full_residues(types_prefix+"full-residues"),
			type_for_forced_id(types_prefix+"id"),
			expression(default_expression),
			full_residues(false)
		{
		}

		void read(CommandInput& input)
		{
			if(input.is_option(type_for_expression))
			{
				expression=input.get_value<std::string>(type_for_expression);
			}
			else if(type_for_expression=="use" && input.is_any_unnamed_value_unused())
			{
				bool found=false;
				for(std::size_t i=0;i<input.get_list_of_unnamed_values().size() && !found;i++)
				{
					if(!input.is_unnamed_value_used(i))
					{
						const std::string& candidate=input.get_list_of_unnamed_values()[i];
						if(!candidate.empty() && candidate.find_first_of("({")==0)
						{
							expression=candidate;
							input.mark_unnamed_value_as_used(i);
							found=true;
						}
					}
				}
			}

			full_residues=input.get_flag(type_for_full_residues);

			{
				const std::vector<std::size_t> forced_ids_vector=input.get_value_vector_or_default<std::size_t>(type_for_forced_id, std::vector<std::size_t>());
				if(!forced_ids_vector.empty())
				{
					forced_ids.insert(forced_ids_vector.begin(), forced_ids_vector.end());
				}
			}
		}
	};

	class CommandParametersForGenericViewing
	{
	public:
		bool mark;
		bool unmark;
		bool show;
		bool hide;
		unsigned int color;
		std::set<std::size_t> visual_ids_;

		CommandParametersForGenericViewing() :
			mark(false),
			unmark(false),
			show(false),
			hide(false),
			color(0)
		{
		}

		void assert_state() const
		{
			if(hide && show)
			{
				throw std::runtime_error(std::string("Cannot show and hide at the same time."));
			}

			if(mark && unmark)
			{
				throw std::runtime_error(std::string("Cannot mark and unmark at the same time."));
			}
		}

		bool apply_to_display_state(const std::size_t id, std::vector<DisplayState>& display_states) const
		{
			bool updated=false;
			if((show || hide || mark || unmark || color>0) && id<display_states.size())
			{
				DisplayState& ds=display_states[id];
				if(ds.implemented())
				{
					if(mark || unmark)
					{
						updated=(updated || (ds.marked!=mark));
						ds.marked=mark;
					}

					if(show || hide || color>0)
					{
						if(visual_ids_.empty())
						{
							for(std::size_t i=0;i<ds.visuals.size();i++)
							{
								if(apply_to_display_state_visual(ds.visuals[i]))
								{
									updated=true;
								}
							}
						}
						else
						{
							for(std::set<std::size_t>::const_iterator jt=visual_ids_.begin();jt!=visual_ids_.end();++jt)
							{
								const std::size_t visual_id=(*jt);
								if(visual_id<ds.visuals.size())
								{
									if(apply_to_display_state_visual(ds.visuals[visual_id]))
									{
										updated=true;
									}
								}
							}
						}
					}
				}
			}
			return updated;
		}

		bool apply_to_display_states(const std::set<std::size_t>& ids, std::vector<DisplayState>& display_states) const
		{
			bool updated=false;
			if(show || hide || mark || unmark || color>0)
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					if(apply_to_display_state((*it), display_states))
					{
						updated=true;
					}
				}
			}
			return updated;
		}

		bool apply_to_display_states(std::vector<DisplayState>& display_states) const
		{
			bool updated=false;
			if(show || hide || mark || unmark || color>0)
			{
				for(std::size_t i=0;i<display_states.size();i++)
				{
					if(apply_to_display_state(i, display_states))
					{
						updated=true;
					}
				}
			}
			return updated;
		}

		bool apply_to_display_state_visual(DisplayState::Visual& visual) const
		{
			bool updated=false;

			if(visual.implemented)
			{
				if(show || hide)
				{
					updated=(updated || (visual.visible!=show));
					visual.visible=show;
				}

				if(color>0)
				{
					updated=(updated || (visual.color!=color));
					visual.color=color;
				}
			}

			return updated;
		}
	};

	class CommandParametersForGenericRepresentationSelecting
	{
	public:
		const std::vector<std::string>& available_representations;
		std::set<std::size_t> visual_ids_;

		explicit CommandParametersForGenericRepresentationSelecting(const std::vector<std::string>& available_representations) : available_representations(available_representations)
		{
		}

		void read(CommandInput& input)
		{
			if(input.is_option("rep"))
			{
				const std::vector<std::string> names=input.get_value_vector<std::string>("rep");
				std::set<std::size_t> ids;
				for(std::size_t i=0;i<names.size();i++)
				{
					const std::string& name=names[i];
					std::size_t id=find_name_id(available_representations, name);
					if(id<available_representations.size())
					{
						ids.insert(id);
					}
					else
					{
						throw std::runtime_error(std::string("Representation '")+name+"' does not exist.");
					}
				}
				visual_ids_.swap(ids);
			}
		}
	};

	class CommandParametersForGenericColoring
	{
	public:
		auxiliaries::ColorUtilities::ColorInteger color;

		CommandParametersForGenericColoring() : color(auxiliaries::ColorUtilities::null_color())
		{
		}

		void read(CommandInput& input)
		{
			if(input.is_option("col"))
			{
				color=auxiliaries::ColorUtilities::color_from_name(input.get_value<std::string>("col"));
			}
			else if(input.is_any_unnamed_value_unused())
			{
				bool found=false;
				for(std::size_t i=0;i<input.get_list_of_unnamed_values().size() && !found;i++)
				{
					if(!input.is_unnamed_value_used(i))
					{
						const std::string& candidate_str=input.get_list_of_unnamed_values()[i];
						if(candidate_str.size()>2 && candidate_str.compare(0, 2, "0x")==0)
						{
							auxiliaries::ColorUtilities::ColorInteger candidate_color=auxiliaries::ColorUtilities::color_from_name(candidate_str);
							if(candidate_color!=auxiliaries::ColorUtilities::null_color())
							{
								color=candidate_color;
								input.mark_unnamed_value_as_used(i);
								found=true;
							}
						}
					}
				}
			}
		}
	};

	class CommandParametersForGenericTablePrinting
	{
	public:
		bool reversed_sorting;
		bool expanded_descriptors;
		std::size_t limit;
		std::string sort_column;

		CommandParametersForGenericTablePrinting() :
			reversed_sorting(false),
			expanded_descriptors(false),
			limit(std::numeric_limits<std::size_t>::max())
		{
		}

		void read(CommandInput& input)
		{
			reversed_sorting=input.get_flag("desc");
			expanded_descriptors=input.get_flag("expand");
			limit=input.get_value_or_default<std::size_t>("limit", std::numeric_limits<std::size_t>::max());
			sort_column=input.get_value_or_default<std::string>("sort", "");
		}
	};

	class CommandParametersForContactsTablePrinting : public CommandParametersForGenericTablePrinting
	{
	public:
		bool inter_residue;

		CommandParametersForContactsTablePrinting() : inter_residue(false)
		{
		}

		void read(CommandInput& input)
		{
			CommandParametersForGenericTablePrinting::read(input);
			inter_residue=input.get_flag("inter-residue");
		}
	};

	class CommandParametersForGenericOutputDestinations
	{
	public:
		std::string file;
		bool use_stdout;
		std::ofstream foutput;

		explicit CommandParametersForGenericOutputDestinations(const bool use_stdout) : use_stdout(use_stdout)
		{
		}

		void read(const bool allow_use_of_unnamed_value, CommandInput& input)
		{
			if(input.is_option("file") || (allow_use_of_unnamed_value && input.is_any_unnamed_value_unused()))
			{
				const std::string str=(allow_use_of_unnamed_value ? input.get_value_or_first_unused_unnamed_value("file") : input.get_value<std::string>("file"));
				if(!str.empty() && str.find_first_of("?*$'\";:<>,|")==std::string::npos)
				{
					file=str;
				}
				else
				{
					throw std::runtime_error(std::string("Invalid file name '")+str+"'.");
				}
			}

			if(input.is_option("use-stdout"))
			{
				use_stdout=input.get_flag("use-stdout");
			}
		}

		std::vector<std::ostream*> get_output_destinations(std::ostream* stdout_ptr, const bool allow_empty_list=false)
		{
			std::vector<std::ostream*> list;

			if(use_stdout && stdout_ptr!=0)
			{
				list.push_back(stdout_ptr);
			}

			if(!file.empty())
			{
				if(!foutput.is_open())
				{
					foutput.open(file.c_str(), std::ios::out);
					if(!foutput.good())
					{
						throw std::runtime_error(std::string("Failed to open file '")+file+"' for writing.");
					}
				}
				if(!foutput.good())
				{
					throw std::runtime_error(std::string("Failed to use file '")+file+"' for writing.");
				}
				list.push_back(&foutput);
			}

			if(list.empty() && !allow_empty_list)
			{
				throw std::runtime_error(std::string("No output destinations specified."));
			}

			return list;
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

	static void resize_visuals_in_display_states(const std::size_t size, std::vector<DisplayState>& display_states)
	{
		for(std::size_t i=0;i<display_states.size();i++)
		{
			if(display_states[i].drawable && display_states[i].visuals.size()!=size)
			{
				display_states[i].visuals.resize(size);
			}
		}
	}

	static std::size_t find_name_id(const std::vector<std::string>& names, const std::string& name)
	{
		std::size_t id=names.size();
		for(std::size_t i=0;i<names.size() && !(id<names.size());i++)
		{
			if(names[i]==name)
			{
				id=i;
			}
		}
		return id;
	}

	static bool add_names_to_representations(const std::vector<std::string>& names, std::vector<std::string>& representations)
	{
		if(names.empty())
		{
			return false;
		}

		for(std::size_t i=0;i<names.size();i++)
		{
			const std::string& name=names[i];
			if(name.empty())
			{
				return false;
			}
			else if(std::find(representations.begin(), representations.end(), name)!=representations.end())
			{
				return false;
			}
		}

		representations.insert(representations.end(), names.begin(), names.end());

		return true;
	}

	static bool set_representation_implemented_always(
			const std::vector<std::string>& representations,
			const std::size_t representation_id,
			const bool status,
			std::set<std::size_t>& representations_implemented_always)
	{
		if(representation_id>=representations.size())
		{
			return false;
		}

		if(status)
		{
			representations_implemented_always.insert(representation_id);
		}
		else
		{
			representations_implemented_always.erase(representation_id);
		}

		return true;
	}

	static bool set_representation_implemented(
			const std::vector<std::string>& representations,
			const std::size_t representation_id,
			const std::vector<bool>& statuses,
			std::vector<DisplayState>& display_states)
	{
		if(statuses.size()!=display_states.size())
		{
			return false;
		}

		if(representation_id>=representations.size())
		{
			return false;
		}

		for(std::size_t i=0;i<display_states.size();i++)
		{
			if(display_states[i].drawable && representation_id>=display_states[i].visuals.size())
			{
				return false;
			}
		}

		for(std::size_t i=0;i<display_states.size();i++)
		{
			if(display_states[i].drawable)
			{
				display_states[i].visuals[representation_id].implemented=statuses[i];
			}
		}

		return true;
	}

	static std::set<std::size_t> filter_drawable_implemented_ids(
			const std::vector<DisplayState>& display_states,
			const std::set<std::size_t>& visual_ids,
			const std::set<std::size_t>& ids,
			const bool only_visible)
	{
		std::set<std::size_t> drawable_ids;
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			if((*it)<display_states.size() && display_states[*it].drawable)
			{
				bool good=false;
				if(visual_ids.empty())
				{
					good=display_states[*it].implemented() && (!only_visible || display_states[*it].visible());
				}
				else
				{
					for(std::set<std::size_t>::const_iterator jt=visual_ids.begin();jt!=visual_ids.end() && !good;++jt)
					{
						good=(good ||
								((*jt)<display_states[*it].visuals.size() &&
										display_states[*it].visuals[*jt].implemented &&
										(!only_visible || display_states[*it].visuals[*jt].visible)));
					}
				}

				if(good)
				{
					drawable_ids.insert(*it);
				}
			}
		}
		return drawable_ids;
	}

	static std::set<std::size_t> filter_drawable_implemented_ids(
			const std::vector<DisplayState>& display_states,
			const std::set<std::size_t>& visual_ids,
			const std::set<std::size_t>& ids)
	{
		return filter_drawable_implemented_ids(display_states, visual_ids, ids, false);
	}

	static void assert_selection_name_input(const std::string& name, const bool allow_empty)
	{
		if(name.empty())
		{
			if(!allow_empty)
			{
				throw std::runtime_error(std::string("Selection name is empty."));
			}
		}
		else if(name.find_first_of("{}()[]<>,;.:\\/+*/='\"@#$%^&`~?|")!=std::string::npos)
		{
			throw std::runtime_error(std::string("Selection name contains invalid symbols."));
		}
		else if(name.compare(0, 1, "-")==0 || name.compare(0, 1, "_")==0)
		{
			throw std::runtime_error(std::string("Selection name starts with invalid symbol."));
		}
	}

	static void assert_tag_input(const std::string& tag)
	{
		if(tag.empty())
		{
			throw std::runtime_error(std::string("Tag is empty."));
		}
		else if(tag.find_first_of("{}()[]<>,;.:\\/+*/'\"@#$%^&`~?|")!=std::string::npos)
		{
			throw std::runtime_error(std::string("Tag contains invalid symbols."));
		}
		else if(tag.compare(0, 1, "-")==0)
		{
			throw std::runtime_error(std::string("Tag starts with invalid symbol."));
		}
	}

	static std::string get_format_from_atoms_file_name(const std::string& filename)
	{
		std::multimap<std::string, std::string> map_of_format_extensions;
		map_of_format_extensions.insert(std::pair<std::string, std::string>("pdb", ".pdb"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("pdb", ".PDB"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("pdb", ".ent"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("pdb", ".ENT"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".cif"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".CIF"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".mmcif"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".mmCIF"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".MMCIF"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("plain", ".atoms"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("plain", ".pa"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("plain", ".pac"));
		for(std::multimap<std::string, std::string>::const_iterator it=map_of_format_extensions.begin();it!=map_of_format_extensions.end();++it)
		{
			const std::string& format=it->first;
			const std::string& extension=it->second;
			const std::size_t pos=filename.find(extension);
			if(pos<filename.size() && (pos+extension.size())==filename.size())
			{
				return format;
			}
		}
		return std::string();
	}

	void assert_atoms_representations_availability() const
	{
		if(atoms_representation_names_.empty())
		{
			throw std::runtime_error(std::string("No atoms representations available."));
		}
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

	void assert_contacts_representations_availability() const
	{
		if(contacts_representation_names_.empty())
		{
			throw std::runtime_error(std::string("No contacts representations available."));
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

	void reset_atoms(std::vector<Atom>& atoms)
	{
		if(atoms.empty())
		{
			throw std::runtime_error(std::string("No atoms to set."));
		}
		atoms_.swap(atoms);
		reset_atoms_display_states();
		contacts_.clear();
		contacts_display_states_.clear();
		primary_structure_info_=ConstructionOfPrimaryStructure::construct_bundle_of_primary_structure(atoms_);
		secondary_structure_info_=ConstructionOfSecondaryStructure::construct_bundle_of_secondary_structure(atoms_, primary_structure_info_);
		selection_manager_=SelectionManagerForAtomsAndContacts(&atoms_, 0);
	}

	void reset_atoms_display_states()
	{
		atoms_display_states_.clear();
		atoms_display_states_.resize(atoms_.size());
		for(std::size_t i=0;i<atoms_display_states_.size();i++)
		{
			atoms_display_states_[i].drawable=true;
		}
		resize_visuals_in_atoms_display_states();
	}

	void resize_visuals_in_atoms_display_states()
	{
		resize_visuals_in_display_states(atoms_representation_names_.size(), atoms_display_states_);
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
		reset_contacts_display_states();
		selection_manager_.set_contacts(&contacts_);
	}

	void reset_contacts_display_states()
	{
		contacts_display_states_.clear();
		contacts_display_states_.resize(contacts_.size());
		for(std::size_t i=0;i<contacts_display_states_.size();i++)
		{
			contacts_display_states_[i].drawable=(!contacts_[i].value.graphics.empty());
		}
		resize_visuals_in_contacts_display_states();
	}

	void resize_visuals_in_contacts_display_states()
	{
		resize_visuals_in_display_states(contacts_representation_names_.size(), contacts_display_states_);
	}

	void sync_atoms_selections_with_display_states()
	{
		if(!atoms_display_states_.empty())
		{
			std::set<std::size_t> ids_visible;
			std::set<std::size_t> ids_marked;
			for(std::size_t i=0;i<atoms_display_states_.size();i++)
			{
				const DisplayState& ds=atoms_display_states_[i];
				if(ds.visible())
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
	}

	void sync_contacts_selections_with_display_states()
	{
		if(!contacts_display_states_.empty())
		{
			std::set<std::size_t> ids_visible;
			std::set<std::size_t> ids_marked;
			for(std::size_t i=0;i<contacts_display_states_.size();i++)
			{
				const DisplayState& ds=contacts_display_states_[i];
				if(ds.visible())
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
	}

	void sync_selections_with_display_states_if_needed(const std::string& command)
	{
		if(command.find("selection")!=std::string::npos || command.find("_marked")!=std::string::npos || command.find("_visible")!=std::string::npos)
		{
			sync_atoms_selections_with_display_states();
			sync_contacts_selections_with_display_states();
		}
	}

	void update_bounding_box_with_atoms(const std::set<std::size_t>& ids, BoundingBox& box) const
	{
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			if((*it)<atoms_.size())
			{
				box.update(atoms_[*it].value);
			}
		}
	}

	void command_load_atoms(CommandArguments& cargs)
	{
		ConstructionOfAtomicBalls::ParametersToCollectAtomicBallsFromFile parameters_to_collect_atoms;
		parameters_to_collect_atoms.include_heteroatoms=cargs.input.get_flag("include-heteroatoms");
		parameters_to_collect_atoms.include_hydrogens=cargs.input.get_flag("include-hydrogens");
		parameters_to_collect_atoms.multimodel_chains=cargs.input.get_flag("as-assembly");
		const std::string atoms_file=cargs.input.get_value_or_first_unused_unnamed_value("file");
		const std::string radii_file=cargs.input.get_value_or_default<std::string>("radii-file", "");
		const double default_radius=cargs.input.get_value_or_default<double>("default-radii", ConstructionOfAtomicBalls::ParametersToCollectAtomicBallsFromFile::default_default_radius());
		const bool only_default_radius=cargs.input.get_flag("same-radius-for-all");
		std::string format=cargs.input.get_value_or_default<std::string>("format", "");

		cargs.input.assert_nothing_unusable();

		if(atoms_file.empty())
		{
			throw std::runtime_error(std::string("Empty input atoms file name."));
		}

		if(format.empty())
		{
			format=get_format_from_atoms_file_name(atoms_file);
			if(format.empty())
			{
				throw std::runtime_error(std::string("Could not deduce format from file name '")+atoms_file+"'.");
			}
		}

		if(format!="pdb" && format!="mmcif" && format!="plain")
		{
			throw std::runtime_error(std::string("Unrecognized format '")+format+"', allowed formats are 'pdb', 'mmcif' or 'plain'.");
		}

		if(!radii_file.empty() || only_default_radius || default_radius!=ConstructionOfAtomicBalls::ParametersToCollectAtomicBallsFromFile::default_default_radius())
		{
			parameters_to_collect_atoms.set_atom_radius_assigner(default_radius, only_default_radius, radii_file);
		}

		std::vector<Atom> atoms;
		bool success=false;

		if(format=="pdb" || format=="mmcif")
		{
			if(format=="mmcif")
			{
				parameters_to_collect_atoms.mmcif=true;
			}
			success=ConstructionOfAtomicBalls::collect_atomic_balls_from_file(parameters_to_collect_atoms, atoms_file, atoms);
		}
		else if(format=="plain")
		{
			auxiliaries::IOUtilities(true, '\n', ' ', "_end_atoms").read_file_lines_to_set(atoms_file, atoms);
			if(!atoms.empty())
			{
				if(!radii_file.empty() || only_default_radius)
				{
					for(std::size_t i=0;i<atoms.size();i++)
					{
						Atom& atom=atoms[i];
						atom.value.r=parameters_to_collect_atoms.atom_radius_assigner.get_atom_radius(atom.crad.resName, atom.crad.name);
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
				cargs.changed_atoms=true;

				cargs.output_for_log << "Read atoms from file '" << atoms_file << "' (";
				SummaryOfAtoms::collect_summary(atoms_).print(cargs.output_for_log);
				cargs.output_for_log << ")\n";
			}
		}
		else
		{
			throw std::runtime_error(std::string("Failed to read atoms from file '")+atoms_file+"' in '"+format+"' format.");
		}
	}

	void command_restrict_atoms(CommandArguments& cargs)
	{
		assert_atoms_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);

		cargs.input.assert_nothing_unusable();

		const std::set<std::size_t> ids=selection_manager_.select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
		if(ids.size()<4)
		{
			throw std::runtime_error(std::string("Less than 4 atoms selected."));
		}

		if(ids.size()<atoms_.size())
		{
			std::vector<Atom> atoms;
			atoms.reserve(ids.size());
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				atoms.push_back(atoms_.at(*it));
			}

			const SummaryOfAtoms old_summary=SummaryOfAtoms::collect_summary(atoms_);

			reset_atoms(atoms);
			cargs.changed_atoms=true;

			cargs.output_for_log << "Restricted atoms from (";
			old_summary.print(cargs.output_for_log);
			cargs.output_for_log << ") to (";
			SummaryOfAtoms::collect_summary(atoms_).print(cargs.output_for_log);
			cargs.output_for_log << ")\n";
		}
		else
		{
			cargs.output_for_log << "No need to restrict because all atoms were selected (";
			SummaryOfAtoms::collect_summary(atoms_).print(cargs.output_for_log);
			cargs.output_for_log << ")\n";
		}
	}

	void command_save_atoms(CommandArguments& cargs)
	{
		assert_atoms_availability();

		CommandParametersForGenericOutputDestinations parameters_for_output_destinations(false);
		parameters_for_output_destinations.read(true, cargs.input);

		cargs.input.assert_nothing_unusable();

		std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(0);

		for(std::size_t i=0;i<outputs.size();i++)
		{
			std::ostream& output=(*(outputs[i]));
			auxiliaries::IOUtilities().write_set(atoms_, output);
		}

		if(!parameters_for_output_destinations.file.empty())
		{
			cargs.output_for_log << "Wrote atoms to file '" << parameters_for_output_destinations.file << "' (";
			SummaryOfAtoms::collect_summary(atoms_).print(cargs.output_for_log);
			cargs.output_for_log << ")\n";
		}
	}

	void command_select_atoms(CommandArguments& cargs)
	{
		assert_atoms_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);
		const std::string name=(cargs.input.is_any_unnamed_value_unused() ?
				cargs.input.get_value_or_first_unused_unnamed_value("name") :
				cargs.input.get_value_or_default<std::string>("name", ""));
		const bool no_marking=cargs.input.get_flag("no-marking");

		cargs.input.assert_nothing_unusable();

		assert_selection_name_input(name, true);

		std::set<std::size_t> ids=selection_manager_.select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		{
			cargs.output_for_log << "Summary of atoms: ";
			SummaryOfAtoms::collect_summary(atoms_, ids).print(cargs.output_for_log);
			cargs.output_for_log << "\n";
		}

		if(!name.empty())
		{
			selection_manager_.set_atoms_selection(name, ids);
			cargs.output_for_log << "Set selection of atoms named '" << name << "'\n";
		}

		if(!no_marking)
		{
			{
				CommandParametersForGenericViewing params;
				params.unmark=true;
				if(params.apply_to_display_states(atoms_display_states_))
				{
					cargs.changed_atoms_display_states=true;
				}
			}
			{
				CommandParametersForGenericViewing params;
				params.mark=true;
				if(params.apply_to_display_states(ids, atoms_display_states_))
				{
					cargs.changed_atoms_display_states=true;
				}
			}
		}

		cargs.output_set_of_ids.swap(ids);
	}

	void command_tag_atoms(CommandArguments& cargs)
	{
		configurable_command_tag_atoms(true, cargs);
	}

	void command_untag_atoms(CommandArguments& cargs)
	{
		configurable_command_tag_atoms(false, cargs);
	}

	void configurable_command_tag_atoms(const bool positive, CommandArguments& cargs)
	{
		assert_atoms_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);
		const std::string tag=cargs.input.get_value_or_first_unused_unnamed_value("tag");

		cargs.input.assert_nothing_unusable();

		assert_tag_input(tag);

		std::set<std::size_t> ids=selection_manager_.select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			Atom& atom=atoms_[*it];
			if(positive)
			{
				atom.value.props.tags.insert(tag);
			}
			else
			{
				atom.value.props.tags.erase(tag);
			}
		}

		{
			cargs.output_for_log << "Summary of atoms: ";
			SummaryOfAtoms::collect_summary(atoms_, ids).print(cargs.output_for_log);
			cargs.output_for_log << "\n";
		}
	}

	void command_mark_atoms(CommandArguments& cargs)
	{
		configurable_command_mark_atoms(true, cargs);
	}

	void command_unmark_atoms(CommandArguments& cargs)
	{
		configurable_command_mark_atoms(false, cargs);
	}

	void configurable_command_mark_atoms(const bool positive, CommandArguments& cargs)
	{
		assert_atoms_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);

		cargs.input.assert_nothing_unusable();

		const std::set<std::size_t> ids=filter_drawable_implemented_ids(
				atoms_display_states_,
				std::set<std::size_t>(),
				selection_manager_.select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues));
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable atoms selected."));
		}

		{
			CommandParametersForGenericViewing parameters_for_viewing;
			parameters_for_viewing.mark=positive;
			parameters_for_viewing.unmark=!positive;
			if(parameters_for_viewing.apply_to_display_states(ids, atoms_display_states_))
			{
				cargs.changed_atoms_display_states=true;
			}
		}

		{
			cargs.output_for_log << "Summary of atoms: ";
			SummaryOfAtoms::collect_summary(atoms_, ids).print(cargs.output_for_log);
			cargs.output_for_log << "\n";
		}
	}

	void command_show_atoms(CommandArguments& cargs)
	{
		configurable_command_show_atoms(true, cargs);
	}

	void command_hide_atoms(CommandArguments& cargs)
	{
		configurable_command_show_atoms(false, cargs);
	}

	void configurable_command_show_atoms(const bool positive, CommandArguments& cargs)
	{
		assert_atoms_availability();
		assert_atoms_representations_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);
		CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(atoms_representation_names_);
		parameters_for_representation_selecting.read(cargs.input);

		cargs.input.assert_nothing_unusable();

		if(positive && parameters_for_representation_selecting.visual_ids_.empty())
		{
			parameters_for_representation_selecting.visual_ids_.insert(0);
		}

		const std::set<std::size_t> ids=filter_drawable_implemented_ids(
				atoms_display_states_,
				parameters_for_representation_selecting.visual_ids_,
				selection_manager_.select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues));
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable atoms selected."));
		}

		CommandParametersForGenericViewing parameters_for_viewing;
		parameters_for_viewing.visual_ids_=parameters_for_representation_selecting.visual_ids_;
		parameters_for_viewing.show=positive;
		parameters_for_viewing.hide=!positive;

		parameters_for_viewing.assert_state();

		if(parameters_for_viewing.apply_to_display_states(ids, atoms_display_states_))
		{
			cargs.changed_atoms_display_states=true;
		}

		{
			cargs.output_for_log << "Summary of atoms: ";
			SummaryOfAtoms::collect_summary(atoms_, ids).print(cargs.output_for_log);
			cargs.output_for_log << "\n";
		}
	}

	void command_color_atoms(CommandArguments& cargs)
	{
		assert_atoms_availability();
		assert_atoms_representations_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);
		CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(atoms_representation_names_);
		parameters_for_representation_selecting.read(cargs.input);
		CommandParametersForGenericColoring parameters_for_coloring;
		parameters_for_coloring.read(cargs.input);

		cargs.input.assert_nothing_unusable();

		if(!auxiliaries::ColorUtilities::color_valid(parameters_for_coloring.color))
		{
			throw std::runtime_error(std::string("Atoms color not specified."));
		}

		const std::set<std::size_t> ids=filter_drawable_implemented_ids(
				atoms_display_states_,
				parameters_for_representation_selecting.visual_ids_,
				selection_manager_.select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues));
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable atoms selected."));
		}

		CommandParametersForGenericViewing parameters_for_viewing;
		parameters_for_viewing.visual_ids_=parameters_for_representation_selecting.visual_ids_;
		parameters_for_viewing.color=parameters_for_coloring.color;

		parameters_for_viewing.assert_state();

		if(parameters_for_viewing.apply_to_display_states(ids, atoms_display_states_))
		{
			cargs.changed_atoms_display_states=true;
		}

		{
			cargs.output_for_log << "Summary of atoms: ";
			SummaryOfAtoms::collect_summary(atoms_, ids).print(cargs.output_for_log);
			cargs.output_for_log << "\n";
		}
	}

	void command_spectrum_atoms(CommandArguments& cargs)
	{
		assert_atoms_availability();
		assert_atoms_representations_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);
		CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(atoms_representation_names_);
		parameters_for_representation_selecting.read(cargs.input);
		const std::string adjunct=cargs.input.get_value_or_default<std::string>("adjunct", "");
		const std::string by=adjunct.empty() ? cargs.input.get_value_or_default<std::string>("by", "residue-number") : std::string("adjunct");
		const std::string scheme=cargs.input.get_value_or_default<std::string>("scheme", "reverse-rainbow");
		const bool min_val_present=cargs.input.is_option("min-val");
		const double min_val=cargs.input.get_value_or_default<double>("min-val", 0.0);
		const bool max_val_present=cargs.input.is_option("max-val");
		const double max_val=cargs.input.get_value_or_default<double>("max-val", 1.0);
		const bool only_summarize=cargs.input.get_flag("only-summarize");

		cargs.input.assert_nothing_unusable();

		if(by!="residue-number" && by!="adjunct" && by!="chain" && by!="residue-id")
		{
			throw std::runtime_error(std::string("Invalid 'by' value '")+by+"'.");
		}

		if(by=="adjunct" && adjunct.empty())
		{
			throw std::runtime_error(std::string("No adjunct name provided."));
		}

		if(by!="adjunct" && !adjunct.empty())
		{
			throw std::runtime_error(std::string("Adjunct name provided when coloring not by adjunct."));
		}

		if(!auxiliaries::ColorUtilities::color_valid(auxiliaries::ColorUtilities::color_from_gradient(scheme, 0.5)))
		{
			throw std::runtime_error(std::string("Invalid 'scheme' value '")+scheme+"'.");
		}

		if(min_val_present && max_val_present && max_val<=min_val)
		{
			throw std::runtime_error(std::string("Minimum and maximum values do not define range."));
		}

		const std::set<std::size_t> ids=filter_drawable_implemented_ids(
				atoms_display_states_,
				parameters_for_representation_selecting.visual_ids_,
				selection_manager_.select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues));
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable atoms selected."));
		}

		std::map<std::size_t, double> map_of_ids_values;

		if(by=="adjunct")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::map<std::string, double>& adjuncts=atoms_[*it].value.props.adjuncts;
				std::map<std::string, double>::const_iterator jt=adjuncts.find(adjunct);
				if(jt!=adjuncts.end())
				{
					map_of_ids_values[*it]=jt->second;
				}
			}
		}
		else if(by=="residue-number")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				map_of_ids_values[*it]=atoms_[*it].crad.resSeq;
			}
		}
		else if(by=="chain")
		{
			std::map<std::string, double> chains_to_values;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				chains_to_values[atoms_[*it].crad.chainID]=0.5;
			}
			if(chains_to_values.size()>1)
			{
				int i=0;
				for(std::map<std::string, double>::iterator it=chains_to_values.begin();it!=chains_to_values.end();++it)
				{
					it->second=static_cast<double>(i)/static_cast<double>(chains_to_values.size()-1);
					i++;
				}
			}
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				map_of_ids_values[*it]=chains_to_values[atoms_[*it].crad.chainID];
			}
		}
		else if(by=="residue-id")
		{
			std::map<common::ChainResidueAtomDescriptor, double> residue_ids_to_values;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				residue_ids_to_values[atoms_[*it].crad.without_atom()]=0.5;
			}
			if(residue_ids_to_values.size()>1)
			{
				int i=0;
				for(std::map<common::ChainResidueAtomDescriptor, double>::iterator it=residue_ids_to_values.begin();it!=residue_ids_to_values.end();++it)
				{
					it->second=static_cast<double>(i)/static_cast<double>(residue_ids_to_values.size()-1);
					i++;
				}
			}
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				map_of_ids_values[*it]=residue_ids_to_values[atoms_[*it].crad.without_atom()];
			}
		}

		if(map_of_ids_values.empty())
		{
			throw std::runtime_error(std::string("Nothing colorable."));
		}

		double min_val_actual=0.0;
		double max_val_actual=0.0;

		{
			for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
			{
				const double val=it->second;
				if(it==map_of_ids_values.begin() || min_val_actual>val)
				{
					min_val_actual=val;
				}
				if(it==map_of_ids_values.begin() || max_val_actual<val)
				{
					max_val_actual=val;
				}
			}

			const double min_val_to_use=(min_val_present ? min_val : min_val_actual);
			const double max_val_to_use=(max_val_present ? max_val : max_val_actual);

			if(max_val_to_use<=min_val_to_use)
			{
				throw std::runtime_error(std::string("Minimum and maximum values do not define range."));
			}

			for(std::map<std::size_t, double>::iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
			{
				double& val=it->second;
				if(val<=min_val_to_use)
				{
					val=0.0;
				}
				else if(val>=max_val_to_use)
				{
					val=1.0;
				}
				else
				{
					val=(val-min_val_to_use)/(max_val_to_use-min_val_to_use);
				}
			}
		}

		if(!only_summarize)
		{
			CommandParametersForGenericViewing parameters_for_viewing;
			parameters_for_viewing.visual_ids_=parameters_for_representation_selecting.visual_ids_;
			parameters_for_viewing.assert_state();

			for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
			{
				parameters_for_viewing.color=auxiliaries::ColorUtilities::color_from_gradient(scheme, it->second);
				if(parameters_for_viewing.apply_to_display_state(it->first, atoms_display_states_))
				{
					cargs.changed_atoms_display_states=true;
				}
			}
		}

		{
			cargs.output_for_log << "Summary: ";
			cargs.output_for_log << "count=" << ids.size() << " ";
			cargs.output_for_log << "min=" << min_val_actual << " ";
			cargs.output_for_log << "max=" << max_val_actual;
			cargs.output_for_log << "\n";
		}
	}

	void command_set_secondary_structure_tags(CommandArguments& cargs)
	{
		assert_atoms_availability();

		const std::string ss_tag_alpha=cargs.input.get_value_or_default<std::string>("name-alpha", "ss_a");
		const std::string ss_tag_beta=cargs.input.get_value_or_default<std::string>("name-beta", "ss_b");

		cargs.input.assert_nothing_unusable();

		if(!primary_structure_info_.valid(atoms_))
		{
			throw std::runtime_error(std::string("Failed to assign primary structure."));
		}

		if(!secondary_structure_info_.valid(atoms_, primary_structure_info_))
		{
			throw std::runtime_error(std::string("Failed to assign secondary structure."));
		}

		int number_of_ss_alpha=0;
		int number_of_ss_beta=0;
		int number_of_ss_other=0;

		for(std::size_t i=0;i<primary_structure_info_.residues.size();i++)
		{
			const ConstructionOfSecondaryStructure::SecondaryStructureType sstype=secondary_structure_info_.residue_descriptors[i].secondary_structure_type;
			std::string ss_tag;
			if(sstype==ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_ALPHA_HELIX)
			{
				ss_tag=ss_tag_alpha;
				number_of_ss_alpha++;
			}
			else if(sstype==ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_BETA_STRAND)
			{
				ss_tag=ss_tag_beta;
				number_of_ss_beta++;
			}
			else
			{
				number_of_ss_other++;
			}
			for(std::size_t j=0;j<primary_structure_info_.residues[i].atom_ids.size();j++)
			{
				const std::size_t atom_id=primary_structure_info_.residues[i].atom_ids[j];
				if(atoms_[atom_id].value.props.tags.erase(ss_tag_alpha)>0)
				{
					cargs.changed_atoms_tags=true;
				}
				if(atoms_[atom_id].value.props.tags.erase(ss_tag_beta)>0)
				{
					cargs.changed_atoms_tags=true;
				}
				if(!ss_tag.empty())
				{
					atoms_[atom_id].value.props.tags.insert(ss_tag);
					cargs.changed_atoms_tags=true;
				}
			}
		}

		{
			cargs.output_for_log << "Summary: ";
			cargs.output_for_log << "alpha=" << number_of_ss_alpha << " ";
			cargs.output_for_log << "beta=" << number_of_ss_beta << " ";
			cargs.output_for_log << "other=" << number_of_ss_other;
			cargs.output_for_log << "\n";
		}
	}

	void command_print_atoms(CommandArguments& cargs)
	{
		assert_atoms_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);
		CommandParametersForGenericTablePrinting parameters_for_printing;
		parameters_for_printing.read(cargs.input);
		CommandParametersForGenericOutputDestinations parameters_for_output_destinations(true);
		parameters_for_output_destinations.read(false, cargs.input);

		cargs.input.assert_nothing_unusable();

		const std::set<std::size_t> ids=selection_manager_.select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(&cargs.output_for_data);

		for(std::size_t i=0;i<outputs.size();i++)
		{
			std::ostream& output=(*(outputs[i]));
			TablePrinting::print_atoms(atoms_, ids, parameters_for_printing, output);
		}

		{
			cargs.output_for_log << "Summary of atoms: ";
			SummaryOfAtoms::collect_summary(atoms_, ids).print(cargs.output_for_log);
			cargs.output_for_log << "\n";
		}
	}

	void command_list_selections_of_atoms(CommandArguments& cargs)
	{
		cargs.input.assert_nothing_unusable();
		assert_atoms_selections_availability();
		const std::map< std::string, std::set<std::size_t> >& map_of_selections=selection_manager_.map_of_atoms_selections();
		cargs.output_for_log << "Selections of atoms:\n";
		for(std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_selections.begin();it!=map_of_selections.end();++it)
		{
			cargs.output_for_log << "  name='" << (it->first) << "' ";
			SummaryOfAtoms::collect_summary(atoms_, it->second).print(cargs.output_for_log);
			cargs.output_for_log << "\n";
		}
	}

	void command_delete_all_selections_of_atoms(CommandArguments& cargs)
	{
		cargs.input.assert_nothing_unusable();
		assert_atoms_selections_availability();
		selection_manager_.delete_atoms_selections();
		cargs.output_for_log << "Removed all selections of atoms\n";
	}

	void command_delete_selections_of_atoms(CommandArguments& cargs)
	{
		assert_atoms_selections_availability();

		const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();
		cargs.input.mark_all_unnamed_values_as_used();

		cargs.input.assert_nothing_unusable();

		if(names.empty())
		{
			throw std::runtime_error(std::string("No atoms selections names provided."));
		}

		assert_atoms_selections_availability(names);

		for(std::size_t i=0;i<names.size();i++)
		{
			selection_manager_.delete_atoms_selection(names[i]);
		}

		cargs.output_for_log << "Removed selections of atoms:";
		for(std::size_t i=0;i<names.size();i++)
		{
			cargs.output_for_log << " " << names[i];
		}
		cargs.output_for_log << "\n";
	}

	void command_rename_selection_of_atoms(CommandArguments& cargs)
	{
		assert_atoms_selections_availability();

		const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();

		if(names.size()!=2)
		{
			throw std::runtime_error(std::string("Not exactly two names provided for renaming."));
		}

		cargs.input.mark_all_unnamed_values_as_used();

		cargs.input.assert_nothing_unusable();

		const std::set<std::size_t> ids=selection_manager_.get_atoms_selection(names[0]);
		selection_manager_.set_atoms_selection(names[1], ids);
		selection_manager_.delete_atoms_selection(names[0]);
		cargs.output_for_log << "Renamed selection of atoms from '" << names[0] << "' to '" << names[1] << "'\n";
	}

	void command_construct_contacts(CommandArguments& cargs)
	{
		assert_atoms_availability();

		ConstructionOfContacts::ParametersToConstructBundleOfContactInformation parameters_to_construct_contacts;
		parameters_to_construct_contacts.probe=cargs.input.get_value_or_default<double>("probe", parameters_to_construct_contacts.probe);
		parameters_to_construct_contacts.calculate_volumes=cargs.input.get_flag("calculate-volumes");
		ConstructionOfContacts::ParametersToEnhanceContacts parameters_to_enhance_contacts;
		parameters_to_enhance_contacts.probe=parameters_to_construct_contacts.probe;
		parameters_to_enhance_contacts.tag_centrality=cargs.input.get_flag("tag-centrality");
		parameters_to_enhance_contacts.tag_peripherial=cargs.input.get_flag("tag-peripherial");
		CommandParametersForGenericSelecting render_parameters_for_selecting("render-", "{--min-seq-sep 1}");
		render_parameters_for_selecting.read(cargs.input);
		const bool render=(cargs.input.get_flag("render-default") ||
				cargs.input.is_option(render_parameters_for_selecting.type_for_expression) ||
				cargs.input.is_option(render_parameters_for_selecting.type_for_full_residues) ||
				cargs.input.is_option(render_parameters_for_selecting.type_for_forced_id));

		cargs.input.assert_nothing_unusable();

		ConstructionOfTriangulation::BundleOfTriangulationInformation bundle_of_triangulation_information;
		ConstructionOfContacts::BundleOfContactInformation bundle_of_contact_information;

		if(ConstructionOfContacts::construct_bundle_of_contact_information(parameters_to_construct_contacts, common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(atoms_), bundle_of_triangulation_information, bundle_of_contact_information))
		{
			reset_contacts(bundle_of_contact_information.contacts);
			cargs.changed_contacts=true;

			if(parameters_to_construct_contacts.calculate_volumes)
			{
				for(std::size_t i=0;i<bundle_of_contact_information.volumes.size() && i<atoms_.size();i++)
				{
					atoms_[i].value.props.adjuncts["volume"]=bundle_of_contact_information.volumes[i];
				}
			}

			std::set<std::size_t> draw_ids;
			if(render)
			{
				draw_ids=selection_manager_.select_contacts(render_parameters_for_selecting.forced_ids, render_parameters_for_selecting.expression, render_parameters_for_selecting.full_residues);
			}

			ConstructionOfContacts::enhance_contacts(parameters_to_enhance_contacts, bundle_of_triangulation_information, draw_ids, contacts_);

			reset_contacts_display_states();

			cargs.output_for_log << "Constructed contacts (";
			SummaryOfContacts::collect_summary(contacts_).print(cargs.output_for_log);
			cargs.output_for_log << ")\n";
		}
		else
		{
			throw std::runtime_error(std::string("Failed to construct contacts."));
		}
	}

	void command_save_contacts(CommandArguments& cargs)
	{
		assert_contacts_availability();

		CommandParametersForGenericOutputDestinations parameters_for_output_destinations(false);
		parameters_for_output_destinations.read(true, cargs.input);
		const bool no_graphics=cargs.input.get_flag("no-graphics");

		cargs.input.assert_nothing_unusable();

		std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(0);

		for(std::size_t i=0;i<outputs.size();i++)
		{
			std::ostream& output=(*(outputs[i]));
			enabled_output_of_ContactValue_graphics()=!no_graphics;
			auxiliaries::IOUtilities().write_set(contacts_, output);
		}

		if(!parameters_for_output_destinations.file.empty())
		{
			cargs.output_for_log << "Wrote contacts to file '" << parameters_for_output_destinations.file << "' (";
			SummaryOfContacts::collect_summary(contacts_).print(cargs.output_for_log);
			cargs.output_for_log << ")\n";
		}
	}

	void command_load_contacts(CommandArguments& cargs)
	{
		assert_atoms_availability();

		const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");

		cargs.input.assert_nothing_unusable();

		if(file.empty())
		{
			throw std::runtime_error(std::string("Empty input contacts file name."));
		}

		std::vector<Contact> contacts;

		auxiliaries::IOUtilities().read_file_lines_to_set(file, contacts);

		if(!contacts.empty())
		{
			reset_contacts(contacts);
			cargs.changed_contacts=true;

			cargs.output_for_log << "Read contacts from file '" << file << "' (";
			SummaryOfContacts::collect_summary(contacts_).print(cargs.output_for_log);
			cargs.output_for_log << ")\n";
		}
		else
		{
			throw std::runtime_error(std::string("Failed to read contacts from file '")+file+"'.");
		}
	}

	void command_select_contacts(CommandArguments& cargs)
	{
		assert_contacts_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);
		const std::string name=(cargs.input.is_any_unnamed_value_unused() ?
				cargs.input.get_value_or_first_unused_unnamed_value("name") :
				cargs.input.get_value_or_default<std::string>("name", ""));
		const bool no_marking=cargs.input.get_flag("no-marking");

		cargs.input.assert_nothing_unusable();

		assert_selection_name_input(name, true);

		std::set<std::size_t> ids=selection_manager_.select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		{
			cargs.output_for_log << "Summary of contacts: ";
			SummaryOfContacts::collect_summary(contacts_, ids).print(cargs.output_for_log);
			cargs.output_for_log << "\n";
		}

		if(!name.empty())
		{
			selection_manager_.set_contacts_selection(name, ids);
			cargs.output_for_log << "Set selection of contacts named '" << name << "'\n";
		}

		if(!no_marking)
		{
			{
				CommandParametersForGenericViewing params;
				params.unmark=true;
				if(params.apply_to_display_states(contacts_display_states_))
				{
					cargs.changed_contacts_display_states=true;
				}
			}
			{
				CommandParametersForGenericViewing params;
				params.mark=true;
				if(params.apply_to_display_states(ids, contacts_display_states_))
				{
					cargs.changed_contacts_display_states=true;
				}
			}
		}

		cargs.output_set_of_ids.swap(ids);
	}

	void command_tag_contacts(CommandArguments& cargs)
	{
		configurable_command_tag_contacts(true, cargs);
	}

	void command_untag_contacts(CommandArguments& cargs)
	{
		configurable_command_tag_contacts(false, cargs);
	}

	void configurable_command_tag_contacts(const bool positive, CommandArguments& cargs)
	{
		assert_contacts_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);
		const std::string tag=cargs.input.get_value_or_first_unused_unnamed_value("tag");

		cargs.input.assert_nothing_unusable();

		assert_tag_input(tag);

		std::set<std::size_t> ids=selection_manager_.select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			Contact& contact=contacts_[*it];
			if(positive)
			{
				contact.value.props.tags.insert(tag);
			}
			else
			{
				contact.value.props.tags.erase(tag);
			}
		}

		{
			cargs.output_for_log << "Summary of contacts: ";
			SummaryOfContacts::collect_summary(contacts_, ids).print(cargs.output_for_log);
			cargs.output_for_log << "\n";
		}
	}

	void command_mark_contacts(CommandArguments& cargs)
	{
		configurable_command_mark_contacts(true, cargs);
	}

	void command_unmark_contacts(CommandArguments& cargs)
	{
		configurable_command_mark_contacts(false, cargs);
	}

	void configurable_command_mark_contacts(const bool positive, CommandArguments& cargs)
	{
		assert_contacts_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);

		cargs.input.assert_nothing_unusable();

		const std::set<std::size_t> ids=filter_drawable_implemented_ids(
				contacts_display_states_,
				std::set<std::size_t>(),
				selection_manager_.select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues));
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable contacts selected."));
		}

		{
			CommandParametersForGenericViewing parameters_for_viewing;
			parameters_for_viewing.mark=positive;
			parameters_for_viewing.unmark=!positive;
			if(parameters_for_viewing.apply_to_display_states(ids, contacts_display_states_))
			{
				cargs.changed_contacts_display_states=true;
			}
		}

		{
			cargs.output_for_log << "Summary of contacts: ";
			SummaryOfContacts::collect_summary(contacts_, ids).print(cargs.output_for_log);
			cargs.output_for_log << "\n";
		}
	}

	void command_show_contacts(CommandArguments& cargs)
	{
		configurable_command_show_contacts(true, cargs);
	}

	void command_hide_contacts(CommandArguments& cargs)
	{
		configurable_command_show_contacts(false, cargs);
	}

	void configurable_command_show_contacts(const bool positive, CommandArguments& cargs)
	{
		assert_contacts_availability();
		assert_contacts_representations_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);
		CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(contacts_representation_names_);
		parameters_for_representation_selecting.read(cargs.input);

		cargs.input.assert_nothing_unusable();

		if(positive && parameters_for_representation_selecting.visual_ids_.empty() && contacts_representation_names_.size()>1)
		{
			parameters_for_representation_selecting.visual_ids_.insert(0);
		}

		const std::set<std::size_t> ids=filter_drawable_implemented_ids(
				contacts_display_states_,
				parameters_for_representation_selecting.visual_ids_,
				selection_manager_.select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues));
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable contacts selected."));
		}

		CommandParametersForGenericViewing parameters_for_viewing;
		parameters_for_viewing.visual_ids_=parameters_for_representation_selecting.visual_ids_;
		parameters_for_viewing.show=positive;
		parameters_for_viewing.hide=!positive;

		parameters_for_viewing.assert_state();

		if(parameters_for_viewing.apply_to_display_states(ids, contacts_display_states_))
		{
			cargs.changed_contacts_display_states=true;
		}

		{
			cargs.output_for_log << "Summary of contacts: ";
			SummaryOfContacts::collect_summary(contacts_, ids).print(cargs.output_for_log);
			cargs.output_for_log << "\n";
		}
	}

	void command_color_contacts(CommandArguments& cargs)
	{
		assert_contacts_availability();
		assert_contacts_representations_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);
		CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(contacts_representation_names_);
		parameters_for_representation_selecting.read(cargs.input);
		CommandParametersForGenericColoring parameters_for_coloring;
		parameters_for_coloring.read(cargs.input);

		cargs.input.assert_nothing_unusable();

		if(!auxiliaries::ColorUtilities::color_valid(parameters_for_coloring.color))
		{
			throw std::runtime_error(std::string("Contacts color not specified."));
		}

		const std::set<std::size_t> ids=filter_drawable_implemented_ids(
				contacts_display_states_,
				parameters_for_representation_selecting.visual_ids_,
				selection_manager_.select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues));
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable contacts selected."));
		}

		CommandParametersForGenericViewing parameters_for_viewing;
		parameters_for_viewing.visual_ids_=parameters_for_representation_selecting.visual_ids_;
		parameters_for_viewing.color=parameters_for_coloring.color;

		parameters_for_viewing.assert_state();

		if(parameters_for_viewing.apply_to_display_states(ids, contacts_display_states_))
		{
			cargs.changed_contacts_display_states=true;
		}

		{
			cargs.output_for_log << "Summary of contacts: ";
			SummaryOfContacts::collect_summary(contacts_, ids).print(cargs.output_for_log);
			cargs.output_for_log << "\n";
		}
	}

	void command_spectrum_contacts(CommandArguments& cargs)
	{
		assert_contacts_availability();
		assert_contacts_representations_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);
		CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(contacts_representation_names_);
		parameters_for_representation_selecting.read(cargs.input);
		const std::string adjunct=cargs.input.get_value_or_default<std::string>("adjunct", "");
		const std::string by=adjunct.empty() ? cargs.input.get_value<std::string>("by") : std::string("adjunct");
		const std::string scheme=cargs.input.get_value_or_default<std::string>("scheme", "reverse-rainbow");
		const bool min_val_present=cargs.input.is_option("min-val");
		const double min_val=cargs.input.get_value_or_default<double>("min-val", 0.0);
		const bool max_val_present=cargs.input.is_option("max-val");
		const double max_val=cargs.input.get_value_or_default<double>("max-val", 1.0);
		const bool only_summarize=cargs.input.get_flag("only-summarize");

		cargs.input.assert_nothing_unusable();

		if(by!="area" && by!="adjunct" && by!="dist-centers" && by!="dist-balls" && by!="seq-sep")
		{
			throw std::runtime_error(std::string("Invalid 'by' value '")+by+"'.");
		}

		if(by=="adjunct" && adjunct.empty())
		{
			throw std::runtime_error(std::string("No adjunct name provided."));
		}

		if(by!="adjunct" && !adjunct.empty())
		{
			throw std::runtime_error(std::string("Adjunct name provided when coloring not by adjunct."));
		}

		if(!auxiliaries::ColorUtilities::color_valid(auxiliaries::ColorUtilities::color_from_gradient(scheme, 0.5)))
		{
			throw std::runtime_error(std::string("Invalid 'scheme' value '")+scheme+"'.");
		}

		if(min_val_present && max_val_present && max_val<=min_val)
		{
			throw std::runtime_error(std::string("Minimum and maximum values do not define range."));
		}

		const std::set<std::size_t> ids=filter_drawable_implemented_ids(
				contacts_display_states_,
				parameters_for_representation_selecting.visual_ids_,
				selection_manager_.select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues));
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable contacts selected."));
		}

		std::map<std::size_t, double> map_of_ids_values;

		if(by=="adjunct")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::map<std::string, double>& adjuncts=contacts_[*it].value.props.adjuncts;
				std::map<std::string, double>::const_iterator jt=adjuncts.find(adjunct);
				if(jt!=adjuncts.end())
				{
					map_of_ids_values[*it]=jt->second;
				}
			}
		}
		else if(by=="area")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				map_of_ids_values[*it]=contacts_[*it].value.area;
			}
		}
		else if(by=="dist-centers")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				map_of_ids_values[*it]=contacts_[*it].value.dist;
			}
		}
		else if(by=="dist-balls")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::size_t id0=contacts_[*it].ids[0];
				const std::size_t id1=contacts_[*it].ids[1];
				if(contacts_[*it].solvent())
				{
					map_of_ids_values[*it]=(contacts_[*it].value.dist-atoms_[id0].value.r)/3.0*2.0;
				}
				else
				{
					map_of_ids_values[*it]=apollota::minimal_distance_from_sphere_to_sphere(atoms_[id0].value, atoms_[id1].value);
				}
			}
		}
		else if(by=="seq-sep")
		{
			double max_seq_sep=0.0;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::size_t id0=contacts_[*it].ids[0];
				const std::size_t id1=contacts_[*it].ids[1];
				if(atoms_[id0].crad.chainID==atoms_[id1].crad.chainID)
				{
					const double seq_sep=fabs(static_cast<double>(atoms_[id0].crad.resSeq-atoms_[id1].crad.resSeq));
					map_of_ids_values[*it]=seq_sep;
					max_seq_sep=((max_seq_sep<seq_sep) ? seq_sep : max_seq_sep);
				}
			}
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::size_t id0=contacts_[*it].ids[0];
				const std::size_t id1=contacts_[*it].ids[1];
				if(atoms_[id0].crad.chainID!=atoms_[id1].crad.chainID)
				{
					map_of_ids_values[*it]=max_seq_sep+1.0;
				}
			}
		}

		if(map_of_ids_values.empty())
		{
			throw std::runtime_error(std::string("Nothing colorable."));
		}

		double min_val_actual=0.0;
		double max_val_actual=0.0;

		{
			for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
			{
				const double val=it->second;
				if(it==map_of_ids_values.begin() || min_val_actual>val)
				{
					min_val_actual=val;
				}
				if(it==map_of_ids_values.begin() || max_val_actual<val)
				{
					max_val_actual=val;
				}
			}

			const double min_val_to_use=(min_val_present ? min_val : min_val_actual);
			const double max_val_to_use=(max_val_present ? max_val : max_val_actual);

			if(max_val_to_use<=min_val_to_use)
			{
				throw std::runtime_error(std::string("Minimum and maximum values do not define range."));
			}

			for(std::map<std::size_t, double>::iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
			{
				double& val=it->second;
				if(val<=min_val_to_use)
				{
					val=0.0;
				}
				else if(val>=max_val_to_use)
				{
					val=1.0;
				}
				else
				{
					val=(val-min_val_to_use)/(max_val_to_use-min_val_to_use);
				}
			}
		}

		if(!only_summarize)
		{
			CommandParametersForGenericViewing parameters_for_viewing;
			parameters_for_viewing.visual_ids_=parameters_for_representation_selecting.visual_ids_;
			parameters_for_viewing.assert_state();

			for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
			{
				parameters_for_viewing.color=auxiliaries::ColorUtilities::color_from_gradient(scheme, it->second);
				if(parameters_for_viewing.apply_to_display_state(it->first, contacts_display_states_))
				{
					cargs.changed_contacts_display_states=true;
				}
			}
		}

		{
			cargs.output_for_log << "Summary: ";
			cargs.output_for_log << "count=" << ids.size() << " ";
			cargs.output_for_log << "min=" << min_val_actual << " ";
			cargs.output_for_log << "max=" << max_val_actual;
			cargs.output_for_log << "\n";
		}
	}

	void command_print_contacts(CommandArguments& cargs)
	{
		assert_contacts_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);
		CommandParametersForContactsTablePrinting parameters_for_printing;
		parameters_for_printing.read(cargs.input);
		CommandParametersForGenericOutputDestinations parameters_for_output_destinations(true);
		parameters_for_output_destinations.read(false, cargs.input);

		cargs.input.assert_nothing_unusable();

		const std::set<std::size_t> ids=selection_manager_.select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(&cargs.output_for_data);

		for(std::size_t i=0;i<outputs.size();i++)
		{
			std::ostream& output=(*(outputs[i]));
			TablePrinting::print_contacts(atoms_, contacts_, ids, parameters_for_printing, output);
		}

		{
			cargs.output_for_log << "Summary of contacts: ";
			SummaryOfContacts::collect_summary(contacts_, ids).print(cargs.output_for_log);
			cargs.output_for_log << "\n";
		}
	}

	void command_write_contacts_as_pymol_cgo(CommandArguments& cargs)
	{
		assert_contacts_availability();
		assert_contacts_representations_availability();

		std::string name=cargs.input.get_value_or_default<std::string>("name", "contacts");
		bool wireframe=cargs.input.get_flag("wireframe");
		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);
		CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(contacts_representation_names_);
		parameters_for_representation_selecting.read(cargs.input);
		CommandParametersForGenericOutputDestinations parameters_for_output_destinations(false);
		parameters_for_output_destinations.read(false, cargs.input);

		cargs.input.assert_nothing_unusable();

		if(name.empty())
		{
			throw std::runtime_error(std::string("Missing object name."));
		}

		if(parameters_for_representation_selecting.visual_ids_.empty())
		{
			parameters_for_representation_selecting.visual_ids_.insert(0);
		}

		if(parameters_for_representation_selecting.visual_ids_.size()>1)
		{
			throw std::runtime_error(std::string("More than one representation requested."));
		}

		const std::set<std::size_t> ids=filter_drawable_implemented_ids(
				contacts_display_states_,
				parameters_for_representation_selecting.visual_ids_,
				selection_manager_.select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues),
				true);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable visible contacts selected."));
		}

		std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(0);

		auxiliaries::OpenGLPrinter opengl_printer;
		{
			unsigned int prev_color=0;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::size_t id=(*it);
				for(std::set<std::size_t>::const_iterator jt=parameters_for_representation_selecting.visual_ids_.begin();jt!=parameters_for_representation_selecting.visual_ids_.end();++jt)
				{
					const std::size_t visual_id=(*jt);
					if(visual_id<contacts_display_states_[id].visuals.size())
					{
						const DisplayState::Visual& dsv=contacts_display_states_[id].visuals[visual_id];
						if(prev_color==0 || dsv.color!=prev_color)
						{
							opengl_printer.add_color(dsv.color);
						}
						prev_color=dsv.color;
						if(wireframe)
						{
							opengl_printer.add_as_wireframe(contacts_[id].value.graphics);
						}
						else
						{
							opengl_printer.add(contacts_[id].value.graphics);
						}
					}
				}
			}
		}

		for(std::size_t i=0;i<outputs.size();i++)
		{
			std::ostream& output=(*(outputs[i]));
			opengl_printer.print_pymol_script(name, true, output);
		}

		if(!parameters_for_output_destinations.file.empty())
		{
			cargs.output_for_log << "Wrote contacts as PyMol CGO to file '" << parameters_for_output_destinations.file << "' (";
			SummaryOfContacts::collect_summary(contacts_).print(cargs.output_for_log);
			cargs.output_for_log << ")\n";
		}
	}

	void command_list_selections_of_contacts(CommandArguments& cargs)
	{
		cargs.input.assert_nothing_unusable();
		assert_contacts_selections_availability();
		const std::map< std::string, std::set<std::size_t> >& map_of_selections=selection_manager_.map_of_contacts_selections();
		cargs.output_for_log << "Selections of contacts:\n";
		for(std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_selections.begin();it!=map_of_selections.end();++it)
		{
			cargs.output_for_log << "  name='" << (it->first) << "' ";
			SummaryOfContacts::collect_summary(contacts_, it->second).print(cargs.output_for_log);
			cargs.output_for_log << "\n";
		}
	}

	void command_delete_all_selections_of_contacts(CommandArguments& cargs)
	{
		cargs.input.assert_nothing_unusable();
		assert_contacts_selections_availability();
		selection_manager_.delete_contacts_selections();
		cargs.output_for_log << "Removed all selections of contacts\n";
	}

	void command_delete_selections_of_contacts(CommandArguments& cargs)
	{
		assert_contacts_selections_availability();

		const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();
		cargs.input.mark_all_unnamed_values_as_used();

		cargs.input.assert_nothing_unusable();

		if(names.empty())
		{
			throw std::runtime_error(std::string("No contacts selections names provided."));
		}

		assert_contacts_selections_availability(names);

		for(std::size_t i=0;i<names.size();i++)
		{
			selection_manager_.delete_contacts_selection(names[i]);
		}

		cargs.output_for_log << "Removed selections of contacts:";
		for(std::size_t i=0;i<names.size();i++)
		{
			cargs.output_for_log << " " << names[i];
		}
		cargs.output_for_log << "\n";
	}

	void command_rename_selection_of_contacts(CommandArguments& cargs)
	{
		assert_contacts_selections_availability();

		const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();

		if(names.size()!=2)
		{
			throw std::runtime_error(std::string("Not exactly two names provided for renaming."));
		}

		cargs.input.mark_all_unnamed_values_as_used();

		cargs.input.assert_nothing_unusable();

		const std::set<std::size_t> ids=selection_manager_.get_contacts_selection(names[0]);
		selection_manager_.set_contacts_selection(names[1], ids);
		selection_manager_.delete_contacts_selection(names[0]);
		cargs.output_for_log << "Renamed selection of contacts from '" << names[0] << "' to '" << names[1] << "'\n";
	}

	void command_save_atoms_and_contacts(CommandArguments& cargs)
	{
		assert_atoms_availability();
		assert_contacts_availability();

		CommandParametersForGenericOutputDestinations parameters_for_output_destinations(false);
		parameters_for_output_destinations.read(true, cargs.input);
		const bool no_graphics=cargs.input.get_flag("no-graphics");

		cargs.input.assert_nothing_unusable();

		std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(0);

		for(std::size_t i=0;i<outputs.size();i++)
		{
			std::ostream& output=(*(outputs[i]));
			auxiliaries::IOUtilities().write_set(atoms_, output);
			output << "_end_atoms\n";
			enabled_output_of_ContactValue_graphics()=!no_graphics;
			auxiliaries::IOUtilities().write_set(contacts_, output);
			output << "_end_contacts\n";
		}

		if(!parameters_for_output_destinations.file.empty())
		{
			cargs.output_for_log << "Wrote atoms and contacts to file '" << parameters_for_output_destinations.file << "'";
			cargs.output_for_log << " (";
			SummaryOfAtoms::collect_summary(atoms_).print(cargs.output_for_log);
			cargs.output_for_log << ")";
			cargs.output_for_log << " (";
			SummaryOfContacts::collect_summary(contacts_).print(cargs.output_for_log);
			cargs.output_for_log << ")\n";
		}
	}

	void command_load_atoms_and_contacts(CommandArguments& cargs)
	{
		const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");

		cargs.input.assert_nothing_unusable();

		if(file.empty())
		{
			throw std::runtime_error(std::string("Empty input file name."));
		}

		std::ifstream finput(file.c_str(), std::ios::in);
		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
		}

		std::vector<Atom> atoms;
		std::vector<Contact> contacts;

		auxiliaries::IOUtilities(true, '\n', ' ', "_end_atoms").read_lines_to_set(finput, atoms);

		if(atoms.empty())
		{
			throw std::runtime_error(std::string("Failed to read atoms from file '")+file+"'.");
		}
		else if(atoms.size()<4)
		{
			throw std::runtime_error(std::string("Less than 4 atoms read."));
		}
		else
		{
			reset_atoms(atoms);
			cargs.changed_atoms=true;

			cargs.output_for_log << "Read atoms from file '" << file << "' (";
			SummaryOfAtoms::collect_summary(atoms_).print(cargs.output_for_log);
			cargs.output_for_log << ")\n";
		}

		auxiliaries::IOUtilities(true, '\n', ' ', "_end_contacts").read_lines_to_set(finput, contacts);

		if(contacts.empty())
		{
			cargs.output_for_log << "No contacts read from file '" << file << "'.";
		}
		else
		{
			reset_contacts(contacts);
			cargs.changed_contacts=true;

			cargs.output_for_log << "Read contacts from file '" << file << "' (";
			SummaryOfContacts::collect_summary(contacts_).print(cargs.output_for_log);
			cargs.output_for_log << ")\n";
		}
	}

	void command_zoom_by_atoms(CommandArguments& cargs)
	{
		assert_atoms_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);

		cargs.input.assert_nothing_unusable();

		const std::set<std::size_t> ids=selection_manager_.select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		update_bounding_box_with_atoms(ids, cargs.bounding_box);

		cargs.output_for_log << "Bounding box: (" << cargs.bounding_box.p_min << ") (" << cargs.bounding_box.p_max << ")\n";
	}

	void command_zoom_by_contacts(CommandArguments& cargs)
	{
		assert_contacts_availability();

		CommandParametersForGenericSelecting parameters_for_selecting;
		parameters_for_selecting.read(cargs.input);

		cargs.input.assert_nothing_unusable();

		const std::set<std::size_t> contacts_ids=selection_manager_.select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
		if(contacts_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		std::set<std::size_t> atoms_ids;
		for(std::set<std::size_t>::const_iterator it=contacts_ids.begin();it!=contacts_ids.end();++it)
		{
			atoms_ids.insert(contacts_[*it].ids[0]);
			atoms_ids.insert(contacts_[*it].ids[1]);
		}
		if(atoms_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		update_bounding_box_with_atoms(atoms_ids, cargs.bounding_box);

		cargs.output_for_log << "Bounding box: (" << cargs.bounding_box.p_min << ") (" << cargs.bounding_box.p_max << ")\n";
	}

	std::map<std::string, CommandFunctionPointer> map_of_command_function_pointers_;
	std::vector<std::string> atoms_representation_names_;
	std::vector<std::string> contacts_representation_names_;
	std::set<std::size_t> atoms_representations_implemented_always_;
	std::set<std::size_t> contacts_representations_implemented_always_;
	std::vector<Atom> atoms_;
	std::vector<Contact> contacts_;
	std::vector<DisplayState> atoms_display_states_;
	std::vector<DisplayState> contacts_display_states_;
	ConstructionOfPrimaryStructure::BundleOfPrimaryStructure primary_structure_info_;
	ConstructionOfSecondaryStructure::BundleOfSecondaryStructure secondary_structure_info_;
	SelectionManagerForAtomsAndContacts selection_manager_;
};

}

#endif /* COMMON_COMMANDING_MANAGER_FOR_ATOMS_AND_CONTACTS_H_ */
