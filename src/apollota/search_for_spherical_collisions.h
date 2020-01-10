#ifndef APOLLOTA_SEARCH_FOR_SPHERICAL_COLLISIONS_H_
#define APOLLOTA_SEARCH_FOR_SPHERICAL_COLLISIONS_H_

#include <set>

#include "bounding_spheres_hierarchy.h"

namespace voronota
{

namespace apollota
{

class SearchForSphericalCollisions
{
public:
	static std::vector<std::size_t> find_collisions(const BoundingSpheresHierarchy& bsh, const SimpleSphere& target, const bool one_hit_is_enough)
	{
		NodeCheckerForCollisions node_checker(target);
		LeafCheckerForCollisions leaf_checker(target, one_hit_is_enough);
		return bsh.search(node_checker, leaf_checker);
	}

	static std::vector<std::size_t> find_all_collisions(const BoundingSpheresHierarchy& bsh, const SimpleSphere& target)
	{
		return find_collisions(bsh, target, false);
	}

	static std::vector<std::size_t> find_any_collision(const BoundingSpheresHierarchy& bsh, const SimpleSphere& target)
	{
		return find_collisions(bsh, target, true);
	}

	static std::set<std::size_t> find_all_hidden_spheres(const BoundingSpheresHierarchy& bsh)
	{
		std::set<std::size_t> result;
		for(std::size_t i=0;i<bsh.leaves_spheres().size();i++)
		{
			std::vector<std::size_t> candidates=find_all_collisions(bsh, custom_sphere_from_object<SimpleSphere>(bsh.leaves_spheres()[i]));
			for(std::size_t j=0;j<candidates.size();j++)
			{
				if(i!=candidates[j])
				{
					const bool i_contains_candidate_j=sphere_contains_sphere(bsh.leaves_spheres()[i], bsh.leaves_spheres()[candidates[j]]);
					const bool candidate_j_contains_i=sphere_contains_sphere(bsh.leaves_spheres()[candidates[j]], bsh.leaves_spheres()[i]);
					if(i_contains_candidate_j && candidate_j_contains_i)
					{
						if(i<candidates[j])
						{
							result.insert(candidates[j]);
						}
						else
						{
							result.insert(i);
						}
					}
					else if(i_contains_candidate_j)
					{
						result.insert(candidates[j]);
					}
					else if(candidate_j_contains_i)
					{
						result.insert(i);
					}
				}
			}
		}
		return result;
	}

private:
	struct NodeCheckerForCollisions
	{
		const SimpleSphere& target;

		explicit NodeCheckerForCollisions(const SimpleSphere& target) : target(target) {}

		bool operator()(const SimpleSphere& sphere) const
		{
			return sphere_intersects_sphere(sphere, target);
		}
	};

	struct LeafCheckerForCollisions
	{
		const SimpleSphere& target;
		const bool one_hit_is_enough;

		explicit LeafCheckerForCollisions(const SimpleSphere& target) : target(target), one_hit_is_enough(true) {}

		LeafCheckerForCollisions(const SimpleSphere& target, const bool one_hit_is_enough) : target(target), one_hit_is_enough(one_hit_is_enough) {}

		template<typename Sphere>
		std::pair<bool, bool> operator()(const std::size_t /*id*/, const Sphere& sphere) const
		{
			if(sphere_intersects_sphere(sphere, target))
			{
				return std::make_pair(true, one_hit_is_enough);
			}
			return std::make_pair(false, false);
		}
	};
};

}

}

#endif /* APOLLOTA_SEARCH_FOR_SPHERICAL_COLLISIONS_H_ */
