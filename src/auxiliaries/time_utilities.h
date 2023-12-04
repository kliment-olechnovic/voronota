#ifndef AUXILIARIES_TIME_UTILITIES_H_
#define AUXILIARIES_TIME_UTILITIES_H_

#if __cplusplus >= 201103L

#include <chrono>

namespace voronota
{

namespace auxiliaries
{

class ElapsedProcessorTime
{
public:
	ElapsedProcessorTime()
	{
		reset();
	}

	void reset()
	{
		start_=std::chrono::steady_clock::now();
	}

	double elapsed_miliseconds() const
	{
		const std::chrono::time_point<std::chrono::steady_clock> end=std::chrono::steady_clock::now();
		const std::chrono::duration<double, std::milli> diff=end-start_;
		return diff.count();
	}

private:
	std::chrono::time_point<std::chrono::steady_clock> start_;
};

}

}

#else

#include <ctime>

namespace voronota
{

namespace auxiliaries
{

class ElapsedProcessorTime
{
public:
	ElapsedProcessorTime()
	{
		reset();
	}

	void reset()
	{
		start_=std::clock();
	}

	double elapsed_miliseconds() const
	{
		const std::clock_t end=std::clock();
		return (1000.0*static_cast<double>(end-start_)/static_cast<double>(CLOCKS_PER_SEC));
	}

private:
	std::clock_t start_;
};

}

}

#endif

#endif /* AUXILIARIES_TIME_UTILITIES_H_ */
