#ifndef COMMON_TESTING_OF_ATOMS_AND_CONTACTS_H_
#define COMMON_TESTING_OF_ATOMS_AND_CONTACTS_H_

#include "construction_of_atomic_balls.h"
#include "construction_of_contacts.h"
#include "matching_utilities.h"

namespace common
{

class TestingOfAtomsAndContacts
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
};

inline std::istream& operator>>(std::istream& input, TestingOfAtomsAndContacts::test_atom& tester)
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
			else
			{
				token.clear();
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

inline std::istream& operator>>(std::istream& input, TestingOfAtomsAndContacts::test_contact& tester)
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
			else
			{
				token.clear();
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
inline std::istream& operator>>(std::istream& input, TestingOfAtomsAndContacts::TestingExpressionToken<Tester>& token)
{
	typedef TestingOfAtomsAndContacts::TestingExpressionToken<Tester> Token;

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

#endif /* COMMON_TESTING_OF_ATOMS_AND_CONTACTS_H_ */
