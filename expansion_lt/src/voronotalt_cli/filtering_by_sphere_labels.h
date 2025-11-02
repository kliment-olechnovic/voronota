#ifndef VORONOTALT_CLI_FILTERING_BY_SPHERE_LABELS_H_
#define VORONOTALT_CLI_FILTERING_BY_SPHERE_LABELS_H_

#include <string>
#include <vector>
#include <utility>
#include <set>
#include <sstream>
#include <limits>
#include <cstdlib>

#include "sphere_labeling.h"

namespace voronotalt
{

class FilteringBySphereLabels
{
public:
	struct ExpressionResult
	{
		bool expression_valid;
		bool expression_matched;

		ExpressionResult() noexcept : expression_valid(false), expression_matched(false)
		{
		}

		ExpressionResult(const bool expression_matched) noexcept : expression_valid(true), expression_matched(expression_matched)
		{
		}
	};

	struct VectorExpressionResult
	{
		bool expression_valid;
		bool expression_matched_all;
		std::vector<std::size_t> expression_matched_ids;

		VectorExpressionResult() noexcept : expression_valid(false), expression_matched_all(false)
		{
		}

		bool expression_matched() const noexcept
		{
			return (expression_valid && (expression_matched_all || !expression_matched_ids.empty()));
		}
	};

private:
	class FilterSingle
	{
	public:
		std::set<std::string> chains_yes;
		std::set<std::string> chains_no;
		std::set<std::string> rid_yes;
		std::set<std::string> rid_no;
		std::vector< std::pair<int, int> > rnum_yes;
		std::vector< std::pair<int, int> > rnum_no;
		std::set<std::string> rname_yes;
		std::set<std::string> rname_no;
		std::set<std::string> aname_yes;
		std::set<std::string> aname_no;

		bool empty() const noexcept
		{
			return (chains_yes.empty() && chains_no.empty() && rid_yes.empty() && rid_no.empty() && rnum_yes.empty()&& rnum_no.empty() && rname_yes.empty() && rname_no.empty() && aname_yes.empty() && aname_no.empty());
		}

		bool match(const SphereLabeling::SphereLabel& v) const noexcept
		{
			if(!((chains_yes.empty() || chains_yes.count(v.chain_id)>0) && (chains_no.empty() || chains_no.count(v.chain_id)==0)))
			{
				return false;
			}
			else if(!((aname_yes.empty() || aname_yes.count(v.atom_name)>0) && (aname_no.empty() || aname_no.count(v.atom_name)==0)))
			{
				return false;
			}
			if(!((rid_yes.empty() || rid_yes.count(v.residue_id)>0) && (rid_no.empty() || rid_no.count(v.residue_id)==0)))
			{
				return false;
			}
			else if(!(rname_yes.empty() && rname_no.empty() && rnum_yes.empty() && rnum_no.empty()))
			{
				if(!v.expanded_residue_id.valid)
				{
					return false;
				}
				else if(!v.expanded_residue_id.rname.empty() && v.expanded_residue_id.rname!="." && !((rname_yes.empty() || rname_yes.count(v.expanded_residue_id.rname)>0) && (rname_no.empty() || rname_no.count(v.expanded_residue_id.rname)==0)))
				{
					return false;
				}
				else if(!(rnum_yes.empty() || any_interval_matching(v.expanded_residue_id.rnum, rnum_yes)))
				{
					return false;
				}
				else if(!(rnum_no.empty() || !any_interval_matching(v.expanded_residue_id.rnum, rnum_no)))
				{
					return false;
				}
			}
			return true;
		}

		bool not_empty_and_match(const SphereLabeling::SphereLabel& v) const noexcept
		{
			return (!empty() && match(v));
		}

	private:
		static bool any_interval_matching(const int value, const std::vector< std::pair<int, int> >& intervals) noexcept
		{
			for(std::size_t i=0;i<intervals.size();i++)
			{
				if(value>=intervals[i].first && value<=intervals[i].second)
				{
					return true;
				}
			}
			return false;
		}
	};

	class PairOfSphereLabels
	{
	public:
		PairOfSphereLabels(const SphereLabeling::SphereLabel& a, const SphereLabeling::SphereLabel& b) noexcept : a_ptr_(&a), b_ptr_(&b)
		{
		}

