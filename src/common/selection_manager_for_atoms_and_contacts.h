#ifndef COMMON_SELECTION_MANAGER_FOR_ATOMS_AND_CONTACTS_H_
#define COMMON_SELECTION_MANAGER_FOR_ATOMS_AND_CONTACTS_H_

#include "testing_of_atoms_and_contacts.h"

namespace common
{

class SelectionManagerForAtomsAndContacts : public TestingOfAtomsAndContacts
{
public:
	SelectionManagerForAtomsAndContacts() :
		atoms_ptr_(0),
		contacts_ptr_(0)
	{
	}

	SelectionManagerForAtomsAndContacts(const std::vector<Atom>* atoms_ptr, const std::vector<Contact>* contacts_ptr=0) :
		atoms_ptr_(0),
		contacts_ptr_(0)
	{
		set_atoms(atoms_ptr);
		set_contacts(contacts_ptr);
	}

	static bool check_contacts_compatibility_with_atoms(const std::vector<Atom>& atoms, const std::vector<Contact>& contacts)
	{
		for(std::size_t i=0;i<contacts.size();i++)
		{
			if(!(contacts[i].ids[0]<atoms.size() && contacts[i].ids[1]<atoms.size()))
			{
				return false;
			}
		}
		return true;
	}

	const std::vector<Atom>& atoms() const
	{
		static const std::vector<Atom> empty_atoms;
		if(atoms_ptr_==0)
		{
			return empty_atoms;
		}
		else
		{
			return (*atoms_ptr_);
		}
	}

	const std::vector<Contact>& contacts() const
	{
		static const std::vector<Contact> empty_contacts;
		if(contacts_ptr_==0)
		{
			return empty_contacts;
		}
		else
		{
			return (*contacts_ptr_);
		}
	}

	void set_atoms(const std::vector<Atom>* atoms_ptr)
	{
		atoms_ptr_=atoms_ptr;
		map_of_atoms_selections_.clear();
		construct_atoms_residues_definition_and_reference();
		set_contacts(0);
	}

	void set_contacts(const std::vector<Contact>* contacts_ptr)
	{
		if(contacts_ptr==0 || check_contacts_compatibility_with_atoms(atoms(), *contacts_ptr))
		{
			contacts_ptr_=contacts_ptr;
			map_of_contacts_selections_.clear();
			construct_contacts_residues_definition_and_reference();
		}
		else
		{
			throw std::runtime_error(std::string("Contacts do not accord to atoms."));
		}
	}

	std::set<std::size_t> get_atoms_selection(const std::string& name) const
	{
		if(atoms().empty())
		{
			throw std::runtime_error(std::string("No atoms to get selection for."));
		}
		std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_atoms_selections_.find(name);
		if(it==map_of_atoms_selections_.end())
		{
			throw std::runtime_error(std::string("No atoms selection with name '")+name+"'.");
			return std::set<std::size_t>();
		}
		else
		{
			return (it->second);
		}
	}


	std::set<std::size_t> select_atoms(const std::set<std::size_t>& from_ids, const std::string& expression_string, const bool full_residues=false) const
	{
		if(atoms().empty())
		{
			throw std::runtime_error(std::string("No atoms to select from."));
			return std::set<std::size_t>();
		}
		else
		{
			std::set<std::size_t> result;
			try
			{
				result=select_atoms(from_ids.empty(), from_ids, read_expression_from_string<test_atom>(expression_string), false);
			}
			catch(const std::exception& e)
			{
				throw std::runtime_error(std::string("Failed to select atoms with expression '")+expression_string+"': "+e.what());
			}
			return (full_residues ? get_ids_for_full_residues(result, atoms_residues_definition_, atoms_residues_reference_) : result);
		}
	}

	std::set<std::size_t> select_atoms(const std::string& expression_string, const bool full_residues=false) const
	{
		return select_atoms(std::set<std::size_t>(), expression_string, full_residues);
	}

	void set_atoms_selection(const std::string& name, const std::set<std::size_t>& ids)
	{
		if(atoms().empty())
		{
			throw std::runtime_error(std::string("No atoms to set selection for."));
		}
		else
		{
			set_selection(name, ids, atoms().size(), map_of_atoms_selections_);
		}
	}

	void delete_atoms_selection(const std::string& name)
	{
		map_of_atoms_selections_.erase(name);
	}

