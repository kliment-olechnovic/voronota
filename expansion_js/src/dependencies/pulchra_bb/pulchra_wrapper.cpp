#include <vector>

#include "pulchra_wrapper.h"

int run_pulchra_bb(char* input_buffer, const size_t input_buffer_size, char** output_buffer, size_t* output_buffer_size);

namespace PULCHRAWrapper
{

int run_pulchra(const std::string& input, std::ostream& output)
{
	std::vector<char> input_buffer(input.size());
	std::copy(input.begin(), input.end(), &input_buffer[0]);
	char* output_buffer=0;
	size_t output_buffer_size=0;
	int result_code=run_pulchra_bb(&input_buffer[0], input_buffer.size(), &output_buffer, &output_buffer_size);
	if(result_code==0)
	{
		if(output_buffer!=0 && output_buffer_size>0)
		{
			output.write(output_buffer, output_buffer_size);
		}
		else
		{
			result_code=-2;
		}
	}
	if(output_buffer_size>0)
	{
		free(output_buffer);
	}
	return result_code;
}

}

