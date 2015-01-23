#ifndef AUXILIARIES_IO_UTILITIES__H_
#define AUXILIARIES_IO_UTILITIES__H_

#include <iostream>
#include <sstream>
#include <list>
#include <set>
#include <map>
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

template<typename Container>
inline bool read_line_to_sequential_container(
		std::istream& input,
		Container& container)
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
inline void read_lines_to_sequential_container(
		std::istream& input,
		Container& container)
{
	read_lines_to_container(input, read_line_to_sequential_container<Container>, container);
}

}

#endif /* AUXILIARIES_IO_UTILITIES__H_ */
