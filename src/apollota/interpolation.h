#ifndef APOLLOTA_INTERPOLATION_H_
#define APOLLOTA_INTERPOLATION_H_

#include <vector>

#include "basic_operations_on_points.h"

namespace apollota
{

std::vector<SimplePoint> interpolate_using_cubic_Hermite_spline(
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

}

#endif /* APOLLOTA_INTERPOLATION_H_ */
