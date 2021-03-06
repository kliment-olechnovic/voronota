#ifndef SCRIPTING_DATA_MANAGER_H_
#define SCRIPTING_DATA_MANAGER_H_

#include "../auxiliaries/color_utilities.h"

#include "../common/construction_of_secondary_structure.h"
#include "../common/construction_of_bonding_links.h"

#include "selection_manager.h"

namespace voronota
{

namespace scripting
{

class DataManager
{
public:
	class ChangeIndicator
	{
	public:
		ChangeIndicator() :
			changed_atoms_(false),
			changed_contacts_(false),
			changed_figures_(false),
			changed_atoms_tags_(false),
			changed_contacts_tags_(false),
			changed_atoms_adjuncts_(false),
			changed_contacts_adjuncts_(false),
			changed_atoms_display_states_(false),
			changed_contacts_display_states_(false),
			changed_figures_display_states_(false)
		{
		}

		bool changed_atoms() const { return changed_atoms_; }
		bool changed_contacts() const { return changed_contacts_; }
		bool changed_figures() const { return changed_figures_; }
		bool changed_atoms_tags() const { return changed_atoms_tags_; }
		bool changed_contacts_tags() const { return changed_contacts_tags_; }
		bool changed_atoms_adjuncts() const { return changed_atoms_adjuncts_; }
		bool changed_contacts_adjuncts() const { return changed_contacts_adjuncts_; }
		bool changed_atoms_display_states() const { return changed_atoms_display_states_; }
		bool changed_contacts_display_states() const { return changed_contacts_display_states_; }
		bool changed_figures_display_states() const { return changed_figures_display_states_; }

		void set_changed_atoms(const bool value) { changed_atoms_=value; ensure_correctness(); }
		void set_changed_contacts(const bool value) { changed_contacts_=value; ensure_correctness(); }
		void set_changed_figures(const bool value) { changed_figures_=value; ensure_correctness(); }
		void set_changed_atoms_tags(const bool value) { changed_atoms_tags_=value; ensure_correctness(); }
		void set_changed_contacts_tags(const bool value) { changed_contacts_tags_=value; ensure_correctness(); }
		void set_changed_atoms_adjuncts(const bool value) { changed_atoms_adjuncts_=value; ensure_correctness(); }
		void set_changed_contacts_adjuncts(const bool value) { changed_contacts_adjuncts_=value; ensure_correctness(); }
		void set_changed_atoms_display_states(const bool value) { changed_atoms_display_states_=value; ensure_correctness(); }
		void set_changed_contacts_display_states(const bool value) { changed_contacts_display_states_=value; ensure_correctness(); }
		void set_changed_figures_display_states(const bool value) { changed_figures_display_states_=value; ensure_correctness(); }

		bool changed() const
		{
			return (changed_atoms_
					|| changed_contacts_
					|| changed_figures_
					|| changed_atoms_tags_
					|| changed_contacts_tags_
					|| changed_atoms_adjuncts_
					|| changed_contacts_adjuncts_
					|| changed_atoms_display_states_
					|| changed_contacts_display_states_
					|| changed_figures_display_states_);
		}

	private:
		void ensure_correctness()
		{
			changed_contacts_=(changed_contacts_ || changed_atoms_);
			changed_atoms_tags_=(changed_atoms_tags_ || changed_atoms_);
			changed_contacts_tags_=(changed_contacts_tags_ || changed_contacts_);
			changed_atoms_adjuncts_=(changed_atoms_adjuncts_ || changed_atoms_);
			changed_contacts_adjuncts_=(changed_contacts_adjuncts_ || changed_contacts_);
			changed_atoms_display_states_=(changed_atoms_display_states_ || changed_atoms_);
			changed_contacts_display_states_=(changed_contacts_display_states_ || changed_contacts_);
			changed_figures_display_states_=(changed_figures_display_states_ || changed_figures_);
		}

		bool changed_atoms_;
		bool changed_contacts_;
		bool changed_figures_;
		bool changed_atoms_tags_;
		bool changed_contacts_tags_;
		bool changed_atoms_adjuncts_;
		bool changed_contacts_adjuncts_;
		bool changed_atoms_display_states_;
		bool changed_contacts_display_states_;
		bool changed_figures_display_states_;
	};

	struct RepresentationsDescriptor
	{
		std::vector<std::string> names;

		std::size_t id_by_name(const std::string& name) const
		{
			for(std::size_t i=0;i<names.size();i++)
			{
				if(names[i]==name)
				{
					return i;
				}
			}
			return names.size();
		}

		std::set<std::size_t> ids_by_names(const std::vector<std::string>& input_names) const
		{
			std::set<std::size_t> ids;
			for(std::size_t i=0;i<input_names.size();i++)
			{
				const std::size_t id=id_by_name(input_names[i]);
				if(id<names.size())
				{
					ids.insert(id);
				}
				else
				{
					throw std::runtime_error(std::string("Representation '")+input_names[i]+"' does not exist.");
				}
			}
			return ids;
		}
	};

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

