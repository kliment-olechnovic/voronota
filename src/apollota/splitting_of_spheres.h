#ifndef APOLLOTA_SPLITTING_OF_SPHERES_H_
#define APOLLOTA_SPLITTING_OF_SPHERES_H_

#include <vector>
#include <algorithm>

namespace voronota
{

namespace apollota
{

class SplittingOfSpheres
{
public:
	template<typename SphereType>
	static std::vector< std::vector<std::size_t> > split_for_number_of_parts(const std::vector<SphereType>& spheres, const std::size_t number_of_parts)
	{
		return binary_split(spheres, estimate_splitting_depth_by_number_of_parts(number_of_parts));
	}

	template<typename SphereType>
	static std::vector< std::vector<std::size_t> > split_for_size_of_part(const std::vector<SphereType>& spheres, const std::size_t size_of_part)
	{
		return binary_split(spheres, estimate_splitting_depth_by_number_of_parts(spheres.size()/size_of_part));
	}

private:
	template<typename PointType>
	static std::vector< std::vector<std::size_t> > binary_split(const std::vector<PointType>& points, const unsigned int depth)
	{
		std::vector<std::size_t> ids(points.size());
		for(std::size_t i=0;i<points.size();i++)
		{
			ids[i]=i;
		}
		std::vector< std::vector<std::size_t> > result(1, ids);
		for(unsigned int k=0;k<depth;k++)
		{
			std::vector< std::vector<std::size_t> > updated_result;
			for(std::size_t i=0;i<result.size();i++)
			{
				const std::vector< std::vector<std::size_t> > addition=split_by_median(points, result[i], k);
				updated_result.insert(updated_result.end(), addition.begin(), addition.end());
			}
			result=updated_result;
		}
		return result;
	}

	template<typename PointType>
	static double coord(const PointType& p, const unsigned int k)
	{
		const unsigned int d=k%3;
		return (d==0 ? p.x : (d==1 ? p.y : p.z));
	}

	template<typename PointType>
	static std::vector< std::vector<std::size_t> > split_by_median(const std::vector<PointType>& points, const std::vector<std::size_t>& ids, const unsigned int k)
	{
		return split_by_middle(order_for_median(points, ids, k));
	}

	template<typename PointType>
	static std::vector<std::size_t> order_for_median(const std::vector<PointType>& points, const std::vector<std::size_t>& ids, const unsigned int k)
	{
		if(ids.size()<=1)
		{
			return ids;
		}
		else
		{
			std::vector< std::pair<double, std::size_t> > coord_positions(ids.size());
			for(std::size_t i=0;i<ids.size();i++)
			{
				coord_positions[i].first=coord(points[ids[i]], k);
				coord_positions[i].second=ids[i];
			}
			std::nth_element(coord_positions.begin(), (coord_positions.begin()+(coord_positions.size()/2)), coord_positions.end());
			std::vector<std::size_t> result(ids.size());
			for(std::size_t i=0;i<ids.size();i++)
			{
				result[i]=coord_positions[i].second;
			}
			return result;
		}
	}

	static std::vector< std::vector<std::size_t> > split_by_middle(const std::vector<std::size_t>& ids)
	{
		if(ids.empty())
		{
			return std::vector< std::vector<std::size_t> >();
		}
		if(ids.size()==1)
		{
			return std::vector< std::vector<std::size_t> >(1, ids);
		}
		else
		{
			std::vector< std::vector<std::size_t> > result(2);
			std::vector<std::size_t>::const_iterator middle_it=(ids.begin()+(ids.size()/2));
			result[0].insert(result[0].end(), ids.begin(), middle_it);
			result[1].insert(result[1].end(), middle_it, ids.end());
			return result;
		}
	}

	static unsigned int estimate_splitting_depth_by_number_of_parts(const unsigned long number_of_parts)
	{
		unsigned int depth=0;
		while(((1UL << depth)<number_of_parts) && (depth<16))
		{
			depth++;
		}
		return depth;
	}
};

}

}

#endif /* APOLLOTA_SPLITTING_OF_SPHERES_H_ */