	void delete_atoms_selections()
	{
		map_of_atoms_selections_.clear();
	}

	std::set<std::size_t> get_contacts_selection(const std::string& name) const
	{
		if(contacts().empty())
		{
			throw std::runtime_error(std::string("No contacts to get selection for."));
		}
		std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_contacts_selections_.find(name);
		if(it==map_of_contacts_selections_.end())
		{
			throw std::runtime_error(std::string("No contacts selection with name '")+name+"'.");
			return std::set<std::size_t>();
		}
		else
		{
			return (it->second);
		}
	}

	std::set<std::size_t> select_contacts(const std::set<std::size_t>& from_ids, const std::string& expression_string, const bool full_residues=false) const
	{
		if(contacts().empty())
		{
			throw std::runtime_error(std::string("No contacts to select from."));
			return std::set<std::size_t>();
		}
		else
		{
			std::set<std::size_t> result;
			try
			{
				result=select_contacts(from_ids.empty(), from_ids, read_expression_from_string<test_contact>(expression_string), false);
			}
			catch(const std::exception& e)
			{
				throw std::runtime_error(std::string("Failed to select contacts with expression '")+expression_string+"': "+e.what());
			}
			return (full_residues ? get_ids_for_full_residues(result, contacts_residues_definition_, contacts_residues_reference_) : result);
		}
	}

	std::set<std::size_t> select_contacts(const std::string& expression_string, const bool full_residues=false) const
	{
		return select_contacts(std::set<std::size_t>(), expression_string, full_residues);
	}

	void set_contacts_selection(const std::string& name, const std::set<std::size_t>& ids)
	{
		if(contacts().empty())
		{
			throw std::runtime_error(std::string("No contacts to set selection for."));
		}
		else
		{
			set_selection(name, ids, contacts().size(), map_of_contacts_selections_);
		}
	}

	void delete_contacts_selection(const std::string& name)
	{
		map_of_contacts_selections_.erase(name);
	}

	void delete_contacts_selections()
	{
		map_of_contacts_selections_.clear();
	}

	const std::map< std::string, std::set<std::size_t> >& map_of_atoms_selections() const
	{
		return map_of_atoms_selections_;
	}

	const std::map< std::string, std::set<std::size_t> >& map_of_contacts_selections() const
	{
		return map_of_contacts_selections_;
	}

private:
	static bool check_selection_ids(const std::set<std::size_t>& ids, const std::size_t id_limit)
	{
		if(ids.size()<=id_limit)
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				if((*it)>=id_limit)
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}

	static void set_selection(const std::string& name, const std::set<std::size_t>& ids, const std::size_t id_limit, std::map< std::string, std::set<std::size_t> >& map_of_selections)
	{
		if(name.empty())
		{
			throw std::runtime_error(std::string("No name for setting selection."));
		}
		else
		{
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No ids provided for selection '")+name+"'.");
			}
			else
			{
				if(!check_selection_ids(ids, id_limit))
				{
					throw std::runtime_error(std::string("Invalid ids provided for selection '")+name+"'.");
				}
				else
				{
					map_of_selections[name]=ids;
				}
			}
		}
	}

	template<typename Tester>
	static std::vector< TestingExpressionToken<Tester> > read_expression_from_string(const std::string& expression_string)
	{
		std::vector< TestingExpressionToken<Tester> > result;
		if(!expression_string.empty())
		{
			std::string prepared_string;
			char last_nws=0;
			std::size_t i=0;
			while(i<expression_string.size())
			{
				const char c=expression_string[i];
				if(c>0 && c<=32)
				{
					if(last_nws!=0 && i+1<expression_string.size())
					{
						if(!(last_nws==',' || last_nws==';' || last_nws=='|' || last_nws=='&' || last_nws=='<'))
						{
							char next_nws=0;
							std::size_t j=i+1;
							while(j<expression_string.size() && next_nws==0)
							{
								const char d=expression_string[j];
								if(d>32)
								{
									next_nws=d;
									i=j-1;
								}
								j++;
							}
							if(next_nws!=0)
							{
								if(!(next_nws==',' || next_nws==';' || next_nws=='|' || next_nws=='&' || next_nws=='<' || next_nws=='>'))
								{
									prepared_string.push_back(' ');
								}
							}
						}
					}
				}
				else if(c>0)
				{
					last_nws=c;
					if(c=='{' || c=='}' || c=='(' || c==')')
					{
						if(!prepared_string.empty() && prepared_string[prepared_string.size()-1]>32)
						{
							prepared_string.push_back(' ');
						}
						prepared_string.push_back(c);
						if(i+1<expression_string.size() && expression_string[i+1]>32)
						{
							prepared_string.push_back(' ');
						}
					}
					else
					{
						prepared_string.push_back(c);
					}
				}
				i++;
			}
			std::istringstream input(prepared_string);
			while(input.good())
			{
				TestingExpressionToken<Tester> token;
				input >> token;
				result.push_back(token);
			}
		}
		return result;
	}