		bool visible(const std::size_t visual_id) const
		{
			return (drawable && visual_id<visuals.size() && visuals[visual_id].visible);
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

		bool implemented(const std::size_t visual_id) const
		{
			return (drawable && visual_id<visuals.size() && visuals[visual_id].implemented);
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

	class DisplayStateUpdater
	{
	public:
		bool mark;
		bool unmark;
		bool show;
		bool hide;
		auxiliaries::ColorUtilities::ColorInteger color;
		std::set<std::size_t> visual_ids;

		DisplayStateUpdater() :
			mark(false),
			unmark(false),
			show(false),
			hide(false),
			color(auxiliaries::ColorUtilities::null_color())
		{
		}

		DisplayStateUpdater& set_mark(const bool value)
		{
			mark=value;
			return (*this);
		}

		DisplayStateUpdater& set_unmark(const bool value)
		{
			unmark=value;
			return (*this);
		}

		DisplayStateUpdater& set_show(const bool value)
		{
			show=value;
			return (*this);
		}

		DisplayStateUpdater& set_hide(const bool value)
		{
			hide=value;
			return (*this);
		}

		DisplayStateUpdater& set_color(const auxiliaries::ColorUtilities::ColorInteger value)
		{
			color=value;
			return (*this);
		}

		DisplayStateUpdater& set_visual_ids(const std::set<std::size_t>& value)
		{
			visual_ids=value;
			return (*this);
		}

		bool color_valid() const
		{
			return auxiliaries::ColorUtilities::color_valid(color);
		}

		void assert_correctness() const
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

		bool update_display_state(DisplayState& ds) const
		{
			bool updated=false;
			if(show || hide || mark || unmark || color_valid())
			{
				if(mark || unmark)
				{
					updated=(updated || (ds.marked!=mark));
					ds.marked=mark;
				}

				{
					if(show || hide || color_valid())
					{
						if(visual_ids.empty())
						{
							for(std::size_t i=0;i<ds.visuals.size();i++)
							{
								if(update_display_state_visual(ds.visuals[i]))
								{
									updated=true;
								}
							}
						}
						else
						{
							for(std::set<std::size_t>::const_iterator jt=visual_ids.begin();jt!=visual_ids.end();++jt)
							{
								const std::size_t visual_id=(*jt);
								if(visual_id<ds.visuals.size())
								{
									if(update_display_state_visual(ds.visuals[visual_id]))
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

		bool update_display_state(const std::size_t id, std::vector<DisplayState>& display_states) const
		{
			assert_correctness();
			bool updated=false;
			if(id<display_states.size() && update_display_state(display_states[id]))
			{
				updated=true;
			}
			return updated;
		}

		bool update_display_states(const std::set<std::size_t>& ids, std::vector<DisplayState>& display_states) const
		{
			assert_correctness();
			bool updated=false;
			if(show || hide || mark || unmark || color_valid())
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					if((*it)<display_states.size() && update_display_state(display_states[*it]))
					{
						updated=true;
					}
				}
			}
			return updated;
		}

		bool update_display_states(std::vector<DisplayState>& display_states) const
		{
			assert_correctness();
			bool updated=false;
			if(show || hide || mark || unmark || color_valid())
			{
				for(std::size_t i=0;i<display_states.size();i++)
				{
					if(update_display_state(display_states[i]))
					{
						updated=true;
					}
				}
			}
			return updated;
		}

	private:
		bool update_display_state_visual(DisplayState::Visual& visual) const
		{
			bool updated=false;

			{
				if(show || hide)
				{
					updated=(updated || (visual.visible!=show));
					visual.visible=show;
				}

				if(color_valid())
				{
					updated=(updated || (visual.color!=color));
					visual.color=color;
				}
			}

			return updated;
		}
	};

	DataManager()
	{
		add_atoms_representation("balls");
		add_atoms_representation("sticks");
		add_atoms_representation("trace");
		add_atoms_representation("cartoon");

		add_contacts_representation("faces");
		add_contacts_representation("sas-mesh");
		add_contacts_representation("edges");
		add_contacts_representation("skin-shape");

		add_figures_representation("solid");
		add_figures_representation("mesh");
	}

	DataManager(const DataManager& dm)
	{
		(*this)=dm;
	}

	virtual ~DataManager()
	{
	}

	DataManager& operator=(const DataManager& dm)
	{
		atoms_representations_descriptor_=dm.atoms_representations_descriptor_;
		contacts_representations_descriptor_=dm.contacts_representations_descriptor_;
		figures_representations_descriptor_=dm.figures_representations_descriptor_;
		atoms_=dm.atoms_;
		contacts_=dm.contacts_;
		figures_=dm.figures_;
		global_numeric_adjuncts_=dm.global_numeric_adjuncts_;
		atoms_display_states_=dm.atoms_display_states_;
		contacts_display_states_=dm.contacts_display_states_;
		figures_display_states_=dm.figures_display_states_;
		primary_structure_info_=dm.primary_structure_info_;
		secondary_structure_info_=dm.secondary_structure_info_;
		bonding_links_info_=dm.bonding_links_info_;
		triangulation_info_=dm.triangulation_info_;
		selection_manager_=dm.selection_manager_.make_adjusted_copy(atoms_, contacts_);
		history_of_actions_on_contacts_=dm.history_of_actions_on_contacts_;
		return (*this);
	}

	const std::vector<Atom>& atoms() const
	{
		return atoms_;
	}

	const std::vector<Contact>& contacts() const
	{
		return contacts_;
	}

	const std::vector<Figure>& figures() const
	{
		return figures_;
	}

	const std::map<std::string, double>& global_numeric_adjuncts() const
	{
		return global_numeric_adjuncts_;
	}

	const std::vector<DisplayState>& atoms_display_states() const
	{
		return atoms_display_states_;
	}

	const std::vector<DisplayState>& contacts_display_states() const
	{
		return contacts_display_states_;
	}

	const std::vector<DisplayState>& figures_display_states() const
	{
		return figures_display_states_;
	}

	const common::ConstructionOfPrimaryStructure::BundleOfPrimaryStructure& primary_structure_info() const
	{
		return primary_structure_info_;
	}

	const common::ConstructionOfSecondaryStructure::BundleOfSecondaryStructure& secondary_structure_info() const
	{
		return secondary_structure_info_;
	}

	const common::ConstructionOfBondingLinks::BundleOfBondingLinks& bonding_links_info() const
	{
		return bonding_links_info_;
	}

	const common::ConstructionOfTriangulation::BundleOfTriangulationInformation& triangulation_info() const
	{
		return triangulation_info_;
	}

	const RepresentationsDescriptor& atoms_representation_descriptor() const
	{
		return atoms_representations_descriptor_;
	}

	const RepresentationsDescriptor& contacts_representation_descriptor() const
	{
		return contacts_representations_descriptor_;
	}

	const RepresentationsDescriptor& figures_representation_descriptor() const
	{
		return figures_representations_descriptor_;
	}

	const ChangeIndicator& change_indicator() const
	{
		return change_indicator_;
	}

	void assert_atoms_representations_availability() const
	{
		if(atoms_representations_descriptor_.names.empty())
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

	void assert_atoms_tag_availability(const std::string& tag) const
	{
		if(!is_any_atom_with_tag(tag))
		{
			throw std::runtime_error(std::string("No atoms with tag '")+tag+"'.");
		}
	}

	void assert_atoms_adjunct_availability(const std::string& adjunct) const
	{
		if(!is_any_atom_with_adjunct(adjunct))
		{
			throw std::runtime_error(std::string("No atoms with adjunct '")+adjunct+"'.");
		}
	}

	void assert_triangulation_info_availability() const
	{
		if(triangulation_info_.quadruples_map.empty())
		{
			throw std::runtime_error(std::string("No triangulation info available."));
		}
	}

	void assert_contacts_representations_availability() const
	{
		if(contacts_representations_descriptor_.names.empty())
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

	void assert_contacts_tag_availability(const std::string& tag) const
	{
		if(!is_any_contact_with_tag(tag))
		{
			throw std::runtime_error(std::string("No contacts with tag '")+tag+"'.");
		}
	}

	void assert_contacts_adjunct_availability(const std::string& adjunct) const
	{
		if(!is_any_contact_with_adjunct(adjunct))
		{
			throw std::runtime_error(std::string("No contacts with adjunct '")+adjunct+"'.");
		}
	}

	void assert_figures_representations_availability() const
	{
		if(figures_representations_descriptor_.names.empty())
		{
			throw std::runtime_error(std::string("No figures representations available."));
		}
	}

	void assert_figures_availability() const
	{
		if(figures_.empty())
		{
			throw std::runtime_error(std::string("No figures available."));
		}
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

	bool is_any_atom_with_tag(const std::string& tag) const
	{
		for(std::size_t i=0;i<atoms_.size();i++)
		{
			if(atoms_[i].value.props.tags.count(tag)>0)
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_atom_with_adjunct(const std::string& adjunct) const
	{
		for(std::size_t i=0;i<atoms_.size();i++)
		{
			if(atoms_[i].value.props.adjuncts.count(adjunct)>0)
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

	bool is_any_contact_with_tag(const std::string& tag) const
	{
		for(std::size_t i=0;i<contacts_.size();i++)
		{
			if(contacts_[i].value.props.tags.count(tag)>0)
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_contact_with_adjunct(const std::string& adjunct) const
	{
		for(std::size_t i=0;i<contacts_.size();i++)
		{
			if(contacts_[i].value.props.adjuncts.count(adjunct)>0)
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_figure_visible() const
	{
		for(std::size_t i=0;i<figures_display_states_.size();i++)
		{
			if(figures_display_states_[i].visible())
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_figure_marked() const
	{
		for(std::size_t i=0;i<figures_display_states_.size();i++)
		{
			if(figures_display_states_[i].marked)
			{
				return true;
			}
		}
		return false;
	}

	std::size_t count_residues_by_atom_ids(const std::set<std::size_t>& atom_ids) const
	{
		std::set<std::size_t> residue_ids;
		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			residue_ids.insert(primary_structure_info().map_of_atoms_to_residues.at(*it));
		}
		return residue_ids.size();
	}

	SelectionManager& selection_manager()
	{
		return selection_manager_;
	}

	std::set<std::string>& atom_tags_mutable(const std::size_t id)
	{
		change_indicator_.set_changed_atoms_tags(true);
		return atoms_[id].value.props.tags;
	}

	std::map<std::string, double>& atom_adjuncts_mutable(const std::size_t id)
	{
		change_indicator_.set_changed_atoms_adjuncts(true);
		return atoms_[id].value.props.adjuncts;
	}

	std::set<std::string>& contact_tags_mutable(const std::size_t id)
	{
		change_indicator_.set_changed_contacts_tags(true);
		return contacts_[id].value.props.tags;
	}

	std::map<std::string, double>& contact_adjuncts_mutable(const std::size_t id)
	{
		change_indicator_.set_changed_contacts_adjuncts(true);
		return contacts_[id].value.props.adjuncts;
	}

	std::map<std::string, double>& global_numeric_adjuncts_mutable()
	{
		return global_numeric_adjuncts_;
	}

	bool set_atoms_representation_implemented(const std::size_t representation_id, const std::vector<bool>& statuses)
	{
		if(set_representation_implemented(atoms_representations_descriptor_.names, representation_id, statuses, atoms_display_states_))
		{
			change_indicator_.set_changed_atoms_display_states(true);
			return true;
		}
		return false;
	}

	bool set_contacts_representation_implemented(const std::size_t representation_id, const std::vector<bool>& statuses)
	{
		if(set_representation_implemented(contacts_representations_descriptor_.names, representation_id, statuses, contacts_display_states_))
		{
			change_indicator_.set_changed_contacts_display_states(true);
			return true;
		}
		return false;
	}

	bool set_figures_representation_implemented(const std::size_t representation_id, const std::vector<bool>& statuses)
	{
		if(set_representation_implemented(figures_representations_descriptor_.names, representation_id, statuses, figures_display_states_))
		{
			change_indicator_.set_changed_figures_display_states(true);
			return true;
		}
		return false;
	}

	void update_atoms_display_state(const DisplayStateUpdater& dsu, const std::size_t id)
	{
		if(dsu.update_display_state(id, atoms_display_states_))
		{
			change_indicator_.set_changed_atoms_display_states(true);
		}
	}

	void update_atoms_display_states(const DisplayStateUpdater& dsu, const std::set<std::size_t>& ids)
	{
		if(dsu.update_display_states(ids, atoms_display_states_))
		{
			change_indicator_.set_changed_atoms_display_states(true);
		}
	}

	void update_atoms_display_states(const DisplayStateUpdater& dsu)
	{
		if(dsu.update_display_states(atoms_display_states_))
		{
			change_indicator_.set_changed_atoms_display_states(true);
		}
	}

	void update_contacts_display_state(const DisplayStateUpdater& dsu, const std::size_t id)
	{
		if(dsu.update_display_state(id, contacts_display_states_))
		{
			change_indicator_.set_changed_contacts_display_states(true);
		}
	}

	void update_contacts_display_states(const DisplayStateUpdater& dsu, const std::set<std::size_t>& ids)
	{
		if(dsu.update_display_states(ids, contacts_display_states_))
		{
			change_indicator_.set_changed_contacts_display_states(true);
		}
	}

	void update_contacts_display_states(const DisplayStateUpdater& dsu)
	{
		if(dsu.update_display_states(contacts_display_states_))
		{
			change_indicator_.set_changed_contacts_display_states(true);
		}
	}

	void update_figures_display_state(const DisplayStateUpdater& dsu, const std::size_t id)
	{
		if(dsu.update_display_state(id, figures_display_states_))
		{
			change_indicator_.set_changed_figures_display_states(true);
		}
	}

	void update_figures_display_states(const DisplayStateUpdater& dsu, const std::set<std::size_t>& ids)
	{
		if(dsu.update_display_states(ids, figures_display_states_))
		{
			change_indicator_.set_changed_figures_display_states(true);
		}
	}

	void update_figures_display_states(const DisplayStateUpdater& dsu)
	{
		if(dsu.update_display_states(figures_display_states_))
		{
			change_indicator_.set_changed_figures_display_states(true);
		}
	}

	void reset_change_indicator()
	{
		change_indicator_=ChangeIndicator();
	}

	void reset_atoms_by_swapping(std::vector<Atom>& atoms)
	{
		if(atoms.empty())
		{
			throw std::runtime_error(std::string("No atoms to set."));
		}
		change_indicator_.set_changed_atoms(true);
		atoms_.swap(atoms);
		reset_atoms_display_states();
		reset_data_dependent_on_atoms();
	}

	void reset_atoms_by_copying(const std::vector<Atom>& atoms)
	{
		std::vector<Atom> atoms_copy=atoms;
		reset_atoms_by_swapping(atoms_copy);
	}

	void reset_atoms_display_states()
	{
		change_indicator_.set_changed_atoms_display_states(true);
		atoms_display_states_.clear();
		atoms_display_states_.resize(atoms_.size());
		for(std::size_t i=0;i<atoms_display_states_.size();i++)
		{
			atoms_display_states_[i].drawable=true;
		}
		resize_visuals_in_display_states(atoms_representations_descriptor_.names.size(), atoms_display_states_);
	}

	void restrict_atoms(const std::set<std::size_t>& ids)
	{
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No ids provided to restrict atoms."));
		}

		if(*ids.rbegin()>=atoms_.size())
		{
			throw std::runtime_error(std::string("Invalid ids provided to restrict atoms."));
		}

		std::vector<Atom> restricted_atoms;
		restricted_atoms.reserve(ids.size());

		std::vector<DisplayState> restricted_atoms_display_states;
		restricted_atoms_display_states.reserve(ids.size());

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::size_t id=(*it);
			restricted_atoms.push_back(atoms_[id]);
			restricted_atoms_display_states.push_back(atoms_display_states_[id]);
		}

		change_indicator_.set_changed_atoms(true);

		atoms_.swap(restricted_atoms);
		atoms_display_states_.swap(restricted_atoms_display_states);

		reset_data_dependent_on_atoms();
	}

	void restrict_atoms_and_renumber_residues_by_adjunct(const std::string& name)
	{
		std::set<std::size_t> ids;
		for(std::size_t i=0;i<atoms_.size();i++)
		{
			std::map<std::string, double>::const_iterator it=atoms_[i].value.props.adjuncts.find(name);
			if(it!=atoms_[i].value.props.adjuncts.end())
			{
				atoms_[i].crad.resSeq=static_cast<int>(it->second);
				ids.insert(i);
			}
		}

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms with adjunct '")+name+"'.");
		}

		restrict_atoms(ids);
	}

	void transform_coordinates_of_atoms(
			const std::set<std::size_t>& ids,
			const std::vector<double>& pre_translation_vector,
			const std::vector<double>& rotation_matrix,
			const std::vector<double>& rotation_axis_and_angle,
			const std::vector<double>& post_translation_vector)
	{
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No ids provided to transform atoms."));
		}

		if(pre_translation_vector.empty() && post_translation_vector.empty() && rotation_matrix.empty() && rotation_axis_and_angle.empty())
		{
			throw std::runtime_error(std::string("No transformations provided to transform atoms."));
		}

		if(*ids.rbegin()>=atoms_.size())
		{
			throw std::runtime_error(std::string("Invalid ids provided to transform atoms."));
		}

		if(!pre_translation_vector.empty() && pre_translation_vector.size()!=3)
		{
			throw std::runtime_error(std::string("Invalid translation-before vector provided to transform atoms."));
		}

		if(!post_translation_vector.empty() && post_translation_vector.size()!=3)
		{
			throw std::runtime_error(std::string("Invalid translation-after vector provided to transform atoms."));
		}

		if(!rotation_matrix.empty() && rotation_matrix.size()!=9)
		{
			throw std::runtime_error(std::string("Invalid rotation matrix provided to transform atoms."));
		}

		if(!rotation_axis_and_angle.empty() && rotation_axis_and_angle.size()!=4)
		{
			throw std::runtime_error(std::string("Invalid rotation axis and angle vector provided to transform atoms."));
		}

		change_indicator_.set_changed_atoms(true);

		if(!pre_translation_vector.empty())
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				common::BallValue& ball=atoms_[*it].value;
				ball.x+=pre_translation_vector[0];
				ball.y+=pre_translation_vector[1];
				ball.z+=pre_translation_vector[2];
			}
		}

		if(!rotation_matrix.empty())
		{
			const std::vector<double>& m=rotation_matrix;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				common::BallValue& ball=atoms_[*it].value;
				const apollota::SimplePoint p(ball);
				ball.x=p.x*m[0]+p.y*m[1]+p.z*m[2];
				ball.y=p.x*m[3]+p.y*m[4]+p.z*m[5];
				ball.z=p.x*m[6]+p.y*m[7]+p.z*m[8];
			}
		}

		if(!rotation_axis_and_angle.empty())
		{
			apollota::Rotation rotation(
					apollota::SimplePoint(rotation_axis_and_angle[0], rotation_axis_and_angle[1], rotation_axis_and_angle[2]),
					rotation_axis_and_angle[3]);

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				common::BallValue& ball=atoms_[*it].value;
				const apollota::SimplePoint p=rotation.rotate<apollota::SimplePoint>(ball);
				ball.x=p.x;
				ball.y=p.y;
				ball.z=p.z;
			}
		}

		if(!post_translation_vector.empty())
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				common::BallValue& ball=atoms_[*it].value;
				ball.x+=post_translation_vector[0];
				ball.y+=post_translation_vector[1];
				ball.z+=post_translation_vector[2];
			}
		}

		reset_data_dependent_on_atoms();
	}

	void remove_bonding_links_info()
	{
		bonding_links_info_=common::ConstructionOfBondingLinks::BundleOfBondingLinks();
	}

	void reset_bonding_links_info_by_swapping(common::ConstructionOfBondingLinks::BundleOfBondingLinks& bonding_links_info)
	{
		if(!bonding_links_info.valid(atoms_, primary_structure_info_))
		{
			throw std::runtime_error(std::string("Invalid bonding links info provided."));
		}

		bonding_links_info_.swap(bonding_links_info);
	}

	void reset_bonding_links_info_by_copying(const common::ConstructionOfBondingLinks::BundleOfBondingLinks& bonding_links_info)
	{
		common::ConstructionOfBondingLinks::BundleOfBondingLinks bonding_links_info_copy=bonding_links_info;
		reset_bonding_links_info_by_swapping(bonding_links_info_copy);
	}

	void reset_bonding_links_info_by_creating(const common::ConstructionOfBondingLinks::ParametersToConstructBundleOfBondingLinks& parameters)
	{
		common::ConstructionOfBondingLinks::BundleOfBondingLinks bundle_of_bonding_links;
		if(common::ConstructionOfBondingLinks::construct_bundle_of_bonding_links(
				parameters,
				atoms(),
				primary_structure_info(),
				bundle_of_bonding_links))
		{
			reset_bonding_links_info_by_swapping(bundle_of_bonding_links);
		}
		else
		{
			throw std::runtime_error(std::string("Failed to define bonding links."));
		}
	}

	void remove_triangulation_info()
	{
		triangulation_info_=common::ConstructionOfTriangulation::BundleOfTriangulationInformation();
		reset_data_dependent_on_triangulation_info();
	}

	void reset_triangulation_info_by_swapping(common::ConstructionOfTriangulation::BundleOfTriangulationInformation& triangulation_info)
	{
		if(triangulation_info.quadruples_map.empty())
		{
			throw std::runtime_error(std::string("No triangulation info to set."));
		}

		if(!triangulation_info.matching(common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(atoms_)))
		{
			throw std::runtime_error(std::string("Triangulation info does not match atoms."));
		}

		triangulation_info_.swap(triangulation_info);

		reset_data_dependent_on_triangulation_info();
	}

	void reset_triangulation_info_by_copying(const common::ConstructionOfTriangulation::BundleOfTriangulationInformation& triangulation_info)
	{
		common::ConstructionOfTriangulation::BundleOfTriangulationInformation triangulation_info_copy=triangulation_info;
		reset_triangulation_info_by_swapping(triangulation_info_copy);
	}

	void reset_triangulation_info_by_creating(const common::ConstructionOfTriangulation::ParametersToConstructBundleOfTriangulationInformation& parameters)
	{
		const std::vector<apollota::SimpleSphere> atomic_balls=common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(atoms());

		if(triangulation_info().equivalent(parameters, atomic_balls))
		{
			return;
		}

		common::ConstructionOfTriangulation::BundleOfTriangulationInformation bundle_of_triangulation_information;

		if(!common::ConstructionOfTriangulation::construct_bundle_of_triangulation_information(parameters, atomic_balls, bundle_of_triangulation_information))
		{
			throw std::runtime_error(std::string("Failed to construct triangulation."));
		}

		reset_triangulation_info_by_swapping(bundle_of_triangulation_information);
	}

	void remove_contacts()
	{
		change_indicator_.set_changed_contacts(true);
		contacts_.clear();
		contacts_display_states_.clear();
		selection_manager_.set_contacts(0);
		history_of_actions_on_contacts_.clear();
	}

	void reset_contacts_by_swapping(std::vector<Contact>& contacts)
	{
		if(contacts.empty())
		{
			throw std::runtime_error(std::string("No contacts to set."));
		}
		assert_atoms_availability();
		if(!SelectionManager::check_contacts_compatibility_with_atoms(atoms_, contacts))
		{
			throw std::runtime_error(std::string("Contacts are not compatible with atoms."));
		}
		change_indicator_.set_changed_contacts(true);
		contacts_.swap(contacts);
		reset_contacts_display_states();
		selection_manager_.set_contacts(&contacts_);
		history_of_actions_on_contacts_.clear();
	}

	void reset_contacts_by_copying(const std::vector<Contact>& contacts)
	{
		std::vector<Contact> contacts_copy=contacts;
		reset_contacts_by_swapping(contacts_copy);
	}

	void reset_contacts_by_creating(
			const common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation& parameters_to_construct_contacts,
			const common::ConstructionOfContacts::ParametersToEnhanceContacts& parameters_to_enhance_contacts)
	{
		{
			common::ConstructionOfTriangulation::ParametersToConstructBundleOfTriangulationInformation parameters_to_construct_triangulation;
			parameters_to_construct_triangulation.artificial_boundary_shift=std::max(parameters_to_construct_contacts.probe*2.0, 5.0);
			reset_triangulation_info_by_creating(parameters_to_construct_triangulation);
		}

		if(!contacts_.empty()
				&& !history_of_actions_on_contacts_.constructing.empty()
				&& !parameters_to_construct_contacts.supersedes(history_of_actions_on_contacts_.constructing.back())
				&& !history_of_actions_on_contacts_.enhancing.empty()
				&& !parameters_to_enhance_contacts.supersedes(history_of_actions_on_contacts_.enhancing.back())
				&& SelectionManager::check_contacts_compatibility_with_atoms(atoms_, contacts_))
		{
			return;
		}

		common::ConstructionOfContacts::BundleOfContactInformation bundle_of_contact_information;

		if(!common::ConstructionOfContacts::construct_bundle_of_contact_information(parameters_to_construct_contacts, triangulation_info(), bundle_of_contact_information))
		{
			throw std::runtime_error(std::string("Failed to construct contacts."));
		}

		reset_contacts_by_swapping(bundle_of_contact_information.contacts);

		if(parameters_to_construct_contacts.calculate_volumes)
		{
			change_indicator_.set_changed_atoms_adjuncts(true);
			for(std::size_t i=0;i<bundle_of_contact_information.volumes.size() && i<atoms().size();i++)
			{
				atoms_[i].value.props.adjuncts["volume"]=bundle_of_contact_information.volumes[i];
			}
		}

		if(parameters_to_construct_contacts.calculate_bounding_arcs)
		{
			for(std::size_t i=0;i<bundle_of_contact_information.bounding_arcs.size() && i<contacts().size();i++)
			{
				contacts_[i].value.props.adjuncts["boundary"]=bundle_of_contact_information.bounding_arcs[i];
			}
		}

		history_of_actions_on_contacts_.constructing.clear();
		history_of_actions_on_contacts_.constructing.push_back(parameters_to_construct_contacts);

		common::ConstructionOfContacts::enhance_contacts(parameters_to_enhance_contacts, triangulation_info(), contacts_);

		history_of_actions_on_contacts_.enhancing.clear();
		history_of_actions_on_contacts_.enhancing.push_back(parameters_to_enhance_contacts);
	}

	void reset_contacts_display_states()
	{
		change_indicator_.set_changed_contacts_display_states(true);
		contacts_display_states_.clear();
		contacts_display_states_.resize(contacts_.size());
		for(std::size_t i=0;i<contacts_display_states_.size();i++)
		{
			contacts_display_states_[i].drawable=(!contacts_[i].value.graphics.empty());
		}
		resize_visuals_in_display_states(contacts_representations_descriptor_.names.size(), contacts_display_states_);
	}

	void remove_contacts_graphics(const std::set<std::size_t>& ids)
	{
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::size_t id=(*it);
			if(id<contacts_.size())
			{
				if(!contacts_[id].value.graphics.empty())
				{
					change_indicator_.set_changed_contacts(true);
					contacts_[id].value.graphics.clear();
					history_of_actions_on_contacts_.graphics_creating.erase(id);
				}
			}
		}
		update_contacts_display_states_drawable(ids);
	}

	void reset_contacts_graphics_by_creating(
			const common::ConstructionOfContacts::ParametersToDrawContacts& parameters_to_draw_contacts,
			const std::set<std::size_t>& ids,
			const bool lazy)
	{
		assert_contacts_availability();

		std::set<std::size_t> ids_for_updating;

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::size_t id=(*it);
			if(id<contacts_.size())
			{
				if(contacts_[id].value.graphics.empty())
				{
					ids_for_updating.insert(id);
				}
				else
				{
					std::map<std::size_t, common::ConstructionOfContacts::ParametersToDrawContacts>::const_iterator jt=history_of_actions_on_contacts_.graphics_creating.find(id);
					bool need_to_update=false;
					if(jt==history_of_actions_on_contacts_.graphics_creating.end())
					{
						if(lazy)
						{
							need_to_update=parameters_to_draw_contacts.enable_alt;
						}
						else
						{
							need_to_update=true;
						}
					}
					else
					{
						const common::ConstructionOfContacts::ParametersToDrawContacts& current_draw_parameters=jt->second;
						if(parameters_to_draw_contacts.enable_alt && !current_draw_parameters.enable_alt)
						{
							need_to_update=true;
						}
						else
						{
							if(lazy)
							{
								need_to_update=false;
							}
							else
							{
								need_to_update=!parameters_to_draw_contacts.equals(current_draw_parameters);
							}
						}
					}
					if(need_to_update)
					{
						ids_for_updating.insert(id);
					}
				}
			}
		}

		if(ids_for_updating.empty())
		{
			return;
		}

		assert_triangulation_info_availability();

		if(triangulation_info().parameters_of_construction.artificial_boundary_shift<(parameters_to_draw_contacts.probe*2))
		{
			throw std::runtime_error(std::string("The triangulation artificial boundary is not compatible with the probe radius."));
		}

		change_indicator_.set_changed_contacts(true);

		common::ConstructionOfContacts::draw_contacts(parameters_to_draw_contacts, triangulation_info(), ids_for_updating, contacts_);

		update_contacts_display_states_drawable(ids_for_updating);

		for(std::set<std::size_t>::const_iterator it=ids_for_updating.begin();it!=ids_for_updating.end();++it)
		{
			history_of_actions_on_contacts_.graphics_creating[*it]=parameters_to_draw_contacts;
		}
	}

	void remove_figures()
	{
		change_indicator_.set_changed_figures(true);
		figures_.clear();
		figures_display_states_.clear();
	}

	void remove_figures(const std::set<std::size_t>& ids)
	{
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No ids provided to remove figures."));
		}
		if(*ids.rbegin()>=atoms_.size())
		{
			throw std::runtime_error(std::string("Invalid ids provided to remove atoms."));
		}
		change_indicator_.set_changed_figures(true);
		for(std::set<std::size_t>::const_reverse_iterator it=ids.rbegin();it!=ids.rend();++it)
		{
			figures_.erase(figures_.begin()+(*it));
			figures_display_states_.erase(figures_display_states_.begin()+(*it));
		}
	}

	void add_figures(const std::vector<Figure>& new_figures)
	{
		if(new_figures.empty())
		{
			throw std::runtime_error(std::string("No figures to add."));
		}
		change_indicator_.set_changed_figures(true);
		for(std::size_t i=0;i<new_figures.size();i++)
		{
			if(LongName::match(new_figures, new_figures[i].name).size()>1)
			{
				throw std::runtime_error(std::string("Repeating figure name."));
			}
			if(!LongName::match(figures(), new_figures[i].name).empty())
			{
				throw std::runtime_error(std::string("Redundant figure name."));
			}
			if(!new_figures[i].valid())
			{
				throw std::runtime_error(std::string("Figure is not valid."));
			}
		}
		std::vector<DisplayState> new_figures_display_states(new_figures.size());
		for(std::size_t i=0;i<new_figures_display_states.size();i++)
		{
			new_figures_display_states[i].drawable=true;
		}
		resize_visuals_in_display_states(figures_representations_descriptor_.names.size(), new_figures_display_states);
		figures_.insert(figures_.end(), new_figures.begin(), new_figures.end());
		figures_display_states_.insert(figures_display_states_.end(), new_figures_display_states.begin(), new_figures_display_states.end());
	}

	void add_figure(const Figure& figure)
	{
		add_figures(std::vector<Figure>(1, figure));
	}

	void reset_figures_display_states()
	{
		change_indicator_.set_changed_figures_display_states(true);
		figures_display_states_.clear();
		figures_display_states_.resize(contacts_.size());
		for(std::size_t i=0;i<figures_display_states_.size();i++)
		{
			figures_display_states_[i].drawable=true;
		}
		resize_visuals_in_display_states(figures_representations_descriptor_.names.size(), figures_display_states_);
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
			std::set<std::size_t> ids_drawable;

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

				if(ds.drawable)
				{
					ids_drawable.insert(i);
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

			if(ids_drawable.empty())
			{
				selection_manager_.delete_contacts_selection("_drawable");
			}
			else
			{
				selection_manager_.set_contacts_selection("_drawable", ids_drawable);
			}
		}
	}

	void sync_selections_with_display_states_if_requested_in_string(const std::string& request)
	{
		if(request.find("_marked")!=std::string::npos || request.find("_visible")!=std::string::npos || request.find("_drawable")!=std::string::npos)
		{
			sync_atoms_selections_with_display_states();
			sync_contacts_selections_with_display_states();
		}
	}

private:
	struct HistoryOfActionsOnContacts
	{
		std::vector<common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation> constructing;
		std::vector<common::ConstructionOfContacts::ParametersToEnhanceContacts> enhancing;
		std::map<std::size_t, common::ConstructionOfContacts::ParametersToDrawContacts> graphics_creating;

		void clear()
		{
			constructing.clear();
			enhancing.clear();
			graphics_creating.clear();
		}
	};

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

	bool add_atoms_representation(const std::string& name)
	{
		if(add_names_to_representations(std::vector<std::string>(1, name), atoms_representations_descriptor_.names))
		{
			if(resize_visuals_in_display_states(atoms_representations_descriptor_.names.size(), atoms_display_states_))
			{
				change_indicator_.set_changed_atoms_display_states(true);
			}
			return true;
		}
		return false;
	}

	bool add_contacts_representation(const std::string& name)
	{
		if(add_names_to_representations(std::vector<std::string>(1, name), contacts_representations_descriptor_.names))
		{
			if(resize_visuals_in_display_states(contacts_representations_descriptor_.names.size(), contacts_display_states_))
			{
				change_indicator_.set_changed_contacts_display_states(true);
			}
			return true;
		}
		return false;
	}

	bool add_figures_representation(const std::string& name)
	{
		if(add_names_to_representations(std::vector<std::string>(1, name), figures_representations_descriptor_.names))
		{
			if(resize_visuals_in_display_states(figures_representations_descriptor_.names.size(), figures_display_states_))
			{
				change_indicator_.set_changed_figures_display_states(true);
			}
			return true;
		}
		return false;
	}

	static bool resize_visuals_in_display_states(const std::size_t size, std::vector<DisplayState>& display_states)
	{
		bool resized=false;
		for(std::size_t i=0;i<display_states.size();i++)
		{
			if(display_states[i].visuals.size()!=size)
			{
				display_states[i].visuals.resize(size);
				resized=true;
			}
		}
		return resized;
	}

	void reset_data_dependent_on_atoms()
	{
		primary_structure_info_=common::ConstructionOfPrimaryStructure::construct_bundle_of_primary_structure(atoms_);
		secondary_structure_info_=common::ConstructionOfSecondaryStructure::construct_bundle_of_secondary_structure(atoms_, primary_structure_info_);
		selection_manager_=SelectionManager(&atoms_, 0);

		remove_bonding_links_info();
		remove_triangulation_info();
		remove_contacts();
	}

	void reset_data_dependent_on_triangulation_info()
	{
		remove_contacts();
	}

	void update_contacts_display_states_drawable(const std::set<std::size_t>& ids)
	{
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::size_t id=(*it);
			if(id<contacts_.size())
			{
				DisplayState& ds=contacts_display_states_[id];
				ds.drawable=(!contacts_[id].value.graphics.empty());
				for(std::size_t j=0;j<ds.visuals.size();j++)
				{
					ds.visuals[j].implemented=false;
				}
			}
		}
		resize_visuals_in_display_states(contacts_representations_descriptor_.names.size(), contacts_display_states_);
	}

	RepresentationsDescriptor atoms_representations_descriptor_;
	RepresentationsDescriptor contacts_representations_descriptor_;
	RepresentationsDescriptor figures_representations_descriptor_;
	std::vector<Atom> atoms_;
	std::vector<Contact> contacts_;
	std::vector<Figure> figures_;
	std::map<std::string, double> global_numeric_adjuncts_;
	std::vector<DisplayState> atoms_display_states_;
	std::vector<DisplayState> contacts_display_states_;
	std::vector<DisplayState> figures_display_states_;
	common::ConstructionOfPrimaryStructure::BundleOfPrimaryStructure primary_structure_info_;
	common::ConstructionOfSecondaryStructure::BundleOfSecondaryStructure secondary_structure_info_;
	common::ConstructionOfBondingLinks::BundleOfBondingLinks bonding_links_info_;
	common::ConstructionOfTriangulation::BundleOfTriangulationInformation triangulation_info_;
	SelectionManager selection_manager_;
	HistoryOfActionsOnContacts history_of_actions_on_contacts_;
	ChangeIndicator change_indicator_;
};

}

}

#endif /* SCRIPTING_DATA_MANAGER_H_ */

