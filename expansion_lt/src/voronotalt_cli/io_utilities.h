#ifndef VORONOTALT_IO_UTILITIES_H_
#define VORONOTALT_IO_UTILITIES_H_

#include <string>
#include <vector>
#include <cstdlib>
#include <cctype>
#include <cstring>

#ifdef VORONOTALT_OPENMP
#include <omp.h>
#endif

namespace voronotalt
{

inline bool read_double_values_from_text_string(const std::string& input_data, std::vector<double>& values) noexcept
{
	values.clear();

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

	return (!values.empty());
}

inline bool read_non_empty_lines_from_text_string(const std::string& input_data, const std::size_t max_num_of_lines, std::vector< std::pair<std::size_t, std::size_t> >& line_ranges) noexcept
{
	line_ranges.clear();

	if(!input_data.empty())
	{
		std::size_t start=input_data.find_first_not_of(" \t\v\n\r");
		while(start!=std::string::npos)
		{
			std::size_t end_of_line=input_data.find_first_of("\n\r", start);
			std::size_t next_start=std::string::npos;
			if(end_of_line==std::string::npos)
			{
				end_of_line=input_data.size();
			}
			else
			{
				next_start=input_data.find_first_not_of(" \n\r\t\v", end_of_line);
			}
			while(end_of_line>start && std::isspace(static_cast<unsigned char>(input_data[end_of_line-1])))
			{
				end_of_line--;
			}
			if(end_of_line>start)
			{
				line_ranges.push_back(std::pair<std::size_t, std::size_t>(start, end_of_line));
				if(max_num_of_lines>0 && line_ranges.size()==max_num_of_lines)
				{
					return true;
				}
			}
			start=next_start;
		}
	}

	return (!line_ranges.empty());
}

bool read_token_from_text_string(const std::string& input_data, const std::pair<std::size_t, std::size_t>& search_range, std::pair<std::size_t, std::size_t>& token_range) noexcept
{
	if(search_range.first<search_range.second && search_range.first<input_data.size())
	{
		const std::size_t start=search_range.first+std::strspn(&input_data[search_range.first], " \t\v\n\r");
		const std::size_t end=std::min(start+std::strcspn(&input_data[start], " \t\v\n\r"), search_range.second);
		if(end>start)
		{
			token_range.first=start;
			token_range.second=end;
			return true;
		}
	}
	return false;
}

inline bool read_string_ids_and_double_values_from_text_string(const std::size_t number_of_double_values_per_line, const std::string& input_data, std::vector<std::string>& string_ids, std::vector<double>& values) noexcept
{
	string_ids.clear();
	values.clear();

	std::vector<std::string> first_line_tokens;

	{
		std::vector< std::pair<std::size_t, std::size_t> > first_line_ranges;
		if(!read_non_empty_lines_from_text_string(input_data, 1, first_line_ranges))
		{
			return false;
		}
		std::pair<std::size_t, std::size_t> search_range=first_line_ranges[0];
		std::pair<std::size_t, std::size_t> token_range;
		while(read_token_from_text_string(input_data, search_range, token_range))
		{
			search_range.first=token_range.second;
			first_line_tokens.push_back(input_data.substr(token_range.first, token_range.second-token_range.first));
		}
	}

	const std::size_t number_of_tokens_in_first_line=first_line_tokens.size();

	if(number_of_tokens_in_first_line<number_of_double_values_per_line)
	{
		return false;
	}

	if(number_of_tokens_in_first_line==number_of_double_values_per_line)
	{
		return (read_double_values_from_text_string(input_data, values) && values.size()%number_of_double_values_per_line==0);
	}

	const std::size_t number_of_string_ids_per_line=(number_of_tokens_in_first_line-number_of_double_values_per_line);

	const bool string_ids_tailing=(first_line_tokens[number_of_double_values_per_line]=="#");

	std::vector< std::pair<std::size_t, std::size_t> > line_ranges;

	if(!read_non_empty_lines_from_text_string(input_data, 0, line_ranges))
	{
		return false;
	}

	string_ids.resize(line_ranges.size()*number_of_string_ids_per_line);
	values.resize(line_ranges.size()*number_of_double_values_per_line, 0.0);

	std::vector<int> failures(line_ranges.size(), 0);

	{
		#pragma omp parallel
		{
			#pragma omp for
			for(std::size_t i=0;i<line_ranges.size();i++)
			{
				std::pair<std::size_t, std::size_t> search_range=line_ranges[i];
				std::pair<std::size_t, std::size_t> token_range;

				for(int block=0;block<2 && failures[i]==0;block++)
				{
					if((block==0 && !string_ids_tailing) || (block==1 && string_ids_tailing))
					{
						for(std::size_t j=0;j<number_of_string_ids_per_line && failures[i]==0;j++)
						{
							if(read_token_from_text_string(input_data, search_range, token_range))
							{
								search_range.first=token_range.second;
								string_ids[i*number_of_string_ids_per_line+j]=input_data.substr(token_range.first, token_range.second-token_range.first);
							}
							else
							{
								failures[i]++;
							}
						}
					}
					else
					{
						for(std::size_t j=0;j<number_of_double_values_per_line && failures[i]==0;j++)
						{
							if(read_token_from_text_string(input_data, search_range, token_range))
							{
								search_range.first=token_range.second;
								const char* str=&input_data[token_range.first];
								char* str_end=0;
								const double value=std::strtod(str, &str_end);
								if(str==str_end)
								{
									failures[i]++;
								}
								else
								{
									values[i*number_of_double_values_per_line+j]=value;
								}
							}
							else
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
