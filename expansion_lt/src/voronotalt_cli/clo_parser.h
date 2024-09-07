#ifndef VORONOTALT_CLO_PARSER_H_
#define VORONOTALT_CLO_PARSER_H_

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

namespace voronotalt
{

class CLOParser
{
public:
	struct Option
	{
		std::string name;
		std::vector<std::string> args_strings;
		std::vector<double> args_doubles;
		std::vector<int> args_ints;
		std::vector<unsigned int> args_hexints;

		bool is_flag() const noexcept
		{
			return (args_strings.empty() || (args_strings.size()==1 && (args_strings.front()=="true" || args_strings.front()=="false")));
		}

		bool is_flag_and_true() const noexcept
		{
			return (args_strings.empty() || (args_strings.size()==1 && args_strings.front()=="true"));
		}
	};

	static void print_options(const std::vector<Option>& options, std::ostream& output) noexcept
	{
		output << "Command Line Options (" << options.size() << "):\n";
		for(std::size_t i=0;i<options.size();i++)
		{
			const Option& option=options[i];

			output << "option '" << option.name << "' ";

			output << "; arguments as strings (" << option.args_strings.size() << ")";
			for(std::size_t j=0;j<option.args_strings.size();j++)
			{
				output << (j==0 ? ": ['" : ", '") << option.args_strings[j] << ((j+1)==option.args_strings.size() ? "']" : "'");
			}

			output << "; arguments as doubles (" << option.args_doubles.size() << ")";
			for(std::size_t j=0;j<option.args_doubles.size();j++)
			{
				output << (j==0 ? ": ['" : ", '") << option.args_doubles[j] << ((j+1)==option.args_doubles.size() ? "']" : "'");
			}

			output << "; arguments as ints (" << option.args_ints.size() << ")";
			for(std::size_t j=0;j<option.args_ints.size();j++)
			{
				output << (j==0 ? ": ['" : ", '") << option.args_ints[j] << ((j+1)==option.args_ints.size() ? "']" : "'");
			}

			output << "\n";
		}
	}

	static std::vector<Option> read_options(const int argc, const char** argv) noexcept
	{
		std::vector<Option> options;

		for(int i=1;i<argc;i++)
		{
			const std::string value_string(argv[i]);
			if(value_string.size()>=2 && value_string[0]=='-' && value_string[1]!='-' && (!(static_cast<int>(value_string[1])>=static_cast<int>('0') && static_cast<int>(value_string[1])<=static_cast<int>('9'))))
			{
				Option option;
				option.name=value_string.substr(1);
				options.push_back(option);
			}
			else if(value_string.size()>=3 && value_string[0]=='-' && value_string[1]=='-' && value_string[2]!='-')
			{
				Option option;
				option.name=value_string.substr(2);
				options.push_back(option);
			}
			else
			{
				if(options.empty())
				{
					options.push_back(Option());
				}
				options.back().args_strings.push_back(value_string);
			}
		}

		for(std::size_t i=0;i<options.size();i++)
		{
			Option& option=options[i];
			bool cancel=false;
			for(std::size_t j=0;j<option.args_strings.size() && !cancel;j++)
			{
				std::istringstream input(option.args_strings[j]);
				double value=0.0;
				input >> value;
				if(input.fail())
				{
					cancel=true;
				}
				else
				{
					option.args_doubles.push_back(value);
				}
			}
			if(cancel)
			{
				option.args_doubles.clear();
			}
		}

		for(std::size_t i=0;i<options.size();i++)
		{
			Option& option=options[i];
			bool cancel=false;
			for(std::size_t j=0;j<option.args_strings.size() && !cancel;j++)
			{
				std::istringstream input(option.args_strings[j]);
				int value=0;
				input >> value;
				if(input.fail())
				{
					cancel=true;
				}
				else
				{
					option.args_ints.push_back(value);
				}
			}
			if(cancel)
			{
				option.args_ints.clear();
			}
		}

		for(std::size_t i=0;i<options.size();i++)
		{
			Option& option=options[i];
			bool cancel=false;
			for(std::size_t j=0;j<option.args_strings.size() && !cancel;j++)
			{
				const std::string& value_string=option.args_strings[j];
				if(value_string.rfind("0x", 0)==0 && value_string.find_first_not_of("0123456789ABCDEF", 2)==std::string::npos)
				{
					std::istringstream input(value_string);
					unsigned int value=0;
					input >> std::hex >> value;
					if(input.fail())
					{
						cancel=true;
					}
					else
					{
						option.args_hexints.push_back(value);
					}
				}
				else
				{
					cancel=true;
				}
			}
			if(cancel)
			{
				option.args_hexints.clear();
			}
		}

		return options;
	}
};

}

#endif /* VORONOTALT_CLO_PARSER_H_ */