		const SphereLabeling::SphereLabel& a() const noexcept
		{
			return (*a_ptr_);
		}

		const SphereLabeling::SphereLabel& b() const noexcept
		{
			return (*b_ptr_);
		}

	private:
		const SphereLabeling::SphereLabel* a_ptr_;
		const SphereLabeling::SphereLabel* b_ptr_;
	};

	class FilterPair
	{
	public:
		FilterSingle filter1_yes;
		FilterSingle filter1_no;
		FilterSingle filter2_yes;
		FilterSingle filter2_no;

		bool match(const PairOfSphereLabels& p) const noexcept
		{
			return ((filter1_yes.match(p.a()) && !filter1_no.not_empty_and_match(p.a()) && filter2_yes.match(p.b()) && !filter2_no.not_empty_and_match(p.b()))
					|| (filter1_yes.match(p.b()) && !filter1_no.not_empty_and_match(p.b()) && filter2_yes.match(p.a()) && !filter2_no.not_empty_and_match(p.a())));
		}
	};

	template<typename Filter>
	class FilteringExpressionToken
	{
	public:
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
		Filter tester;

		FilteringExpressionToken() noexcept : type(TYPE_TESTER)
		{
		}

		explicit FilteringExpressionToken(const Type type) noexcept : type(type)
		{
		}

		explicit FilteringExpressionToken(const Filter& tester) noexcept : type(TYPE_TESTER), tester(tester)
		{
		}

		bool is_tester() const noexcept
		{
			return (type==TYPE_TESTER);
		}

		bool is_binary_operator() const noexcept
		{
			return (type==TYPE_OPERATOR_OR || type==TYPE_OPERATOR_AND);
		}

		bool is_unary_operator() const noexcept
		{
			return (type==TYPE_OPERATOR_NOT);
		}

		bool is_operator() const noexcept
		{
			return (is_binary_operator() || is_unary_operator());
		}

		bool is_bracket() const noexcept
		{
			return (type==TYPE_BRACKET_OPEN || type==TYPE_BRACKET_CLOSE);
		}
	};

	static bool read_bundle_of_strings(std::istream& input, std::set<std::string>& output_strings) noexcept
	{
		std::string input_str;
		input >> input_str;
		if(input_str.empty())
		{
			return false;
		}
		std::set<std::string> strings;
		std::string str;
		for(std::size_t i=0;i<input_str.size()+1;i++)
		{
			const char c=(i<input_str.size() ? input_str[i] : ';');
			if(c==',' || c==';')
			{
				if(!str.empty())
				{
					strings.insert(str);
					str.clear();
				}
			}
			else
			{
				str.push_back(c);
			}
		}
		output_strings.swap(strings);
		return (!output_strings.empty());
	}

