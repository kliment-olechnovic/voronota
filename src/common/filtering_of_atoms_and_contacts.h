#ifndef COMMON_FILTERING_OF_ATOMS_AND_CONTACTS_H_
#define COMMON_FILTERING_OF_ATOMS_AND_CONTACTS_H_

#include "construction_of_atomic_balls.h"
#include "construction_of_contacts.h"
#include "matching_utilities.h"

namespace common
{

class FilteringOfAtomsAndContacts
{
public:
	typedef ConstructionOfAtomicBalls::AtomicBall Atom;
	typedef ConstructionOfContacts::Contact Contact;

	class test_id
	{
	public:
		std::set<std::size_t> allowed_ids;

		virtual bool operator()(const std::size_t id) const
		{
			if(allowed_ids.empty() || allowed_ids.count(id)>0)
			{
				return true;
			}
			return false;
		}
	};

	class test_atom : public test_id
	{
	public:
		const std::vector<Atom>* atoms_ptr;
		std::string name_of_base_selection_of_atoms;
		std::string name_of_base_selection_of_contacts;
		std::string match_crad;
		std::string match_crad_not;
		std::string match_tags;
		std::string match_tags_not;
		std::string match_adjuncts;
		std::string match_adjuncts_not;

		explicit test_atom(const std::vector<Atom>* atoms_ptr=0) :
			atoms_ptr(atoms_ptr)
		{
		}

		bool operator()(const std::size_t id) const
		{
			if(atoms_ptr==0)
			{
				throw std::runtime_error(std::string("Atom test functor has no atoms list to refer to."));
			}
			else if(id<atoms_ptr->size())
			{
				return (test_id::operator()(id) && (this->operator()((*atoms_ptr)[id])));
			}
			return false;
		}

		bool operator()(const std::vector<Atom>& atoms, const std::size_t id) const
		{
			if(id<atoms.size())
			{
				return (test_id::operator()(id) && (this->operator()(atoms[id])));
			}
			return false;
		}

		bool operator()(const Atom& atom) const
		{
			if(
					MatchingUtilities::match_crad(atom.crad, match_crad, match_crad_not) &&
					MatchingUtilities::match_set_of_tags(atom.value.props.tags, match_tags, match_tags_not) &&
					MatchingUtilities::match_map_of_adjuncts(atom.value.props.adjuncts, match_adjuncts, match_adjuncts_not)
			)
			{
				return true;
			}
			return false;
		}

		bool operator()(const ChainResidueAtomDescriptor& crad) const
		{
			const PropertiesValue props;
			if(
					MatchingUtilities::match_crad(crad, match_crad, match_crad_not) &&
					MatchingUtilities::match_set_of_tags(props.tags, match_tags, match_tags_not) &&
					MatchingUtilities::match_map_of_adjuncts(props.adjuncts, match_adjuncts, match_adjuncts_not)
			)
			{
				return true;
			}
			return false;
		}
	};

	class test_contact : public test_id
	{
	public:
		const std::vector<Atom>* atoms_ptr;
		const std::vector<Contact>* contacts_ptr;
		std::string name_of_base_selection_of_contacts;
		double match_min_area;
		double match_max_area;
		double match_min_dist;
		double match_max_dist;
		int match_min_sequence_separation;
		int match_max_sequence_separation;
		bool no_solvent;
		bool no_same_chain;
		std::string match_tags;
		std::string match_tags_not;
		std::string match_adjuncts;
		std::string match_adjuncts_not;
		test_atom test_atom_a;
		test_atom test_atom_b;

		test_contact(const std::vector<Atom>* atoms_ptr=0, const std::vector<Contact>* contacts_ptr=0) :
			atoms_ptr(atoms_ptr),
			contacts_ptr(contacts_ptr),
			match_min_area(std::numeric_limits<double>::min()),
			match_max_area(std::numeric_limits<double>::max()),
			match_min_dist(std::numeric_limits<double>::min()),
			match_max_dist(std::numeric_limits<double>::max()),
			match_min_sequence_separation(ChainResidueAtomDescriptor::null_num()),
			match_max_sequence_separation(ChainResidueAtomDescriptor::null_num()),
			no_solvent(false),
			no_same_chain(false)
		{
		}

		bool operator()(const std::size_t id) const
		{
			if(contacts_ptr==0)
			{
				throw std::runtime_error(std::string("Contact test functor has no contacts list to refer to."));
			}
			else if(id<contacts_ptr->size())
			{
				return (test_id::operator()(id) && (this->operator()((*contacts_ptr)[id])));
			}
			return false;
		}

		bool operator()(const std::vector<Contact>& contacts, const std::size_t id) const
		{
			if(id<contacts.size())
			{
				return (test_id::operator()(id) && (this->operator()(contacts[id])));
			}
			return false;
		}

