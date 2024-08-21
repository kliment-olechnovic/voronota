#ifndef VORONOTALT_IO_UTILITIES_H_
#define VORONOTALT_IO_UTILITIES_H_

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>

#ifdef VORONOTALT_OPENMP
#include <omp.h>
#endif

namespace voronotalt
{

inline bool read_double_values_from_text_string(const std::string& input_data, std::vector<double>& values) noexcept
{
	const std::size_t initial_number_of_values=values.size();

	if(!input_data.empty())
	{
		bool read_in_parallel=false;
#ifdef VORONOTALT_OPENMP
		if(input_data.size()>100000)
		{
			const int n_threads=omp_get_max_threads();
			if(n_threads>1)
			{
				const int input_data_size=static_cast<int>(input_data.size());
				const int approximate_portion_size=(input_data_size/n_threads);
				if(approximate_portion_size>1000)
				{
					std::vector< std::vector<double> > thread_values(n_threads);
					std::vector<int> thread_data_starts(n_threads, 0);
					for(int i=0;i<n_threads;i++)
					{
						thread_values[i].reserve(approximate_portion_size/5);
						int thread_data_start=(i>0 ? std::max(approximate_portion_size*i, thread_data_starts[i-1]+1) : 0);
						while(i!=0 && thread_data_start<input_data_size && input_data[thread_data_start]>' ')
						{
							thread_data_start++;
						}
						thread_data_starts[i]=thread_data_start;
					}

					{
						#pragma omp parallel for schedule(static,1)
						for(int i=0;i<n_threads;i++)
						{
							if(thread_data_starts[i]<input_data_size)
							{
								const char* data=input_data.c_str()+thread_data_starts[i];
								const char* end=input_data.c_str()+((i+1)<n_threads ? std::min(thread_data_starts[i+1], input_data_size) : input_data_size);
								while(data<end)
								{
									char* next=0;
									const double value=std::strtod(data, &next);
									if(data==next)
									{
										++data;
									}
									else
									{
										thread_values[i].push_back(value);
										data=next;
									}
								}
							}
						}
					}

					std::size_t total_values_count=0;

					for(std::size_t i=0;i<thread_values.size();i++)
					{
						total_values_count+=thread_values[i].size();
					}

					values.reserve(total_values_count);

					for(std::size_t i=0;i<thread_values.size();i++)
					{
						values.insert(values.end(), thread_values[i].begin(), thread_values[i].end());
					}

					read_in_parallel=true;
				}
			}
		}
#endif
		if(!read_in_parallel)
		{
			const char* data=input_data.c_str();
			const char* end=data+input_data.size();
			values.reserve(input_data.size()/5);
			while(data<end)
			{
				char* next=0;
				const double value=std::strtod(data, &next);
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
	}

	return (values.size()>initial_number_of_values);
}

inline bool read_double_values_from_text_stream(std::istream& input, std::vector<double>& values) noexcept
{
	std::istreambuf_iterator<char> input_eos;
	std::string input_data(std::istreambuf_iterator<char>(input), input_eos);

	return read_double_values_from_text_string(input_data, values);
}

inline bool read_lines_from_text_stream(std::istream& input, std::vector<std::string>& lines) noexcept
{
	const std::size_t initial_number_of_lines=lines.size();

	while(input.good())
	{
		std::string line;
		std::getline(input, line);
		if(!line.empty())
		{
			std::size_t last_content_pos=line.find_last_not_of(" \t\n\r\f\v");
			if(last_content_pos!=std::string::npos)
			{
				if(last_content_pos+1<line.size())
				{
					line=line.substr(0, last_content_pos+1);
				}
				lines.push_back(line);
			}
		}
	}

	return (lines.size()>initial_number_of_lines);
}

inline bool read_string_ids_and_double_values_from_text_stream(const std::size_t number_of_double_values_per_line, std::istream& input, std::vector<std::string>& string_ids, std::vector<double>& values) noexcept
{
	string_ids.clear();
	values.clear();

	std::string first_line;

	do
	{
		std::getline(input, first_line);
	}
	while(first_line.empty() && input.good());

	if(first_line.empty())
	{
		return false;
	}

	std::vector<std::string> first_line_tokens;

	{
		std::istringstream lineinput(first_line);
		std::string token;
		while(lineinput.good())
		{
			token.clear();
			lineinput >> token;
			if(!lineinput.fail() && !token.empty())
			{
				first_line_tokens.push_back(token);
			}
		}
	}

	const std::size_t number_of_tokens_in_first_line=first_line_tokens.size();

	if(number_of_tokens_in_first_line<number_of_double_values_per_line)
	{
		return false;
	}

	if(number_of_tokens_in_first_line==number_of_double_values_per_line)
	{
		const bool valid_values_read=(read_double_values_from_text_string(first_line, values) && read_double_values_from_text_stream(input, values) && values.size()%number_of_double_values_per_line==0);
		if(!valid_values_read)
		{
			values.clear();
		}
		return valid_values_read;
	}

	const std::size_t number_of_string_ids_per_line=(number_of_tokens_in_first_line-number_of_double_values_per_line);

	const bool string_ids_tailing=(first_line_tokens[number_of_double_values_per_line]=="#");

	std::vector<std::string> lines;
	lines.push_back(first_line);
	read_lines_from_text_stream(input, lines);

	string_ids.resize(lines.size()*number_of_string_ids_per_line);
	values.resize(lines.size()*number_of_double_values_per_line, 0.0);

	std::vector<int> failures(lines.size(), 0);

	{
		#pragma omp parallel
		{
			#pragma omp for
			for(std::size_t i=0;i<lines.size();i++)
			{
				std::istringstream lineinput(lines[i]);
				for(int block=0;block<2;block++)
				{
					if((block==0 && !string_ids_tailing) || (block==1 && string_ids_tailing))
					{
						for(std::size_t j=0;j<number_of_string_ids_per_line;j++)
						{
							lineinput >> string_ids[i*number_of_string_ids_per_line+j];
							if(lineinput.fail())
							{
								failures[i]++;
							}
						}
					}
					else
					{
						for(std::size_t j=0;j<number_of_double_values_per_line;j++)
						{
							lineinput >> values[i*number_of_double_values_per_line+j];
							if(lineinput.fail())
							{
								failures[i]++;
							}
						}
					}
				}
			}
		}
	}

	int total_failures=0;
	for(std::size_t i=0;i<failures.size();i++)
	{
		total_failures+=failures[i];
	}

	if(total_failures>0)
	{
		string_ids.clear();
		values.clear();
	}

	return (total_failures==0);
}

}

#endif /* VORONOTALT_IO_UTILITIES_H_ */
