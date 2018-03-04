#ifndef COMMON_COMMAND_INPUT_H_
#define COMMON_COMMAND_INPUT_H_

#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <map>
#include <set>

namespace common
{

class CommandInput
{
public:
	typedef std::map< std::string, std::vector<std::string> > MapOfValues;

	CommandInput()
	{
	}

	explicit CommandInput(const std::string& command_str)
	{
		init(command_str);
	}

	void init(const std::string& command_str)
	{
		command_name_.clear();
		map_of_values_.clear();

		if(command_str.empty())
		{
			throw std::runtime_error(std::string("Empty command string"));
		}

		const std::string canonical_command_str=canonicalize_command_string(command_str);

		if(canonical_command_str.empty())
		{
			throw std::runtime_error(std::string("No content in command string '")+command_str+"'.");
		}

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
				if(token_wrapped==0 && token_str.size()>2 && token_str.compare(0, 2, "--")==0)
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

	std::string get_value_or_first_unused_unnamed_value(const std::string& name)
	{
		if(!is_option(name))
		{
			for(std::size_t i=0;i<list_of_unnamed_values_.size();i++)
			{
				if(!is_unnamed_value_used(i))
				{
					mark_unnamed_value_as_used(i);
					return list_of_unnamed_values_[i];
				}
			}
		}
		return get_value<std::string>(name);
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

	std::string generate_string_from_arguments(const std::set<std::string>& names_to_exclude, const bool exclude_unnamed) const
	{
		std::ostringstream output;
		if(!exclude_unnamed)
		{
			for(std::size_t i=0;i<list_of_unnamed_values_.size();i++)
			{
				output << " \"" << list_of_unnamed_values_[i] << "\"";
			}
		}
		for(MapOfValues::const_iterator it=map_of_values_.begin();it!=map_of_values_.end();++it)
		{
			if(names_to_exclude.count(it->first)==0)
			{
				output << " --" << (it->first);
				for(std::size_t i=0;i<it->second.size();i++)
				{
					output << " \"" << it->second[i] << "\"";
				}
			}
		}
		return output.str();
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

	static std::string remove_spaces_around_equal_signs(const std::string& input_str)
	{
		std::string collapsed_str;

		for(std::size_t i=0;i<input_str.size();i++)
		{
			const char c=input_str[i];
			if(c=='=')
			{
				while(!collapsed_str.empty() && collapsed_str.back()<=32)
				{
					collapsed_str.pop_back();
				}
				collapsed_str.push_back(c);
			}
			else if(c<=32)
			{
				if(collapsed_str.empty() || collapsed_str.back()!='=')
				{
					collapsed_str.push_back(c);
				}
			}
			else
			{
				collapsed_str.push_back(c);
			}
		}

		return collapsed_str;
	}

	static std::string canonicalize_command_string(const std::string& input_str)
	{
		const std::string collapsed_str=remove_spaces_around_equal_signs(input_str);

		std::string canonical_str;
		std::size_t last_eq=0;
		std::size_t last_sep=0;

		for(std::size_t i=0;i<collapsed_str.size();i++)
		{
			const char c=collapsed_str[i];
			if(c=='=')
			{
				if(last_eq<=last_sep && (last_sep+1)<i)
				{
					for(std::size_t j=(last_sep+1);j<i;j++)
					{
						canonical_str.pop_back();
					}
					std::string token=collapsed_str.substr(last_sep+1, i-(last_sep+1));
					if(token.compare(0, 1, "-")!=0)
					{
						canonical_str+="--";
					}
					canonical_str+=token;
					canonical_str+=" ";
				}
				else
				{
					canonical_str.push_back(c);
				}
				last_eq=i;
			}
			else
			{
				if(c<=32 || c=='(' || c=='{')
				{
					last_sep=i;
				}
				canonical_str.push_back(c);
			}
		}

		return canonical_str;
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

	std::string command_name_;
	MapOfValues map_of_values_;
	std::set<std::string> set_of_requested_names_;
	std::vector<std::string> list_of_unnamed_values_;
	std::set<std::size_t> set_of_requested_ids_of_unnamed_values_;
};

}

#endif /* COMMON_COMMAND_INPUT_H_ */