	static std::set<std::size_t> get_ids_for_full_residues(
			const std::set<std::size_t>& initial_ids,
			const std::vector< std::vector<std::size_t> >& residues_definition,
			const std::vector<std::size_t>& residues_reference)
	{
		std::set<std::size_t> result;
		for(std::set<std::size_t>::const_iterator it=initial_ids.begin();it!=initial_ids.end();++it)
		{
			const std::size_t id=(*it);
			if(id<residues_reference.size() && residues_reference[id]<residues_definition.size())
			{
				const std::vector<std::size_t>& residue_ids=residues_definition[residues_reference[id]];
				result.insert(residue_ids.begin(), residue_ids.end());
			}
			else
			{
				throw std::runtime_error(std::string("Invalid mapping of residue."));
			}
		}
		return result;
	}

	void fix_atom_tester(test_atom& tester) const
	{
		tester.atoms_ptr=atoms_ptr_;

		if(!tester.name_of_base_selection_of_atoms.empty())
		{
			const std::set<std::size_t> sel=get_atoms_selection(tester.name_of_base_selection_of_atoms);
			tester.allowed_ids.insert(sel.begin(), sel.end());
		}

		if(!tester.name_of_base_selection_of_contacts.empty())
		{
			const std::set<std::size_t> sel=get_contacts_selection(tester.name_of_base_selection_of_contacts);
			for(std::set<std::size_t>::const_iterator it=sel.begin();it!=sel.end();++it)
			{
				const Contact& contact=contacts()[*it];
				tester.allowed_ids.insert(contact.ids[0]);
				tester.allowed_ids.insert(contact.ids[1]);
			}
		}
	}

	void fix_contact_tester(test_contact& tester) const
	{
		tester.atoms_ptr=atoms_ptr_;
		tester.contacts_ptr=contacts_ptr_;

		if(!tester.name_of_base_selection_of_contacts.empty())
		{
			std::set<std::size_t> sel=get_contacts_selection(tester.name_of_base_selection_of_contacts);
			tester.allowed_ids.insert(sel.begin(), sel.end());
		}

		fix_atom_tester(tester.test_atom_a);
		fix_atom_tester(tester.test_atom_a_not);
		fix_atom_tester(tester.test_atom_b);
		fix_atom_tester(tester.test_atom_b_not);
	}

	std::set<std::size_t> select_atoms(const bool from_all, const std::set<std::size_t>& from_ids, const std::vector< TestingExpressionToken<test_atom> >& expression, const bool postfix) const
	{
		std::set<std::size_t> result;

		if(from_all || !from_ids.empty())
		{
			std::vector< TestingExpressionToken<test_atom> > postfix_expression=(postfix ? expression : convert_testing_expression_from_infix_to_postfix_form(expression));

			for(std::size_t i=0;i<postfix_expression.size();i++)
			{
				if(postfix_expression[i].is_tester())
				{
					fix_atom_tester(postfix_expression[i].tester);
				}
			}

			if(from_all)
			{
				for(std::size_t id=0;id<atoms().size();id++)
				{
					if(evaluate_testing_expression_in_postfix_form(postfix_expression, id))
					{
						result.insert(id);
					}
				}
			}
			else
			{
				for(std::set<std::size_t>::const_iterator it=from_ids.begin();it!=from_ids.end();++it)
				{
					const std::size_t id=(*it);
					if(id<atoms().size() && evaluate_testing_expression_in_postfix_form(postfix_expression, id))
					{
						result.insert(id);
					}
				}
			}
		}

		return result;
	}

