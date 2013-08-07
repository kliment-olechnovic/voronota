#ifndef APOLLOTA_SPLITTING_SET_OF_SPHERES_H_
#define APOLLOTA_SPLITTING_SET_OF_SPHERES_H_

#include <vector>
#include <limits>

#include "basic_operations_on_spheres.h"
#include "subdivided_icosahedron.h"

namespace apollota
{

class SplittingSetOfSpheres
{
public:
	template<typename SphereType>
	static std::vector< std::vector<std::size_t> > split(const std::vector<SphereType>& spheres, const unsigned int depth)
	{
		std::vector<std::size_t> ids;
		ids.reserve(spheres.size());
		for(std::size_t i=0;i<spheres.size();i++)
		{
			ids.push_back(i);
		}
		return split(spheres, ids, depth);
	}

private:
	template<typename SphereType>
	static std::vector< std::vector<std::size_t> > split(const std::vector<SphereType>& spheres, const std::vector<std::size_t>& ids, const unsigned int depth)
	{
		std::vector< std::vector<std::size_t> > complex_result(1, ids);
		for(unsigned int i=0;i<depth;i++)
		{
			std::vector< std::vector<std::size_t> > new_complex_result;
			for(std::size_t j=0;j<complex_result.size();j++)
			{
				const std::vector< std::vector<std::size_t> > simple_result=split(spheres, complex_result[j]);
				new_complex_result.insert(new_complex_result.end(), simple_result.begin(), simple_result.end());
			}
			complex_result=new_complex_result;
		}
		return complex_result;
	}

	template<typename SphereType>
	static std::vector< std::vector<std::size_t> > split(const std::vector<SphereType>& spheres, const std::vector<std::size_t>& ids)
	{
		if(ids.size()>1)
		{
			const SimplePoint plane_point=calc_mass_center(spheres, ids);
			double plane_normal_value=std::numeric_limits<double>::max();
			SimplePoint plane_normal(1, 0, 0);
			const SubdividedIcosahedron sih(2);
			for(std::size_t i=0;i<sih.vertices().size();i++)
			{
				const SimplePoint& vertex=sih.vertices()[i];
				const std::vector< std::vector<std::size_t> > temp_result=split(spheres, ids, plane_point, vertex);
				if(temp_result.size()==2 && !temp_result[0].empty() && !temp_result[1].empty())
				{
					const double r0=calc_bounding_sphere_radius(spheres, temp_result[0]);
					const double r1=calc_bounding_sphere_radius(spheres, temp_result[1]);
					const double vertex_value=(r0*r0*r0)+(r1*r1*r1);
					if(vertex_value<plane_normal_value)
					{
						plane_normal_value=vertex_value;
						plane_normal=vertex;
					}
				}
			}
			if(plane_normal_value<std::numeric_limits<double>::max())
			{
				return split(spheres, ids, plane_point, plane_normal);
			}
		}
		return std::vector< std::vector<std::size_t> >(1, ids);
	}

	template<typename SphereType>
	static std::vector< std::vector<std::size_t> > split(const std::vector<SphereType>& spheres, const std::vector<std::size_t>& ids, const SimplePoint& plane_point, const SimplePoint& plane_normal)
	{
		std::vector<std::size_t> left;
		std::vector<std::size_t> right;
		for(std::size_t i=0;i<ids.size();i++)
		{
			if(halfspace_of_sphere(plane_point, plane_normal, spheres[ids[i]])<0)
			{
				left.push_back(ids[i]);
			}
			else
			{
				right.push_back(ids[i]);
			}
		}
		std::vector< std::vector<std::size_t> > result;
		if(!left.empty())
		{
			result.push_back(left);
		}
		if(!right.empty())
		{
			result.push_back(right);
		}
		return result;
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
	static double calc_bounding_sphere_radius(const std::vector<SphereType>& spheres, const std::vector<std::size_t>& ids)
	{
		const SimplePoint center=calc_mass_center(spheres, ids);
		double max_dist=0.0;
		for(std::size_t i=0;i<ids.size();i++)
		{
			max_dist=std::max(max_dist, maximal_distance_from_point_to_sphere(center, spheres[ids[i]]));
		}
		return max_dist;
	}
};

}

#endif /* APOLLOTA_SPLITTING_SET_OF_SPHERES_H_ */
