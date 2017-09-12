#ifndef COMMON_FILTERING_OF_ATOMIC_BALLS_AND_CONTACTS_H_
#define COMMON_FILTERING_OF_ATOMIC_BALLS_AND_CONTACTS_H_

#include "construction_of_atomic_balls.h"
#include "construction_of_contacts.h"
#include "matching_utilities.h"

namespace common
{

class FilteringOfAtomicBallsAndContacts
{
public:
	typedef ConstructionOfAtomicBalls::AtomicBall Atom;
	typedef ConstructionOfContacts::Contact Contact;

	class test_id
	{
	public:
		std::set<std::size_t> allowed_ids;
		std::string name_of_allowed_ids;

		virtual bool operator()(const std::size_t id) const
		{
			if(allowed_ids.empty() || allowed_ids.count(id)>0)
			{
				return true;
			}
			return false;
		}

		bool load_allowed_ids_by_name(const std::map< std::string, std::set<std::size_t> >& map_of_names_to_allowed_ids)
		{
			if(!name_of_allowed_ids.empty())
			{
				std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_names_to_allowed_ids.find(name_of_allowed_ids);
				if(it!=map_of_names_to_allowed_ids.end())
				{
					allowed_ids=it->second;
					return true;
				}
			}
			return false;
		}
	};

	class test_atom : public test_id
	{
	public:
		const std::vector<Atom>* atoms_ptr;
		std::string match_crad;
		std::string match_crad_not;
		std::string match_tags;
		std::string match_tags_not;
		std::string match_adjuncts;
		std::string match_adjuncts_not;

		test_atom(const std::vector<Atom>* atoms_ptr=0) :
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
	};

	class test_contact_between_atoms : public test_id
	{
	public:
		const std::vector<Atom>* atoms_ptr;
		const std::vector<Contact>* contacts_ptr;
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

