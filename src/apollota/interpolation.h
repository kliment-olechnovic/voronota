#ifndef APOLLOTA_INTERPOLATION_H_
#define APOLLOTA_INTERPOLATION_H_

#include <vector>

#include "basic_operations_on_points.h"

namespace apollota
{

std::vector<SimplePoint> interpolate_using_cubic_hermite_spline(
		const SimplePoint& a,
		const SimplePoint& b,
		const SimplePoint& c,
		const SimplePoint& d,
		const double k,
		const int steps)
{
	std::vector<SimplePoint> result(steps+1);
	const double step=1.0/static_cast<double>(steps);
	const SimplePoint mb=(c-a)*k;
	const SimplePoint mc=(d-b)*k;
	for(int i=0;i<=steps;i++)
	{
		const double t=(i<steps ? static_cast<double>(i)*step : 1.0);
		const double h00=(2*t*t*t)-(3*t*t)+1;
		const double h10=(t*t*t)-(2*t*t)+t;
		const double h01=0-(2*t*t*t)+(3*t*t);
		const double h11=(t*t*t)-(t*t);
		result[i]=((b*h00)+(mb*h10)+(c*h01)+(mc*h11));
	}
	return result;
}

double binomial_coefficient(const unsigned long n, const unsigned long k)
{
	if(k==0 || k==n)
	{
		return 1.0;
	}
	else if(k==1)
	{
		return static_cast<double>(n);
	}
	else if(k>n)
	{
		return 0.0;
	}
	else if((n-k)<k)
	{
		return binomial_coefficient(n, n-k);
	}
	else
	{
		double result=1.0;
		for(unsigned long i=1;i<=k;i++)
		{
			result*=static_cast<double>(n-k+i)/static_cast<double>(i);
		}
		return result;
	}
}

double power(const double x, const unsigned long n)
{
	double result=1.0;
	for(unsigned long i=0;i<n;i++)
	{
		result*=x;
	}
	return result;
}

double bernstein_polynomial(const unsigned long n, const unsigned long k, const double t)
{
	if(k>n)
	{
		return 0.0;
	}
	else
	{
		return (binomial_coefficient(n, k)*power(t, k)*power(1-t, n-k));
	}
}

SimplePoint bezier_curve_point(const std::vector<SimplePoint>& controls, const double t)
{
	SimplePoint result;
	for(std::size_t i=0;i<controls.size();i++)
	{
		result=result+(controls[i]*bernstein_polynomial(controls.size()-1, i, t));
	}
	return result;
}

}

#endif /* APOLLOTA_INTERPOLATION_H_ */
