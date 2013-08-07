#ifndef APOLLOTA_COMPARISON_OF_TRIANGULATIONS_H_
#define APOLLOTA_COMPARISON_OF_TRIANGULATIONS_H_

#include <vector>
#include <algorithm>

#include "tuple.h"
#include "bounding_spheres_hierarchy.h"
#include "search_for_spherical_collisions.h"
#include "tangent_sphere_of_four_spheres.h"

namespace apollota
{

class ComparisonOfTriangulations
{
public:
	struct Result
	{
		std::vector<Quadruple> all_differences;
		std::vector<Quadruple> confirmed_differences;
	};

	template<typename SphereType, typename InputQuadruples>
	static Result calculate_directional_difference_between_two_sets_of_quadruples(const std::vector<SphereType>& spheres, const double initial_radius_for_spheres_bucketing, const InputQuadruples& input_quadruples1, const InputQuadruples& input_quadruples2)
	{
		Result result;
		if(!input_quadruples1.empty())
		{
			const std::vector<Quadruple> quadruples1=get_sorted_unique_quadruples(input_quadruples1);
			const std::vector<Quadruple> quadruples2=get_sorted_unique_quadruples(input_quadruples2);

			result.all_differences.resize(quadruples1.size());
			std::vector<Quadruple>::iterator it=std::set_difference(quadruples1.begin(), quadruples1.end(), quadruples2.begin(), quadruples2.end(), result.all_differences.begin());
			result.all_differences.resize(it-result.all_differences.begin());

			if(!result.all_differences.empty())
			{
				const BoundingSpheresHierarchy bsh(spheres, initial_radius_for_spheres_bucketing, 1);
				for(std::size_t i=0;i<result.all_differences.size();i++)
				{
					const Quadruple q=result.all_differences[i];
					if(q.get(0)<bsh.leaves_spheres().size() && q.get(1)<bsh.leaves_spheres().size() && q.get(2)<bsh.leaves_spheres().size() && q.get(3)<bsh.leaves_spheres().size())
					{
						const std::vector<SimpleSphere> tangents=TangentSphereOfFourSpheres::calculate(bsh.leaves_spheres().at(q.get(0)), bsh.leaves_spheres().at(q.get(1)), bsh.leaves_spheres().at(q.get(2)), bsh.leaves_spheres().at(q.get(3)));
						if(
								(tangents.size()==1 && SearchForSphericalCollisions::find_any_collision(bsh, tangents.front()).empty())
								|| (tangents.size()==2 && (SearchForSphericalCollisions::find_any_collision(bsh, tangents.front()).empty() || SearchForSphericalCollisions::find_any_collision(bsh, tangents.back()).empty()))
							)
						{
							result.confirmed_differences.push_back(q);
						}
					}
				}
			}
		}
		return result;
	}

private:
	template<typename InputQuadruples>
	static std::vector<Quadruple> get_sorted_unique_quadruples(const InputQuadruples& input_quadruples)
	{
		std::vector<Quadruple> result(input_quadruples.begin(), input_quadruples.end());
		if(!result.empty())
		{
			std::sort(result.begin(), result.end());
			std::vector<Quadruple>::iterator it=std::unique(result.begin(), result.end());
			result.resize(it-result.begin());
		}
		return result;
	}
};

}

#endif /* APOLLOTA_COMPARISON_OF_TRIANGULATIONS_H_ */
