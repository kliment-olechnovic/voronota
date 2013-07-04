#ifndef AUXILIARIES_COMMAND_LINE_OPTIONS_H_
#define AUXILIARIES_COMMAND_LINE_OPTIONS_H_

#include <string>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace auxiliaries
{

class CommandLineOptions
{
public:
	CommandLineOptions(const int argc, const char** argv)
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

	bool isopt(const std::string& name) const
	{
		return (options_.find(name)!=options_.end());
	}

	bool isarg(const std::string& name) const
	{
		return (isopt(name) && !options_.find(name)->second.empty());
	}

	template<typename T>
	T arg(const std::string& name) const
	{
		if(!isarg(name))
		{
			throw std::runtime_error(std::string("Missing command line argument: ")+name);
		}
		std::istringstream input(options_.find(name)->second);
		T value;
		input >> value;
		if(input.fail())
		{
			throw std::runtime_error(std::string("Invalid command line argument: ")+name);
		}
		return value;
	}

	template<typename T>
	std::vector<T> arg_vector(const std::string& name, const char delimiter) const
	{
		std::vector<T> result;
		if(isarg(name))
		{
			std::string contents=options_.find(name)->second;
			for(std::size_t i=0;i<contents.size();i++)
			{
				if(contents[i]==delimiter)
				{
					contents[i]=' ';
				}
			}
			std::istringstream input(contents);
			while(input.good())
			{
				T val;
				input >> val;
				if(input.fail())
				{
					throw std::runtime_error(std::string("Invalid command line argument list: ")+name);
				}
				result.push_back(val);
			}
		}
		if(result.empty())
		{
			throw std::runtime_error(std::string("Empty command line argument list: ")+name);
		}
		return result;
	}

	void check_allowed_options(const std::string& allowed_options) const
	{
		std::map<std::string, bool> allowed_options_map;
		if(!allowed_options.empty())
		{
			std::istringstream input(allowed_options);
			while(input.good())
			{
				std::string token;
				input >> token;
				const std::size_t arg_pos=token.find(":");
				allowed_options_map[token.substr(0, arg_pos)]=(arg_pos!=std::string::npos);
			}
		}
		for(std::map<std::string, std::string>::const_iterator it=options_.begin();it!=options_.end();++it)
		{
			const std::string& option=it->first;
			std::map<std::string, bool>::const_iterator jt=allowed_options_map.find(option);
			if(jt==allowed_options_map.end())
			{
				throw std::runtime_error(std::string("Unrecognized command line option: ")+option+" (allowed options: "+allowed_options+")");
			}
			else if((!it->second.empty())!=jt->second)
			{
				if(jt->second)
				{
					throw std::runtime_error(std::string("Command line option should have arguments: ")+option);
				}
				else
				{
					throw std::runtime_error(std::string("Command line option cannot have arguments: ")+option);
				}
			}
		}
	}

	void remove_option(const std::string& name)
	{
		options_.erase(name);
	}

private:
	std::map<std::string, std::string> options_;
};

}

#endif /* AUXILIARIES_COMMAND_LINE_OPTIONS_H_ */
