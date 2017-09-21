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
		std::vector<bool> visibility;
		std::vector<unsigned int> colors;
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

	const DisplayState& display_state_of_atoms() const
	{
		return display_state_of_atoms_;
	}

	const DisplayState& display_state_of_contacts() const
	{
		return display_state_of_contacts_;
	}

	void execute(const std::string& command, std::ostream& output)
	{
		if(!command.empty())
		{
			std::istringstream input(command);

			std::string token;
			input >> token;

			if(token=="read-atoms")
			{
				read_atoms(input, output);
			}
			else if(token=="restrict-atoms")
			{
				restrict_atoms(input, output);
			}
			else if(token=="write-atoms")
			{
				write_atoms(input, output);
			}
			else if(token=="read-atoms-and-contacts")
			{
				read_atoms_and_contacts(input, output);
			}
			else if(token=="construct-contacts")
			{
				construct_contacts(input, output);
			}
			else if(token=="write-atoms-and-contacts")
			{
				write_atoms_and_contacts(input, output);
			}
			else if(token=="select-atoms")
			{
				select_atoms(input, output);
			}
			else if(token=="select-contacts")
			{
				select_contacts(input, output);
			}
			else if(token=="list-selections-of-atoms")
			{
				list_selections_of_atoms(input, output);
			}
			else if(token=="list-selections-of-contacts")
			{
				list_selections_of_contacts(input, output);
			}
			else if(token=="clear-selections-of-atoms")
			{
				clear_selections_of_atoms(input, output);
			}
			else if(token=="clear-selections-of-contacts")
			{
				clear_selections_of_contacts(input, output);
			}
			else if(token=="print-atoms")
			{
				print_atoms(input, output);
			}
			else if(token=="print-contacts")
			{
				print_contacts(input, output);
			}
			else if(token=="show-atoms")
			{
				show_atoms(input, output);
			}
			else if(token=="show-contacts")
			{
				show_contacts(input, output);
			}
			else if(token=="hide-atoms")
			{
				hide_atoms(input, output);
			}
			else if(token=="hide-contacts")
			{
				hide_contacts(input, output);
			}
			else if(token=="color-atoms")
			{
				color_atoms(input, output);
			}
			else if(token=="color-contacts")
			{
				color_contacts(input, output);
			}
			else
			{
				throw std::runtime_error(std::string("Unrecognized command."));
			}
		}
	}

private:
	void read_atoms(const std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void restrict_atoms(const std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void write_atoms(const std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void read_atoms_and_contacts(const std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void construct_contacts(const std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void write_atoms_and_contacts(const std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void select_atoms(const std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void select_contacts(const std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void list_selections_of_atoms(const std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void list_selections_of_contacts(const std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void clear_selections_of_atoms(const std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void clear_selections_of_contacts(const std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void print_atoms(const std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void print_contacts(const std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void show_atoms(const std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void show_contacts(const std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void hide_atoms(const std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void hide_contacts(const std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void color_atoms(const std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void color_contacts(const std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	std::vector<Atom> atoms_;
	std::vector<Contact> contacts_;
	DisplayState display_state_of_atoms_;
	DisplayState display_state_of_contacts_;
	SelectionManagerForAtomsAndContacts selection_manager_;
};

}

#endif /* COMMON_MANIPULATION_MANAGER_FOR_ATOMS_AND_CONTACTS_H_ */
