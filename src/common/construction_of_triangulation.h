#ifndef COMMON_CONSTRUCTION_OF_TRIANGULATION_H_
#define COMMON_CONSTRUCTION_OF_TRIANGULATION_H_

#include "../apollota/triangulation.h"
#include "../apollota/spheres_boundary_construction.h"

namespace common
{

class ConstructionOfTriangulation
{
public:
	struct BundleOfTriangulationInformation
	{
		std::size_t number_of_input_spheres;
		std::vector<apollota::SimpleSphere> spheres;
		apollota::Triangulation::QuadruplesMap quadruples_map;

		BundleOfTriangulationInformation() : number_of_input_spheres(0)
		{
		}
	};

	class construct_bundle_of_triangulation_information
	{
	public:
		double artificial_boundary_shift;
		bool exclude_hidden_balls;
		double init_radius_for_BSH;

		construct_bundle_of_triangulation_information() :
			artificial_boundary_shift(2.8),
			exclude_hidden_balls(false),
			init_radius_for_BSH(3.5)
		{
		}

		template<typename ContainerOfBalls>
		bool operator()(const ContainerOfBalls& balls, BundleOfTriangulationInformation& bundle_of_triangulation_information) const
		{
			bundle_of_triangulation_information=BundleOfTriangulationInformation();

			if(balls.size()<4)
			{
				return false;
			}

			bundle_of_triangulation_information.spheres.reserve(balls.size());
			for(typename ContainerOfBalls::const_iterator it=balls.begin();it!=balls.end();++it)
			{
				bundle_of_triangulation_information.spheres.push_back(apollota::SimpleSphere(*it));
			}

			bundle_of_triangulation_information.number_of_input_spheres=bundle_of_triangulation_information.spheres.size();
			if(artificial_boundary_shift>0.0)
			{
				const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(bundle_of_triangulation_information.spheres, artificial_boundary_shift);
				bundle_of_triangulation_information.spheres.insert(bundle_of_triangulation_information.spheres.end(), artificial_boundary.begin(), artificial_boundary.end());
			}

			apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(bundle_of_triangulation_information.spheres, init_radius_for_BSH, exclude_hidden_balls, false);

			bundle_of_triangulation_information.quadruples_map.swap(triangulation_result.quadruples_map);

			if(bundle_of_triangulation_information.quadruples_map.empty())
			{
				return false;
			}

			return true;
		}
	};
};

}

#endif /* COMMON_CONSTRUCTION_OF_TRIANGULATION_H_ */
