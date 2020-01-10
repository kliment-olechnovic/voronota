#ifndef COMMON_STATISTICS_UTILITIES_H_
#define COMMON_STATISTICS_UTILITIES_H_

#include <cmath>
#include <iostream>

namespace voronota
{

namespace common
{

struct ValueStat
{
	double sum;
	double sum_of_squares;
	unsigned long count;

	ValueStat() : sum(0.0), sum_of_squares(0.0), count(0)
	{
	}

	void add(const double value)
	{
		sum+=value;
		sum_of_squares+=(value*value);
		count++;
	}

	double mean() const
	{
		return (sum/static_cast<double>(count));
	}

	double sd() const
	{
		return sqrt((sum_of_squares/static_cast<double>(count))-(mean()*mean()));
	}
};

struct NormalDistributionParameters
{
	double mean;
	double sd;

	NormalDistributionParameters() : mean(0.0), sd(1.0)
	{
	}

	NormalDistributionParameters(const double mean, const double sd) : mean(mean), sd(sd)
	{
	}
};

inline std::ostream& operator<<(std::ostream& output, const NormalDistributionParameters& v)
{
	output << v.mean << " " << v.sd;
	return output;
}

inline std::istream& operator>>(std::istream& input, NormalDistributionParameters& v)
{
	input >> v.mean >> v.sd;
	return input;
}

}

}

#endif /* COMMON_STATISTICS_UTILITIES_H_ */