		bool operator()(const Contact& contact) const
		{
			if(atoms_ptr==0)
			{
				throw std::runtime_error(std::string("Contact test functor has no atoms list to refer to."));
			}
			else
			{
				return (this->operator()(*atoms_ptr, contact));
			}
			return false;
		}

		bool operator()(const std::vector<Atom>& atoms, const Contact& contact) const
		{
			if(
					contact.ids[0]<atoms.size() &&
					contact.ids[1]<atoms.size() &&
					contact.value.area>=match_min_area &&
					contact.value.area<=match_max_area &&
					contact.value.dist>=match_min_dist &&
					contact.value.dist<=match_max_dist &&
					(!no_solvent || !contact.solvent())
			)
			{
				const Atom& atom_a=atoms[contact.ids[0]];
				const Atom& atom_b=atoms[contact.ids[1]];
				const ChainResidueAtomDescriptor& crad_a=atom_a.crad;
				const ChainResidueAtomDescriptor& crad_b=(contact.solvent() ? ChainResidueAtomDescriptor::solvent() : atom_b.crad);
				if(
						(!no_same_chain || crad_a.chainID!=crad_b.chainID) &&
						ChainResidueAtomDescriptor::match_with_sequence_separation_interval(crad_a, crad_b, match_min_sequence_separation, match_max_sequence_separation, true) &&
						MatchingUtilities::match_set_of_tags(contact.value.props.tags, match_tags, match_tags_not) &&
						MatchingUtilities::match_map_of_adjuncts(contact.value.props.adjuncts, match_adjuncts, match_adjuncts_not)
				)
				{
					if(contact.solvent())
					{
						if((test_atom_b.allowed_ids.empty() && test_atom_b(crad_b) && test_atom_a(atoms, contact.ids[0])) || (test_atom_a.allowed_ids.empty() && test_atom_a(crad_b) && test_atom_b(atoms, contact.ids[0])))
						{
							return true;
						}
					}
					else if((test_atom_a(atoms, contact.ids[0]) && test_atom_b(atoms, contact.ids[1])) || (test_atom_a(atoms, contact.ids[1]) && test_atom_b(atoms, contact.ids[0])))
					{
						return true;
					}
				}
			}
			return false;
		}
	};

	template<typename Tester>
	struct TestingExpressionToken
	{
		enum Type
		{
			TYPE_TESTER,
			TYPE_OPERATOR_OR,
			TYPE_OPERATOR_AND,
			TYPE_OPERATOR_NOT,
			TYPE_BRACKET_OPEN,
			TYPE_BRACKET_CLOSE
		};

		Type type;
		Tester tester;

		TestingExpressionToken() : type(TYPE_TESTER)
		{
		}

		explicit TestingExpressionToken(const Type type) : type(type)
		{
		}

		explicit TestingExpressionToken(const Tester& tester) : type(TYPE_TESTER), tester(tester)
		{
		}

		bool is_tester() const
		{
			return (type==TYPE_TESTER);
		}

		bool is_binary_operator() const
		{
			return (type==TYPE_OPERATOR_OR || type==TYPE_OPERATOR_AND);
		}

		bool is_unary_operator() const
		{
			return (type==TYPE_OPERATOR_NOT);
		}

		bool is_operator() const
		{
			return (is_binary_operator() || is_unary_operator());
		}

		bool is_bracket() const
		{
			return (type==TYPE_BRACKET_OPEN || type==TYPE_BRACKET_CLOSE);
		}
	};

	template<typename Tester, typename Value>
	static bool evaluate_testing_expression_in_postfix_form(const std::vector< TestingExpressionToken<Tester> >& postfix_expression, const Value& val)
	{
		typedef TestingExpressionToken<Tester> Token;

		if(!postfix_expression.empty())
		{
			std::vector<bool> operands_stack;

			for(typename std::vector<Token>::const_iterator it=postfix_expression.begin();it!=postfix_expression.end();++it)
			{
				const Token& token=(*it);

				if(token.is_tester())
				{
					operands_stack.push_back(token.tester(val));
				}
				else if(token.is_binary_operator())
				{
					if(operands_stack.size()<2)
					{
						throw std::runtime_error(std::string("Invalid binary operation in the expression."));
					}
					else
					{
						const bool a=operands_stack.back();
						operands_stack.pop_back();
						const bool b=operands_stack.back();
						operands_stack.pop_back();
						if(token.type==Token::TYPE_OPERATOR_OR)
						{
							operands_stack.push_back(a || b);
						}
						else if(token.type==Token::TYPE_OPERATOR_AND)
						{
							operands_stack.push_back(a && b);
						}
						else
						{
							throw std::runtime_error(std::string("Binary operation not implemented."));
						}
					}
				}
				else if(token.is_unary_operator())
				{
					if(operands_stack.empty())
					{
						throw std::runtime_error(std::string("Invalid unary operation in the expression."));
					}
					else
					{
						const bool a=operands_stack.back();
						operands_stack.pop_back();
						if(token.type==Token::TYPE_OPERATOR_NOT)
						{
							operands_stack.push_back(!a);
						}
						else
						{
							throw std::runtime_error(std::string("Unary operation not implemented."));
						}
					}
				}
				else
				{
					throw std::runtime_error(std::string("Invalid token in the postfix expression."));
				}
			}

			if(operands_stack.size()!=1)
			{
				throw std::runtime_error(std::string("Invalid expression."));
			}

			return operands_stack.back();
		}

		return Token().tester(val);
	}

