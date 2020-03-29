#ifndef NNPORT_READ_VECTOR_H_
#define NNPORT_READ_VECTOR_H_

#include <sstream>
#include <vector>

namespace voronota
{

namespace nnport
{

template<typename T>
std::vector<T> read_vector(const std::string& line, const std::size_t expected_size=std::string::npos)
{
	std::vector<T> input_vector;
	if(!line.empty() && line.find_first_not_of(" \t\n")!=std::string::npos)
	{
		std::istringstream line_input(line);
		input_vector.clear();
		while(line_input.good())
		{
			T value;
			line_input >> value;
			if(line_input.fail())
			{
				throw std::runtime_error(std::string("Failed to parse line '")+line+"'");
			}
			input_vector.push_back(value);
		}
		if(!input_vector.empty())
		{
			if(expected_size!=std::string::npos && input_vector.size()!=expected_size)
			{
				throw std::runtime_error(std::string("Invalid input dimensions in line '")+line+"'");
			}
		}
	}
	return input_vector;
}

template<typename T>
std::vector<T> read_vector(std::istream& input_stream, const std::size_t expected_size=std::string::npos)
{
	if(!input_stream.good())
	{
		throw std::runtime_error(std::string("Invalid input stream"));
	}
	std::string line;
	std::getline(input_stream, line);
	return read_vector<T>(line, expected_size);
}

}

}

#endif /* NNPORT_READ_VECTOR_H_ */
