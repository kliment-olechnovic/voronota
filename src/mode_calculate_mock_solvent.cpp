#include <iostream>
#include <stdexcept>

#include "apollota/triangulation_queries.h"
#include "apollota/subdivided_icosahedron.h"
#include "apollota/spheres_boundary_construction.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

#include "modescommon/ball_value.h"
#include "modescommon/mock_solvent_utilities.h"

void calculate_mock_solvent(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'annotation x y z r tags adjuncts')");
	pohw.describe_io("stdout", false, true, "list of balls (line format: 'annotation x y z r tags adjuncts')");

	const double solvent_radius=poh.restrict_value_in_range(1.0, 2.0, poh.argument<double>(pohw.describe_option("--solvent-radius", "number", "solvent atom radius"), 1.4));
	const double solvent_distance=poh.restrict_value_in_range(0.0, 10.0, poh.argument<double>(pohw.describe_option("--solvent-distance", "number", "min distance from non-solvent atoms to solvent atoms"), 1.0));
	const int sih_depth=poh.restrict_value_in_range(0, 3, poh.argument<int>(pohw.describe_option("--sih-depth", "number", "spherical surface optimization depth"), 1));
	const bool sparse_mode=poh.contains_option(pohw.describe_option("--sparse-mode", "", "flag to use sparse mode"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const double probe=(solvent_radius+solvent_distance);

	std::vector<apollota::SimpleSphere> input_spheres;
	std::vector< std::pair<auxiliaries::ChainResidueAtomDescriptor, BallValue> > ball_records;
	auxiliaries::IOUtilities().read_lines_to_map(std::cin, ball_records);
	input_spheres.reserve(ball_records.size());
	for(std::size_t i=0;i<ball_records.size();i++)
	{
		input_spheres.push_back(apollota::SimpleSphere(ball_records[i].second));
	}
	if(input_spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	std::vector<apollota::SimpleSphere> mock_solvent_spheres;

	if(sparse_mode)
	{
		int stage=0;
		while(stage<1)
		{
			std::vector<apollota::SimpleSphere> spheres=input_spheres;
			if(!mock_solvent_spheres.empty())
			{
				spheres.insert(spheres.end(), mock_solvent_spheres.begin(), mock_solvent_spheres.end());
			}
			const std::size_t real_spheres_count=spheres.size();

			const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(spheres, probe*2.0);
			spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());

			const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, false, false);
			const apollota::Triangulation::VerticesVector vertices=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);
			const apollota::TriangulationQueries::IDsGraph ids_graph=apollota::TriangulationQueries::collect_ids_graph_from_ids_map(apollota::TriangulationQueries::collect_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map), real_spheres_count);

			apollota::TriangulationQueries::TriplesMap triples_exposed;
			for(std::size_t i=0;i<vertices.size();i++)
			{
				if(vertices[i].second.r>probe)
				{
					const apollota::Quadruple& q=vertices[i].first;
					for(unsigned int j=0;j<4;j++)
					{
						const apollota::Triple& t=q.exclude(j);
						const std::size_t n=q.get(j);
						if((stage==0 && t.get_min_max().second<real_spheres_count && t.get_min_max().first<input_spheres.size()) || stage>0)
						{
							triples_exposed[t].insert(n);
						}
					}
				}
			}

			std::vector<apollota::SimpleSphere> new_mock_solvent_spheres;
			new_mock_solvent_spheres.reserve(triples_exposed.size()*2);
			for(apollota::TriangulationQueries::TriplesMap::const_iterator it=triples_exposed.begin();it!=triples_exposed.end();++it)
			{
				const apollota::Triple& t=it->first;
				const std::vector<apollota::SimpleSphere> tangents=apollota::TangentSphereOfThreeSpheres::calculate(spheres[t.get(0)], spheres[t.get(1)], spheres[t.get(2)], probe);
				if(!tangents.empty())
				{
					std::set<std::size_t> ns;
					ns.insert(ids_graph[t.get(0)].begin(), ids_graph[t.get(0)].end());
					ns.insert(ids_graph[t.get(1)].begin(), ids_graph[t.get(1)].end());
					ns.insert(ids_graph[t.get(2)].begin(), ids_graph[t.get(2)].end());
					for(std::size_t i=0;i<tangents.size();i++)
					{
						bool candidate_good=true;
						for(std::set<std::size_t>::const_iterator jt=ns.begin();jt!=ns.end() && candidate_good;++jt)
						{
							if(apollota::sphere_intersects_sphere(tangents[i], spheres[*jt]))
							{
								candidate_good=false;
							}
						}
						if(candidate_good)
						{
							new_mock_solvent_spheres.push_back(apollota::SimpleSphere(tangents[i], solvent_radius));
						}
					}
				}
			}

			if(!new_mock_solvent_spheres.empty())
			{
				mock_solvent_spheres.insert(mock_solvent_spheres.end(), new_mock_solvent_spheres.begin(), new_mock_solvent_spheres.end());
			}

			if((stage==0 && new_mock_solvent_spheres.empty()) || stage>0)
			{
				stage++;
			}
		}
	}
	else
	{
		std::vector<apollota::SimpleSphere> spheres=input_spheres;
		const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(spheres, probe*2.0);
		spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());

		const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, false, false);
		const apollota::TriangulationQueries::IDsMap ids_map=apollota::TriangulationQueries::collect_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map);
		const apollota::TriangulationQueries::IDsGraph ids_graph=apollota::TriangulationQueries::collect_ids_graph_from_ids_map(ids_map, input_spheres.size());

		const apollota::SubdividedIcosahedron initial_sih(sih_depth);

		mock_solvent_spheres.reserve(ids_graph.size()*2);
		for(std::size_t i=0;i<ids_graph.size();i++)
		{
			apollota::SubdividedIcosahedron sih=initial_sih;
			sih.fit_into_sphere(spheres[i], spheres[i].r+probe);
			for(std::size_t l=0;l<sih.vertices().size();l++)
			{
				apollota::SimpleSphere sphere_l(sih.vertices()[l], solvent_radius);
				const double dist_i=apollota::minimal_distance_from_point_to_sphere(sphere_l, spheres[i]);
				bool good=true;
				for(std::size_t j=0;j<ids_graph[i].size() && good;j++)
				{
					const double dist_j=apollota::minimal_distance_from_point_to_sphere(sphere_l, spheres[ids_graph[i][j]]);
					good=(dist_i<dist_j);
				}
				if(good)
				{
					mock_solvent_spheres.push_back(sphere_l);
				}
			}
		}
	}

	ball_records.reserve(ball_records.size()+mock_solvent_spheres.size());
	for(std::size_t i=0;i<mock_solvent_spheres.size();i++)
	{
		const apollota::SimpleSphere& sphere=mock_solvent_spheres[i];
		const BallValue value=apollota::custom_sphere<BallValue>(sphere.x, sphere.y, sphere.z, sphere.r);
		auxiliaries::ChainResidueAtomDescriptor crad(mock_solvent_name());
		crad.resSeq=(i+1);
		crad.resName=mock_solvent_name();
		crad.name=mock_solvent_name();
		ball_records.push_back(std::make_pair(crad, value));
	}

	for(std::size_t i=0;i<ball_records.size();i++)
	{
		std::cout << ball_records[i].first << " " << ball_records[i].second << "\n";
	}
}
