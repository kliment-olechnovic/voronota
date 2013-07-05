#ifndef AUXILIARIES_IO_UTILITIES__H_
#define AUXILIARIES_IO_UTILITIES__H_

#include <iostream>
#include <sstream>
#include <vector>

namespace auxiliaries
{

template<typename LineReader, typename Container>
inline void read_lines_to_container(std::istream& input, const std::string& comments_marker, LineReader line_reader, Container& container)
{
	while(input.good())
	{
		std::string line;
		std::getline(input, line);
		if(!comments_marker.empty())
		{
			line.substr(0, line.find(comments_marker, 0));
		}
		if(!line.empty())
		{
			std::istringstream line_input(line);
			line_reader(line_input, container);
		}
	}
}

}

#endif /* AUXILIARIES_IO_UTILITIES__H_ */
