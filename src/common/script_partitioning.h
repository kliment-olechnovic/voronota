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

	std::vector<Sentence> partition(const std::string& script)
	{
		return translate_sentences(split_script_into_sentences(script));
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

	std::vector<Sentence> translate_sentences(const std::vector<Sentence>& input_sentences)
	{
		std::vector<Sentence> result;

		for(std::size_t i=0;i<input_sentences.size();i++)
		{
			std::vector<Sentence> subsentences=split_script_into_sentences(translate_sentence_body_with_alias(input_sentences[i].body));
			const bool used_alias=(subsentences.size()>1 || subsentences[0].body!=input_sentences[i].body);
			for(std::size_t j=0;j<subsentences.size();j++)
			{
				subsentences[j].parents=input_sentences[i].parents;
				if(used_alias)
				{
					subsentences[j].parents.push_back(input_sentences[i].body);
				}
			}
			result.insert(result.end(), subsentences.begin(), subsentences.end());
		}

		return result;
	}

	std::map<std::string, std::string> aliases_;
};

}

#endif /* COMMON_SCRIPT_PARTITIONING_H_ */
