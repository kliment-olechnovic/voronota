#ifndef COMMON_SCRIPTING_ALIASING_H_
#define COMMON_SCRIPTING_ALIASING_H_

#include "command_input.h"

namespace common
{

class ScriptingAliasing
{
public:
	ScriptingAliasing()
	{
	}

	const std::map<std::string, std::string>& aliases() const
	{
		return aliases_;
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

	std::string translate_alias(const std::string& alias_str) const
	{
		if(alias_str.empty())
		{
			throw std::runtime_error(std::string("Command string is empty."));
		}

		std::istringstream input(alias_str);
		std::vector< std::pair<int, std::string> > tokens;
		CommandInput::read_all_strings_considering_quotes_and_brackets(input, tokens);

		if(tokens.empty())
		{
			throw std::runtime_error(std::string("Failed to read command string '")+alias_str+"'.");
		}

		const std::string& alias_name=tokens[0].second;

		if(aliases_.count(alias_name)==0)
		{
			return alias_str;
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

private:
	std::map<std::string, std::string> aliases_;
};

}

#endif /* COMMON_SCRIPTING_ALIASING_H_ */
