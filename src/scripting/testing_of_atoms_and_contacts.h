#ifndef SCRIPTING_TESTING_OF_ATOMS_AND_CONTACTS_H_
#define SCRIPTING_TESTING_OF_ATOMS_AND_CONTACTS_H_

#include <cmath>

#include "../common/construction_of_atomic_balls.h"
#include "../common/construction_of_contacts.h"
#include "../common/matching_utilities.h"

#include "basic_types.h"

namespace voronota
{

namespace scripting
{

class TestingOfAtomsAndContacts
{
public:
	class TesterOfID
	{
	public:
		std::set<std::size_t> allowed_ids;

		TesterOfID()
		{
		}

		virtual ~TesterOfID()
		{
		}

		virtual bool operator()(const std::size_t id) const
		{
			if(allowed_ids.empty() || allowed_ids.count(id)>0)
			{
				return true;
			}
			return false;
		}
	};

	class TesterOfAtom : public TesterOfID
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
		std::map<std::string, std::string> special_match_crad_map;
		std::map< std::string, std::vector<double> > geometric_constraints_map;
		std::set<std::string> additional_conditions_set;

		explicit TesterOfAtom(const std::vector<Atom>* atoms_ptr=0) :
			atoms_ptr(atoms_ptr)
		{
		}

		virtual ~TesterOfAtom()
		{
		}

		bool empty() const
		{
			return (allowed_ids.empty() &&
					name_of_base_selection_of_atoms.empty() &&
					name_of_base_selection_of_contacts.empty() &&
					match_crad.empty() &&
					match_crad_not.empty() &&
					match_tags.empty() &&
					match_tags_not.empty() &&
					match_adjuncts.empty() &&
					match_adjuncts_not.empty() &&
					special_match_crad_map.empty() &&
					geometric_constraints_map.empty() &&
					additional_conditions_set.empty());
		}

		bool operator()(const std::size_t id) const
		{
			if(atoms_ptr==0)
			{
				throw std::runtime_error(std::string("Atom test functor has no atoms list to refer to."));
			}
			else if(id<atoms_ptr->size())
			{
				return (TesterOfID::operator()(id) && (this->operator()((*atoms_ptr)[id])));
			}
			return false;
		}

		bool operator()(const std::vector<Atom>& atoms, const std::size_t id) const
		{
			if(id<atoms.size())
			{
				return (TesterOfID::operator()(id) && (this->operator()(atoms[id])));
			}
			return false;
		}

		bool operator()(const Atom& atom) const
		{
			if(
					common::MatchingUtilities::match_crad(atom.crad, match_crad, match_crad_not) &&
					check_crad_with_special_match_crad_map(atom.crad) &&
					check_ball_with_geometric_constraints_map(atom.value) &&
					check_crad_and_props_with_additional_conditions_set(atom.crad, atom.value.props) &&
					common::MatchingUtilities::match_set_of_tags(atom.value.props.tags, match_tags, match_tags_not) &&
					common::MatchingUtilities::match_map_of_adjuncts(atom.value.props.adjuncts, match_adjuncts, match_adjuncts_not)
			)
			{
				return true;
			}
			return false;
		}

		bool operator()(const common::ChainResidueAtomDescriptor& crad) const
		{
			const common::PropertiesValue props;
			if(
					common::MatchingUtilities::match_crad(crad, match_crad, match_crad_not) &&
					check_crad_with_special_match_crad_map(crad) &&
					check_crad_and_props_with_additional_conditions_set(crad, props) &&
					common::MatchingUtilities::match_set_of_tags(props.tags, match_tags, match_tags_not) &&
					common::MatchingUtilities::match_map_of_adjuncts(props.adjuncts, match_adjuncts, match_adjuncts_not)
			)
			{
				return true;
			}
			return false;
		}