	static bool read_bundle_of_intervals(std::istream& input, std::vector< std::pair<int, int> >& output_intervals) noexcept
	{
		std::string input_str;
		input >> input_str;
		if(input_str.empty())
		{
			return false;
		}
		std::vector< std::pair<int, int> > intervals;
		std::string a_str;
		std::string b_str;
		int mode=1;
		for(std::size_t i=0;i<input_str.size()+1;i++)
		{
			const char c=(i<input_str.size() ? input_str[i] : ';');
			if(c==',' || c==';')
			{
				if(mode==1 && !a_str.empty())
				{
					const int a_val=std::atoi(a_str.c_str());
					if(a_val==0 && a_str!="0")
					{
						return false;
					}
					intervals.push_back(std::pair<int, int>(a_val, a_val));
					a_str.clear();
					b_str.clear();
				}
				else if(mode==2)
				{
					if(a_str.empty())
					{
						return false;
					}
					const int a_val=std::atoi(a_str.c_str());
					if(a_val==0 && a_str!="0")
					{
						return false;
					}
					if(b_str.empty())
					{
						intervals.push_back(std::pair<int, int>(a_val, std::numeric_limits<int>::max()));
						a_str.clear();
						b_str.clear();
					}
					else
					{
						const int b_val=std::atoi(b_str.c_str());
						if((b_val==0 && b_str!="0") || b_val<a_val)
						{
							return false;
						}
						intervals.push_back(std::pair<int, int>(a_val, b_val));
						a_str.clear();
						b_str.clear();
					}
				}
				mode=1;
			}
			else if(c==':')
			{
				if(mode==1 && !a_str.empty())
				{
					mode=2;
				}
				else
				{
					return false;
				}
			}
			else if(c>='0' && c<='9')
			{
				if(mode==1 && a_str!="0" && !(c=='0' && a_str=="-"))
				{
					a_str.push_back(c);
				}
				else if(mode==2 && b_str!="0" && !(c=='0' && b_str=="-"))
				{
					b_str.push_back(c);
				}
				else
				{
					return false;
				}
			}
			else if(c=='-')
			{
				if(mode==1 && a_str.empty())
				{
					a_str.push_back(c);
				}
				else if(mode==2 && b_str.empty())
				{
					b_str.push_back(c);
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		output_intervals.swap(intervals);
		return (!output_intervals.empty());
	}

	static bool read_filter(std::istream& input, FilterSingle& tester) noexcept
	{
		std::string token;
		input >> token;

		if(token!="[")
		{
			return false;
		}
		else
		{
			bool end=false;
			while(input.good() && !end)
			{
				input >> token;

				if(token=="]")
				{
					end=true;
				}
				else if(token=="-chain" || token=="-c")
				{
					if(!read_bundle_of_strings(input, tester.chains_yes))
					{
						return false;
					}
				}
				else if(token=="-chain-not" || token=="-c!")
				{
					if(!read_bundle_of_strings(input, tester.chains_no))
					{
						return false;
					}
				}
				else if(token=="-residue-id" || token=="-rid")
				{
					if(!read_bundle_of_strings(input, tester.rid_yes))
					{
						return false;
					}
				}
				else if(token=="-residue-id-not" || token=="-rid!")
				{
					if(!read_bundle_of_strings(input, tester.rid_no))
					{
						return false;
					}
				}
				else if(token=="-residue-number" || token=="-rnum")
				{
					if(!read_bundle_of_intervals(input, tester.rnum_yes))
					{
						return false;
					}
				}
				else if(token=="-residue-number-not" || token=="-rnum!")
				{
					if(!read_bundle_of_intervals(input, tester.rnum_no))
					{
						return false;
					}
				}
				else if(token=="-residue-name" || token=="-rname")
				{
					if(!read_bundle_of_strings(input, tester.rname_yes))
					{
						return false;
					}
				}
				else if(token=="-residue-name-not" || token=="-rname!")
				{
					if(!read_bundle_of_strings(input, tester.rname_no))
					{
						return false;
					}
				}
				else if(token=="-atom-name" || token=="-aname")
				{
					if(!read_bundle_of_strings(input, tester.aname_yes))
					{
						return false;
					}
				}
				else if(token=="-atom-name-not" || token=="-aname!")
				{
					if(!read_bundle_of_strings(input, tester.aname_no))
					{
						return false;
					}
				}
				else
				{
					return false;
				}

				input >> std::ws;
			}
			if(!end)
			{
				return false;
			}
		}

		return true;
	}

	static bool read_filter(std::istream& input, FilterPair& tester) noexcept
	{
		std::string token;
		input >> token;

		if(token!="[")
		{
			return false;
		}
		else
		{
			bool end=false;
			while(input.good() && !end)
			{
				input >> token;

				if(token=="]")
				{
					end=true;
				}
				else if(token=="-atom1" || token=="-a1")
				{
					if(!read_filter(input, tester.filter1_yes))
					{
						return false;
					}
				}
				else if(token=="-atom1-not" || token=="-a1!")
				{
					if(!read_filter(input, tester.filter1_no))
					{
						return false;
					}
				}
				else if(token=="-atom2" || token=="-a2")
				{
					if(!read_filter(input, tester.filter2_yes))
					{
						return false;
					}
				}
				else if(token=="-atom2-not" || token=="-a2!")
				{
					if(!read_filter(input, tester.filter2_no))
					{
						return false;
					}
				}
				else
				{
					return false;
				}

				input >> std::ws;
			}
			if(!end)
			{
				return false;
			}
		}

		return true;
	}

	template<typename Tester>
	static bool read_expression_token(std::istream& input, FilteringExpressionToken<Tester>& token) noexcept
	{
		typedef FilteringExpressionToken<Tester> Token;

		input >> std::ws;

		const int c=input.peek();

		if(c==std::char_traits<char>::eof())
		{
			return false;
		}
		else if(c==std::char_traits<char>::to_int_type('['))
		{
			if(!read_filter(input, token.tester))
			{
				return false;
			}
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

			if(opname=="or" || opname=="|" || opname=="||")
			{
				input.get();
				token.type=Token::TYPE_OPERATOR_OR;
			}
			else if(opname=="and" || opname=="&" || opname=="&&")
			{
				input.get();
				token.type=Token::TYPE_OPERATOR_AND;
			}
			else if(opname=="not" || opname=="!")
			{
				input.get();
				token.type=Token::TYPE_OPERATOR_NOT;
			}
			else
			{
				return false;
			}
		}

		input >> std::ws;

		return true;
	}

	template<typename Tester>
	static bool read_expression_from_string(const std::string& expression_string, std::vector< FilteringExpressionToken<Tester> >& postfix_expression) noexcept
	{
		if(expression_string.empty())
		{
			return false;
		}
		std::string prepared_string;
		char last_nws=0;
		std::size_t i=0;
		while(i<expression_string.size())
		{
			const char c=expression_string[i];
			if(c>0 && c<=32)
			{
				if(last_nws!=0 && last_nws!=',' && last_nws!=';' && i+1<expression_string.size())
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
					if(next_nws!=0 && next_nws!=',' && next_nws!=';')
					{
						prepared_string.push_back(' ');
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

		std::vector< FilteringExpressionToken<Tester> > infix_expression;
		std::istringstream input(prepared_string);
		while(input.good())
		{
			FilteringExpressionToken<Tester> token;
			if(read_expression_token<Tester>(input, token))
			{
				infix_expression.push_back(token);
			}
			else
			{
				return false;
			}
		}

		std::vector< FilteringExpressionToken<Tester> > result;
		if(convert_filtering_expression_from_infix_to_postfix_form(infix_expression, result))
		{
			postfix_expression.swap(result);
		}
		else
		{
			return false;
		}

		return (!postfix_expression.empty());
	}

	template<typename Tester>
	static bool convert_filtering_expression_from_infix_to_postfix_form(const std::vector< FilteringExpressionToken<Tester> >& infix_expression, std::vector< FilteringExpressionToken<Tester> >& postfix_expression) noexcept
	{
		typedef FilteringExpressionToken<Tester> Token;

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
					return false;
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
			return false;
		}

		postfix_expression.swap(output);

		return true;
	}

	template<typename Filter, typename Value>
	static ExpressionResult evaluate_filtering_expression_in_postfix_form(const std::vector< FilteringExpressionToken<Filter> >& postfix_expression, const Value& val) noexcept
	{
		typedef FilteringExpressionToken<Filter> Token;

		if(postfix_expression.empty())
		{
			return ExpressionResult();
		}

		std::vector<bool> operands_stack;

		for(typename std::vector<Token>::const_iterator it=postfix_expression.begin();it!=postfix_expression.end();++it)
		{
			const Token& token=(*it);

			if(token.is_tester())
			{
				operands_stack.push_back(token.tester.match(val));
			}
			else if(token.is_binary_operator())
			{
				if(operands_stack.size()<2)
				{
					return ExpressionResult();
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
						return ExpressionResult();
					}
				}
			}
			else if(token.is_unary_operator())
			{
				if(operands_stack.empty())
				{
					return ExpressionResult();
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
						return ExpressionResult();
					}
				}
			}
			else
			{
				return ExpressionResult();
			}
		}

		if(operands_stack.size()!=1)
		{
			return ExpressionResult();
		}

		return ExpressionResult(operands_stack.back());
	}

public:
	class ExpressionForSingle
	{
	public:
		ExpressionForSingle() noexcept : allow_all_(true)
		{
		}

		ExpressionForSingle(const std::string& expression_str) noexcept : allow_all_(expression_str.empty() || expression_str=="[]")
		{
			if(!allow_all_)
			{
				read_expression_from_string<FilterSingle>(expression_str, postfix_expression_);
			}
		}

		bool valid() const
		{
			return (allow_all_ || !postfix_expression_.empty());
		}

		bool allow_all() const
		{
			return allow_all_;
		}

		ExpressionResult filter(const SphereLabeling::SphereLabel& v) const noexcept
		{
			if(allow_all_)
			{
				return ExpressionResult(true);
			}
			return evaluate_filtering_expression_in_postfix_form<FilterSingle, SphereLabeling::SphereLabel>(postfix_expression_, v);
		}

		VectorExpressionResult filter_vector(const std::vector<SphereLabeling::SphereLabel>& labels) const noexcept
		{
			VectorExpressionResult ver;
			ver.expression_valid=valid();
			if(allow_all_)
			{
				ver.expression_matched_all=true;
			}
			else
			{
				for(std::size_t i=0;i<labels.size() && ver.expression_valid;i++)
				{
					const ExpressionResult er=filter(labels[i]);
					if(er.expression_valid && er.expression_matched)
					{
						ver.expression_matched_ids.push_back(i);
					}
					else
					{
						ver.expression_valid=er.expression_valid;
					}
				}
				if(!ver.expression_valid)
				{
					ver.expression_matched_ids.clear();
				}
				else if(ver.expression_matched_ids.size()==labels.size())
				{
					ver.expression_matched_all=true;
					ver.expression_matched_ids.clear();
				}
			}
			return ver;
		}

	private:
		bool allow_all_;
		std::vector< FilteringExpressionToken<FilterSingle> > postfix_expression_;
	};

	class ExpressionForPair
	{
	public:
		ExpressionForPair() noexcept : allow_all_(true)
		{
		}

		ExpressionForPair(const std::string& expression_str) noexcept : allow_all_(expression_str.empty() || expression_str=="[]")
		{
			if(!allow_all_)
			{
				read_expression_from_string<FilterPair>(expression_str, postfix_expression_);
			}
		}

		bool valid() const
		{
			return (allow_all_ || !postfix_expression_.empty());
		}

		bool allow_all() const
		{
			return allow_all_;
		}

		ExpressionResult filter(const SphereLabeling::SphereLabel& a, const SphereLabeling::SphereLabel& b) const noexcept
		{
			if(allow_all_)
			{
				return ExpressionResult(true);
			}
			return evaluate_filtering_expression_in_postfix_form<FilterPair, PairOfSphereLabels>(postfix_expression_, PairOfSphereLabels(a, b));
		}

		template<class ContainerType>
		VectorExpressionResult filter_vector(const std::vector<SphereLabeling::SphereLabel>& labels, const ContainerType& container) const noexcept
		{
			VectorExpressionResult ver;
			ver.expression_valid=valid();
			if(allow_all_)
			{
				ver.expression_matched_all=true;
			}
			else
			{
				for(std::size_t i=0;i<container.size() && ver.expression_valid;i++)
				{
					const std::size_t id_a=static_cast<std::size_t>(container[i].first);
					const std::size_t id_b=static_cast<std::size_t>(container[i].second);
					const ExpressionResult er=(id_a<labels.size() && id_b<labels.size()) ? filter(labels[id_a], labels[id_b]) : ExpressionResult();
					if(er.expression_valid && er.expression_matched)
					{
						ver.expression_matched_ids.push_back(i);
					}
					else
					{
						ver.expression_valid=er.expression_valid;
					}
				}
				if(!ver.expression_valid)
				{
					ver.expression_matched_ids.clear();
				}
				else if(ver.expression_matched_ids.size()==container.size())
				{
					ver.expression_matched_all=true;
					ver.expression_matched_ids.clear();
				}
			}
			return ver;
		}

		template<class ContainerType>
		static std::vector< std::pair<std::size_t, std::size_t> > adapt_indices_container(const ContainerType& container) noexcept
		{
			std::vector< std::pair<std::size_t, std::size_t> > result(container.size());
			for(std::size_t i=0;i<container.size();i++)
			{
				result[i].first=static_cast<std::size_t>(container[i].id_a);
				result[i].second=static_cast<std::size_t>(container[i].id_b);
			}
			return result;
		}

	private:
		bool allow_all_;
		std::vector< FilteringExpressionToken<FilterPair> > postfix_expression_;
	};
};

}

#endif /* VORONOTALT_CLI_FILTERING_BY_SPHERE_LABELS_H_ */
