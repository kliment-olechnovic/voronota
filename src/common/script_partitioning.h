#ifndef COMMON_SCRIPT_PARTITIONING_H_
#define COMMON_SCRIPT_PARTITIONING_H_

#include "command_input.h"

namespace common
{

class ScriptPartitioning
{
public:
	struct Sentence
	{
		std::string body;
		std::vector<std::string> parents;

		Sentence()
		{
		}

		explicit Sentence(const std::string& body) : body(body)
		{
		}
	};

	ScriptPartitioning()
	{
	}

	const std::map<std::string, std::string>& aliases() const
	{
		return aliases_;
	}

	std::vector<Sentence> partition_script_into_sentences(const std::string& script) const
	{
		return translate_sentences_fully(split_script_into_sentences(script));
	}

	void set_alias(const std::string& name, const std::string& script_template)
	{
		if(name.empty())
		{
			throw std::runtime_error(std::string("Alias name is empty."));
		}

		if(name.find_first_of(" \t\n", 0)!=std::string::npos)
		{
			throw std::runtime_error(std::string("Alias name '")+name+"' contains whitespace characters.");
		}

		if(script_template.empty())
		{
			throw std::runtime_error(std::string("Script template is empty for alias '")+name+"'.");
		}

		aliases_[name]=script_template;
	}

	bool unset_alias(const std::string& name)
	{
		return (aliases_.erase(name)>0);
	}

private:
	static std::vector<Sentence> split_script_into_sentences(const std::string& script)
	{
		std::vector<Sentence> sentences;

		std::size_t p1=0;
		while(p1<script.size())
		{
			p1=script.find_first_not_of(";\n \t", p1);
			if(p1<script.size())
			{
				std::size_t p2=script.find_first_of(";\n", p1);

				if(p2<script.size() && script[p2]==';')
				{
					const char symbols_open[6]={'\'', '"', '{', '(', '<', '['};
					const char symbols_close[6]={'\'', '"', '}', ')', '>', ']'};
					int counts_open[6];
					for(int i=0;i<6;i++)
					{
						counts_open[i]=0;
					}
					std::size_t p1_extended=p1;
					bool check_for_extension=true;
					while(check_for_extension)
					{
						for(std::size_t t=p1_extended;t<p2;t++)
						{
							const char c=script[t];
							for(int i=0;i<6;i++)
							{
								if(c==symbols_close[i] && counts_open[i]>0)
								{
									counts_open[i]--;
								}
								else if(c==symbols_open[i])
								{
									counts_open[i]++;
								}
							}
						}
						int sum_open=0;
						for(int i=0;i<6;i++)
						{
							sum_open+=counts_open[i];
						}
						if(sum_open==0)
						{
							check_for_extension=false;
						}
						else
						{
							p1_extended=script.find_first_not_of(";\n \t", p2);
							p2=script.find_first_of(";\n", p1_extended);
							check_for_extension=(p2<script.size() && script[p2]==';');
						}
					}
				}

				sentences.push_back(Sentence(script.substr(p1, p2-p1)));
				p1=p2;
			}
		}

		return sentences;
	}

	std::string translate_sentence_body_with_alias(const std::string& sentence_body) const
	{
		if(sentence_body.empty())
		{
			throw std::runtime_error(std::string("Command string is empty."));
		}

		std::istringstream input(sentence_body);
		std::vector< std::pair<int, std::string> > tokens;
		CommandInput::read_all_strings_considering_quotes_and_brackets(input, tokens);

		if(tokens.empty())
		{
			throw std::runtime_error(std::string("Failed to read command string '")+sentence_body+"'.");
		}

		const std::string& alias_name=tokens[0].second;

		if(aliases_.count(alias_name)==0)
		{
			return sentence_body;
		}

		const std::string& script_template=aliases_.find(alias_name)->second;

		std::string result=script_template;

		for(std::size_t i=1;i<tokens.size();i++)
		{
			std::ostringstream id_output;
			id_output << "${" << i << "}";
			const std::string id=id_output.str();
			bool found_id=false;
			std::size_t pos=0;
			while(pos<result.size())
			{
				pos=result.find(id, pos);
				if(pos!=std::string::npos)
				{
					found_id=true;
					result.replace(pos, id.size(), tokens[i].second);
					pos=pos+tokens[i].second.size()+1;
				}
			}
			if(!found_id)
			{
				throw std::runtime_error(std::string()+"Failed to find variable "+id+" in alias '"+alias_name+"' template '"+script_template+"'.");
			}
		}

		if(result.find("${")!=std::string::npos)
		{
			throw std::runtime_error(std::string()+"Some substrings starting with '${' were left unsubstituted in alias '"+alias_name+"' translation '"+result+"'.");
		}

		return result;
	}

	std::vector<Sentence> translate_sentences(const std::vector<Sentence>& input_sentences, bool& any_aliased_used) const
	{
		std::vector<Sentence> result;

		any_aliased_used=false;

		for(std::size_t i=0;i<input_sentences.size();i++)
		{
			const std::string& input_body=input_sentences[i].body;
			std::vector<Sentence> subsentences=split_script_into_sentences(translate_sentence_body_with_alias(input_body));
			if(!subsentences.empty())
			{
				const bool alias_used=(subsentences.size()>1 || subsentences[0].body!=input_body);
				any_aliased_used=(any_aliased_used || alias_used);
				if(alias_used)
				{
					for(std::size_t j=0;j<input_sentences[i].parents.size();j++)
					{
						if(input_body==input_sentences[i].parents[j])
						{
							throw std::runtime_error(std::string()+"Recursive translation for script sentence '"+input_body+"'.");
						}
					}
				}
				for(std::size_t j=0;j<subsentences.size();j++)
				{
					subsentences[j].parents=input_sentences[i].parents;
					if(alias_used)
					{
						subsentences[j].parents.push_back(input_body);
					}
				}
				result.insert(result.end(), subsentences.begin(), subsentences.end());
			}
		}

		return result;
	}

	std::vector<Sentence> translate_sentences_fully(const std::vector<Sentence>& input_sentences) const
	{
		std::vector<Sentence> result=input_sentences;
		bool any_aliased_used=false;
		do
		{
			result=translate_sentences(result, any_aliased_used);
		}
		while(any_aliased_used);
		return result;
	}

	std::map<std::string, std::string> aliases_;
};

}

#endif /* COMMON_SCRIPT_PARTITIONING_H_ */