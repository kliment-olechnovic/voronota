#ifndef APOLLOTA_POLYNOMIAL_CURVES_H_
#define APOLLOTA_POLYNOMIAL_CURVES_H_

#include <vector>
#include <list>

#include "basic_operations_on_points.h"

namespace voronota
{

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

inline double bernstein_polynomial(const unsigned long n, const unsigned long k, const double t)
{
	if(k>n)
	{
		return 0.0;
	}
	else
	{
		return (binomial_coefficient(n, k)*pow(t, k)*pow(1-t, n-k));
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

inline std::vector<SimplePoint> bezier_curve_points(const std::vector<SimplePoint>& controls, const unsigned int n, const bool de_casteljau=true)
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


inline SimplePoint rational_bezier_curve_point_de_casteljau(const std::vector<SimplePoint>& controls, const std::vector<double>& weights, const double t)
{
	const std::size_t size=std::min(controls.size(), weights.size());
	static std::vector<SimplePoint> buffer_of_points(4);
	static std::vector<double> buffer_of_weights(4);
	if(buffer_of_points.size()<size)
	{
		buffer_of_points.resize(size);
	}
	if(buffer_of_weights.size()<size)
	{
		buffer_of_weights.resize(size);
	}
	for(std::size_t i=0;i<size;i++)
	{
		buffer_of_points[i]=controls[i];
		buffer_of_weights[i]=weights[i];
	}
	for(std::size_t j=0;j+1<size;j++)
	{
		for(std::size_t i=0;i+1<(size-j);i++)
		{
			const double w=(buffer_of_weights[i]*(1-t))+(buffer_of_weights[i+1]*t);
			buffer_of_points[i]=(buffer_of_points[i]*((1-t)*(buffer_of_weights[i]/w)))+(buffer_of_points[i+1]*(t*(buffer_of_weights[i+1]/w)));
			buffer_of_weights[i]=w;
		}
	}
	return buffer_of_points.at(0);
}

inline std::vector<SimplePoint> rational_bezier_curve_points(const std::vector<SimplePoint>& controls, const std::vector<double>& weights, const unsigned int n, const bool de_casteljau=true)
{
	std::vector<SimplePoint> result(n+1);
	for(std::size_t i=0;i<(n+1);i++)
	{
		const double t=static_cast<double>(i)/static_cast<double>(n);
		result[i]=(de_casteljau ? rational_bezier_curve_point_de_casteljau(controls, weights, t) : rational_bezier_curve_point(controls, weights, t));
	}
	return result;
}

inline std::vector<SimplePoint> bezier_curve_using_explicit_de_casteljau_subdivision(const std::vector<SimplePoint>& controls, const int depth)
{
	const std::size_t dimensions=controls.size();
	std::list<SimplePoint> approximation(controls.begin(), controls.end());
	std::vector< std::list<SimplePoint>::iterator > subcontrols(dimensions);
	for(int level=0;level<depth;level++)
	{
		std::list<SimplePoint>::iterator it=approximation.begin();
		while(it!=approximation.end())
		{
			if(it!=approximation.begin())
			{
				--it;
			}
			for(std::size_t i=0;i<dimensions;i++)
			{
				subcontrols[i]=it;
				++it;
			}
			for(std::size_t j=0;j+1<dimensions;j++)
			{
				for(std::size_t i=0;i+1<(dimensions-j);i++)
				{
					const SimplePoint p=((*(subcontrols[i]))*0.5)+((*(subcontrols[i+1]))*0.5);
					std::list<SimplePoint>::iterator p_it=approximation.insert(subcontrols[i+1], p);
					if(i+2<(dimensions-j))
					{
						approximation.erase(subcontrols[i+1]);
					}
					subcontrols[i]=p_it;
				}
			}
		}
	}
	return std::vector<SimplePoint>(approximation.begin(), approximation.end());
}

inline std::vector<SimplePoint> rational_bezier_curve_using_explicit_de_casteljau_subdivision(const std::vector<SimplePoint>& controls, const std::vector<double>& weights, const int depth)
{
	typedef std::pair<SimplePoint, double> WeightedPoint;
	const std::size_t dimensions=std::min(controls.size(), weights.size());
	std::list<WeightedPoint> approximation;
	for(std::size_t i=0;i<dimensions;i++)
	{
		approximation.push_back(WeightedPoint(controls[i], weights[i]));
	}
	std::vector< std::list<WeightedPoint>::iterator > subcontrols(dimensions);
	for(int level=0;level<depth;level++)
	{
		std::list<WeightedPoint>::iterator it=approximation.begin();
		while(it!=approximation.end())
		{
			if(it!=approximation.begin())
			{
				--it;
			}
			for(std::size_t i=0;i<dimensions;i++)
			{
				subcontrols[i]=it;
				++it;
			}
			for(std::size_t j=0;j+1<dimensions;j++)
			{
				for(std::size_t i=0;i+1<(dimensions-j);i++)
				{
					const double w=((subcontrols[i]->second)*0.5)+((subcontrols[i+1]->second)*0.5);
					const SimplePoint p=((subcontrols[i]->first)*(0.5*((subcontrols[i]->second)/w)))+((subcontrols[i+1]->first)*(0.5*((subcontrols[i+1]->second)/w)));
					std::list<WeightedPoint>::iterator wp_it=approximation.insert(subcontrols[i+1], WeightedPoint(p, w));
					if(i+2<(dimensions-j))
					{
						approximation.erase(subcontrols[i+1]);
					}
					subcontrols[i]=wp_it;
				}
			}
		}
	}
	std::vector<SimplePoint> result;
	result.reserve(approximation.size());
	for(std::list<WeightedPoint>::const_iterator it=approximation.begin();it!=approximation.end();++it)
	{
		result.push_back(it->first);
	}
	return result;
}

template<typename ControlPointsList>
inline ControlPointsList transform_points_to_increase_bezier_curve_degree(const ControlPointsList& controls)
{
	const std::size_t n=controls.size();
	if(n>1)
	{
		ControlPointsList new_controls;
		std::size_t i=0;
		for(typename ControlPointsList::const_iterator it=controls.begin();it!=controls.end();++it)
		{
			if(it==controls.begin())
			{
				new_controls.push_back(*it);
			}
			else
			{
				typename ControlPointsList::const_iterator prev_it=it;
				--prev_it;
				new_controls.push_back(((*prev_it)*(static_cast<double>(i)/static_cast<double>(n)))+((*it)*(1.0-static_cast<double>(i)/static_cast<double>(n))));
			}
			i++;
		}
		new_controls.push_back(controls.back());
		return new_controls;
	}
	else
	{
		return controls;
	}
}

inline SimplePoint barycentric_coordinates(const SimplePoint& a, const SimplePoint& b, const SimplePoint& c, const SimplePoint& p)
{
	const SimplePoint v0=(b-a);
	const SimplePoint v1=(c-a);
	const SimplePoint v2=(p-a);
	const double d00=(v0*v0);
	const double d01=(v0*v1);
	const double d11=(v1*v1);
	const double d20=(v2*v0);
	const double d21=(v2*v1);
	const double denom=(d00*d11-d01*d01);
	const double v=(d11*d20-d01*d21)/denom;
	const double w=(d00*d21-d01*d20)/denom;
	const double u=1.0-v-w;
	return SimplePoint(u, v, w);
}

}

}

#endif /* APOLLOTA_POLYNOMIAL_CURVES_H_ */
