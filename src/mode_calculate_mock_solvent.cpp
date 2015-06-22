#include <iostream>
#include <stdexcept>

#include "apollota/triangulation_queries.h"
#include "apollota/subdivided_icosahedron.h"
#include "apollota/spheres_boundary_construction.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

#include "modescommon/ball_value.h"

void calculate_mock_solvent(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--solvent-radius", "number", "solvent atom radius"));
		ods.push_back(OD("--solvent-distance", "number", "min distance from non-solvent atoms to solvent atoms"));
		ods.push_back(OD("--sih-depth", "number", "spherical surface optimization depth"));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false,
					"list of balls (line format: 'annotation x y z r tags adjuncts')");
			poh.print_io_description("stdout", false, true,
					"list of balls (line format: 'annotation x y z r tags adjuncts')");
			return;
		}
	}

	const double solvent_radius=std::max(1.0, std::min(2.0, poh.argument<double>("--solvent-radius", 1.4)));
	const double solvent_distance=std::max(0.0, std::min(100.0, poh.argument<double>("--solvent-distance", 1.0)));
	const int sih_depth=std::max(0, std::min(3, poh.argument<int>("--sih-depth", 0)));

	const double probe=(solvent_radius+solvent_distance);

	std::vector<apollota::SimpleSphere> spheres;
	std::vector< std::pair<auxiliaries::ChainResidueAtomDescriptor, BallValue> > ball_records;
	auxiliaries::IOUtilities().read_lines_to_map(std::cin, ball_records);
	spheres.reserve(ball_records.size());
	for(std::size_t i=0;i<ball_records.size();i++)
	{
		spheres.push_back(apollota::SimpleSphere(ball_records[i].second));
	}
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	const std::size_t input_spheres_count=spheres.size();
	const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(spheres, probe*2.0);
	spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, false, false);
	const apollota::TriangulationQueries::IDsMap ids_map=apollota::TriangulationQueries::collect_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map);
	const apollota::TriangulationQueries::IDsGraph ids_graph=apollota::TriangulationQueries::collect_ids_graph_from_ids_map(ids_map, input_spheres_count);

	const apollota::SubdividedIcosahedron initial_sih(sih_depth);

	std::vector<apollota::SimpleSphere> mock_solvent_spheres;
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

	ball_records.reserve(ball_records.size()+mock_solvent_spheres.size());
	for(std::size_t i=0;i<mock_solvent_spheres.size();i++)
	{
		const apollota::SimpleSphere& sphere=mock_solvent_spheres[i];
		const BallValue value=apollota::custom_sphere<BallValue>(sphere.x, sphere.y, sphere.z, sphere.r);
		auxiliaries::ChainResidueAtomDescriptor crad("w");
		crad.resSeq=(i+1);
		crad.resName="w";
		crad.name="w";
		ball_records.push_back(std::make_pair(crad, value));
	}

	for(std::size_t i=0;i<ball_records.size();i++)
	{
		std::cout << ball_records[i].first << " " << ball_records[i].second << "\n";
	}
}
