#ifndef SCRIPTING_COMMAND_INPUT_H_
#define SCRIPTING_COMMAND_INPUT_H_

#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <map>
#include <set>

namespace voronota
{

namespace scripting
{

class CommandInput
{
public:
	typedef std::map< std::string, std::vector<std::string> > MapOfValues;

	CommandInput() : initialized_(false)
	{
	}

	explicit CommandInput(const std::string& command_str) : initialized_(false)
	{
		init_from_string(command_str);
	}

	explicit CommandInput(const int argc, const char** argv) : initialized_(false)
	{
		init_from_array(argc, argv);
	}

	static int read_string_considering_quotes_and_brackets(std::istream& input, std::string& output)
	{
		output.clear();
		input >> std::ws;
		const char opener=std::char_traits<char>::to_char_type(input.peek());
		int wrapped=0;
		if(opener=='"' || opener=='\'')
		{
			wrapped=1;
			input.get();
			std::getline(input, output, opener);
		}
		else if(opener=='{' || opener=='(' || opener=='[')
		{
			wrapped=2;
			const char closer=(opener=='{' ? '}' : (opener=='(' ? ')' : ']'));
			int level=0;
			do
			{
				const char curent_char=std::char_traits<char>::to_char_type(input.get());
				output.push_back(curent_char);
				if(curent_char==opener)
				{
					level++;
				}
				else if(curent_char==closer)
				{
					level--;
				}
			}
			while(level>0 && input.good());
		}
		else
		{
			input >> output;
		}
		return wrapped;
	}

	static void read_all_strings_considering_quotes_and_brackets(std::istream& input, std::vector< std::pair<int, std::string> >& output)
	{
		std::vector< std::pair<int, std::string> > tokens;
		while(input.good())
		{
			std::pair<int, std::string> token;
			token.first=read_string_considering_quotes_and_brackets(input, token.second);
			if(!token.second.empty() || token.first==1)
			{
				tokens.push_back(token);
			}
		}
		output.swap(tokens);
	}

	static std::string canonicalize_command_string(const std::string& input_str)
	{
		std::string canonical_str;

		for(std::size_t i=0;i<input_str.size();i++)
		{
			if(input_str[i]=='-' && (i+1)<input_str.size())
			{
				if(i==0 || input_str[i-1]<=' ' || input_str[i-1]=='[' || input_str[i-1]=='(')
				{
					if((input_str[i+1]>='a' && input_str[i+1]<='z') || (input_str[i+1]>='A' && input_str[i+1]<='Z'))
					{
						canonical_str.push_back('-');
					}
				}
			}
			canonical_str.push_back(input_str[i]);
		}

		return canonical_str;
	}

	bool initialized() const
	{
		return initialized_;
	}

	const std::string& get_input_command_string() const
	{
		return input_command_string_;
	}

	const std::string& get_canonical_input_command_string() const
	{
		return canonical_input_command_string_;
	}

	const std::string& get_command_name() const
	{
		return command_name_;
	}

	const MapOfValues& get_map_of_values() const
	{
		return map_of_values_;
	}

	const std::vector<std::string>& get_list_of_unnamed_values() const
	{
		return list_of_unnamed_values_;
	}

	bool is_option(const std::string& name) const
	{
		return (map_of_values_.count(name)>0);
	}

	bool is_option_with_values(const std::string& name) const
	{
		MapOfValues::const_iterator it=map_of_values_.find(name);
		return (it!=map_of_values_.end() && !it->second.empty());
	}

	bool is_unnamed_value_used(const std::size_t id) const
	{
		return (set_of_requested_ids_of_unnamed_values_.count(id)>0);
	}

