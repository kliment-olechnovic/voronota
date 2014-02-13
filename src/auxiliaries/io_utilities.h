#ifndef AUXILIARIES_IO_UTILITIES__H_
#define AUXILIARIES_IO_UTILITIES__H_

#include <iostream>
#include <sstream>
#include <list>

namespace auxiliaries
{

template<typename LineReader, typename Container>
inline void read_lines_to_container(
		std::istream& input,
		const std::string& comments_marker,
		LineReader line_reader,
		Container& container,
		std::list< std::pair<std::size_t, std::string> >* comments_list_ptr=0)
{
	std::size_t line_num=0;
	while(input.good())
	{
		std::string line;
		std::getline(input, line);
		if(!comments_marker.empty())
		{
			const std::size_t comments_pos=line.find(comments_marker, 0);
			if(comments_pos!=std::string::npos)
			{
				if(comments_list_ptr!=0)
				{
					comments_list_ptr->push_back(std::make_pair(line_num, line.substr(comments_pos)));
				}
				line=line.substr(0, comments_pos);
			}
		}
		if(!line.empty())
		{
			std::istringstream line_input(line);
			line_reader(line_input, container);
			line_num++;
		}
	}
}

}

#endif /* AUXILIARIES_IO_UTILITIES__H_ */
