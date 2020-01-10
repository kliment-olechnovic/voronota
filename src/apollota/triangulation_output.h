#ifndef APOLLOTA_TRIANGULATION_OUTPUT_H_
#define APOLLOTA_TRIANGULATION_OUTPUT_H_

#include <iostream>

#include "triangulation.h"

namespace voronota
{

namespace apollota
{

class TriangulationOutput
{
public:
	static void print_status(const Triangulation::Result& tr, std::ostream& output)
	{
		output << "quadruples " << tr.quadruples_map.size() << "\n";
		output << "tangent_spheres " << Triangulation::count_tangent_spheres_in_quadruples_map(tr.quadruples_map) << "\n";
		output << "processed_triples " << tr.quadruples_search_log.processed_faces << "\n";
		output << "loose_triples " << tr.quadruples_search_log.encountered_difficult_faces << "\n";
		output << "first_iterations " << tr.quadruples_search_log.performed_iterations_for_finding_first_faces << "\n";
		output << "surplus_tangent_spheres " << tr.surplus_quadruples_search_log.surplus_tangent_spheres << "\n";
		output << "excluded_hidden_balls " << tr.excluded_hidden_spheres_ids.size() << "\n";
		output << "ignored_balls " << tr.ignored_spheres_ids.size();
		for(std::set<std::size_t>::const_iterator it=tr.ignored_spheres_ids.begin();it!=tr.ignored_spheres_ids.end();++it)
		{
			output << (it==tr.ignored_spheres_ids.begin() ? ":" : ",") << (*it);
		}
		output << "\n";
	}

	static void print_vertices_vector(const Triangulation::VerticesVector& vertices_vector, std::ostream& output)
	{
		output.precision(std::numeric_limits<double>::digits10);
		output << std::fixed;
		for(Triangulation::VerticesVector::const_iterator it=vertices_vector.begin();it!=vertices_vector.end();++it)
		{
			const Quadruple& quadruple=it->first;
			const SimpleSphere& tangent_sphere=it->second;
			output << quadruple.get(0) << " " << quadruple.get(1) << " " << quadruple.get(2) << " " << quadruple.get(3) << " ";
			output << tangent_sphere.x << " " << tangent_sphere.y << " " << tangent_sphere.z << " " << tangent_sphere.r << "\n";
		}
	}

	static void print_vertices_vector_with_vertices_graph(const Triangulation::VerticesVector& vertices_vector, const Triangulation::VerticesGraph& vertices_graph, std::ostream& output)
	{
		if(vertices_vector.size()==vertices_graph.size())
		{
			output.precision(std::numeric_limits<double>::digits10);
			output << std::fixed;
			for(std::size_t i=0;i<vertices_vector.size();i++)
			{
				const Quadruple& quadruple=vertices_vector[i].first;
				const SimpleSphere& tangent_sphere=vertices_vector[i].second;
				const std::vector<std::size_t>& links=vertices_graph[i];
				output << quadruple.get(0) << " " << quadruple.get(1) << " " << quadruple.get(2) << " " << quadruple.get(3) << " ";
				output << tangent_sphere.x << " " << tangent_sphere.y << " " << tangent_sphere.z << " " << tangent_sphere.r << " ";
				for(std::size_t j=0;j<links.size();j++)
				{
					if(links[j]==npos)
					{
						output << "-1";
					}
					else
					{
						output << links[j];
					}
					output << (j+1<links.size() ? " " : "\n");
				}
			}
		}
	}

};

}

}

#endif /* APOLLOTA_TRIANGULATION_OUTPUT_H_ */