	bool is_any_unnamed_value_unused() const
	{
		for(std::size_t id=0;id<list_of_unnamed_values_.size();id++)
		{
			if(set_of_requested_ids_of_unnamed_values_.count(id)==0)
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_unnamed_value_unused_and_starting_with_prefix(const std::string& prefix, const bool prefix_as_chars) const
	{
		for(std::size_t i=0;i<list_of_unnamed_values_.size();i++)
		{
			if(set_of_requested_ids_of_unnamed_values_.count(i)==0)
			{
				const std::string& candidate=list_of_unnamed_values_[i];
				if(!candidate.empty() && ((prefix_as_chars && candidate.find_first_of(prefix)==0) || (!prefix_as_chars && candidate.rfind(prefix, 0)==0)))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool get_flag(const std::string& name)
	{
		MapOfValues::const_iterator it=map_of_values_.find(name);
		set_of_requested_names_.insert(name);
		if(it!=map_of_values_.end())
		{
			if(it->second.empty())
			{
				return true;
			}
			else if(it->second.size()==1 && is_flag_string_true(it->second.front()))
			{
				return true;
			}
			else if(it->second.size()==1 && is_flag_string_false(it->second.front()))
			{
				return false;
			}
			else
			{
				throw std::runtime_error(std::string("Invalid flag '")+name+"'.");
			}
		}
		return false;
	}

	template<typename T>
	std::vector<T> get_value_vector(const std::string& name)
	{
		std::vector<T> values;
		get_value_vector(name, values);
		return values;
	}

	template<typename T>
	std::vector<T> get_value_vector_or_default(const std::string& name, const std::vector<T>& default_values)
	{
		if(is_option(name))
		{
			return get_value_vector<T>(name);
		}
		else
		{
			return default_values;
		}
	}

	template<typename T>
	T get_value(const std::string& name)
	{
		T val;
		get_value(name, val);
		return val;
	}

	template<typename T>
	T get_value_or_default(const std::string& name, const T& default_value)
	{
		if(is_option(name))
		{
			return get_value<T>(name);
		}
		else
		{
			return default_value;
		}
	}

	std::string get_first_unused_unnamed_value()
	{
		for(std::size_t i=0;i<list_of_unnamed_values_.size();i++)
		{
			if(!is_unnamed_value_used(i))
			{
				mark_unnamed_value_as_used(i);
				return list_of_unnamed_values_[i];
			}
		}
		throw std::runtime_error(std::string("No unused unnamed value."));
		return std::string();
	}

	std::string get_value_or_first_unused_unnamed_value(const std::string& name)
	{
		if(!is_option(name))
		{
			return get_first_unused_unnamed_value();
		}
		return get_value<std::string>(name);
	}

	std::string get_value_or_first_unused_unnamed_value_or_default(const std::string& name, const std::string& default_value)
	{
		if(!is_option(name) && is_any_unnamed_value_unused())
		{
			return get_first_unused_unnamed_value();
		}
		return get_value_or_default<std::string>(name, default_value);
	}

	std::string get_first_unused_unnamed_value_starting_with_prefix(const std::string& prefix, const bool prefix_as_chars)
	{
		for(std::size_t i=0;i<list_of_unnamed_values_.size();i++)
		{
			if(!is_unnamed_value_used(i))
			{
				const std::string& candidate=list_of_unnamed_values_[i];
				if(!candidate.empty() && ((prefix_as_chars && candidate.find_first_of(prefix)==0) || (!prefix_as_chars && candidate.rfind(prefix, 0)==0)))
				{
					mark_unnamed_value_as_used(i);
					return candidate;
				}
			}
		}
		throw std::runtime_error(std::string("No unused unnamed value starting with prefix '")+prefix+"'.");
		return std::string();
	}

	std::string get_value_or_first_unused_unnamed_value_starting_with_prefix(const std::string& name, const std::string& prefix, const bool prefix_as_chars)
	{
		if(!prefix.empty() && !is_option(name))
		{
			return get_first_unused_unnamed_value_starting_with_prefix(prefix, prefix_as_chars);
		}
		return get_value<std::string>(name);
	}

	std::string get_value_or_first_unused_unnamed_value_starting_with_prefix_or_default(const std::string& name, const std::string& prefix, const bool prefix_as_chars, const std::string& default_value)
	{
		if(!prefix.empty() && !is_option(name) && is_any_unnamed_value_unused_and_starting_with_prefix(prefix, prefix_as_chars))
		{
			return get_first_unused_unnamed_value_starting_with_prefix(prefix, prefix_as_chars);
		}
		return get_value_or_default<std::string>(name, default_value);
	}

	std::vector<std::string> get_value_vector_or_all_unused_unnamed_values(const std::string& name)
	{
		if(is_option(name))
		{
			return get_value_vector<std::string>(name);
		}
		else
		{
			std::vector<std::string> result;
			for(std::size_t i=0;i<list_of_unnamed_values_.size();i++)
			{
				if(!is_unnamed_value_used(i))
				{
					mark_unnamed_value_as_used(i);
					result.push_back(list_of_unnamed_values_[i]);
				}
			}
			return result;
		}
	}

	void assert_nothing_unusable() const
	{
		for(MapOfValues::const_iterator it=map_of_values_.begin();it!=map_of_values_.end();++it)
		{
			const std::string& name=it->first;
			if(set_of_requested_names_.count(name)==0)
			{
				throw std::runtime_error(std::string("Unusable option '")+name+"'.");
			}
		}

		for(std::size_t id=0;id<list_of_unnamed_values_.size();id++)
		{
			if(set_of_requested_ids_of_unnamed_values_.count(id)==0)
			{
				throw std::runtime_error(std::string("Unusable unnamed value '")+list_of_unnamed_values_[id]+"'.");
			}
		}
	}

	CommandInput& unset(const std::string& name)
	{
		if(name.empty())
		{
			list_of_unnamed_values_.clear();
		}
		else
		{
			map_of_values_.erase(name);
		}
		return (*this);
	}

	template<typename T>
	CommandInput& set(const std::string& name, const T& value, const bool add)
	{
		std::ostringstream output;
		output << value;
		if(name.empty())
		{
			if(add)
			{
				list_of_unnamed_values_.push_back(output.str());
			}
			else
			{
				list_of_unnamed_values_=std::vector<std::string>(1, output.str());
			}
		}
		else
		{
			if(add)
			{
				map_of_values_[name].push_back(output.str());
			}
			else
			{
				map_of_values_[name]=std::vector<std::string>(1, output.str());
			}
		}
		return (*this);
	}

	template<typename T>
	CommandInput& set(const std::string& name, const T& value)
	{
		return set(name, value, false);
	}

	template<typename T>
	CommandInput& add(const std::string& name, const T& value)
	{
		return set(name, value, true);
	}

	template<typename T>
	CommandInput& setv(const std::string& name, const T& values, const bool add)
	{
		if(!add)
		{
			unset(name);
		}
		for(typename T::const_iterator it=values.begin();it!=values.end();++it)
		{
			set(name, (*it), true);
		}
		return (*this);
	}

	template<typename T>
	CommandInput& setv(const std::string& name, const T& value)
	{
		return setv(name, value, false);
	}

	template<typename T>
	CommandInput& addv(const std::string& name, const T& value)
	{
		return setv(name, value, true);
	}

	bool check_for_any_value_with_string(const std::string& str_mark) const
	{
		for(std::size_t i=0;i<list_of_unnamed_values_.size();i++)
		{
			if(list_of_unnamed_values_[i].find(str_mark)!=std::string::npos)
			{
				return true;
			}
		}
		for(MapOfValues::const_iterator it=map_of_values_.begin();it!=map_of_values_.end();++it)
		{
			const std::vector<std::string>& values=it->second;
			for(std::size_t i=0;i<values.size();i++)
			{
				if(values[i].find(str_mark)!=std::string::npos)
				{
					return true;
				}
			}
		}
		return false;
	}

	int replace_string_in_values(const std::string& str_mark, const std::string& str_replacement)
	{
		int replaced=0;
		for(std::size_t i=0;i<list_of_unnamed_values_.size();i++)
		{
			replaced+=replace_all(list_of_unnamed_values_[i], str_mark, str_replacement);
		}
		for(MapOfValues::iterator it=map_of_values_.begin();it!=map_of_values_.end();++it)
		{
			std::vector<std::string>& values=it->second;
			for(std::size_t i=0;i<values.size();i++)
			{
				replaced+=replace_all(values[i], str_mark, str_replacement);
			}
		}
		return replaced;
	}

private:
	static bool is_flag_string_true(const std::string& str)
	{
		return (str=="true" || str=="1");
	}

	static bool is_flag_string_false(const std::string& str)
	{
		return (str=="false" || str=="0");
	}

	static int replace_all(std::string& content, const std::string& str_mark, const std::string& str_replacement)
	{
		int replaced=0;
		std::size_t pos=content.find(str_mark);
		while(pos<content.size())
		{
			content.replace(pos, str_mark.size(), str_replacement);
			replaced++;
			pos+=str_replacement.size();
			pos=content.find(str_mark, pos);
		}
		return replaced;
	}

	void init_from_string(const std::string& command_str)
	{
		if(command_str.empty())
		{
			throw std::runtime_error(std::string("Empty command string."));
		}

		input_command_string_=command_str;

		const std::string canonical_command_str=canonicalize_command_string(command_str);

		if(canonical_command_str.empty())
		{
			throw std::runtime_error(std::string("No content in command string '")+command_str+"'.");
		}

		canonical_input_command_string_=canonical_command_str;

		std::istringstream input(canonical_command_str);
		std::vector< std::pair<int, std::string> > tokens;
		read_all_strings_considering_quotes_and_brackets(input, tokens);

		if(tokens.empty())
		{
			throw std::runtime_error(std::string("Failed to read command string '")+canonical_command_str+"'.");
		}

		std::string current_key;
		for(std::size_t i=0;i<tokens.size();i++)
		{
			const int token_wrapped=tokens[i].first;
			const std::string& token_str=tokens[i].second;
			if(i==0)
			{
				if(token_wrapped!=0 || token_str.empty())
				{
					throw std::runtime_error(std::string("Invalid command name in string '")+canonical_command_str+"'.");
				}
				else
				{
					command_name_=token_str;
				}
			}
			else
			{
				if(token_wrapped==0 && token_str.size()>2 && token_str.rfind("--", 0)==0)
				{
					current_key=token_str.substr(2);
					map_of_values_[current_key];
				}
				else
				{
					if(current_key.empty())
					{
						list_of_unnamed_values_.push_back(token_str);
					}
					else
					{
						map_of_values_[current_key].push_back(token_str);
					}
				}
			}
		}

		initialized_=true;
	}

	void init_from_array(const int argc, const char** argv)
	{
		if(argc<1)
		{
			throw std::runtime_error(std::string("Empty argument array."));
		}

		std::ostringstream output;

		for(int i=0;i<argc;i++)
		{
			if(argv[i]==0)
			{
				throw std::runtime_error(std::string("Invalid argument array."));
			}

			std::string token=argv[i];

			if(i==0)
			{
				output << token;
			}
			else
			{
				output << " ";
				if(!token.empty() && token[0]=='-')
				{
					output << token;
				}
				else if(token.find('\'')==std::string::npos)
				{
					output << "'" << token << "'";
				}
				else if(token.find('"')==std::string::npos)
				{
					output << "\"" << token << "\"";
				}
				else
				{
					output << token;
				}
			}
		}

		init_from_string(output.str());
	}

	const std::vector<std::string>& get_value_vector_ref(const std::string& name)
	{
		MapOfValues::const_iterator it=map_of_values_.find(name);
		set_of_requested_names_.insert(name);
		if(it==map_of_values_.end() || it->second.empty())
		{
			throw std::runtime_error(std::string("No values for option '")+name+"'.");
		}
		return it->second;
	}

	template<typename T>
	void get_value_vector(const std::string& name, std::vector<T>& output)
	{
		const std::vector<std::string>& value_vector=get_value_vector_ref(name);
		for(std::size_t i=0;i<value_vector.size();i++)
		{
			if(value_vector[i].find_first_of(" \t\n")!=std::string::npos)
			{
				throw std::runtime_error(std::string("Some value of option '")+name+"' contains whitespace.");
			}
		}
		std::vector<T> tmp_output;
		tmp_output.reserve(value_vector.size());
		for(std::size_t i=0;i<value_vector.size();i++)
		{
			std::istringstream input(value_vector[i]);
			T val;
			input >> val;
			if(input.fail())
			{
				throw std::runtime_error(std::string("Could not parse value '")+value_vector[i]+"' of option '"+name+"'.");
			}
			tmp_output.push_back(val);
		}
		output.swap(tmp_output);
	}

	void get_value_vector(const std::string& name, std::vector<std::string>& output)
	{
		output=get_value_vector_ref(name);
	}

	template<typename T>
	void get_value(const std::string& name, T& output)
	{
		std::vector<T> values;
		get_value_vector<T>(name, values);
		if(values.size()!=1)
		{
			throw std::runtime_error(std::string("Not exactly one value for option '")+name+"'.");
		}
		output=values.front();
	}

	void get_value(const std::string& name, std::string& output)
	{
		const std::vector<std::string>& values=get_value_vector_ref(name);
		if(values.size()!=1)
		{
			throw std::runtime_error(std::string("Not exactly one value for option '")+name+"'.");
		}
		output=values.front();
	}

	void mark_unnamed_value_as_used(const std::size_t id)
	{
		if(id<list_of_unnamed_values_.size())
		{
			set_of_requested_ids_of_unnamed_values_.insert(id);
		}
	}

	void mark_all_unnamed_values_as_used()
	{
		for(std::size_t id=0;id<list_of_unnamed_values_.size();id++)
		{
			mark_unnamed_value_as_used(id);
		}
	}

	bool initialized_;
	std::string input_command_string_;
	std::string canonical_input_command_string_;
	std::string command_name_;
	MapOfValues map_of_values_;
	std::set<std::string> set_of_requested_names_;
	std::vector<std::string> list_of_unnamed_values_;
	std::set<std::size_t> set_of_requested_ids_of_unnamed_values_;
};

}

}

#endif /* SCRIPTING_COMMAND_INPUT_H_ */

