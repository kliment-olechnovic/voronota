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

		void reset(const std::size_t n, const bool visible=false, const unsigned int color=0)
		{
			visibility.clear();
			colors.clear();
			if(n>0)
			{
				visibility.resize(n, visible);
				colors.resize(n, color);
			}
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

			input >> std::ws;

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
	static bool read_bracketed_string(std::istream& input, std::string& output)
	{
		input >> std::ws;
		const int c=input.peek();
		if(c==std::char_traits<char>::to_int_type('['))
		{
			input.get();
			output.clear();
			std::getline(input, output, ']');
			return true;
		}
		return false;
	}

	void assert_atoms_availability() const
	{
		if(atoms_.empty())
		{
			throw std::runtime_error(std::string("No atoms available."));
		}
	}

	void assert_contacts_availability() const
	{
		if(contacts_.empty())
		{
			throw std::runtime_error(std::string("No contacts available."));
		}
	}

	void reset_atoms(std::vector<Atom>& atoms)
	{
		atoms_.swap(atoms);
		contacts_.clear();
		display_state_of_atoms_.reset(atoms_.size(), true, 0xFFFFFF);
		display_state_of_contacts_.reset(0);
		selection_manager_=SelectionManagerForAtomsAndContacts(&atoms_, 0);
	}

	void reset_contacts(std::vector<Contact>& contacts)
	{
		contacts_.swap(contacts);
		display_state_of_contacts_.reset(contacts_.size(), false, 0xFFFFFF);
		selection_manager_.set_contacts(&contacts_);
	}

	void read_atoms(std::istringstream& input, std::ostream& output)
	{
		ConstructionOfAtomicBalls::collect_atomic_balls_from_file collect_atomic_balls_from_file;
		std::string atoms_file;
		std::string radii_file;
		double default_radius=ConstructionOfAtomicBalls::collect_atomic_balls_from_file::default_default_radius();
		bool only_default_radius=false;

		{
			std::string token;
			while(input.good())
			{
				input >> token;

				if(token=="file")
				{
					input >> atoms_file;
				}
				else if(token=="radii-file")
				{
					input >> radii_file;
				}
				else if(token=="default-radius")
				{
					input >> default_radius;
				}
				else if(token=="only-default-radius")
				{
					only_default_radius=true;
				}
				else if(token=="mmcif")
				{
					collect_atomic_balls_from_file.mmcif=true;
				}
				else if(token=="heteroatoms")
				{
					collect_atomic_balls_from_file.include_heteroatoms=true;
				}
				else if(token=="hydrogens")
				{
					collect_atomic_balls_from_file.include_hydrogens=true;
				}
				else if(token=="multimodel")
				{
					collect_atomic_balls_from_file.multimodel_chains=true;
				}
				else
				{
					throw std::runtime_error(std::string("Invalid token '")+token+"'.");
				}

				if(input.fail() || token.empty())
				{
					throw std::runtime_error(std::string("Invalid command."));
				}

				input >> std::ws;
			}
		}

		if(atoms_file.empty())
		{
			throw std::runtime_error(std::string("Missing atoms file."));
		}

		if(!radii_file.empty() || only_default_radius || default_radius!=ConstructionOfAtomicBalls::collect_atomic_balls_from_file::default_default_radius())
		{
			collect_atomic_balls_from_file.set_atom_radius_assigner(default_radius, only_default_radius, radii_file);
		}

		std::vector<Atom> atoms;
		if(collect_atomic_balls_from_file(atoms_file, atoms))
		{
			if(atoms.size()<4)
			{
				throw std::runtime_error(std::string("Less than 4 atoms read."));
			}
			else
			{
				reset_atoms(atoms);
				output << "Read " << atoms_.size() << " atoms from file '" << atoms_file << "'.\n";
			}
		}
		else
		{
			throw std::runtime_error(std::string("Failed to read atoms from file."));
		}
	}

	void restrict_atoms(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_availability();

		std::string restriction_expression;
		bool full_residues=false;

		{
			std::string token;
			while(input.good())
			{
				input >> token;

				if(token=="sel")
				{
					read_bracketed_string(input, restriction_expression);
				}
				else if(token=="full-residues")
				{
					full_residues=true;
				}
				else
				{
					throw std::runtime_error(std::string("Invalid token '")+token+"'.");
				}

				if(input.fail() || token.empty())
				{
					throw std::runtime_error(std::string("Invalid command."));
				}

				input >> std::ws;
			}
		}

		const std::set<std::size_t> ids=selection_manager_.select_atoms(restriction_expression, full_residues);

		if(ids.size()<4)
		{
			throw std::runtime_error(std::string("Less than 4 atoms selected."));
		}

		std::vector<Atom> atoms;
		atoms.reserve(ids.size());
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			atoms.push_back(atoms.at(*it));
		}

		reset_atoms(atoms);
		output << "Restricted from " << atoms.size() << " to " << atoms_.size() << " atoms.\n";
	}

	void write_atoms(std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void read_atoms_and_contacts(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void construct_contacts(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_availability();

		ConstructionOfContacts::construct_bundle_of_contact_information construct_bundle_of_contact_information;
		construct_bundle_of_contact_information.calculate_volumes=true;

		ConstructionOfContacts::enhance_bundle_of_contact_information enhance_bundle_of_contact_information;
		enhance_bundle_of_contact_information.tag_centrality=true;
		enhance_bundle_of_contact_information.tag_peripherial=true;

		bool draw=false;
		std::string drawing_expression="{min-seq-sep 1}";

		{
			std::string token;
			while(input.good())
			{
				input >> token;

				if(token=="probe")
				{
					input >> construct_bundle_of_contact_information.probe;
					enhance_bundle_of_contact_information.probe=construct_bundle_of_contact_information.probe;
				}
				else if(token=="draw")
				{
					draw=true;
				}
				else if(token=="draw-sel")
				{
					draw=true;
					read_bracketed_string(input, drawing_expression);
				}
				else
				{
					throw std::runtime_error(std::string("Invalid token '")+token+"'.");
				}

				if(input.fail() || token.empty())
				{
					throw std::runtime_error(std::string("Invalid command."));
				}

				input >> std::ws;
			}
		}

		ConstructionOfContacts::BundleOfContactInformation bundle_of_contact_information;

		if(construct_bundle_of_contact_information(common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(atoms_), bundle_of_contact_information))
		{
			std::set<std::size_t> draw_ids;
			if(draw)
			{
				draw_ids=SelectionManagerForAtomsAndContacts(&atoms_, &bundle_of_contact_information.contacts).select_contacts(drawing_expression, true);
			}
			enhance_bundle_of_contact_information(bundle_of_contact_information, draw_ids);

			reset_contacts(bundle_of_contact_information.contacts);

			for(std::size_t i=0;i<bundle_of_contact_information.volumes.size() && i<atoms_.size();i++)
			{
				atoms_[i].value.props.adjuncts["volume"]=bundle_of_contact_information.volumes[i];
			}
		}
		else
		{
			throw std::runtime_error(std::string("Failed to construct contacts."));
		}
	}

	void write_atoms_and_contacts(std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void select_atoms(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void select_contacts(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void list_selections_of_atoms(std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void list_selections_of_contacts(std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void clear_selections_of_atoms(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void clear_selections_of_contacts(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void print_atoms(std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void print_contacts(std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void show_atoms(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void show_contacts(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void hide_atoms(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void hide_contacts(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void color_atoms(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); //TODO implement
	}

	void color_contacts(std::istringstream& input, std::ostream& output)
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