		test_contact_between_atoms(const std::vector<Atom>* atoms_ptr=0, const std::vector<Contact>* contacts_ptr=0) :
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
					if(
							(test_atom_a(atom_a) && test_atom_b(atom_b)) ||
							(test_atom_a(atom_b) && test_atom_b(atom_a))
					)
					{
						return true;
					}
				}
			}
			return false;
		}
	};

	template<typename Tester>
	class test_id_using_expression
	{
	public:
		struct ExpressionToken
		{
			enum Type
			{
				TYPE_TESTER,
				TYPE_OPERATOR_OR,
				TYPE_OPERATOR_AND,
				TYPE_OPERATOR_NOT,
				TYPE_BRACKET_OPEN,
				TYPE_BRACKET_CLOSE,
			};

			Type type;
			Tester tester;

			ExpressionToken(const Type type) : type(type)
			{
			}

			ExpressionToken(const Tester& tester) : type(TYPE_TESTER), tester(tester)
			{
			}

			bool is_binary_operator() const
			{
				return (type==ExpressionToken::TYPE_OPERATOR_OR || type==ExpressionToken::TYPE_OPERATOR_AND);
			}

			bool is_unary_operator() const
			{
				return (type==ExpressionToken::TYPE_OPERATOR_NOT);
			}

			bool is_operator() const
			{
				return (is_binary_operator() || is_unary_operator());
			}

			bool is_bracket() const
			{
				return (type==ExpressionToken::TYPE_BRACKET_OPEN || type==ExpressionToken::TYPE_BRACKET_CLOSE);
			}
		};

		test_id_using_expression(const std::vector<ExpressionToken>& expression, const bool postfix)
		{
			if(postfix)
			{
				postfix_expression_=expression;
			}
			else
			{
				postfix_expression_=convert_infix_expression_to_postfix_expression(expression);
			}
		}

		bool operator()(const std::size_t id) const
		{
			if(!postfix_expression_.empty())
			{
				std::vector<bool> operands_stack;

				for(std::vector<ExpressionToken>::const_iterator it=postfix_expression_.begin();it!=postfix_expression_.end();++it)
				{
					const ExpressionToken& token=(*it);

					if(token.type==ExpressionToken::TYPE_TESTER)
					{
						operands_stack.push_back(token.tester(id));
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
							if(token.type==ExpressionToken::TYPE_OPERATOR_OR)
							{
								operands_stack.push_back(a || b);
							}
							else if(token.type==ExpressionToken::TYPE_OPERATOR_AND)
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
							if(token.type==ExpressionToken::TYPE_OPERATOR_NOT)
							{
								operands_stack.push_back(!a);
							}
							else
							{
								throw std::runtime_error(std::string("Binary operation not implemented."));
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

			return false;
		}

	private:
		static std::vector<ExpressionToken> convert_infix_expression_to_postfix_expression(const std::vector<ExpressionToken>& infix_expression)
		{
			std::vector<ExpressionToken> output;
			std::vector<ExpressionToken> operators_stack;

			for(std::vector<ExpressionToken>::const_iterator it=infix_expression.begin();it!=infix_expression.end();++it)
			{
				const ExpressionToken& token=(*it);

				if(token.type==ExpressionToken::TYPE_TESTER)
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
				else if(token.type==ExpressionToken::TYPE_BRACKET_OPEN)
				{
					operators_stack.push_back(token);
				}
				else if(token.type==ExpressionToken::TYPE_BRACKET_CLOSE)
				{
					while(!operators_stack.empty() && operators_stack.back().is_operator())
					{
						output.push_back(operators_stack.back());
						operators_stack.pop_back();
					}

					if(operators_stack.empty() || operators_stack.back().type!=ExpressionToken::TYPE_BRACKET_OPEN)
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

		std::vector<ExpressionToken> postfix_expression_;
	};

	class SelectionManager
	{
	public:
		SelectionManager(const std::vector<Atom>& atoms, const std::vector<Contact>& contacts) :
			atoms_ptr_(&atoms),
			contacts_ptr_(&contacts)
		{
		}

		const std::vector<Atom>& atoms() const
		{
			return (*atoms_ptr_);
		}

		const std::vector<Contact>& contacts() const
		{
			return (*contacts_ptr_);
		}

		std::set<std::size_t> get_atoms_selection(const std::string& name) const
		{
			return get_selection(name, map_of_atoms_selections_);
		}

		std::set<std::size_t> get_contacts_selection(const std::string& name) const
		{
			return get_selection(name, map_of_contacts_selections_);
		}

		bool set_atoms_selection(const std::string& name, const std::set<std::size_t>& ids)
		{
			return set_selection(name, ids, atoms().size(), map_of_atoms_selections_);
		}

		bool set_contacts_selection(const std::string& name, const std::set<std::size_t>& ids)
		{
			return set_selection(name, ids, contacts().size(), map_of_contacts_selections_);
		}

	private:
		static std::set<std::size_t> get_selection(const std::string& name, const std::map< std::string, std::set<std::size_t> >& map_of_selections) const
		{
			std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_selections.find(name);
			if(it!=map_of_selections.end())
			{
				return (it->second);
			}
			return std::set<std::size_t>();
		}

		static bool set_selection(const std::string& name, const std::set<std::size_t>& ids, const std::size_t id_limit, std::map< std::string, std::set<std::size_t> >& map_of_selections)
		{
			if(!name.empty() && ids.size()<=id_limit)
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					if((*it)>=id_limit)
					{
						return false;
					}
				}
				map_of_selections[name]=ids;
				return true;
			}
			return false;
		}

		const std::vector<Atom>* atoms_ptr_;
		const std::vector<Contact>* contacts_ptr_;
		std::map< std::string, std::set<std::size_t> > map_of_atoms_selections_;
		std::map< std::string, std::set<std::size_t> > map_of_contacts_selections_;
	};
};

}

#endif /* COMMON_FILTERING_OF_ATOMIC_BALLS_AND_CONTACTS_H_ */
