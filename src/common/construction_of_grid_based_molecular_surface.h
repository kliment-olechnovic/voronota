#ifndef COMMON_CONSTRUCTION_OF_GRID_BASED_MOLECULAR_SURFACE_H_
#define COMMON_CONSTRUCTION_OF_GRID_BASED_MOLECULAR_SURFACE_H_

#include "../auxiliaries/grid_edt_tools.h"
#include "../apollota/basic_operations_on_spheres.h"

#include "construction_of_atomic_balls.h"

namespace voronota
{

namespace common
{

class ConstructionOfGridBasedMolecularSurface
{
public:
	typedef ConstructionOfAtomicBalls::AtomicBall Atom;

	struct Parameters
	{
		auxiliaries::GridEDTTools::SurfaceConstruction::Parameters grid_surface_parameters;
		double balls_radius_addition;

		Parameters() : balls_radius_addition(0.05)
		{
		}

		Parameters(const double vdw_radius_expansion, const int type_id, const double probe, const double grid_step, const int smoothing) : balls_radius_addition(vdw_radius_expansion)
		{
			grid_surface_parameters.solvent_excluded=(type_id==1);
			grid_surface_parameters.probe=probe;
			grid_surface_parameters.grid_step=grid_step;
			grid_surface_parameters.smoothing_iterations=smoothing;
		}
	};

	struct BundleOfMeshInformation
	{
		std::vector<float> global_buffer_of_vertices;
		std::vector<float> global_buffer_of_normals;
		std::vector<unsigned int> global_buffer_of_indices;
		std::vector< std::vector<unsigned int> > mapped_indices;

		void clear()
		{
			global_buffer_of_vertices.clear();
			global_buffer_of_normals.clear();
			global_buffer_of_indices.clear();
			mapped_indices.clear();
		}
	};

	static bool construct_bundle_of_mesh_information(
			const Parameters& parameters,
			const std::vector<Atom>& atoms,
			BundleOfMeshInformation& bundle)
	{
		bundle.clear();

		if(atoms.empty())
		{
			return false;
		}

		std::vector<apollota::SimpleSphere> balls=ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(atoms);
		for(std::size_t i=0;i<balls.size();i++)
		{
			balls[i].r+=parameters.balls_radius_addition;
		}

		const auxiliaries::GridEDTTools::Surface surface=auxiliaries::GridEDTTools::SurfaceConstruction::construct_surface(parameters.grid_surface_parameters, balls);

		if(surface.map_of_ball_nums_to_triple_nums.empty())
		{
			return false;
		}

		bundle.global_buffer_of_vertices.resize(surface.vertices.size()*3);
		bundle.global_buffer_of_normals.resize(surface.vertices.size()*3);
		bundle.global_buffer_of_indices.resize(surface.triples.size());
		bundle.mapped_indices.resize(surface.map_of_ball_nums_to_triple_nums.size());

		for(std::size_t i=0;i<surface.vertices.size();i++)
		{
			bundle.global_buffer_of_vertices[i*3+0]=static_cast<float>(surface.vertices[i].center[0]);
			bundle.global_buffer_of_vertices[i*3+1]=static_cast<float>(surface.vertices[i].center[1]);
			bundle.global_buffer_of_vertices[i*3+2]=static_cast<float>(surface.vertices[i].center[2]);
			bundle.global_buffer_of_normals[i*3+0]=static_cast<float>(surface.vertices[i].normal[0]);
			bundle.global_buffer_of_normals[i*3+1]=static_cast<float>(surface.vertices[i].normal[1]);
			bundle.global_buffer_of_normals[i*3+2]=static_cast<float>(surface.vertices[i].normal[2]);
		}

		for(std::size_t i=0;i<surface.triples.size();i++)
		{
			bundle.global_buffer_of_indices[i]=static_cast<unsigned int>(surface.triples[i]);
		}

		for(std::size_t i=0;i<surface.map_of_ball_nums_to_triple_nums.size();i++)
		{
			bundle.mapped_indices[i].resize(surface.map_of_ball_nums_to_triple_nums[i].size()*3);
			for(std::size_t j=0;j<surface.map_of_ball_nums_to_triple_nums[i].size();j++)
			{
				const int triple_num=surface.map_of_ball_nums_to_triple_nums[i][j];
				bundle.mapped_indices[i][j*3+0]=surface.triples[triple_num*3+0];
				bundle.mapped_indices[i][j*3+1]=surface.triples[triple_num*3+1];
				bundle.mapped_indices[i][j*3+2]=surface.triples[triple_num*3+2];
			}
		}

		return (!bundle.global_buffer_of_indices.empty());
	}
};

}

}

#endif /* COMMON_CONSTRUCTION_OF_GRID_BASED_MOLECULAR_SURFACE_H_ */

