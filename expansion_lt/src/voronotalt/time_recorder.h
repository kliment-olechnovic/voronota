#ifndef VORONOTALT_TIME_RECORDER_H_
#define VORONOTALT_TIME_RECORDER_H_

#include <chrono>
#include <iostream>
#include <vector>
#include <string>

namespace voronotalt
{

class TimeRecorder
{
public:
	TimeRecorder(const bool enabled) : enabled_(enabled)
	{
		reset();
	}

	void reset()
	{
		if(!enabled_)
		{
			return;
		}
		start_=std::chrono::steady_clock::now();
	}

	double get_elapsed_miliseconds() const
	{
		if(!enabled_)
		{
			return 0.0;
		}
		const std::chrono::time_point<std::chrono::steady_clock> end=std::chrono::steady_clock::now();
		const std::chrono::duration<double, std::milli> diff=end-start_;
		return diff.count();
	}

	double get_elapsed_miliseconds_and_reset()
	{
		if(!enabled_)
		{
			return 0.0;
		}
		const double value=get_elapsed_miliseconds();
		reset();
		return value;
	}

	void record_elapsed_miliseconds_and_reset(const std::string& message)
	{
		if(!enabled_)
		{
			return;
		}
		recordings_.push_back(std::pair<std::string, double>(message, get_elapsed_miliseconds_and_reset()));
	}

	void print_recordings(std::ostream& output, const std::string& prefix)
	{
		if(!enabled_)
		{
			return;
		}
		for(std::size_t i=0;i<recordings_.size();i++)
		{
			output << prefix << " '" << recordings_[i].first << "' = " << recordings_[i].second << " ms\n";
		}
	}

private:
	bool enabled_;
	std::chrono::time_point<std::chrono::steady_clock> start_;
	std::vector< std::pair<std::string, double> > recordings_;
};

}

#endif /* VORONOTALT_TIME_RECORDER_H_ */