	template<typename Tester>
	static std::vector< TestingExpressionToken<Tester> > convert_testing_expression_from_infix_to_postfix_form(const std::vector< TestingExpressionToken<Tester> >& infix_expression)
	{
		typedef TestingExpressionToken<Tester> Token;

		std::vector<Token> output;
		std::vector<Token> operators_stack;

		for(typename std::vector<Token>::const_iterator it=infix_expression.begin();it!=infix_expression.end();++it)
		{
			const Token& token=(*it);

			if(token.is_tester())
			{
				output.push_back(token);
			}
			else if(token.is_operator())
			{
				if(token.is_binary_operator())
				{
					while(!operators_stack.empty() && operators_stack.back().is_operator())
					{
						output.push_back(operators_stack.back());
						operators_stack.pop_back();
					}
				}
				operators_stack.push_back(token);
			}
			else if(token.type==Token::TYPE_BRACKET_OPEN)
			{
				operators_stack.push_back(token);
			}
			else if(token.type==Token::TYPE_BRACKET_CLOSE)
			{
				while(!operators_stack.empty() && operators_stack.back().is_operator())
				{
					output.push_back(operators_stack.back());
					operators_stack.pop_back();
				}

				if(operators_stack.empty() || operators_stack.back().type!=Token::TYPE_BRACKET_OPEN)
				{
					throw std::runtime_error(std::string("Mismatched parenthesis in the infix expression."));
				}
				else
				{
					operators_stack.pop_back();
				}
			}
		}

		while(!operators_stack.empty() && operators_stack.back().is_operator())
		{
			output.push_back(operators_stack.back());
			operators_stack.pop_back();
		}

		if(!operators_stack.empty())
		{
			throw std::runtime_error(std::string("Mismatched parenthesis in the infix expression."));
		}

		return output;
	}

	class SelectionManager
	{
	public:
		SelectionManager(const std::vector<Atom>* atoms_ptr, const std::vector<Contact>* contacts_ptr=0) :
			atoms_ptr_(atoms_ptr),
			contacts_ptr_(0)
		{
			construct_atoms_residues_definition_and_reference();
			set_contacts(contacts_ptr);
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

		void set_contacts(const std::vector<Contact>* contacts_ptr)
		{
			if(contacts_ptr==0 || check_contacts_according_to_atoms(atoms(), *contacts_ptr))
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

		std::set<std::size_t> select_atoms(const std::string& expression_string, const bool full_residues=false) const
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
					result=select_atoms(true, std::set<std::size_t>(), read_expression_from_string<test_atom>(expression_string), false);
				}
				catch(const std::exception& e)
				{
					throw std::runtime_error(std::string("Failed to select atoms with expression '")+expression_string+"': "+e.what());
				}
				return (full_residues ? get_ids_for_full_residues(result, atoms_residues_definition_, atoms_residues_reference_) : result);
			}
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

		std::set<std::size_t> select_contacts(const std::string& expression_string, const bool full_residues=false) const
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
					result=select_contacts(true, std::set<std::size_t>(), read_expression_from_string<test_contact>(expression_string), false);
				}
				catch(const std::exception& e)
				{
					throw std::runtime_error(std::string("Failed to select contacts with expression '")+expression_string+"': "+e.what());
				}
				return (full_residues ? get_ids_for_full_residues(result, contacts_residues_definition_, contacts_residues_reference_) : result);
			}
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

	private:
		static bool check_contacts_according_to_atoms(const std::vector<Atom>& atoms, const std::vector<Contact>& contacts)
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
			fix_atom_tester(tester.test_atom_b);
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
};

