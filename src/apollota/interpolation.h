#ifndef APOLLOTA_INTERPOLATION_H_
#define APOLLOTA_INTERPOLATION_H_

#include <vector>

#include "basic_operations_on_points.h"

namespace apollota
{

inline std::vector<SimplePoint> interpolate_using_cubic_hermite_spline(
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

inline double binomial_coefficient(const unsigned long n, const unsigned long k)
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

inline double power(const double x, const unsigned long n)
{
	double result=1.0;
	for(unsigned long i=0;i<n;i++)
	{
		result*=x;
	}
	return result;
}

inline double bernstein_polynomial(const unsigned long n, const unsigned long k, const double t)
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

inline SimplePoint bezier_curve_point(const std::vector<SimplePoint>& controls, const double t)
{
	SimplePoint result;
	for(std::size_t i=0;i<controls.size();i++)
	{
		result=result+(controls[i]*bernstein_polynomial(controls.size()-1, i, t));
	}
	return result;
}

inline SimplePoint bezier_curve_point_de_casteljau(const std::vector<SimplePoint>& controls, const double t)
{
	static std::vector<SimplePoint> buffer(4);
	if(buffer.size()<controls.size())
	{
		buffer.resize(controls.size());
	}
	for(std::size_t i=0;i<controls.size();i++)
	{
		buffer[i]=controls[i];
	}
	for(std::size_t j=0;j+1<controls.size();j++)
	{
		for(std::size_t i=0;i+1<(controls.size()-j);i++)
		{
			buffer[i]=(buffer[i]*(1-t))+(buffer[i+1]*t);
		}
	}
	return buffer.at(0);
}

inline std::vector<SimplePoint> bezier_curve_points(const std::vector<SimplePoint>& controls, unsigned int n, const bool de_casteljau=true)
{
	std::vector<SimplePoint> result(n+1);
	for(std::size_t i=0;i<(n+1);i++)
	{
		const double t=static_cast<double>(i)/static_cast<double>(n);
		result[i]=(de_casteljau ? bezier_curve_point_de_casteljau(controls, t) : bezier_curve_point(controls, t));
	}
	return result;
}

inline SimplePoint rational_bezier_curve_point(const std::vector<SimplePoint>& controls, const std::vector<double>& weights, const double t)
{
	const std::size_t size=std::min(controls.size(), weights.size());
	SimplePoint numerator;
	double denominator=0.0;
	for(std::size_t i=0;i<size;i++)
	{
		numerator=numerator+(controls[i]*(bernstein_polynomial(size-1, i, t)*weights[i]));
		denominator+=(bernstein_polynomial(size-1, i, t)*weights[i]);
	}
	return (numerator*(1.0/denominator));
}

inline std::vector<SimplePoint> rational_bezier_curve_points(const std::vector<SimplePoint>& controls, const std::vector<double>& weights, unsigned int n)
{
	std::vector<SimplePoint> result(n+1);
	for(std::size_t i=0;i<(n+1);i++)
	{
		const double t=static_cast<double>(i)/static_cast<double>(n);
		result[i]=rational_bezier_curve_point(controls, weights, t);
	}
	return result;
}

}

#endif /* APOLLOTA_INTERPOLATION_H_ */
