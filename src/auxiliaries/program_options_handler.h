#ifndef AUXILIARIES_PROGRAM_OPTIONS_HANDLER_H_
#define AUXILIARIES_PROGRAM_OPTIONS_HANDLER_H_

#include <string>
#include <map>
#include <sstream>
#include <stdexcept>
#include <iostream>

namespace auxiliaries
{

class ProgramOptionsHandler
{
public:
	struct OptionDescription
	{
		bool with_argument;
		std::string description_text;

		OptionDescription() : with_argument(false)
		{
		}

		OptionDescription(const bool with_argument, const std::string& description_text) : with_argument(with_argument), description_text(description_text)
		{
		}
	};

	typedef std::map<std::string, OptionDescription> MapOfOptionDescriptions;

	ProgramOptionsHandler(const int argc, const char** argv)
	{
		for(int i=1;i<argc;i++)
		{
			const std::string str(argv[i]);
			if(str.find("--")==0)
			{
				options_[str]="";
			}
			else if(i>0)
			{
				const std::string prev_str(argv[i-1]);
				if(prev_str.find("--")==0)
				{
					options_[prev_str]=str;
				}
			}
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
			throw std::runtime_error(std::string("Missing command line argument for option '")+name+"'.");
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
			throw std::runtime_error(std::string("Invalid command line argument for option '")+name+"'.");
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

	void remove_option(const std::string& name)
	{
		options_.erase(name);
	}

	void compare_with_map_of_option_descriptions(const MapOfOptionDescriptions& map_of_option_descriptions) const
	{
		for(std::map<std::string, std::string>::const_iterator it=options_.begin();it!=options_.end();++it)
		{
			const std::string& option=it->first;
			MapOfOptionDescriptions::const_iterator jt=map_of_option_descriptions.find(option);
			if(jt==map_of_option_descriptions.end())
			{
				throw std::runtime_error(std::string("Unrecognized command line option '")+option+"'.");
			}
			else if(it->second.empty() && jt->second.with_argument)
			{
				throw std::runtime_error(std::string("Command line option '")+option+"' should have arguments.");
			}
			else if(!it->second.empty() && !jt->second.with_argument)
			{
				throw std::runtime_error(std::string("Command line option '")+option+"' cannot have arguments.");
			}
		}
	}

	static void print_map_of_option_descriptions(const MapOfOptionDescriptions& map_of_option_descriptions, std::ostream& output)
	{
		std::size_t max_option_name_length=50;
		for(MapOfOptionDescriptions::const_iterator it=map_of_option_descriptions.begin();it!=map_of_option_descriptions.end();++it)
		{
			max_option_name_length=std::max(max_option_name_length, it->first.size());
		}
		for(MapOfOptionDescriptions::const_iterator it=map_of_option_descriptions.begin();it!=map_of_option_descriptions.end();++it)
		{
			output << "  " << it->first << std::string(max_option_name_length+3-it->first.size(), ' ');
			output << (it->second.with_argument ? "ARG" : "   ") << "   ";
			output << it->second.description_text << "\n";
		}
	}

private:
	std::map<std::string, std::string> options_;
};

}

#endif /* AUXILIARIES_PROGRAM_OPTIONS_HANDLER_H_ */
