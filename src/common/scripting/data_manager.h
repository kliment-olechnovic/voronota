#ifndef COMMON_SCRIPTING_DATA_MANAGER_H_
#define COMMON_SCRIPTING_DATA_MANAGER_H_

#include "../construction_of_secondary_structure.h"

#include "selection_manager.h"

namespace common
{

namespace scripting
{

class DataManager
{
public:
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

	DataManager()
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

	const std::set<std::size_t>& atoms_representations_implemented_always() const
	{
		return atoms_representations_implemented_always_;
	}

	const std::set<std::size_t>& contacts_representations_implemented_always() const
	{
		return contacts_representations_implemented_always_;
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

	std::set<std::size_t> filter_atoms_drawable_implemented_ids(const std::set<std::size_t>& visual_ids, const std::set<std::size_t>& ids, const bool only_visible) const
	{
		return filter_drawable_implemented_ids(atoms_display_states_, visual_ids, ids, only_visible);
	}

	std::set<std::size_t> filter_atoms_drawable_implemented_ids(const std::set<std::size_t>& ids, const bool only_visible) const
	{
		return filter_drawable_implemented_ids(atoms_display_states_, std::set<std::size_t>(), ids, only_visible);
	}

	std::set<std::size_t> filter_contacts_drawable_implemented_ids(const std::set<std::size_t>& visual_ids, const std::set<std::size_t>& ids, const bool only_visible) const
	{
		return filter_drawable_implemented_ids(contacts_display_states_, visual_ids, ids, only_visible);
	}

	std::set<std::size_t> filter_contacts_drawable_implemented_ids(const std::set<std::size_t>& ids, const bool only_visible) const
	{
		return filter_drawable_implemented_ids(contacts_display_states_, std::set<std::size_t>(), ids, only_visible);
	}

	SelectionManager& selection_manager()
	{
		return selection_manager_;
	}

	std::vector<Atom>& atoms_mutable()
	{
		return atoms_;
	}

	std::vector<Contact>& contacts_mutable()
	{
		return contacts_;
	}

	std::vector<DisplayState>& atoms_display_states_mutable()
	{
		return atoms_display_states_;
	}

	std::vector<DisplayState>& contacts_display_states_mutable()
	{
		return contacts_display_states_;
	}

	bool add_atoms_representations(const std::vector<std::string>& names)
	{
		if(add_names_to_representations(names, atoms_representation_names_))
		{
			resize_visuals_in_display_states(atoms_representation_names_.size(), atoms_display_states_);
			return true;
		}
		return false;
	}

	bool add_contacts_representations(const std::vector<std::string>& names)
	{
		if(add_names_to_representations(names, contacts_representation_names_))
		{
			resize_visuals_in_display_states(contacts_representation_names_.size(), contacts_display_states_);
			return true;
		}
		return false;
	}

	bool set_atoms_representation_implemented_always(const std::size_t representation_id, const bool status)
	{
		if(set_representation_implemented_always(atoms_representation_names_, representation_id, status, atoms_representations_implemented_always_))
		{
			set_atoms_representations_implemented_if_required_always();
			return true;
		}
		return false;
	}

	bool set_contacts_representation_implemented_always(const std::size_t representation_id, const bool status)
	{
		if(set_representation_implemented_always(contacts_representation_names_, representation_id, status, contacts_representations_implemented_always_))
		{
			set_contacts_representations_implemented_if_required_always();
			return true;
		}
		return false;
	}

	bool set_atoms_representation_implemented(const std::size_t representation_id, const std::vector<bool>& statuses)
	{
		return set_representation_implemented(atoms_representation_names_, representation_id, statuses, atoms_display_states_);
	}

	bool set_contacts_representation_implemented(const std::size_t representation_id, const std::vector<bool>& statuses)
	{
		return set_representation_implemented(contacts_representation_names_, representation_id, statuses, contacts_display_states_);
	}

	void reset_atoms_by_swapping(std::vector<Atom>& atoms)
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
		selection_manager_=SelectionManager(&atoms_, 0);
	}

	void reset_atoms_by_copying(const std::vector<Atom>& atoms)
	{
		std::vector<Atom> atoms_copy=atoms;
		reset_atoms_by_swapping(atoms_copy);
	}

	void reset_atoms_display_states()
	{
		atoms_display_states_.clear();
		atoms_display_states_.resize(atoms_.size());
		for(std::size_t i=0;i<atoms_display_states_.size();i++)
		{
			atoms_display_states_[i].drawable=true;
		}
		resize_visuals_in_display_states(atoms_representation_names_.size(), atoms_display_states_);
		set_atoms_representations_implemented_if_required_always();
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
		contacts_.swap(contacts);
		reset_contacts_display_states();
		selection_manager_.set_contacts(&contacts_);
	}

	void reset_contacts_by_copying(const std::vector<Contact>& contacts)
	{
		std::vector<Contact> contacts_copy=contacts;
		reset_contacts_by_swapping(contacts_copy);
	}

	void reset_contacts_display_states()
	{
		contacts_display_states_.clear();
		contacts_display_states_.resize(contacts_.size());
		for(std::size_t i=0;i<contacts_display_states_.size();i++)
		{
			contacts_display_states_[i].drawable=(!contacts_[i].value.graphics.empty());
		}
		resize_visuals_in_display_states(contacts_representation_names_.size(), contacts_display_states_);
		set_contacts_representations_implemented_if_required_always();
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

	void sync_selections_with_display_states_if_requested_in_string(const std::string& request)
	{
		if(request.find("_marked")!=std::string::npos || request.find("_visible")!=std::string::npos)
		{
			sync_atoms_selections_with_display_states();
			sync_contacts_selections_with_display_states();
		}
	}

private:
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
						good=(good || (display_states[*it].implemented(*jt) && (!only_visible || display_states[*it].visible(*jt))));
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

	void set_atoms_representations_implemented_if_required_always()
	{
		if(!atoms_representations_implemented_always_.empty() && !atoms_.empty())
		{
			for(std::set<std::size_t>::const_iterator it=atoms_representations_implemented_always_.begin();it!=atoms_representations_implemented_always_.end();++it)
			{
				set_atoms_representation_implemented(*it, std::vector<bool>(atoms_.size(), true));
			}
		}
	}

	void set_contacts_representations_implemented_if_required_always()
	{
		if(!contacts_representations_implemented_always_.empty() && !contacts_.empty())
		{
			for(std::set<std::size_t>::const_iterator it=contacts_representations_implemented_always_.begin();it!=contacts_representations_implemented_always_.end();++it)
			{
				set_contacts_representation_implemented(*it, std::vector<bool>(contacts_.size(), true));
			}
		}
	}

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
	SelectionManager selection_manager_;
};

}

}

#endif /* COMMON_SCRIPTING_DATA_MANAGER_H_ */
