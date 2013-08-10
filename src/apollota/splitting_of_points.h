#ifndef APOLLOTA_SPLITTING_OF_POINTS_H_
#define APOLLOTA_SPLITTING_OF_POINTS_H_

#include <vector>
#include <algorithm>

namespace apollota
{

class SplittingOfSpheres
{
public:
	template<typename SphereType>
	static std::vector< std::vector<std::size_t> > split_for_number_of_parts(const std::vector<SphereType>& spheres, const std::size_t number_of_parts)
	{
		std::vector< std::vector<std::size_t> > result=resplit(spheres, binary_split(spheres, estimate_splitting_depth_by_number_of_parts(number_of_parts)), false);
		while(result.size()>1 && result.size()>number_of_parts)
		{
			result=resplit(spheres, result, true);
		}
		return result;
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

	template<typename SphereType>
	static std::vector< std::vector<std::size_t> > resplit(const std::vector<SphereType>& spheres, const std::vector< std::vector<std::size_t> >& current_splitting, const bool remove_smallest_part)
	{
		if(current_splitting.size()<=1)
		{
			return current_splitting;
		}

		std::vector<SimplePoint> centers;
		if(remove_smallest_part)
		{
			std::size_t smallest_cluster_id=0;
			std::size_t smallest_cluster_size=current_splitting[0].size();
			for(std::size_t i=1;i<current_splitting.size();i++)
			{
				if(smallest_cluster_size>current_splitting[i].size())
				{
					smallest_cluster_size=current_splitting[i].size();
					smallest_cluster_id=i;
				}
			}
			centers.reserve(current_splitting.size()-1);
			for(std::size_t i=0;i<current_splitting.size();i++)
			{
				if(i!=smallest_cluster_id)
				{
					centers.push_back(calc_mass_center(spheres, current_splitting[i]));
				}
			}
		}
		else
		{
			centers.resize(current_splitting.size());
			for(std::size_t i=0;i<current_splitting.size();i++)
			{
				centers[i]=calc_mass_center(spheres, current_splitting[i]);
			}
		}

		std::vector< std::vector<std::size_t> > improved_splitting(centers.size());
		for(std::size_t i=0;i<improved_splitting.size();i++)
		{
			improved_splitting[i].reserve(spheres.size()/centers.size()*2);
		}
		for(std::size_t i=0;i<spheres.size();i++)
		{
			std::size_t best_center_id=0;
			double best_distance=maximal_distance_from_point_to_sphere(centers[0], spheres[i]);
			for(std::size_t j=1;j<centers.size();j++)
			{
				const double distance=maximal_distance_from_point_to_sphere(centers[j], spheres[i]);
				if(distance<best_distance)
				{
					best_distance=distance;
					best_center_id=j;
				}
			}
			improved_splitting[best_center_id].push_back(i);
		}

		if(remove_smallest_part)
		{
			return resplit(spheres, improved_splitting, false);
		}
		else
		{
			return improved_splitting;
		}
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

	template<typename PointType>
	static SimplePoint calc_mass_center(const std::vector<PointType>& points, const std::vector<std::size_t>& ids)
	{
		SimplePoint center(0.0, 0.0, 0.0);
		if(!ids.empty())
		{
			for(std::size_t i=0;i<ids.size();i++)
			{
				center=center+SimplePoint(points[ids[i]]);
			}
			center=center*(1/static_cast<double>(ids.size()));
		}
		return center;
	}

	static unsigned int estimate_splitting_depth_by_number_of_parts(const unsigned int number_of_parts)
	{
		unsigned int depth=0;
		while((static_cast<unsigned int>(1 << depth)<number_of_parts) && (depth<16))
		{
			depth++;
		}
		return depth;
	}
};

}

#endif /* APOLLOTA_SPLITTING_OF_POINTS_H_ */
