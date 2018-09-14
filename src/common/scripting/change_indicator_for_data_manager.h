#ifndef COMMON_SCRIPTING_CHANGE_INDICATOR_FOR_DATA_MANAGER_H_
#define COMMON_SCRIPTING_CHANGE_INDICATOR_FOR_DATA_MANAGER_H_

namespace common
{

namespace scripting
{

struct ChangeIndicatorForDataManager
{
	bool changed_atoms;
	bool changed_contacts;
	bool changed_atoms_tags;
	bool changed_contacts_tags;
	bool changed_atoms_display_states;
	bool changed_contacts_display_states;

	ChangeIndicatorForDataManager() :
		changed_atoms(false),
		changed_contacts(false),
		changed_atoms_tags(false),
		changed_contacts_tags(false),
		changed_atoms_display_states(false),
		changed_contacts_display_states(false)
	{
	}

	void ensure_correctness()
	{
		changed_contacts=(changed_contacts || changed_atoms);
		changed_atoms_tags=(changed_atoms_tags || changed_atoms);
		changed_contacts_tags=(changed_contacts_tags || changed_contacts);
		changed_atoms_display_states=(changed_atoms_display_states || changed_atoms);
		changed_contacts_display_states=(changed_contacts_display_states || changed_contacts);
	}

	bool changed() const
	{
		return (changed_atoms
				|| changed_contacts
				|| changed_atoms_tags
				|| changed_contacts_tags
				|| changed_atoms_display_states
				|| changed_contacts_display_states);
	}
};

}

}

#endif /* COMMON_SCRIPTING_CHANGE_INDICATOR_FOR_DATA_MANAGER_H_ */
