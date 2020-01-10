#ifndef AUXILIARIES_PROGRAM_OPTIONS_HANDLER_H_
#define AUXILIARIES_PROGRAM_OPTIONS_HANDLER_H_

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace voronota
{

namespace auxiliaries
{

class ProgramOptionsHandler
{
public:
	class Exception : public std::runtime_error
	{
	public:
		explicit Exception(const std::string& msg) : std::runtime_error(msg)
		{
		}
	};

	ProgramOptionsHandler(const int argc, const char** argv)
	{
		if(argc>0)
		{
			original_argv_.resize(argc);
			for(int i=0;i<argc;i++)
			{
				original_argv_[i]=unquote_string(std::string(argv[i]));
			}
			for(std::size_t i=0;i<original_argv_.size();i++)
			{
				const std::string& str=original_argv_[i];
				if(i>0 && str.rfind(option_prefix_str(), 0)==0)
				{
					options_[str]="";
				}
				else if(i>1 && original_argv_[i-1].rfind(option_prefix_str(), 0)==0)
				{
					options_[original_argv_[i-1]]=str;
				}
				else
				{
					unused_argv_.push_back(str);
				}
			}
		}
	}

	const std::vector<std::string>& original_argv() const
	{
		return original_argv_;
	}

	std::string original_arg(const std::size_t pos) const
	{
		return (pos<original_argv_.size() ? original_argv_[pos] : std::string());
	}

	const std::vector<std::string>& unused_argv() const
	{
		return unused_argv_;
	}

	const std::map<std::string, std::string>& options() const
	{
		return options_;
	}

	bool empty() const
	{
		return options_.empty();
	}

	void set_option(const std::string& name)
	{
		if(name.rfind(option_prefix_str(), 0)==0)
		{
			options_[name]="";
		}
	}

	template<typename T>
	void set_option_with_argument(const std::string& name, const T value)
	{
		if(name.rfind(option_prefix_str(), 0)==0)
		{
			std::ostringstream output;
			output << value;
			options_[name]=output.str();
		}
	}

	bool contains_option(const std::string& name) const
	{
		return (options_.count(name)>0);
	}

	bool contains_option_with_argument(const std::string& name) const
	{
		return (contains_option(name) && !(options_.find(name)->second.empty()));
	}

	const std::string& argument_string(const std::string& name) const
	{
		if(!contains_option_with_argument(name))
		{
			throw Exception(std::string("Missing command line argument for option '")+name+"'.");
		}
		return options_.find(name)->second;
	}

	template<typename T>
	T argument(const std::string& name) const
	{
		std::istringstream input(argument_string(name));
		T value;
		input >> value;
		if(input.fail())
		{
			throw Exception(std::string("Invalid command line argument for option '")+name+"'.");
		}
		return value;
	}

	template<typename T>
	T argument(const std::string& name, const T default_value) const
	{
		if(contains_option(name))
		{
			return argument<T>(name);
		}
		else
		{
			return default_value;
		}
	}

	std::vector<std::string> argument_strings_vector(const std::string& name, const char delimiter) const
	{
		std::vector<std::string> result;
		if(contains_option(name))
		{
			result=split_string(argument_string(name), delimiter);
			if(result.empty())
			{
				throw Exception(std::string("Missing command line vector argument for option '")+name+"'.");
			}
		}
		return result;
	}

	template<typename T>
	std::vector<T> argument_vector(const std::string& name, const char delimiter) const
	{
		std::vector<T> result;
		const std::vector<std::string> result_as_strings=argument_strings_vector(name, delimiter);
		if(!result_as_strings.empty())
		{
			result=convert_string_vector_to_typed_vector<T>(result_as_strings);
		}
		return result;
	}

	template<typename T>
	std::vector<T> argument_vector(const std::string& name, const char delimiter, const std::vector<T>& default_value) const
	{
		if(contains_option(name))
		{
			return argument_vector<T>(name, delimiter);
		}
		else
		{
			return default_value;
		}
	}

	void remove_option(const std::string& name)
	{
		options_.erase(name);
	}

	template<typename T>
	static T convert_hex_string_to_integer(const std::string& str)
	{
		std::istringstream input(str);
		T value=0;
		input >> std::hex >> value;
		if(input.fail())
		{
			throw Exception(std::string("Invalid hex string '")+str+"'.");
		}
		return value;
	}

	template<typename T>
	static T restrict_value_in_range(const T min_value, const T max_value, const T value)
	{
		return std::max(min_value, std::min(max_value, value));
	}

private:
	static const std::string& option_prefix_str()
	{
		static const std::string str("--");
		return str;
	}

	static std::string unquote_string(const std::string& str)
	{
		if(str.size()>2 && str[0]==str[str.size()-1] && (str[0]=='\'' || str[0]=='"'))
		{
			return str.substr(1, str.size()-2);
		}
		return str;
	}

	static std::vector<std::string> split_string(const std::string& str, const char delimiter)
	{
		std::vector<std::string> result;
		std::istringstream input(str);
		while(input.good())
		{
			std::string token;
			std::getline(input, token, delimiter);
			if(token.empty())
			{
				throw Exception(std::string("Empty substring encountered when splitting '")+str+"'.");
			}
			result.push_back(token);
		}
		return result;
	}

	template<typename T>
	static std::vector<T> convert_string_vector_to_typed_vector(const std::vector<std::string>& strs)
	{
		std::vector<T> result;
		for(std::size_t i=0;i<strs.size();i++)
		{
			std::string token=strs[i];
			const std::size_t end_pos=token.find_last_not_of(' ');
			if(end_pos!=std::string::npos && (end_pos+1)<=token.size())
			{
				token=token.substr(0, end_pos+1);
			}
			std::istringstream token_input(token);
			T value;
			token_input >> value;
			if(token_input.fail())
			{
				throw Exception(std::string("Failed to convert token '")+token+"'.");
			}
			result.push_back(value);
			if(!token_input.eof())
			{
				throw Exception(std::string("Too many spaces in token '")+token+"'.");
			}
		}
		return result;
	}

	std::vector<std::string> original_argv_;
	std::vector<std::string> unused_argv_;
	std::map<std::string, std::string> options_;
};

class ProgramOptionsHandlerWrapper
{
public:
	class Exception : public std::runtime_error
	{
	public:
		explicit Exception(const std::string& msg) : std::runtime_error(msg)
		{
		}
	};

	const ProgramOptionsHandler& poh;

	explicit ProgramOptionsHandlerWrapper(const ProgramOptionsHandler& poh) : poh(poh)
	{
	}

	std::string describe_option(const std::string& name, const std::string& argument_type, const std::string& description_text, const bool required=false)
	{
		list_of_option_descriptions_.push_back(OptionDescription(name, argument_type, description_text, required));
		return name;
	}

	void describe_io(const std::string& name, const bool from, const bool to, const std::string& description)
	{
		list_of_io_descriptions_.push_back(IODescription(name, from, to, description));
	}

	bool assert_or_print_help(const bool allow_unrecognized_options) const
	{
		if(poh.contains_option("--help"))
		{
			{
				std::vector<OptionDescription> basic_list_of_option_descriptions=list_of_option_descriptions_;
				basic_list_of_option_descriptions.push_back(OptionDescription("--help", "", "flag to print usage help to stdout and exit", false));
				print_list_of_option_descriptions(basic_list_of_option_descriptions, std::cout);
			}
			print_list_of_io_descriptions(list_of_io_descriptions_, std::cout);
			return false;
		}
		else
		{
			assert_list_of_option_descriptions(allow_unrecognized_options);
			return true;
		}
	}

private:
	struct OptionDescription
	{
		std::string name;
		std::string argument_type;
		std::string description_text;
		bool required;

		OptionDescription(const std::string& name, const std::string& argument_type, const std::string& description_text, const bool required) :
			name(name), argument_type(argument_type), description_text(description_text), required(required)
		{
		}

		bool operator==(const std::string& check_name) const
		{
			return (check_name==name);
		}

		void print(const std::size_t max_option_name_length, const std::size_t max_argument_type_length, std::ostream& output) const
		{
			output << name << std::string(std::max(max_option_name_length, name.size())+2-name.size(), ' ');
			output << argument_type << std::string(std::max(max_argument_type_length, argument_type.size())+2-argument_type.size(), ' ');
			output << (required ? "*" : " ") << "  ";
			output << description_text << "\n";
		}
	};

	struct IODescription
	{
		std::string name;
		bool from;
		bool to;
		std::string description;

		IODescription(const std::string& name, const bool from, const bool to, const std::string& description) :
			name(name), from(from), to(to), description(description)
		{
		}

		void print(std::ostream& output) const
		{
			const std::size_t max_name_length=std::max(static_cast<std::size_t>(7), name.size());
			output << name << std::string(max_name_length+1-name.size(), ' ');
			if(from && to)
			{
				output << "<->";
			}
			else if(from)
			{
				output << "<- ";
			}
			else if(to)
			{
				output << "-> ";
			}
			std::istringstream strstream(description);
			int linescount=0;
			while(strstream.good())
			{
				std::string line;
				std::getline(strstream, line);
				if(!line.empty())
				{
					if(linescount>0)
					{
						output << std::string(max_name_length+6, ' ');
					}
					output << " " << line << "\n";
					linescount++;
				}
			}
		}
	};

	static void print_list_of_option_descriptions(const std::vector<OptionDescription>& list_of_option_descriptions, std::ostream& output)
	{
		std::size_t max_option_name_length=30;
		std::size_t max_argument_type_length=7;
		for(std::vector<OptionDescription>::const_iterator it=list_of_option_descriptions.begin();it!=list_of_option_descriptions.end();++it)
		{
			max_option_name_length=std::max(max_option_name_length, it->name.size());
			max_argument_type_length=std::max(max_argument_type_length, it->argument_type.size());
		}
		for(std::vector<OptionDescription>::const_iterator it=list_of_option_descriptions.begin();it!=list_of_option_descriptions.end();++it)
		{
			it->print(max_option_name_length, max_argument_type_length, output);
		}
	}

	static void print_list_of_io_descriptions(const std::vector<IODescription>& list_of_io_descriptions, std::ostream& output)
	{
		for(std::vector<IODescription>::const_iterator it=list_of_io_descriptions.begin();it!=list_of_io_descriptions.end();++it)
		{
			it->print(output);
		}
	}

	void assert_list_of_option_descriptions(const bool allow_unrecognized) const
	{
		for(std::vector<OptionDescription>::const_iterator it=list_of_option_descriptions_.begin();it!=list_of_option_descriptions_.end();++it)
		{
			if(it->required && poh.options().count(it->name)==0)
			{
				throw Exception(std::string("Missing required option '")+(it->name)+"'.");
			}
		}
		for(std::map<std::string, std::string>::const_iterator it=poh.options().begin();it!=poh.options().end();++it)
		{
			const std::string& option=it->first;
			std::vector<OptionDescription>::const_iterator jt=std::find(list_of_option_descriptions_.begin(), list_of_option_descriptions_.end(), option);
			if(jt==list_of_option_descriptions_.end())
			{
				if(!allow_unrecognized)
				{
					throw Exception(std::string("Unrecognized command line option '")+option+"'.");
				}
			}
			else if(it->second.empty() && !jt->argument_type.empty())
			{
				throw Exception(std::string("Command line option '")+option+"' should have arguments.");
			}
			else if(!it->second.empty() && jt->argument_type.empty())
			{
				throw Exception(std::string("Command line option '")+option+"' cannot have arguments.");
			}
		}
	}

	std::vector<OptionDescription> list_of_option_descriptions_;
	std::vector<IODescription> list_of_io_descriptions_;
};

}

}

#endif /* AUXILIARIES_PROGRAM_OPTIONS_HANDLER_H_ */
