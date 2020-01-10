#ifndef APOLLOTA_POLAR_SORTING_H_
#define APOLLOTA_POLAR_SORTING_H_

#include <algorithm>

#include "basic_operations_on_points.h"

namespace voronota
{

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

	template<typename CenterPointType, typename AxisPointType, typename RandomAccessIteratorType>
	static void sort_mapping(const CenterPointType& center, const AxisPointType& axis, RandomAccessIteratorType first, RandomAccessIteratorType last)
	{
		std::sort(first, last, LessForPairs(Less(center, axis)));
	}

private:
	class Less
	{
	public:
		template<typename CenterPointType, typename AxisPointType>
		Less(const CenterPointType& center, const AxisPointType& axis) :
			center(custom_point_from_object<PODPoint>(center)),
			axis(unit_point<PODPoint>(axis)),
			normal(any_normal_of_vector<PODPoint>(axis))
		{
		}

		template<typename PointType>
		bool operator()(const PointType& a, const PointType& b) const
		{
			return (calc_angle_with_normal(a)<calc_angle_with_normal(b));
		}

	private:
		PODPoint center;
		PODPoint axis;
		PODPoint normal;

		template<typename PointType>
		double calc_angle_with_normal(const PointType& a) const
		{
			const PODPoint ap=sub_of_points<PODPoint>(a, point_and_number_product<PODPoint>(axis, signed_distance_from_point_to_plane(center, axis, a)));
			return directed_angle(center, sum_of_points<PODPoint>(center, normal), ap, sum_of_points<PODPoint>(center, axis));
		}
	};

	struct LessForPairs
	{
		Less less;

		explicit LessForPairs(const Less& less) : less(less)
		{
		}

		template<typename PairType>
		bool operator()(const PairType& a, const PairType& b) const
		{
			if(less(a.first, b.first))
			{
				return true;
			}
			else if(less(b.first, a.first))
			{
				return false;
			}
			else
			{
				return (a.second<b.second);
			}
		}
	};
};

}

}

#endif /* APOLLOTA_POLAR_SORTING_H_ */
