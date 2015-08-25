#ifndef APOLLOTA_POLAR_SORTING_H_
#define APOLLOTA_POLAR_SORTING_H_

#include <algorithm>

#include "basic_operations_on_points.h"

namespace apollota
{

class PolarSorting
{
public:
	template<typename CenterPointType, typename AxisPointType, typename RandomAccessIteratorType>
	static void sort(const CenterPointType& center, const AxisPointType& axis, RandomAccessIteratorType first, RandomAccessIteratorType last)
	{
		std::sort(first, last, Less(center, axis));
	}

private:
	struct Less
	{
		PODPoint center;
		PODPoint axis;

		template<typename CenterPointType, typename AxisPointType>
		Less(const CenterPointType& center, const AxisPointType& axis) :
			center(custom_point_from_object<PODPoint>(center)),
			axis(unit_point<PODPoint>(axis))
		{
		}

		template<typename PointType>
		bool operator()(const PointType& a, const PointType& b) const
		{
			PODPoint c=cross_product<PODPoint>(sub_of_points<PODPoint>(a, center), sub_of_points<PODPoint>(b, center));
			return (dot_product(axis, c)<0);
		}
	};
};

}

#endif /* APOLLOTA_POLAR_SORTING_H_ */