	private:
		bool check_crad_with_special_match_crad_map(const common::ChainResidueAtomDescriptor& crad) const
		{
			if(!special_match_crad_map.empty())
			{
				for(std::map<std::string, std::string>::const_iterator it=special_match_crad_map.begin();it!=special_match_crad_map.end();++it)
				{
					if(!common::MatchingUtilities::match_crad(crad, it->second, ""))
					{
						return false;
					}
				}
			}
			return true;
		}

		bool check_ball_with_geometric_constraints_map(const common::BallValue& ball) const
		{
			if(!geometric_constraints_map.empty())
			{
				for(std::map< std::string, std::vector<double> >::const_iterator it=geometric_constraints_map.begin();it!=geometric_constraints_map.end();++it)
				{
					const std::string& name=it->first;
					const std::vector<double>& values=it->second;
					if(name=="xmin" && values.size()>=1 && ball.x<values[0])
					{
						return false;
					}
					else if(name=="xmax" && values.size()>=1 && ball.x>values[0])
					{
						return false;
					}
					else if(name=="ymin" && values.size()>=1 && ball.y<values[0])
					{
						return false;
					}
					else if(name=="ymax" && values.size()>=1 && ball.y>values[0])
					{
						return false;
					}
					else if(name=="zmin" && values.size()>=1 && ball.z<values[0])
					{
						return false;
					}
					else if(name=="zmax" && values.size()>=1 && ball.z>values[0])
					{
						return false;
					}
					else if(name=="dist" && values.size()>=4)
					{
						if(std::sqrt((ball.x-values[0])*(ball.x-values[0])+(ball.y-values[1])*(ball.y-values[1])+(ball.z-values[2])*(ball.z-values[2]))>values[3])
						{
							return false;
						}
					}
				}
			}
			return true;
		}

		bool check_crad_and_props_with_additional_conditions_set(const common::ChainResidueAtomDescriptor& crad, const common::PropertiesValue& props) const
		{
			if(!additional_conditions_set.empty())
			{
				for(std::set<std::string>::const_iterator it=additional_conditions_set.begin();it!=additional_conditions_set.end();++it)
				{
					const std::string& condition=(*it);
					if(condition=="hydrogen" || condition=="non-hydrogen")
					{
						const bool is_hydrogen=((!crad.name.empty() && crad.name[0]=='H') || props.tags.count("el=H")>0 || props.tags.count("el=D")>0);
						if((!is_hydrogen && condition=="hydrogen") || (is_hydrogen && condition=="non-hydrogen"))
						{
							return false;
						}
					}
					else
					{
						return false;
					}
				}
			}
			return true;
		}
	};

	class TesterOfContact : public TesterOfID
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
		bool solvent;
		bool no_same_chain;
		std::string match_tags;
		std::string match_tags_not;
		std::string match_adjuncts;
		std::string match_adjuncts_not;
		TesterOfAtom test_atom_a;
		TesterOfAtom test_atom_a_not;
		TesterOfAtom test_atom_b;
		TesterOfAtom test_atom_b_not;

		TesterOfContact(const std::vector<Atom>* atoms_ptr=0, const std::vector<Contact>* contacts_ptr=0) :
			atoms_ptr(atoms_ptr),
			contacts_ptr(contacts_ptr),
			match_min_area((-std::numeric_limits<double>::max())),
			match_max_area(std::numeric_limits<double>::max()),
			match_min_dist((-std::numeric_limits<double>::max())),
			match_max_dist(std::numeric_limits<double>::max()),
			match_min_sequence_separation(common::ChainResidueAtomDescriptor::null_num()),
			match_max_sequence_separation(common::ChainResidueAtomDescriptor::null_num()),
			no_solvent(false),
			solvent(false),
			no_same_chain(false)
		{
		}

