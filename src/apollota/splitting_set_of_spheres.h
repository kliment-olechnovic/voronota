#ifndef APOLLOTA_SPLITTING_SET_OF_SPHERES_H_
#define APOLLOTA_SPLITTING_SET_OF_SPHERES_H_

#include <vector>

#include "basic_operations_on_spheres.h"
#include "subdivided_icosahedron.h"

namespace apollota
{

class SplittingSetOfSpheres
{
public:
	template<typename SphereType>
	static std::vector< std::vector<std::size_t> > split(const std::vector<SphereType>& spheres, const int depth)
	{
		std::vector<std::size_t> ids;
		ids.reserve(spheres.size());
		for(std::size_t i=0;i<spheres.size();i++)
		{
			ids.push_back(i);
		}
		return split(spheres, ids, std::min(depth, 8));
	}

private:
	template<typename SphereType>
	static std::vector< std::vector<std::size_t> > split(const std::vector<SphereType>& spheres, const std::vector<std::size_t>& ids, const int depth)
	{
		if(depth<=0)
		{
			return std::vector< std::vector<std::size_t> >(1, ids);
		}
		else
		{
			std::vector< std::vector<std::size_t> > complex_result;
			const std::vector< std::vector<std::size_t> > simple_result=split(spheres, ids);
			for(std::size_t i=0;i<simple_result.size();i++)
			{
				const std::vector< std::vector<std::size_t> > smaller_result=split(spheres, simple_result[i], depth-1);
				complex_result.insert(complex_result.end(), smaller_result.begin(), smaller_result.end());
			}
			return complex_result;
		}
	}

	template<typename SphereType>
	static std::vector< std::vector<std::size_t> > split(const std::vector<SphereType>& spheres, const std::vector<std::size_t>& ids)
	{
		const SimplePoint plane_point=calc_mass_center(spheres, ids);

		SimplePoint plane_normal(1, 0, 0);
		{
			SubdividedIcosahedron sih(2);
			int best_vertex_value=static_cast<int>(ids.size()*2);
			for(std::size_t i=0;i<sih.vertices().size();i++)
			{
				const SimplePoint& vertex=sih.vertices()[i];
				int vertex_left_count=0;
				for(std::size_t i=0;i<ids.size();i++)
				{
					if(halfspace_of_sphere(plane_point, vertex, spheres[ids[i]])<0)
					{
						vertex_left_count++;
					}
				}
				const int vertex_value=abs(static_cast<int>(ids.size())-(2*vertex_left_count));
				if(vertex_value<best_vertex_value)
				{
					plane_normal=vertex;
					best_vertex_value=vertex_value;
				}
			}
		}

		return split(spheres, ids, plane_point, plane_normal);
	}

	template<typename SphereType>
	static SimplePoint calc_mass_center(const std::vector<SphereType>& spheres, const std::vector<std::size_t>& ids)
	{
		SimplePoint center(0.0, 0.0, 0.0);
		for(std::size_t i=0;i<ids.size();i++)
		{
			center=center+SimplePoint(spheres[ids[i]]);
		}
		center=center*(1/static_cast<double>(ids.size()));
		return center;
	}

	template<typename SphereType>
	static std::vector< std::vector<std::size_t> > split(const std::vector<SphereType>& spheres, const std::vector<std::size_t>& ids, const SimplePoint& plane_point, const SimplePoint& plane_normal)
	{
		std::vector< std::vector<std::size_t> > result(2);
		for(std::size_t i=0;i<ids.size();i++)
		{
			if(halfspace_of_sphere(plane_point, plane_normal, spheres[ids[i]])<0)
			{
				result[0].push_back(ids[i]);
			}
			else
			{
				result[1].push_back(ids[i]);
			}
		}
		return result;
	}
};

}

#endif /* APOLLOTA_SPLITTING_SET_OF_SPHERES_H_ */
