#include "auxiliaries/program_options_handler.h"

#include "common/filtering_of_atoms_and_contacts.h"

namespace
{

class mocking_test
{
public:
	std::string label;

	bool operator()(const bool val) const
	{
		return val;
	}
};

inline std::ostream& operator<<(std::ostream& output, const common::FilteringOfAtomsAndContacts::TestingExpressionToken<mocking_test>& token)
{
	typedef common::FilteringOfAtomsAndContacts::TestingExpressionToken<mocking_test> Token;
	if(token.type==Token::TYPE_TESTER)
	{
		if(token.tester.label.empty())
		{
			output << " . ";
		}
		else
		{
			output << " " << token.tester.label << " ";
		}
	}
	else if(token.type==Token::TYPE_OPERATOR_OR)
	{
		output << " | ";
	}
	else if(token.type==Token::TYPE_OPERATOR_AND)
	{
		output << " & ";
	}
	else if(token.type==Token::TYPE_OPERATOR_NOT)
	{
		output << " ! ";
	}
	else if(token.type==Token::TYPE_BRACKET_OPEN)
	{
		output << " ( ";
	}
	else if(token.type==Token::TYPE_BRACKET_CLOSE)
	{
		output << " ) ";
	}
	return output;
}

void demo_expression_conversion(const std::string& str)
{
	typedef common::FilteringOfAtomsAndContacts::TestingExpressionToken<mocking_test> Token;

	std::vector<Token> infix;

	for(std::size_t i=0;i<str.size();i++)
	{
		const char c=str[i];
		if(c=='(')
		{
			infix.push_back(Token(Token::TYPE_BRACKET_OPEN));
		}
		else if(c==')')
		{
			infix.push_back(Token(Token::TYPE_BRACKET_CLOSE));
		}
		else if(c=='|')
		{
			infix.push_back(Token(Token::TYPE_OPERATOR_OR));
		}
		else if(c=='&')
		{
			infix.push_back(Token(Token::TYPE_OPERATOR_AND));
		}
		else if(c=='!')
		{
			infix.push_back(Token(Token::TYPE_OPERATOR_NOT));
		}
		else if(c>' ')
		{
			Token token(Token::TYPE_TESTER);
			token.tester.label.push_back(c);
			infix.push_back(token);
		}
	}

	std::clog << "demo_expression_conversion '" << str << "'\n";

	for(std::size_t i=0;i<infix.size();i++)
	{
		std::clog << infix[i];
	}
	std::clog << std::endl;

	const std::vector<Token> postfix=common::FilteringOfAtomsAndContacts::convert_testing_expression_from_infix_to_postfix_form(infix);
	for(std::size_t i=0;i<postfix.size();i++)
	{
		std::clog << postfix[i];
	}
	std::clog << std::endl;
}

}

void demo_apilike_functionality(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "input file");
	pohw.describe_io("stdout", false, true, "demo output");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	demo_expression_conversion("A&(B|C)");
	demo_expression_conversion("((A)&(B|(C)))");
	demo_expression_conversion("!A|(B&C)|!D");
	demo_expression_conversion("(A&B)|(C&D)");

	common::ConstructionOfAtomicBalls::collect_atomic_balls_from_file collect_atomic_balls_from_file;

	std::vector<common::ConstructionOfAtomicBalls::AtomicBall> atomic_balls;

	if(collect_atomic_balls_from_file(std::cin, atomic_balls))
	{
		std::clog << atomic_balls.size() << " atoms" << std::endl;

		common::ConstructionOfContacts::construct_bundle_of_contact_information construct_bundle_of_contact_information;
		construct_bundle_of_contact_information.calculate_volumes=true;

		common::ConstructionOfContacts::BundleOfContactInformation bundle_of_contact_information;
		if(construct_bundle_of_contact_information(common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(atomic_balls), bundle_of_contact_information))
		{
			std::clog << bundle_of_contact_information.contacts.size() << " contacts\n";

			common::FilteringOfAtomsAndContacts::SelectionManager selection_manager(atomic_balls, bundle_of_contact_information.contacts);

			std::clog << selection_manager.select_atoms("{match c<A>}").size() << " atoms in chain A" << std::endl;
			std::clog << selection_manager.select_atoms("{match c<B>}").size() << " atoms in chain B" << std::endl;
			std::clog << selection_manager.select_contacts("{atom-first  {    match    c  <  A  >    }atom-second{match c<B>}}").size() << " contacts between A and B" << std::endl;
			selection_manager.set_atoms_selection("chainA", selection_manager.select_atoms("{match c<A>}"));
			selection_manager.set_atoms_selection("chainB", selection_manager.select_atoms("{match c<B>}"));
			std::clog << selection_manager.select_contacts("{atom-first {selection chainA} atom-second {selection chainB}}").size() << " contacts between A and B" << std::endl;
			std::clog << selection_manager.select_contacts("{min-seq-sep 1}").size() << " drawable contacts" << std::endl;
			std::clog << selection_manager.select_contacts("{max-seq-sep 0}").size() << " non-drawable contacts" << std::endl;
			std::clog << selection_manager.select_contacts("{atom-second {match c<solvent>}}").size() << " solvent contacts" << std::endl;
			std::clog << selection_manager.select_contacts("not {no-solvent}").size() << " solvent contacts" << std::endl;
		}
	}
}
