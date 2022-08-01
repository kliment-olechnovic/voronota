#ifndef APOLLOTA_SAFE_COMPARISON_OF_NUMBERS_H_
#define APOLLOTA_SAFE_COMPARISON_OF_NUMBERS_H_

namespace voronota
{

namespace apollota
{

inline double default_comparison_epsilon()
{
	static const double e=0.0000000001;
	return e;
}

inline bool equal(const double a, const double b, const double e)
{
	return (((a-b)<=e) && ((b-a)<=e));
}

inline bool equal(const double a, const double b)
{
	return equal(a, b, default_comparison_epsilon());
}

inline bool less(const double a, const double b)
{
	return ((a+default_comparison_epsilon())<b);
}

inline bool greater(const double a, const double b)
{
	return ((a-default_comparison_epsilon())>b);
}

inline bool less_or_equal(const double a, const double b)
{
	return (less(a, b) || equal(a, b));
}

inline bool greater_or_equal(const double a, const double b)
{
	return (greater(a, b) || equal(a, b));
}

}

}

#endif /* APOLLOTA_SAFE_COMPARISON_OF_NUMBERS_H_ */