		virtual ~TesterOfContact()
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
				return (TesterOfID::operator()(id) && (this->operator()((*contacts_ptr)[id])));
			}
			return false;
		}

		bool operator()(const std::vector<Contact>& contacts, const std::size_t id) const
		{
			if(id<contacts.size())
			{
				return (TesterOfID::operator()(id) && (this->operator()(contacts[id])));
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
					(!no_solvent || !contact.solvent()) &&
					(!solvent || contact.solvent())
			)
			{
				const Atom& atom_a=atoms[contact.ids[0]];
				const Atom& atom_b=atoms[contact.ids[1]];
				const common::ChainResidueAtomDescriptor& crad_a=atom_a.crad;
				const common::ChainResidueAtomDescriptor& crad_b=(contact.solvent() ? common::ChainResidueAtomDescriptor::solvent() : atom_b.crad);
				if(
						(!no_same_chain || crad_a.chainID!=crad_b.chainID) &&
						common::ChainResidueAtomDescriptor::match_with_sequence_separation_interval(crad_a, crad_b, match_min_sequence_separation, match_max_sequence_separation, true) &&
						common::MatchingUtilities::match_set_of_tags(contact.value.props.tags, match_tags, match_tags_not) &&
						common::MatchingUtilities::match_map_of_adjuncts(contact.value.props.adjuncts, match_adjuncts, match_adjuncts_not)
				)
				{
					if(contact.solvent())
					{
						if(
								((test_atom_b.allowed_ids.empty() && test_atom_b(crad_b)) &&
										(test_atom_b_not.empty() || !(test_atom_b_not.allowed_ids.empty() && test_atom_b_not(crad_b))) &&
										test_atom_a(atoms, contact.ids[0]) &&
										(test_atom_a_not.empty() || !test_atom_a_not(atoms, contact.ids[0]))) ||
								((test_atom_a.allowed_ids.empty() && test_atom_a(crad_b)) &&
										(test_atom_a_not.empty() || !(test_atom_a_not.allowed_ids.empty() && test_atom_a_not(crad_b))) &&
										test_atom_b(atoms, contact.ids[0]) &&
										(test_atom_b_not.empty() || !test_atom_b_not(atoms, contact.ids[0]))))
						{
							return true;
						}
					}
					else if(
							(test_atom_a(atoms, contact.ids[0]) &&
									(test_atom_a_not.empty() || !test_atom_a_not(atoms, contact.ids[0])) &&
									test_atom_b(atoms, contact.ids[1]) &&
									(test_atom_b_not.empty() || !test_atom_b_not(atoms, contact.ids[1]))) ||
							(test_atom_a(atoms, contact.ids[1]) &&
									(test_atom_a_not.empty() || !test_atom_a_not(atoms, contact.ids[1])) &&
									test_atom_b(atoms, contact.ids[0]) &&
									(test_atom_b_not.empty() || !test_atom_b_not(atoms, contact.ids[0]))))
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

inline bool read_tester_flag_value(std::istream& input)
{
	input >> std::ws;

	if(!input.good())
	{
		return true;
	}

	const char opener=std::char_traits<char>::to_char_type(input.peek());
	if(opener=='-' || opener==']')
	{
		return true;
	}
	else
	{
		std::string token;
		input >> token;
		if(token=="true" || token=="1")
		{
			return true;
		}
		else if(token=="false" || token=="0")
		{
			return false;
		}
		else
		{
			throw std::runtime_error(std::string("Invalid tester flag value '")+token+"'.");
		}
	}

	return false;
}

inline std::istream& operator>>(std::istream& input, TestingOfAtomsAndContacts::TesterOfAtom& tester)
{
	std::string token;
	input >> token;

	if(token!="[")
	{
		throw std::runtime_error(std::string("Invalid atom tester string start."));
	}
	else
	{
		bool end=false;
		int token_index=0;
		while(input.good() && !end)
		{
			input >> token;

			if(token=="]")
			{
				end=true;
			}
			else if(token=="--sel" || token=="--s")
			{
				input >> tester.name_of_base_selection_of_atoms;
			}
			else if(token=="--sel-of-contacts" || token=="--soc")
			{
				input >> tester.name_of_base_selection_of_contacts;
			}
			else if(token=="--match" || token=="--m")
			{
				input >> tester.match_crad;
			}
			else if(token=="--match-not" || token=="--m!")
			{
				input >> tester.match_crad_not;
			}
			else if(token=="--tags" || token=="--t")
			{
				input >> tester.match_tags;
			}
			else if(token=="--tags-not" || token=="--t!")
			{
				input >> tester.match_tags_not;
			}
			else if(token=="--adjuncts" || token=="--v")
			{
				input >> tester.match_adjuncts;
			}
			else if(token=="--adjuncts-not" || token=="--v!")
			{
				input >> tester.match_adjuncts_not;
			}
			else if(token=="--chain" || token=="--c")
			{
				std::string raw_value;
				input >> raw_value;
				tester.special_match_crad_map["chain"]=std::string("c<")+raw_value+">";
			}
			else if(token=="--residue-number" || token=="--rnum")
			{
				std::string raw_value;
				input >> raw_value;
				tester.special_match_crad_map["residue-number"]=std::string("r<")+raw_value+">";
			}
			else if(token=="--residue-name" || token=="--rname")
			{
				std::string raw_value;
				input >> raw_value;
				tester.special_match_crad_map["residue-name"]=std::string("R<")+raw_value+">";
			}
			else if(token=="--atom-number" || token=="--anum")
			{
				std::string raw_value;
				input >> raw_value;
				tester.special_match_crad_map["atom-number"]=std::string("a<")+raw_value+">";
			}
			else if(token=="--atom-name" || token=="--aname")
			{
				std::string raw_value;
				input >> raw_value;
				tester.special_match_crad_map["atom-name"]=std::string("A<")+raw_value+">";
			}
			else if(token=="--protein")
			{
				tester.special_match_crad_map["protein"]=std::string("R<LEU,ALA,GLY,VAL,GLU,SER,LYS,ILE,ASP,THR,ARG,PRO,ASN,PHE,GLN,TYR,HIS,MET,TRP,CYS,MSE,SEC>");
			}
			else if(token=="--nucleic")
			{
				tester.special_match_crad_map["nucleic"]=std::string("R<DA,DC,DG,DT,DI,A,C,G,U,I>");
			}
			else if(token=="--nucleic-dna")
			{
				tester.special_match_crad_map["nucleic-dna"]=std::string("R<DA,DC,DG,DT,DI>");
			}
			else if(token=="--nucleic-rna")
			{
				tester.special_match_crad_map["nucleic-rna"]=std::string("R<A,C,G,T,I>");
			}
			else if(token=="--xmin" || token=="--xmax" || token=="--ymin" || token=="--ymax" || token=="--zmin" || token=="--zmax")
			{
				std::vector<double>& gc_values=tester.geometric_constraints_map[token.substr(2)];
				gc_values.resize(1);
				input >> gc_values[0];
			}
			else if(token=="--dist")
			{
				std::vector<double>& gc_values=tester.geometric_constraints_map[token.substr(2)];
				gc_values.resize(4);
				input >> gc_values[0] >> gc_values[1] >> gc_values[2] >> gc_values[3];
			}
			else if(token=="--hydrogen")
			{
				tester.additional_conditions_set.insert("hydrogen");
			}
			else if(token=="--non-hydrogen")
			{
				tester.additional_conditions_set.insert("non-hydrogen");
			}
			else if(token_index==0
					&& token.rfind("-", 0)!=0
					&& token.find('<')!=std::string::npos
					&& token.find('>')!=std::string::npos)
			{
				tester.match_crad=token;
			}
			else if(token_index==0
					&& token.rfind("-", 0)!=0
					&& token.find('<')==std::string::npos
					&& token.find('>')==std::string::npos)
			{
				tester.name_of_base_selection_of_atoms=token;
			}
			else
			{
				throw std::runtime_error(std::string("Invalid token '")+token+"'.");
			}

			if(input.fail())
			{
				if(token.empty())
				{
					throw std::runtime_error(std::string("Invalid atom tester string."));
				}
				else
				{
					throw std::runtime_error(std::string("Invalid atom tester string at '")+token+"'.");
				}
			}

			token_index++;
			input >> std::ws;
		}
		if(!end)
		{
			throw std::runtime_error(std::string("Invalid atom tester string end."));
		}
	}

	return input;
}

inline std::istream& operator>>(std::istream& input, TestingOfAtomsAndContacts::TesterOfContact& tester)
{
	std::string token;
	input >> token;

	if(token!="[")
	{
		throw std::runtime_error(std::string("Invalid contact tester string start."));
	}
	else
	{
		bool end=false;
		int token_index=0;
		while(input.good() && !end)
		{
			input >> token;

			if(token=="]")
			{
				end=true;
			}
			else if(token=="--sel" || token=="--s")
			{
				input >> tester.name_of_base_selection_of_contacts;
			}
			else if(token=="--min-area" || token=="--mina")
			{
				input >> tester.match_min_area;
			}
			else if(token=="--max-area" || token=="--maxa")
			{
				input >> tester.match_max_area;
			}
			else if(token=="--min-dist" || token=="--mind")
			{
				input >> tester.match_min_dist;
			}
			else if(token=="--max-dist" || token=="--maxd")
			{
				input >> tester.match_max_dist;
			}
			else if(token=="--min-seq-sep" || token=="--minsep")
			{
				input >> tester.match_min_sequence_separation;
			}
			else if(token=="--max-seq-sep" || token=="--maxsep")
			{
				input >> tester.match_max_sequence_separation;
			}
			else if(token=="--tags" || token=="--t")
			{
				input >> tester.match_tags;
			}
			else if(token=="--tags-not" || token=="--t!")
			{
				input >> tester.match_tags_not;
			}
			else if(token=="--adjuncts" || token=="--v")
			{
				input >> tester.match_adjuncts;
			}
			else if(token=="--adjuncts-not" || token=="--v!")
			{
				input >> tester.match_adjuncts_not;
			}
			else if(token=="--atom1" || token=="--a1")
			{
				input >> tester.test_atom_a;
			}
			else if(token=="--atom1-not" || token=="--a1!")
			{
				input >> tester.test_atom_a_not;
			}
			else if(token=="--atom2" || token=="--a2")
			{
				input >> tester.test_atom_b;
			}
			else if(token=="--atom2-not" || token=="--a2!")
			{
				input >> tester.test_atom_b_not;
			}
			else if(token=="--no-solvent" || token=="--nosolv")
			{
				tester.no_solvent=read_tester_flag_value(input);
			}
			else if(token=="--solvent" || token=="--solv")
			{
				tester.solvent=read_tester_flag_value(input);
			}
			else if(token=="--no-same-chain" || token=="--nschain")
			{
				tester.no_same_chain=read_tester_flag_value(input);
			}
			else if(token=="--inter-chain" || token=="--ichain")
			{
				tester.no_same_chain=read_tester_flag_value(input);
				tester.no_solvent=tester.no_same_chain;
			}
			else if(token_index==0 && token.rfind("-", 0)!=0)
			{
				tester.name_of_base_selection_of_contacts=token;
			}
			else
			{
				throw std::runtime_error(std::string("Invalid token '")+token+"'.");
			}

			if(input.fail())
			{
				if(token.empty())
				{
					throw std::runtime_error(std::string("Invalid contact tester string."));
				}
				else
				{
					throw std::runtime_error(std::string("Invalid contact tester string at '")+token+"'.");
				}
			}

			token_index++;
			input >> std::ws;
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
	else if(c==std::char_traits<char>::to_int_type('['))
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
			if(opname.empty())
			{
				throw std::runtime_error(std::string("Invalid testing expression token."));
			}
			else
			{
				throw std::runtime_error(std::string("Invalid testing expression token '")+opname+"'.");
			}
		}
	}

	input >> std::ws;

	return input;
}

}

}

#endif /* SCRIPTING_TESTING_OF_ATOMS_AND_CONTACTS_H_ */
