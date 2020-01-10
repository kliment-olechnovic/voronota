#ifndef AUXILIARIES_IO_UTILITIES__H_
#define AUXILIARIES_IO_UTILITIES__H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>

#include "../compatability/tr1_usage.h"

#if USE_TR1 > 0
#include <tr1/type_traits>
#else
#include <type_traits>
#endif

namespace voronota
{

namespace auxiliaries
{

class IOUtilities
{
public:
	bool tail_line_delimiter;
	char line_delimiter;
	char internal_separator;
	std::string block_terminator;

	IOUtilities() :
		tail_line_delimiter(true),
		line_delimiter('\n'),
		internal_separator(' ')
	{
	}

	explicit IOUtilities(const char line_delimiter) :
			tail_line_delimiter(false),
			line_delimiter(line_delimiter),
			internal_separator(' ')
	{
	}

	IOUtilities(
			const char line_delimiter,
			const char internal_separator) :
				tail_line_delimiter(false),
				line_delimiter(line_delimiter),
				internal_separator(internal_separator)
	{
	}

	IOUtilities(
			const bool tail_line_delimiter,
			const char line_delimiter,
			const char internal_separator) :
				tail_line_delimiter(tail_line_delimiter),
				line_delimiter(line_delimiter),
				internal_separator(internal_separator)
	{
	}

	IOUtilities(
			const bool tail_line_delimiter,
			const char line_delimiter,
			const char internal_separator,
			const std::string& block_terminator) :
				tail_line_delimiter(tail_line_delimiter),
				line_delimiter(line_delimiter),
				internal_separator(internal_separator),
				block_terminator(block_terminator)
	{
	}

	template<typename LineReader, typename Container>
	void read_lines_to_container(
			std::istream& input,
			LineReader line_reader,
			Container& container) const
	{
		while(input.good())
		{
			std::string line;
			std::getline(input, line, line_delimiter);
			if(!line.empty())
			{
				if(!block_terminator.empty() && line==block_terminator)
				{
					return;
				}
				{
					const std::size_t comments_pos=line.find("#", 0);
					if(comments_pos!=std::string::npos)
					{
						line=line.substr(0, comments_pos);
					}
				}
				if(!line.empty())
				{
					for(std::size_t i=0;i<line.size();i++)
					{
						if(line[i]==internal_separator)
						{
							line[i]=' ';
						}
					}
					std::istringstream line_input(line);
					if(!line_reader(line_input, container))
					{
						throw std::runtime_error(std::string("Failed to read line '")+line+"'.");
					}
				}
			}
		}
	}

	template<typename LineReader, typename Container>
	void read_file_lines_to_container(
			const std::string& filename,
			LineReader line_reader,
			Container& container) const
	{
		if(!filename.empty())
		{
			std::ifstream finput(filename.c_str(), std::ios::in);
			if(finput.good())
			{
				read_lines_to_container(finput, line_reader, container);
			}
		}
	}

	template<typename LineReader, typename Container>
	void read_string_lines_to_container(
			const std::string& str,
			LineReader line_reader,
			Container& container) const
	{
		if(!str.empty())
		{
			std::istringstream input(str);
			read_lines_to_container(input, line_reader, container);
		}
	}

	template<typename Container, typename ElementWriter>
	void write_container(
			const Container& container,
			ElementWriter element_writer,
			std::ostream& output) const
	{
		for(typename Container::const_iterator it=container.begin();it!=container.end() && output.good();++it)
		{
			if(it!=container.begin())
			{
				output << line_delimiter;
			}
			element_writer(*it, output);
		}
		if(tail_line_delimiter && !container.empty())
		{
			output << line_delimiter;
		}
	}

	template<typename Container, typename ElementWriter>
	void write_container_to_file(
			const Container& container,
			ElementWriter element_writer,
			const std::string& filename) const
	{
		if(!filename.empty())
		{
			std::ofstream foutput(filename.c_str(), std::ios::out);
			if(foutput.good())
			{
				write_container(container, element_writer, foutput);
			}
		}
	}

	template<typename Container, typename ElementWriter>
	std::string write_container_to_string(
			const Container& container,
			ElementWriter element_writer) const
	{
		std::ostringstream output;
		write_container(container, element_writer, output);
		return output.str();
	}

