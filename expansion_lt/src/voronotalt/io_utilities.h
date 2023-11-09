#ifndef VORONOTALT_IO_UTILITIES_H_
#define VORONOTALT_IO_UTILITIES_H_

#include <iostream>
#include <cstdlib>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace voronotalt
{

inline bool read_double_values_from_text_stream(std::istream& input, std::vector<double>& values)
{
	values.clear();

	std::istreambuf_iterator<char> input_eos;
	std::string input_data(std::istreambuf_iterator<char>(input), input_eos);

	if(!input_data.empty())
	{
		bool read_in_parallel=false;
#ifdef _OPENMP
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

					#pragma omp parallel
					{
						#pragma omp for schedule(static,1)
						for(int i=0;i<n_threads;i++)
						{
							if(thread_data_starts[i]<input_data_size)
							{
								const char* data=input_data.c_str()+thread_data_starts[i];
								const char* end=input_data.c_str()+((i+1)<n_threads ? std::min(thread_data_starts[i+1], input_data_size) : input_data_size);
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
	}

	return (!values.empty());
}

}

#endif /* VORONOTALT_IO_UTILITIES_H_ */
