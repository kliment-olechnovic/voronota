#ifndef COMMON_COMMAND_INPUT_UTILITIES_H_
#define COMMON_COMMAND_INPUT_UTILITIES_H_

#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace common
{

class CommandInputUtilities
{
public:
	class Guard
	{
	public:
		std::string token;
		bool token_validated;

		Guard() : token_validated(false)
		{
		}

		void on_iteration_start(std::istream& input)
		{
			input >> std::ws;
			input >> token;
			if(input.fail() || token.empty())
			{
				throw std::runtime_error(std::string("Missing command parameters."));
			}
		}

		void on_token_processed(std::istream& input)
		{
			if(input.fail())
			{
				if(!token.empty())
				{
					throw std::runtime_error(std::string("Invalid value for the command parameter '")+token+"'.");
				}
				else
				{
					throw std::runtime_error(std::string("Invalid command."));
				}
			}
			else
			{
				token_validated=true;
			}
		}

		void on_iteration_end(std::istream& input) const
		{
			if(!token_validated)
			{
				if(!token.empty())
				{
					throw std::runtime_error(std::string("Invalid command parameter '")+token+"'.");
				}
				else
				{
					throw std::runtime_error(std::string("Invalid command."));
				}
			}
			input >> std::ws;
		}
	};

	static void assert_absence_of_input(std::istream& input)
	{
		if((input >> std::ws).good())
		{
			throw std::runtime_error(std::string("No additional parameters allowed."));
		}
	}

	static void read_string_considering_quotes(std::istream& input, std::string& output, const bool allow_empty_value_in_quotes=false)
	{
		input >> std::ws;
		const int c=input.peek();
		if(c==std::char_traits<char>::to_int_type('"') || c==std::char_traits<char>::to_int_type('\''))
		{
			input.get();
			output.clear();
			std::getline(input, output, std::char_traits<char>::to_char_type(c));
			if(!allow_empty_value_in_quotes && output.empty())
			{
				throw std::runtime_error(std::string("Empty string in quotes."));
			}
		}
		else
		{
			input >> output;
		}
	}

	static void read_all_strings_considering_quotes(std::istream& input, std::vector<std::string>& output, const bool allow_empty_value_in_quotes=false)
	{
		std::vector<std::string> result;
		while((input >> std::ws).good())
		{
			std::string str;
			read_string_considering_quotes(input, str, allow_empty_value_in_quotes);
			if(!input.fail())
			{
				result.push_back(str);
			}
		}
		output.swap(result);
	}

	static unsigned int read_color_integer_from_string(const std::string& color_str)
	{
		unsigned int color_int=0;
		if(!color_str.empty())
		{
			std::istringstream color_input(color_str);
			color_input >> std::hex >> color_int;
			if(color_input.fail() || color_int>0xFFFFFF)
			{
				throw std::runtime_error(std::string("Invalid hex color string '")+color_str+"'.");
			}
		}
		return color_int;
	}
};

}

#endif /* COMMON_COMMAND_INPUT_UTILITIES_H_ */
