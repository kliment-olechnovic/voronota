#ifndef SCRIPTING_FILTERING_OF_TRIANGULATION_H_
#define SCRIPTING_FILTERING_OF_TRIANGULATION_H_

#include "../common/construction_of_triangulation.h"

namespace voronota
{

namespace scripting
{

class FilteringOfTriangulation
{
public:
	struct Query
	{
		bool strict;
		double max_edge;
		double min_radius;
		double max_radius;
		double expansion;
		double min_buriedness;
		double min_scaled_buriedness;
		std::set<std::size_t> atom_ids;

		Query() :
			strict(false),
			max_edge(std::numeric_limits<double>::max()),
			min_radius(-1000000.0),
			max_radius(std::numeric_limits<double>::max()),
			expansion(0.0),
			min_buriedness(0.0),
			min_scaled_buriedness(0.0)
		{
		}
	};

	struct VertexInfo
	{
		apollota::Quadruple quadruple;
		apollota::SimpleSphere sphere;
		std::size_t id;
		double tetrahedron_volume;

		VertexInfo(
				const apollota::Quadruple& quadruple,
				const apollota::SimpleSphere& sphere,
				const std::size_t id,
				const double tetrahedron_volume) :
					quadruple(quadruple),
					sphere(sphere),
					id(id),
					tetrahedron_volume(tetrahedron_volume)
		{
		}
	};

	struct MatchingResult
	{
		std::vector<VertexInfo> vertices_info;
		double total_relevant_tetrahedron_volume;

		MatchingResult() : total_relevant_tetrahedron_volume(0.0)
		{
		}
	};

	static MatchingResult match_vertices(
			const common::ConstructionOfTriangulation::BundleOfTriangulationInformation& triangulation_info,
			const Query& query)
	{
		MatchingResult result;

		const std::vector<apollota::SimpleSphere>& balls=triangulation_info.spheres;

		const apollota::BoundingSpheresHierarchy bsh=
				((query.expansion>0.0) ? apollota::BoundingSpheresHierarchy(balls, 3.5, 1) : apollota::BoundingSpheresHierarchy());

		const apollota::Triangulation::VerticesVector vertices_vector=
				apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_info.quadruples_map);

		for(std::size_t i=0;i<vertices_vector.size();i++)
		{
			const apollota::Quadruple& quadruple=vertices_vector[i].first;
			const apollota::SimpleSphere& sphere=vertices_vector[i].second;

			bool allowed=(quadruple.get_min_max().second<triangulation_info.number_of_input_spheres);

			allowed=allowed && (sphere.r>query.min_radius) && (sphere.r<query.max_radius);

			std::vector<std::size_t> near_ids;

			if(allowed)
			{
				if(query.expansion>0.0)
				{
					near_ids=apollota::SearchForSphericalCollisions::find_all_collisions(bsh, apollota::SimpleSphere(sphere, sphere.r+query.expansion));
					if(query.strict)
					{
						std::size_t found_count=0;
						for(std::size_t j=0;j<near_ids.size();j++)
						{
							if(query.atom_ids.count(near_ids[j])>0)
							{
								found_count++;
							}
						}
						allowed=(found_count>=4);
					}
					else
					{
						bool found_id=false;
						for(std::size_t j=0;j<near_ids.size() && !found_id;j++)
						{
							found_id=found_id || (query.atom_ids.count(near_ids[j])>0);
						}
						allowed=found_id;
					}
				}
				else
				{
					near_ids.resize(4);
					near_ids[0]=quadruple.get(0);
					near_ids[1]=quadruple.get(1);
					near_ids[2]=quadruple.get(2);
					near_ids[3]=quadruple.get(3);
					if(query.strict)
					{
						allowed=(query.atom_ids.count(quadruple.get(0))>0
								&& query.atom_ids.count(quadruple.get(1))>0
								&& query.atom_ids.count(quadruple.get(2))>0
								&& query.atom_ids.count(quadruple.get(3))>0);
					}
					else
					{
						allowed=(query.atom_ids.count(quadruple.get(0))>0
								|| query.atom_ids.count(quadruple.get(1))>0
								|| query.atom_ids.count(quadruple.get(2))>0
								|| query.atom_ids.count(quadruple.get(3))>0);
					}
				}
			}

			if(allowed && query.max_edge<std::numeric_limits<double>::max())
			{
				for(int a=0;a<3 && allowed;a++)
				{
					for(int b=(a+1);b<4 && allowed;b++)
					{
						allowed=allowed && (apollota::distance_from_point_to_point(balls[quadruple.get(a)], balls[quadruple.get(b)])<query.max_edge);
					}
				}
			}

			if(allowed && (query.min_buriedness>0.0 || query.min_scaled_buriedness>0.0) && near_ids.size()>=4)
			{
				const std::size_t N=near_ids.size();

				std::vector<apollota::SimplePoint> touches(N);
				for(std::size_t j=0;j<N;j++)
				{
					touches[j]=(apollota::SimplePoint(balls[near_ids[j]])-apollota::SimplePoint(sphere)).unit();
				}

				double d_sum=0.0;
				for(std::size_t j=0;(j+1)<N;j++)
				{
					for(std::size_t k=j+1;k<N;k++)
					{
						d_sum+=apollota::distance_from_point_to_point(touches[j], touches[k]);
					}
				}

				const double pi=3.14159265358979323846;
				const double max_d_sum=N*(1.0/tan(pi/(2.0*N)));
				const double buriedness=std::min(d_sum/max_d_sum, 1.0);

				if(query.min_buriedness>0.0)
				{
					allowed=allowed && (buriedness>=query.min_buriedness);
				}

				if(query.min_scaled_buriedness>0.0)
				{
					const double scaling_radius=std::min(sphere.r, query.max_radius);
					const double scaled_buriedness=buriedness*((4.0/3.0)*pi*(scaling_radius*scaling_radius*scaling_radius));

					allowed=allowed && (scaled_buriedness>=query.min_scaled_buriedness);
				}

			}

			if(allowed)
			{
				const double volume=fabs(apollota::signed_volume_of_tetrahedron(
						balls[quadruple.get(0)], balls[quadruple.get(1)], balls[quadruple.get(2)], balls[quadruple.get(3)]));

				result.vertices_info.push_back(VertexInfo(quadruple, sphere, i, volume));
				result.total_relevant_tetrahedron_volume+=volume;
			}
		}

		return result;
	}
};

}



}

#endif /* SCRIPTING_FILTERING_OF_TRIANGULATION_H_ */