inline std::istream& operator>>(std::istream& input, FilteringOfAtomsAndContacts::test_atom& tester)
{
	std::string token;
	input >> token;

	if(token!="{")
	{
		throw std::runtime_error(std::string("Invalid atom tester string start."));
	}
	else
	{
		bool end=false;
		while(input.good() && !end)
		{
			std::string token;
			input >> token;

			if(token=="}")
			{
				end=true;
			}
			else if(token=="selection")
			{
				input >> tester.name_of_base_selection_of_atoms;
			}
			else if(token=="selection-of-contacts")
			{
				input >> tester.name_of_base_selection_of_contacts;
			}
			else if(token=="match")
			{
				input >> tester.match_crad;
			}
			else if(token=="match-not")
			{
				input >> tester.match_crad_not;
			}
			else if(token=="tags")
			{
				input >> tester.match_tags;
			}
			else if(token=="tags-not")
			{
				input >> tester.match_tags_not;
			}
			else if(token=="adjuncts")
			{
				input >> tester.match_adjuncts;
			}
			else if(token=="adjuncts-not")
			{
				input >> tester.match_adjuncts_not;
			}

			if(input.fail() || token.empty())
			{
				throw std::runtime_error(std::string("Invalid atom tester string."));
			}
		}
		if(!end)
		{
			throw std::runtime_error(std::string("Invalid atom tester string end."));
		}
	}

	input >> std::ws;

	return input;
}

inline std::istream& operator>>(std::istream& input, FilteringOfAtomsAndContacts::test_contact& tester)
{
	std::string token;
	input >> token;

	if(token!="{")
	{
		throw std::runtime_error(std::string("Invalid contact tester string start."));
	}
	else
	{
		bool end=false;
		while(input.good() && !end)
		{
			std::string token;
			input >> token;

			if(token=="}")
			{
				end=true;
			}
			else if(token=="selection")
			{
				input >> tester.name_of_base_selection_of_contacts;
			}
			else if(token=="min-area")
			{
				input >> tester.match_min_area;
			}
			else if(token=="max-area")
			{
				input >> tester.match_max_area;
			}
			else if(token=="min-dist")
			{
				input >> tester.match_min_dist;
			}
			else if(token=="max-dist")
			{
				input >> tester.match_max_dist;
			}
			else if(token=="min-seq-sep")
			{
				input >> tester.match_min_sequence_separation;
			}
			else if(token=="max-seq-sep")
			{
				input >> tester.match_max_sequence_separation;
			}
			else if(token=="tags")
			{
				input >> tester.match_tags;
			}
			else if(token=="tags-not")
			{
				input >> tester.match_tags_not;
			}
			else if(token=="adjuncts")
			{
				input >> tester.match_adjuncts;
			}
			else if(token=="adjuncts-not")
			{
				input >> tester.match_adjuncts_not;
			}
			else if(token=="atom-first")
			{
				input >> tester.test_atom_a;
			}
			else if(token=="atom-second")
			{
				input >> tester.test_atom_b;
			}
			else if(token=="no-solvent")
			{
				tester.no_solvent=true;
			}
			else if(token=="no-same-chain")
			{
				tester.no_same_chain=true;
			}

			if(input.fail() || token.empty())
			{
				throw std::runtime_error(std::string("Invalid contact tester string."));
			}
		}
		if(!end)
		{
			throw std::runtime_error(std::string("Invalid contact tester string end."));
		}
	}

	input >> std::ws;

	return input;
}

template<typename Tester>
inline std::istream& operator>>(std::istream& input, FilteringOfAtomsAndContacts::TestingExpressionToken<Tester>& token)
{
	typedef FilteringOfAtomsAndContacts::TestingExpressionToken<Tester> Token;

	input >> std::ws;

	const int c=input.peek();

	if(c==std::char_traits<char>::eof())
	{
		throw std::runtime_error(std::string("Nothing to read for expression token."));
	}
	else if(c==std::char_traits<char>::to_int_type('{'))
	{
		input >> token.tester;
		token.type=Token::TYPE_TESTER;
	}
	else if(c==std::char_traits<char>::to_int_type('('))
	{
		input.get();
		token.type=Token::TYPE_BRACKET_OPEN;
	}
	else if(c==std::char_traits<char>::to_int_type(')'))
	{
		input.get();
		token.type=Token::TYPE_BRACKET_CLOSE;
	}
	else
	{
		std::string opname;
		input >> opname;

		if(opname=="or")
		{
			input.get();
			token.type=Token::TYPE_OPERATOR_OR;
		}
		else if(opname=="and")
		{
			input.get();
			token.type=Token::TYPE_OPERATOR_AND;
		}
		else if(opname=="not")
		{
			input.get();
			token.type=Token::TYPE_OPERATOR_NOT;
		}
		else
		{
			throw std::runtime_error(std::string("Invalid testing expression token."));
		}
	}

	input >> std::ws;

	return input;
}

}

#endif /* COMMON_FILTERING_OF_ATOMS_AND_CONTACTS_H_ */
