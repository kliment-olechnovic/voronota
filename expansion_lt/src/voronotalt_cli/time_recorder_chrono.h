#ifndef VORONOTALT_TIME_RECORDER_CHRONO_H_
#define VORONOTALT_TIME_RECORDER_CHRONO_H_

#include "../voronotalt/time_recorder.h"

#include <chrono>
#include <iostream>
#include <vector>
#include <string>

namespace voronotalt
{

class TimeRecorderChrono : public TimeRecorder
{
public:
	explicit TimeRecorderChrono(const bool enabled) noexcept : enabled_(enabled)
	{
		reset();
	}

	void reset() noexcept override
	{
		if(!enabled_)
		{
			return;
		}
		start_=std::chrono::steady_clock::now();
	}

	void record_elapsed_miliseconds_and_reset(const char* message) noexcept override
	{
		if(!enabled_ || message==0)
		{
			return;
		}
		recordings_.push_back(std::pair<std::string, double>(std::string(message), get_elapsed_miliseconds_and_reset()));
	}

	double get_elapsed_miliseconds() const noexcept
	{
		if(!enabled_)
		{
			return 0.0;
		}
		const std::chrono::time_point<std::chrono::steady_clock> end=std::chrono::steady_clock::now();
		const std::chrono::duration<double, std::milli> diff=end-start_;
		return diff.count();
	}

	double get_elapsed_miliseconds_and_reset() noexcept
	{
		if(!enabled_)
		{
			return 0.0;
		}
		const double value=get_elapsed_miliseconds();
		reset();
		return value;
	}

	void print_elapsed_time(std::ostream& output, const std::string& prefix) const noexcept
	{
		if(!enabled_)
		{
			return;
		}
		output << string_without_whitespaces(prefix) << "_elapsed\t" << get_elapsed_miliseconds() << "\n";
	}

	void print_recordings(std::ostream& output, const std::string& prefix, const bool with_sum) const noexcept
	{
		if(!enabled_)
		{
			return;
		}
		const std::string prefix_without_whitespaces=string_without_whitespaces(prefix);
		for(std::size_t i=0;i<recordings_.size();i++)
		{
			output << prefix_without_whitespaces << "__" << string_without_whitespaces(recordings_[i].first) << "\t" << recordings_[i].second << "\n";
		}
		if(with_sum && recordings_.size()>1)
		{
			double sum=0.0;
			for(std::size_t i=0;i<recordings_.size();i++)
			{
				sum+=recordings_[i].second;
			}
			output << prefix_without_whitespaces << "_total_sum\t" << sum << "\n";
		}
	}

private:
	static std::string string_without_whitespaces(const std::string& input) noexcept
	{
		std::string output=input;
		for(std::size_t i=0;i<output.size();i++)
		{
			if(output[i]<=' ')
			{
				output[i]='_';
			}
		}
		return output;
	}

	bool enabled_;
	std::chrono::time_point<std::chrono::steady_clock> start_;
	std::vector< std::pair<std::string, double> > recordings_;
};

}

#endif /* VORONOTALT_TIME_RECORDER_CHRONO_H_ */