	template<typename Container>
	void read_lines_to_set(std::istream& input, Container& container) const
	{
		read_lines_to_container(input, read_line_to_set<Container>, container);
	}

	template<typename Container>
	Container read_lines_to_set(std::istream& input) const
	{
		Container container;
		read_lines_to_set(input, container);
		return container;
	}

	template<typename Container>
	void read_file_lines_to_set(const std::string& filename, Container& container) const
	{
		read_file_lines_to_container(filename, read_line_to_set<Container>, container);
	}

	template<typename Container>
	Container read_file_lines_to_set(const std::string& filename) const
	{
		Container container;
		read_file_lines_to_set(filename, container);
		return container;
	}

	template<typename Container>
	void read_string_lines_to_set(const std::string& str, Container& container) const
	{
		read_string_lines_to_container(str, read_line_to_set<Container>, container);
	}

	template<typename Container>
	Container read_string_lines_to_set(const std::string& str) const
	{
		Container container;
		read_string_lines_to_set(str, container);
		return container;
	}

	template<typename Container>
	void write_set(const Container& container, std::ostream& output) const
	{
		write_container(container, write_set_element<Container>, output);
	}

	template<typename Container>
	void write_set_to_file(const Container& container, const std::string& filename) const
	{
		write_container_to_file(container, write_set_element<Container>, filename);
	}

	template<typename Container>
	std::string write_set_to_string(const Container& container) const
	{
		return write_container_to_string(container, write_set_element<Container>);
	}

	template<typename Container>
	void read_lines_to_map(std::istream& input, Container& container) const
	{
		read_lines_to_container(input, read_line_to_map<Container>, container);
	}

	template<typename Container>
	Container read_lines_to_map(std::istream& input) const
	{
		Container container;
		read_lines_to_map(input, container);
		return container;
	}

	template<typename Container>
	void read_file_lines_to_map(const std::string& filename, Container& container) const
	{
		read_file_lines_to_container(filename, read_line_to_map<Container>, container);
	}

	template<typename Container>
	Container read_file_lines_to_map(const std::string& filename) const
	{
		Container container;
		read_file_lines_to_map(filename, container);
		return container;
	}

	template<typename Container>
	void read_string_lines_to_map(const std::string& str, Container& container) const
	{
		read_string_lines_to_container(str, read_line_to_map<Container>, container);
	}

	template<typename Container>
	Container read_string_lines_to_map(const std::string& str) const
	{
		Container container;
		read_string_lines_to_map(str, container);
		return container;
	}


	template<typename Container>
	void write_map(const Container& container, std::ostream& output) const
	{
		write_container(container, write_map_element<Container>(internal_separator), output);
	}

	template<typename Container>
	void write_map_to_file(const Container& container, const std::string& filename) const
	{
		write_container_to_file(container, write_map_element<Container>(internal_separator), filename);
	}

	template<typename Container>
	std::string write_map_to_string(const Container& container) const
	{
		return write_container_to_string(container, write_map_element<Container>(internal_separator));
	}

private:
	template<typename Container>
	static inline bool read_line_to_set(std::istream& input, Container& container)
	{
		typename Container::value_type value;
		input >> value;
		if(input.fail())
		{
			return false;
		}
		container.insert(container.end(), value);
		return true;
	}

	template<typename Container>
	static inline void write_set_element(const typename Container::value_type& value, std::ostream& output)
	{
		output << value;
	}

	template<typename Container>
	static inline bool read_line_to_map(std::istream& input, Container& container)
	{
#if USE_TR1 > 0
typename std::tr1::remove_const<typename Container::value_type::first_type>::type key;
#else
typename std::remove_const<typename Container::value_type::first_type>::type key;
#endif

		typename Container::value_type::second_type value;
		input >> key >> value;
		if(input.fail())
		{
			return false;
		}
		container.insert(container.end(), std::make_pair(key, value));
		return true;
	}

	template<typename Container>
	struct write_map_element
	{
		char pair_separator;

		explicit write_map_element(const char pair_separator) : pair_separator(pair_separator)
		{
		}

		inline void operator()(const typename Container::value_type& value, std::ostream& output)
		{
			output << value.first << pair_separator << value.second;
		}
	};
};

}

}

#endif /* AUXILIARIES_IO_UTILITIES__H_ */
