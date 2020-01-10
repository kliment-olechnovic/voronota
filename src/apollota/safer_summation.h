#ifndef APOLLOTA_SAFE_SUMMATION_H_
#define APOLLOTA_SAFE_SUMMATION_H_

namespace voronota
{

namespace apollota
{

inline double safer_sum(const double v0, const double v1)
{
	double sum=0.0;
	double c=0.0;
	double y;
	double t;

	y=v0-c;
	t=sum+y;
	c=(t-sum)-y;
	sum=t;

	y=v1-c;
	sum=sum+y;

	return sum;
}

inline double safer_sum(const double v0, const double v1, const double v2)
{
	double sum=0.0;
	double c=0.0;
	double y;
	double t;

	y=v0-c;
	t=sum+y;
	c=(t-sum)-y;
	sum=t;

	y=v1-c;
	t=sum+y;
	c=(t-sum)-y;
	sum=t;

	y=v2-c;
	sum=sum+y;

	return sum;
}

inline double safer_sum(const double v0, const double v1, const double v2, const double v3)
{
	double sum=0.0;
	double c=0.0;
	double y;
	double t;

	y=v0-c;
	t=sum+y;
	c=(t-sum)-y;
	sum=t;

	y=v1-c;
	t=sum+y;
	c=(t-sum)-y;
	sum=t;

	y=v2-c;
	t=sum+y;
	c=(t-sum)-y;
	sum=t;

	y=v3-c;
	sum=sum+y;

	return sum;
}

inline double safer_sum(const double v0, const double v1, const double v2, const double v3, const double v4, const double v5)
{
	double sum=0.0;
	double c=0.0;
	double y;
	double t;

	y=v0-c;
	t=sum+y;
	c=(t-sum)-y;
	sum=t;

	y=v1-c;
	t=sum+y;
	c=(t-sum)-y;
	sum=t;

	y=v2-c;
	t=sum+y;
	c=(t-sum)-y;
	sum=t;

	y=v3-c;
	t=sum+y;
	c=(t-sum)-y;
	sum=t;

	y=v4-c;
	t=sum+y;
	c=(t-sum)-y;
	sum=t;

	y=v5-c;
	sum=sum+y;

	return sum;
}

}

}

#endif /* APOLLOTA_SAFE_SUMMATION_H_ */
