#ifndef COMMON_BALL_VALUE_H_
#define COMMON_BALL_VALUE_H_

#include "properties_value.h"

namespace voronota
{

namespace common
{

struct BallValue
{
	double x;
	double y;
	double z;
	double r;
	PropertiesValue props;

	BallValue() : x(0.0), y(0.0), z(0.0), r(0.0)
	{
	}
};

inline std::ostream& operator<<(std::ostream& output, const BallValue& value)
{
	output << value.x << " " << value.y << " " << value.z << " " << value.r << " " << value.props;
	return output;
}

inline std::istream& operator>>(std::istream& input, BallValue& value)
{
	input >> value.x >> value.y >> value.z >> value.r >> value.props;
	return input;
}

}

}

#endif /* COMMON_BALL_VALUE_H_ */
