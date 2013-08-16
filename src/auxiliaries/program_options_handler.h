#ifndef AUXILIARIES_PROGRAM_OPTIONS_HANDLER_H_
#define AUXILIARIES_PROGRAM_OPTIONS_HANDLER_H_

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <iostream>

namespace auxiliaries
{

class ProgramOptionsHandler
{
public:
	class Exception : public std::runtime_error
	{
	public:
		Exception(const std::string& msg) : std::runtime_error(msg)
		{
		}
	};

	struct OptionDescription
	{
		std::string argument_type;
		std::string description_text;
		bool required;

		OptionDescription() : required(false)
		{
		}

		void init(const std::string& new_argument_type, const std::string& new_description_text, const bool new_required=false)
		{
			argument_type=new_argument_type;
			description_text=new_description_text;
			required=new_required;
		}
	};

	typedef std::map<std::string, OptionDescription> MapOfOptionDescriptions;

	ProgramOptionsHandler(const int argc, const char** argv)
	{
		if(argc>0)
		{
			original_argv_.resize(argc);
			for(int i=0;i<argc;i++)
			{
				original_argv_[i]=argv[i];
			}
			for(std::size_t i=0;i<original_argv_.size();i++)
			{
				const std::string& str=original_argv_[i];
				if(i>0 && str.find("--")==0)
				{
					options_[str]="";
				}
				else if(i>1)
				{
					const std::string& prev_str=original_argv_[i-1];
					if(prev_str.find("--")==0)
					{
						options_[prev_str]=str;
					}
					else
					{
						unused_argv_.push_back(str);
					}
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

	const std::vector<std::string>& unused_argv() const
	{
		return unused_argv_;
	}

	bool empty() const
	{
		return options_.empty();
	}

	void set_option(const std::string& name)
	{
		if(name.find("--")==0)
		{
			options_[name]="";
		}
	}

	template<typename T>
	void set_option_with_argument(const std::string& name, const T value)
	{
		if(name.find("--")==0)
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

	template<typename T>
	std::vector<T> argument_vector(const std::string& name) const
	{
		std::vector<T> result;
		if(contains_option_with_argument(name))
		{
			std::string input_string=argument_string(name);
			for(std::size_t i=0;i<input_string.size();i++)
			{
				const char c=input_string[i];
				if(c==',' || c==';')
				{
					input_string[i]=' ';
				}
			}
			std::istringstream input(input_string);
			do
			{
				T value;
				input >> value;
				if(input.fail())
				{
					throw Exception(std::string("Invalid command line argument vector for option '")+name+"'.");
				}
				result.push_back(value);
			}
			while(input.good());
		}
		return result;
	}

	void remove_option(const std::string& name)
	{
		options_.erase(name);
	}

	void compare_with_map_of_option_descriptions(const MapOfOptionDescriptions& map_of_option_descriptions, const bool allow_unrecognized=false) const
	{
		for(std::map<std::string, std::string>::const_iterator it=options_.begin();it!=options_.end();++it)
		{
			const std::string& option=it->first;
			MapOfOptionDescriptions::const_iterator jt=map_of_option_descriptions.find(option);
			if(jt==map_of_option_descriptions.end())
			{
				if(!allow_unrecognized)
				{
					throw Exception(std::string("Unrecognized command line option '")+option+"'.");
				}
			}
			else if(it->second.empty() && !jt->second.argument_type.empty())
			{
				throw Exception(std::string("Command line option '")+option+"' should have arguments.");
			}
			else if(!it->second.empty() && jt->second.argument_type.empty())
			{
				throw Exception(std::string("Command line option '")+option+"' cannot have arguments.");
			}
		}
	}

	static void print_map_of_option_descriptions(const MapOfOptionDescriptions& map_of_option_descriptions, std::ostream& output)
	{
		std::size_t max_option_name_length=30;
		std::size_t max_argument_type_length=7;
		for(MapOfOptionDescriptions::const_iterator it=map_of_option_descriptions.begin();it!=map_of_option_descriptions.end();++it)
		{
			max_option_name_length=std::max(max_option_name_length, it->first.size());
			max_argument_type_length=std::max(max_argument_type_length, it->second.argument_type.size());
		}
		for(MapOfOptionDescriptions::const_iterator it=map_of_option_descriptions.begin();it!=map_of_option_descriptions.end();++it)
		{
			const std::string& name=it->first;
			const OptionDescription& od=it->second;
			output << "  " << name << std::string(max_option_name_length+2-name.size(), ' ');
			output << od.argument_type << std::string(max_argument_type_length+2-od.argument_type.size(), ' ');
			output << (od.required ? "*" : " ") << "  ";
			output << od.description_text << "\n";
		}
	}

private:
	std::vector<std::string> original_argv_;
	std::vector<std::string> unused_argv_;
	std::map<std::string, std::string> options_;
};

}

#endif /* AUXILIARIES_PROGRAM_OPTIONS_HANDLER_H_ */