	std::set<std::size_t> select_contacts(const bool from_all, const std::set<std::size_t>& from_ids, const std::vector< TestingExpressionToken<test_contact> >& expression, const bool postfix) const
	{
		std::set<std::size_t> result;

		if(from_all || !from_ids.empty())
		{
			std::vector< TestingExpressionToken<test_contact> > postfix_expression=(postfix ? expression : convert_testing_expression_from_infix_to_postfix_form(expression));

			for(std::size_t i=0;i<postfix_expression.size();i++)
			{
				if(postfix_expression[i].is_tester())
				{
					fix_contact_tester(postfix_expression[i].tester);
				}
			}

			if(from_all)
			{
				for(std::size_t id=0;id<contacts().size();id++)
				{
					if(evaluate_testing_expression_in_postfix_form(postfix_expression, id))
					{
						result.insert(id);
					}
				}
			}
			else
			{
				for(std::set<std::size_t>::const_iterator it=from_ids.begin();it!=from_ids.end();++it)
				{
					const std::size_t id=(*it);
					if(id<contacts().size() && evaluate_testing_expression_in_postfix_form(postfix_expression, id))
					{
						result.insert(id);
					}
				}
			}
		}

		return result;
	}

	void construct_atoms_residues_definition_and_reference()
	{
		atoms_residues_definition_.clear();
		atoms_residues_reference_.clear();
		if(!atoms().empty())
		{
			std::map<ChainResidueAtomDescriptor, std::vector<std::size_t> > map_of_residues;
			for(std::size_t i=0;i<atoms().size();i++)
			{
				map_of_residues[atoms()[i].crad.without_atom()].push_back(i);
			}

			atoms_residues_definition_.resize(map_of_residues.size());
			atoms_residues_reference_.resize(atoms().size(), 0);

			std::size_t residue_id=0;
			for(std::map<ChainResidueAtomDescriptor, std::vector<std::size_t> >::const_iterator it=map_of_residues.begin();it!=map_of_residues.end();++it)
			{
				const std::vector<std::size_t>& atoms_ids=it->second;
				atoms_residues_definition_[residue_id]=atoms_ids;
				for(std::size_t i=0;i<atoms_ids.size();i++)
				{
					atoms_residues_reference_[atoms_ids[i]]=residue_id;
				}
				residue_id++;
			}
		}
	}

	void construct_contacts_residues_definition_and_reference()
	{
		contacts_residues_definition_.clear();
		contacts_residues_reference_.clear();
		if(!contacts().empty())
		{
			std::map<ChainResidueAtomDescriptorsPair, std::vector<std::size_t> > map_of_residues;
			for(std::size_t i=0;i<contacts().size();i++)
			{
				map_of_residues[ChainResidueAtomDescriptorsPair(
										atoms()[contacts()[i].ids[0]].crad.without_atom(),
										atoms()[contacts()[i].ids[1]].crad.without_atom())
								].push_back(i);
			}

			contacts_residues_definition_.resize(map_of_residues.size());
			contacts_residues_reference_.resize(contacts().size(), 0);

			std::size_t residue_id=0;
			for(std::map<ChainResidueAtomDescriptorsPair, std::vector<std::size_t> >::const_iterator it=map_of_residues.begin();it!=map_of_residues.end();++it)
			{
				const std::vector<std::size_t>& contacts_ids=it->second;
				contacts_residues_definition_[residue_id]=contacts_ids;
				for(std::size_t i=0;i<contacts_ids.size();i++)
				{
					contacts_residues_reference_[contacts_ids[i]]=residue_id;
				}
				residue_id++;
			}
		}
	}

	const std::vector<Atom>* atoms_ptr_;
	const std::vector<Contact>* contacts_ptr_;
	std::vector< std::vector<std::size_t> > atoms_residues_definition_;
	std::vector<std::size_t> atoms_residues_reference_;
	std::vector< std::vector<std::size_t> > contacts_residues_definition_;
	std::vector<std::size_t> contacts_residues_reference_;
	std::map< std::string, std::set<std::size_t> > map_of_atoms_selections_;
	std::map< std::string, std::set<std::size_t> > map_of_contacts_selections_;
};

}

#endif /* COMMON_SELECTION_MANAGER_FOR_ATOMS_AND_CONTACTS_H_ */
