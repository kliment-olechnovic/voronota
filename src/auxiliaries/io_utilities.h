#ifndef AUXILIARIES_IO_UTILITIES__H_
#define AUXILIARIES_IO_UTILITIES__H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>

namespace auxiliaries
{

template<typename LineReader, typename Container>
inline void read_lines_to_container(
		std::istream& input,
		LineReader line_reader,
		Container& container)
{
	std::size_t line_num=0;
	while(input.good())
	{
		std::string line;
		std::getline(input, line);
		if(!line.empty())
		{
			{
				const std::size_t comments_pos=line.find("#", 0);
				if(comments_pos!=std::string::npos)
				{
					line=line.substr(0, comments_pos);
				}
			}
			if(!line.empty())
			{
				std::istringstream line_input(line);
				if(!line_reader(line_input, container))
				{
					throw std::runtime_error(std::string("Failed to read line '")+line+"'.");
				}
				line_num++;
			}
		}
	}
}

template<typename LineReader, typename Container>
inline void read_file_lines_to_container(
		const std::string& filename,
		LineReader line_reader,
		Container& container)
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

template<typename Container>
inline bool read_line_to_sequential_container(std::istream& input, Container& container)
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
inline void read_lines_to_sequential_container(std::istream& input, Container& container)
{
	read_lines_to_container(input, read_line_to_sequential_container<Container>, container);
}

template<typename Container>
inline void read_file_lines_to_sequential_container(const std::string& filename, Container& container)
{
	read_file_lines_to_container(filename, read_line_to_sequential_container<Container>, container);
}

template<typename Container>
inline bool read_line_to_map_container(std::istream& input, Container& container)
{
	typename Container::key_type key;
	typename Container::mapped_type value;
	input >> key >> value;
	if(input.fail())
	{
		return false;
	}
	container[key]=value;
	return true;
}

template<typename Container>
inline void read_lines_to_map_container(std::istream& input, Container& container)
{
	read_lines_to_container(input, read_line_to_map_container<Container>, container);
}

template<typename Container>
inline void read_file_lines_to_map_container(const std::string& filename, Container& container)
{
	read_file_lines_to_container(filename, read_line_to_map_container<Container>, container);
}

}

#endif /* AUXILIARIES_IO_UTILITIES__H_ */
