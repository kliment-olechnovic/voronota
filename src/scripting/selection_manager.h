#ifndef SCRIPTING_SELECTION_MANAGER_H_
#define SCRIPTING_SELECTION_MANAGER_H_

#include "../common/conversion_of_descriptors.h"

#include "testing_of_atoms_and_contacts.h"

namespace scripting
{

class SelectionManager
{
public:
	struct Query
	{
		std::set<std::size_t> from_ids;
		std::string expression_string;
		bool full_residues;

		Query() :
			expression_string("[]"),
			full_residues(false)
		{
		}

		explicit Query(const std::string& expression_string) :
			expression_string(expression_string),
			full_residues(false)
		{
		}

		Query(const std::string& expression_string, const bool full_residues) :
			expression_string(expression_string),
			full_residues(full_residues)
		{
		}

		Query(const std::set<std::size_t>& from_ids, const std::string& expression_string, const bool full_residues) :
			from_ids(from_ids),
			expression_string(expression_string),
			full_residues(full_residues)
		{
		}
	};

	SelectionManager() :
		atoms_ptr_(0),
		contacts_ptr_(0)
	{
	}

	SelectionManager(const std::vector<Atom>* atoms_ptr, const std::vector<Contact>* contacts_ptr=0) :
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

	SelectionManager make_adjusted_copy(const std::vector<Atom>& copy_of_atoms, const std::vector<Contact>& copy_of_contacts) const
	{
		if(atoms_ptr_==0 && contacts_ptr_==0)
		{
			return (*this);
		}

		if(atoms_ptr_!=0 && copy_of_atoms.size()!=atoms_ptr_->size())
		{
			throw std::runtime_error(std::string("Invalid atoms for making adjusted copy."));
		}

		if(contacts_ptr_!=0 && copy_of_contacts.size()!=contacts_ptr_->size())
		{
			throw std::runtime_error(std::string("Invalid contacts for making adjusted copy."));
		}

		if(contacts_ptr_!=0 && !check_contacts_compatibility_with_atoms(copy_of_atoms, copy_of_contacts))
		{
			throw std::runtime_error(std::string("Incompatable atoms and contacts for making adjusted copy."));
		}

		SelectionManager copy_of_sm=(*this);

		if(copy_of_sm.atoms_ptr_!=0)
		{
			copy_of_sm.atoms_ptr_=&copy_of_atoms;
		}

		if(copy_of_sm.contacts_ptr_!=0)
		{
			copy_of_sm.contacts_ptr_=&copy_of_contacts;
		}

		return copy_of_sm;
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

	std::set<std::size_t> select_atoms(const Query& query) const
	{
		if(atoms().empty())
		{
			return std::set<std::size_t>();
		}
		else
		{
			std::set<std::size_t> result;
			try
			{
				result=select_atoms(query.from_ids.empty(), query.from_ids, read_expression_from_string<TAC::TesterOfAtom>(query.expression_string), false);
			}
			catch(const std::exception& e)
			{
				throw std::runtime_error(std::string("Failed to select atoms with expression '")+query.expression_string+"': "+e.what());
			}
			return (query.full_residues ? get_ids_for_full_residues(result, atoms_residues_definition_, atoms_residues_reference_) : result);
		}
	}

	std::set<std::size_t> select_atoms_by_contacts(const std::set<std::size_t>& from_ids, const std::set<std::size_t>& contact_ids, const bool full_residues) const
	{
		const std::set<std::size_t> result=select_atoms_by_contacts(from_ids.empty(), from_ids, contact_ids);
		return (full_residues ? get_ids_for_full_residues(result, atoms_residues_definition_, atoms_residues_reference_) : result);
	}

	std::set<std::size_t> select_atoms_by_contacts(const std::set<std::size_t>& contact_ids, const bool full_residues) const
	{
		return select_atoms_by_contacts(std::set<std::size_t>(), contact_ids, full_residues);
	}

	std::set<std::size_t> select_atoms_by_set_of_crads(const std::set<common::ChainResidueAtomDescriptor>& set_of_crads) const
	{
		std::set<std::size_t> result;
		for(std::size_t i=0;i<atoms().size();i++)
		{
			if(common::MatchingUtilities::match_crad_with_set_of_crads(atoms()[i].crad, set_of_crads))
			{
				result.insert(i);
			}
		}
		return result;
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

	std::set<std::size_t> select_contacts(const Query& query) const
	{
		if(contacts().empty())
		{
			return std::set<std::size_t>();
		}
		else
		{
			std::set<std::size_t> result;
			try
			{
				result=select_contacts(query.from_ids.empty(), query.from_ids, read_expression_from_string<TAC::TesterOfContact>(query.expression_string), false);
			}
			catch(const std::exception& e)
			{
				throw std::runtime_error(std::string("Failed to select contacts with expression '")+query.expression_string+"': "+e.what());
			}
			return (query.full_residues ? get_ids_for_full_residues(result, contacts_residues_definition_, contacts_residues_reference_) : result);
		}
	}

	std::set<std::size_t> select_contacts_by_atoms(const std::set<std::size_t>& from_ids, const std::set<std::size_t>& atom_ids, const bool full_residues) const
	{
		const std::set<std::size_t> result=select_contacts_by_atoms(from_ids.empty(), from_ids, atom_ids);
		return (full_residues ? get_ids_for_full_residues(result, contacts_residues_definition_, contacts_residues_reference_) : result);
	}

	std::set<std::size_t> select_contacts_by_atoms(const std::set<std::size_t>& atom_ids, const bool full_residues) const
	{
		return select_contacts_by_atoms(std::set<std::size_t>(), atom_ids, full_residues);
	}

	std::set<std::size_t> select_contacts_by_atoms_and_atoms(
			const std::set<std::size_t>& from_ids, const std::set<std::size_t>& atom_ids1, const std::set<std::size_t>& atom_ids2, const bool full_residues) const
	{
		const std::set<std::size_t> result=select_contacts_by_atoms_and_atoms(from_ids.empty(), from_ids, atom_ids1, atom_ids2);
		return (full_residues ? get_ids_for_full_residues(result, contacts_residues_definition_, contacts_residues_reference_) : result);
	}

	std::set<std::size_t> select_contacts_by_atoms_and_atoms(
			const std::set<std::size_t>& atom_ids1, const std::set<std::size_t>& atom_ids2, const bool full_residues) const
	{
		return select_contacts_by_atoms_and_atoms(std::set<std::size_t>(), atom_ids1, atom_ids2, full_residues);
	}

	std::set<std::size_t> select_contacts_by_set_of_crads_pairs(const std::set<common::ChainResidueAtomDescriptorsPair>& set_of_crads_pairs) const
	{
		std::set<std::size_t> result;
		for(std::size_t i=0;i<contacts().size();i++)
		{
			if(common::MatchingUtilities::match_crads_pair_with_set_of_crads_pairs(
					common::ConversionOfDescriptors::get_contact_descriptor(atoms(), contacts()[i]), set_of_crads_pairs))
			{
				result.insert(i);
			}
		}
		return result;
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

	std::vector<std::string> get_names_of_atoms_selections() const
	{
		return collect_names_from_map(map_of_atoms_selections_);
	}

	std::vector<std::string> get_names_of_contacts_selections() const
	{
		return collect_names_from_map(map_of_contacts_selections_);
	}

private:
	typedef TestingOfAtomsAndContacts TAC;

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
	static std::vector< TAC::TestingExpressionToken<Tester> > read_expression_from_string(const std::string& expression_string)
	{
		std::vector< TAC::TestingExpressionToken<Tester> > result;
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
					if(c=='[' || c==']' || c=='(' || c==')')
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
				TAC::TestingExpressionToken<Tester> token;
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

	template<class T>
	static std::vector<std::string> collect_names_from_map(const T& map)
	{
		std::vector<std::string> names;
		names.reserve(map.size());
		for(typename T::const_iterator it=map.begin();it!=map.end();++it)
		{
			names.push_back(it->first);
		}
		return names;
	}

	void fix_atom_tester(TAC::TesterOfAtom& tester) const
	{
		tester.atoms_ptr=&atoms();

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

	void fix_contact_tester(TAC::TesterOfContact& tester) const
	{
		tester.atoms_ptr=&atoms();
		tester.contacts_ptr=&contacts();

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

	std::set<std::size_t> select_atoms(const bool from_all, const std::set<std::size_t>& from_ids, const std::vector< TAC::TestingExpressionToken<TAC::TesterOfAtom> >& expression, const bool postfix) const
	{
		std::set<std::size_t> result;

		if(from_all || !from_ids.empty())
		{
			std::vector< TAC::TestingExpressionToken<TAC::TesterOfAtom> > postfix_expression=(postfix ? expression : TAC::convert_testing_expression_from_infix_to_postfix_form(expression));

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
					if(TAC::evaluate_testing_expression_in_postfix_form(postfix_expression, id))
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
					if(id<atoms().size() && TAC::evaluate_testing_expression_in_postfix_form(postfix_expression, id))
					{
						result.insert(id);
					}
				}
			}
		}

		return result;
	}

	std::set<std::size_t> select_contacts(const bool from_all, const std::set<std::size_t>& from_ids, const std::vector< TAC::TestingExpressionToken<TAC::TesterOfContact> >& expression, const bool postfix) const
	{
		std::set<std::size_t> result;

		if(from_all || !from_ids.empty())
		{
			std::vector< TAC::TestingExpressionToken<TAC::TesterOfContact> > postfix_expression=(postfix ? expression : TAC::convert_testing_expression_from_infix_to_postfix_form(expression));

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
					if(TAC::evaluate_testing_expression_in_postfix_form(postfix_expression, id))
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
					if(id<contacts().size() && TAC::evaluate_testing_expression_in_postfix_form(postfix_expression, id))
					{
						result.insert(id);
					}
				}
			}
		}

		return result;
	}

	std::set<std::size_t> select_atoms_by_contacts(const bool from_all, const std::set<std::size_t>& from_ids, const std::set<std::size_t>& contact_ids) const
	{
		std::set<std::size_t> result;
		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const std::size_t id=(*it);
			if(id<contacts().size())
			{
				const Contact& contact=contacts()[id];
				if(from_all || from_ids.count(contact.ids[0])>0)
				{
					result.insert(contact.ids[0]);
				}
				if(contact.ids[0]!=contact.ids[1] && (from_all || from_ids.count(contact.ids[1])>0))
				{
					result.insert(contact.ids[1]);
				}
			}
		}
		return result;
	}

	std::set<std::size_t> select_contacts_by_atoms(const bool from_all, const std::set<std::size_t>& from_ids, const std::set<std::size_t>& atom_ids) const
	{
		std::set<std::size_t> result;
		if(from_all)
		{
			for(std::size_t id=0;id<contacts().size();id++)
			{
				const Contact& contact=contacts()[id];
				if(atom_ids.count(contact.ids[0])>0 || (contact.ids[0]!=contact.ids[1] && atom_ids.count(contact.ids[1])>0))
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
				if(id<contacts().size())
				{
					const Contact& contact=contacts()[id];
					if(atom_ids.count(contact.ids[0])>0 || (contact.ids[0]!=contact.ids[1] && atom_ids.count(contact.ids[1])>0))
					{
						result.insert(id);
					}
				}
			}
		}
		return result;
	}

	std::set<std::size_t> select_contacts_by_atoms_and_atoms(
			const bool from_all, const std::set<std::size_t>& from_ids, const std::set<std::size_t>& atom_ids1, const std::set<std::size_t>& atom_ids2) const
	{
		std::set<std::size_t> result;
		if(from_all)
		{
			for(std::size_t id=0;id<contacts().size();id++)
			{
				const Contact& contact=contacts()[id];
				if(!contact.solvent()
						&& ((atom_ids1.count(contact.ids[0])>0 && atom_ids2.count(contact.ids[1])>0)
								|| (atom_ids2.count(contact.ids[0])>0 && atom_ids1.count(contact.ids[1])>0)))
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
				if(id<contacts().size())
				{
					const Contact& contact=contacts()[id];
					if(!contact.solvent()
							&& ((atom_ids1.count(contact.ids[0])>0 && atom_ids2.count(contact.ids[1])>0)
									|| (atom_ids2.count(contact.ids[0])>0 && atom_ids1.count(contact.ids[1])>0)))
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
			std::map<common::ChainResidueAtomDescriptor, std::vector<std::size_t> > map_of_residues;
			for(std::size_t i=0;i<atoms().size();i++)
			{
				map_of_residues[atoms()[i].crad.without_atom()].push_back(i);
			}

			atoms_residues_definition_.resize(map_of_residues.size());
			atoms_residues_reference_.resize(atoms().size(), 0);

			std::size_t residue_id=0;
			for(std::map<common::ChainResidueAtomDescriptor, std::vector<std::size_t> >::const_iterator it=map_of_residues.begin();it!=map_of_residues.end();++it)
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
			std::map<common::ChainResidueAtomDescriptorsPair, std::vector<std::size_t> > map_of_residues;
			for(std::size_t i=0;i<contacts().size();i++)
			{
				map_of_residues[common::ChainResidueAtomDescriptorsPair(
										atoms()[contacts()[i].ids[0]].crad.without_atom(),
										atoms()[contacts()[i].ids[1]].crad.without_atom())
								].push_back(i);
			}

			contacts_residues_definition_.resize(map_of_residues.size());
			contacts_residues_reference_.resize(contacts().size(), 0);

			std::size_t residue_id=0;
			for(std::map<common::ChainResidueAtomDescriptorsPair, std::vector<std::size_t> >::const_iterator it=map_of_residues.begin();it!=map_of_residues.end();++it)
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

#endif /* SCRIPTING_SELECTION_MANAGER_H_ */
