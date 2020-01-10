#ifndef APOLLOTA_SAFE_QUADRATIC_EQUATION_ROOT_H_
#define APOLLOTA_SAFE_QUADRATIC_EQUATION_ROOT_H_

#include "safer_summation.h"

namespace voronota
{

namespace apollota
{

inline bool check_if_quadratic_equation_is_solvable(const double a, const double b)
{
	return (a>0.0 || a<0.0 || b>0.0 || b<0.0);
}

template<typename ResultsContainer>
inline bool solve_quadratic_equation(const double a, const double b, const double c, ResultsContainer& roots)
{
	if(check_if_quadratic_equation_is_solvable(a, b))
	{
		if(a>0.0 || a<0.0)
		{
			const double D = safer_sum(b*b, -4*a*c);
			if(D>=0.0)
			{
				if(D==0.0)
				{
					roots.push_back((-b)/(2*a));
				}
				else
				{
					if(b>0.0 || b<0.0)
					{
						const double b_sign=(b>0.0 ? 1.0 : -1.0);
						const double q=-(0.5*safer_sum(b, b_sign*sqrt(D)));
						roots.push_back(q/a);
						roots.push_back(c/q);
					}
					else
					{
						roots.push_back((-sqrt(D))/(2*a));
						roots.push_back((+sqrt(D))/(2*a));
					}
				}
				return true;
			}
		}
		else if(b>0.0 || b<0.0)
		{
			roots.push_back((-c)/b);
			return true;
		}
	}
	return false;
}

}

}

#endif /* APOLLOTA_SAFE_QUADRATIC_EQUATION_ROOT_H_ */
