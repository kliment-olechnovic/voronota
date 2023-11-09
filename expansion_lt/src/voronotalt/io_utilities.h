#ifndef VORONOTALT_IO_UTILITIES_H_
#define VORONOTALT_IO_UTILITIES_H_

#include <iostream>
#include <cstdlib>
#include <vector>

namespace voronotalt
{

inline bool read_double_values_from_text_stream(std::istream& input, std::vector<double>& values)
{
	values.clear();

	std::istreambuf_iterator<char> input_eos;
	std::string input_data(std::istreambuf_iterator<char>(input), input_eos);

	if(!input_data.empty())
	{
		const char* data=input_data.c_str();
		const char* end=data+input_data.size();
		values.reserve(input_data.size()/6);
		while(data<end)
		{
			char* next=0;
			const double value=strtod(data, &next);
			if(data==next)
			{
				++data;
			}
			else
			{
				values.push_back(value);
				data=next;
			}
		}
	}

	return (!values.empty());
}

}

#endif /* VORONOTALT_IO_UTILITIES_H_ */
