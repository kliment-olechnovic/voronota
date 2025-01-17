#ifndef COMMON_CONSTRUCTION_OF_TRIANGULATION_H_
#define COMMON_CONSTRUCTION_OF_TRIANGULATION_H_

#include "../apollota/triangulation.h"
#include "../apollota/triangulation_output.h"
#include "../apollota/spheres_boundary_construction.h"

namespace voronota
{

namespace common
{

class ConstructionOfTriangulation
{
public:
	struct ParametersToConstructBundleOfTriangulationInformation
	{
		double artificial_boundary_shift;
		bool exclude_hidden_balls;
		double init_radius_for_BSH;

		ParametersToConstructBundleOfTriangulationInformation() :
			artificial_boundary_shift(2.8),
			exclude_hidden_balls(false),
			init_radius_for_BSH(3.5)
		{
		}

		bool equals(const ParametersToConstructBundleOfTriangulationInformation& b) const
		{
			return (artificial_boundary_shift==b.artificial_boundary_shift
					&& exclude_hidden_balls==b.exclude_hidden_balls
					&& init_radius_for_BSH==b.init_radius_for_BSH);
		}
	};

	struct BundleOfTriangulationInformation
	{
		ParametersToConstructBundleOfTriangulationInformation parameters_of_construction;
		std::size_t number_of_input_spheres;
		std::vector<apollota::SimpleSphere> spheres;
		apollota::Triangulation::QuadruplesMap quadruples_map;

		BundleOfTriangulationInformation() : number_of_input_spheres(0)
		{
		}

		template<typename ContainerOfBalls>
		bool matching(const ContainerOfBalls& balls) const
		{
			if(balls.empty() || number_of_input_spheres!=balls.size())
			{
				return false;
			}

			{
				std::vector<apollota::SimpleSphere>::const_iterator spheres_it=spheres.begin();
				typename ContainerOfBalls::const_iterator balls_it=balls.begin();
				while(spheres_it!=spheres.end() && balls_it!=balls.end())
				{
					if(!apollota::spheres_equal(*spheres_it, *balls_it, apollota::default_comparison_epsilon()))
					{
						return false;
					}
					++spheres_it;
					++balls_it;
				}
			}

			return true;
		}

		template<typename ContainerOfBalls>
		bool equivalent(const ParametersToConstructBundleOfTriangulationInformation& parameters, const ContainerOfBalls& balls) const
		{
			return (parameters_of_construction.equals(parameters) && matching(balls));
		}

		void swap(BundleOfTriangulationInformation& b)
		{
			parameters_of_construction=b.parameters_of_construction;
			number_of_input_spheres=b.number_of_input_spheres;
			spheres.swap(b.spheres);
			quadruples_map.swap(b.quadruples_map);
		}
	};

	template<typename ContainerOfBalls>
	static bool construct_bundle_of_triangulation_information(
			const ParametersToConstructBundleOfTriangulationInformation& parameters,
			const ContainerOfBalls& balls,
			BundleOfTriangulationInformation& bundle_of_triangulation_information)
	{
		bundle_of_triangulation_information=BundleOfTriangulationInformation();
		bundle_of_triangulation_information.parameters_of_construction=parameters;

		if(balls.size()<4 && parameters.artificial_boundary_shift<=0.0)
		{
			return false;
		}

		bundle_of_triangulation_information.spheres.reserve(balls.size());
		for(typename ContainerOfBalls::const_iterator it=balls.begin();it!=balls.end();++it)
		{
			bundle_of_triangulation_information.spheres.push_back(apollota::SimpleSphere(*it));
		}

		bundle_of_triangulation_information.number_of_input_spheres=bundle_of_triangulation_information.spheres.size();
		if(parameters.artificial_boundary_shift>0.0)
		{
			const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(bundle_of_triangulation_information.spheres, parameters.artificial_boundary_shift);
			bundle_of_triangulation_information.spheres.insert(bundle_of_triangulation_information.spheres.end(), artificial_boundary.begin(), artificial_boundary.end());
		}

		apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(bundle_of_triangulation_information.spheres, parameters.init_radius_for_BSH, parameters.exclude_hidden_balls, false);

		bundle_of_triangulation_information.quadruples_map.swap(triangulation_result.quadruples_map);

		if(bundle_of_triangulation_information.quadruples_map.empty())
		{
			return false;
		}

		return true;
	}
};

}

}

#endif /* COMMON_CONSTRUCTION_OF_TRIANGULATION_H_ */
