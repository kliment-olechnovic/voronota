#ifndef AUXILIARIES_GRID_EDT_TOOLS_H_
#define AUXILIARIES_GRID_EDT_TOOLS_H_

#include <vector>
#include <set>
#include <queue>
#include <algorithm>
#include <limits>
#include <cmath>

namespace voronota
{

namespace auxiliaries
{

class GridEDTTools
{
public:
	struct Ball
	{
		double xyz[3];
		double r;
	};

	struct Surface
	{
		struct Vertex
		{
			int ball_num;
			double center_weight;
			double normal_weight;
			double center[3];
			double normal[3];

			Vertex() : ball_num(0), center_weight(0.0), normal_weight(0.0)
			{
				center[0]=0.0;
				center[1]=0.0;
				center[2]=0.0;
				normal[0]=0.0;
				normal[1]=0.0;
				normal[2]=0.0;
			}
		};

		std::vector<Vertex> vertices;
		std::vector<int> triples;
		std::vector< std::vector<int> > map_of_ball_nums_to_triple_nums;
		std::vector<int> map_of_triple_nums_to_ball_nums;
	};

	class Grid
	{
	public:
		struct Voxel
		{
			enum VoxelType
			{
				VOXEL_TYPE_UNASSIGNED,
				VOXEL_TYPE_CENTER,
				VOXEL_TYPE_STAGE1_IN_SAS,
				VOXEL_TYPE_STAGE1_OUT_SAS,
				VOXEL_TYPE_STAGE1_SURFACE_SAS,
				VOXEL_TYPE_STAGE2_IN_SAS_OUT_SES,
				VOXEL_TYPE_STAGE2_IN_SAS_IN_SES,
				VOXEL_TYPE_STAGE2_SURFACE_SES
			};

			VoxelType voxel_type;
			int ball_num;
			int source_voxel_id;
			double distance_to_source;

			Voxel() :
				voxel_type(VOXEL_TYPE_UNASSIGNED),
				ball_num(std::numeric_limits<int>::max()),
				source_voxel_id(std::numeric_limits<int>::max()),
				distance_to_source(std::numeric_limits<double>::max())
			{
			}

			inline bool in_sas() const
			{
				return (voxel_type==VOXEL_TYPE_STAGE1_SURFACE_SAS || voxel_type==VOXEL_TYPE_STAGE2_IN_SAS_OUT_SES || in_ses());
			}

			inline bool in_ses() const
			{
				return (voxel_type==VOXEL_TYPE_STAGE2_SURFACE_SES || voxel_type==VOXEL_TYPE_STAGE2_IN_SAS_IN_SES || voxel_type==VOXEL_TYPE_STAGE1_IN_SAS || voxel_type==VOXEL_TYPE_CENTER);
			}

			inline bool surface_sas() const
			{
				return (voxel_type==VOXEL_TYPE_STAGE1_SURFACE_SAS);
			}

			inline bool surface_ses() const
			{
				return (voxel_type==VOXEL_TYPE_STAGE2_SURFACE_SES);
			}
		};

		Grid() : probe_(0.0), step_(0.0), number_of_cell_shifts_to_use_(0)
		{
			int cell_shift_id=0;
			for(int x=-1;x<=1;x++)
			{
				for(int y=-1;y<=1;y++)
				{
					for(int z=-1;z<=1;z++)
					{
						if((std::abs(x)+std::abs(y)+std::abs(z))==1)
						{
							cell_shifts_[cell_shift_id][0]=x;
							cell_shifts_[cell_shift_id][1]=y;
							cell_shifts_[cell_shift_id][2]=z;
							cell_shift_id++;
						}
					}
				}
			}
			for(int x=-1;x<=1;x++)
			{
				for(int y=-1;y<=1;y++)
				{
					for(int z=-1;z<=1;z++)
					{
						if(!((x==0 && y==0 && z==0) || (std::abs(x)+std::abs(y)+std::abs(z))==1))
						{
							cell_shifts_[cell_shift_id][0]=x;
							cell_shifts_[cell_shift_id][1]=y;
							cell_shifts_[cell_shift_id][2]=z;
							cell_shift_id++;
						}
					}
				}
			}
		}

		void init(
				const double probe,
				const double step,
				const int max_allowed_voxels_count,
				const bool reduced_step_directions,
				const std::vector<Ball>& balls)
		{
			probe_=probe;
			step_=step;
			number_of_cell_shifts_to_use_=(reduced_step_directions ? 6 : 26);
			balls_=balls;
			grid_vector_.clear();

			Ball min_coords=balls_[0];
			Ball max_coords=balls_[0];

			for(std::size_t i=0;i<balls_.size();i++)
			{
				for(int j=0;j<3;j++)
				{
					min_coords.xyz[j]=std::min(min_coords.xyz[j], balls_[i].xyz[j]);
					max_coords.xyz[j]=std::max(max_coords.xyz[j], balls_[i].xyz[j]);
				}
				min_coords.r=std::min(min_coords.r, balls_[i].r);
				max_coords.r=std::max(max_coords.r, balls_[i].r);
			}

			const double padding=max_coords.r+probe_*1.5;

			for(int j=0;j<3;j++)
			{
				min_coords.xyz[j]-=padding;
				max_coords.xyz[j]+=padding;
			}

			for(int j=0;j<3;j++)
			{
				init_shift_[j]=-min_coords.xyz[j];
				min_coords.xyz[j]+=init_shift_[j];
				max_coords.xyz[j]+=init_shift_[j];
			}

			for(int j=0;j<3;j++)
			{
				dimensions_[j]=static_cast<int>(max_coords.xyz[j]/step_)+1;
			}

			int voxels_count=(dimensions_[0]*dimensions_[1]*dimensions_[2]);

			while(voxels_count<0)
			{
				step_*=2.0;
				for(int j=0;j<3;j++)
				{
					dimensions_[j]=static_cast<int>(max_coords.xyz[j]/step_)+1;
				}
				voxels_count=(dimensions_[0]*dimensions_[1]*dimensions_[2]);
			}

			if(voxels_count>max_allowed_voxels_count)
			{
				step_*=std::pow(static_cast<double>(voxels_count)/static_cast<double>(max_allowed_voxels_count), 1.0/3.0);
				for(int j=0;j<3;j++)
				{
					dimensions_[j]=static_cast<int>(max_coords.xyz[j]/step_)+1;
				}
				voxels_count=(dimensions_[0]*dimensions_[1]*dimensions_[2]);
			}

			grid_vector_.resize(dimensions_[0]*dimensions_[1]*dimensions_[2]);

			for(std::size_t i=0;i<balls_.size();i++)
			{
				int grid_coords[3]={0,0,0};
				get_grid_coords_from_real_coords(balls_[i].xyz, grid_coords);
				const int id=get_id_from_grid_coords(grid_coords);
				Voxel& voxel=grid_vector_[id];
				voxel.voxel_type=Voxel::VOXEL_TYPE_CENTER;
				voxel.ball_num=static_cast<int>(i);
			}

			propagate();
		}

		void construct_surface(
				const bool solvent_excluded,
				const int smoothing_iterations,
				const double smoothing_self_proportion,
				const bool correct_burried_vertices,
				const int split_mode,
				Surface& surface)
		{
			for(std::size_t i=0;i<grid_vector_.size();i++)
			{
				Voxel& voxel=grid_vector_[i];
				if(solvent_excluded)
				{
					voxel.source_voxel_id=(voxel.in_ses() ? 1 : 0);
				}
				else
				{
					voxel.source_voxel_id=(voxel.in_sas() ? 1 : 0);
				}
			}

			std::vector<int> surface_triples_of_grid_ids;
			surface_triples_of_grid_ids.reserve(balls_.size()*100);
			std::vector<double> surface_triples_normals;
			surface_triples_normals.reserve(balls_.size()*100);

			const VertexConnectedMarchingCubesMapping vcmc_mapping;

			{
				int start_gc[3];
				int neighbor_gc[3];
				int cube_ids[8];
				for(int start_id=0;start_id<static_cast<int>(grid_vector_.size());start_id++)
				{
					get_grid_coords_from_id(start_id, start_gc);
					if((start_gc[0]+1)<dimensions_[0] && (start_gc[1]+1)<dimensions_[1] && (start_gc[2]+1)<dimensions_[2])
					{
						int code_num=0;
						for(int i=0;i<8;i++)
						{
							neighbor_gc[0]=start_gc[0]+vcmc_mapping.basic_cube_cordinates[i][0];
							neighbor_gc[1]=start_gc[1]+vcmc_mapping.basic_cube_cordinates[i][1];
							neighbor_gc[2]=start_gc[2]+vcmc_mapping.basic_cube_cordinates[i][2];
							cube_ids[i]=get_id_from_grid_coords(neighbor_gc);
							code_num+=(grid_vector_[cube_ids[i]].source_voxel_id << i);
						}
						for(int i=0;i<vcmc_mapping.map_of_triangles[code_num][0];i++)
						{
							surface_triples_of_grid_ids.push_back(cube_ids[vcmc_mapping.map_of_triangles[code_num][1+i*4+0]]);
							surface_triples_of_grid_ids.push_back(cube_ids[vcmc_mapping.map_of_triangles[code_num][1+i*4+1]]);
							surface_triples_of_grid_ids.push_back(cube_ids[vcmc_mapping.map_of_triangles[code_num][1+i*4+2]]);
							surface_triples_normals.push_back(vcmc_mapping.map_of_normals[code_num][i*3+0]);
							surface_triples_normals.push_back(vcmc_mapping.map_of_normals[code_num][i*3+1]);
							surface_triples_normals.push_back(vcmc_mapping.map_of_normals[code_num][i*3+2]);
						}
					}
				}
			}

			{
				for(std::size_t i=0;i<grid_vector_.size();i++)
				{
					grid_vector_[i].source_voxel_id=-1;
				}

				int surface_vertices_count=0;

				for(std::size_t i=0;i<surface_triples_of_grid_ids.size();i++)
				{
					Voxel& voxel=grid_vector_[surface_triples_of_grid_ids[i]];
					if(voxel.source_voxel_id<0)
					{
						voxel.source_voxel_id=surface_vertices_count++;
					}
				}

				surface.vertices.resize(surface_vertices_count);
				surface.triples.resize(surface_triples_of_grid_ids.size());
				surface.map_of_ball_nums_to_triple_nums.clear();
				surface.map_of_triple_nums_to_ball_nums.clear();

				for(std::size_t i=0;i<surface_triples_of_grid_ids.size();i++)
				{
					const int center_id=surface_triples_of_grid_ids[i];
					Voxel& voxel=grid_vector_[surface_triples_of_grid_ids[i]];
					surface.triples[i]=voxel.source_voxel_id;
					Surface::Vertex& vertex=surface.vertices[voxel.source_voxel_id];
					if(vertex.center_weight<1.0)
					{
						vertex.ball_num=voxel.ball_num;
						int center_gc[3];
						get_grid_coords_from_id(center_id, center_gc);
						get_real_coords_from_grid_coords(center_gc, vertex.center);
						vertex.center_weight=1.0;
					}
				}

				for(std::size_t i=0;i<surface.triples.size()/3;i++)
				{
					const int ids[3]={surface.triples[i*3+0], surface.triples[i*3+1], surface.triples[i*3+2]};
					for(int j=0;j<3;j++)
					{
						surface.vertices[ids[j]].normal[0]+=surface_triples_normals[i*3+0];
						surface.vertices[ids[j]].normal[1]+=surface_triples_normals[i*3+1];
						surface.vertices[ids[j]].normal[2]+=surface_triples_normals[i*3+2];
						surface.vertices[ids[j]].normal_weight+=1.0;
					}
				}

				for(std::size_t i=0;i<surface.vertices.size();i++)
				{
					for(int j=0;j<3;j++)
					{
						surface.vertices[i].normal[j]/=surface.vertices[i].normal_weight;
					}
					surface.vertices[i].normal_weight=1.0;
				}
			}

			for(int si=0;si<smoothing_iterations;si++)
			{
				std::vector<Surface::Vertex> smoothed_vertices=surface.vertices;

				for(std::size_t i=0;i<smoothed_vertices.size();i++)
				{
					smoothed_vertices[i].center_weight=0.0;
					smoothed_vertices[i].normal_weight=0.0;
					for(int j=0;j<3;j++)
					{
						smoothed_vertices[i].center[j]*=smoothed_vertices[i].center_weight;
						smoothed_vertices[i].normal[j]*=smoothed_vertices[i].normal_weight;
					}
				}

				for(std::size_t i=0;i<surface.triples.size()/3;i++)
				{
					const int ids[3]={surface.triples[i*3+0], surface.triples[i*3+1], surface.triples[i*3+2]};
					{
						const double* va0=&(surface.vertices[ids[0]].center[0]);
						const double* va1=&(surface.vertices[ids[1]].center[0]);
						const double* va2=&(surface.vertices[ids[2]].center[0]);
						const double vsum[3][3]={
								{va1[0]+va2[0], va1[1]+va2[1], va1[2]+va2[2]},
								{va0[0]+va2[0], va0[1]+va2[1], va0[2]+va2[2]},
								{va0[0]+va1[0], va0[1]+va1[1], va0[2]+va1[2]}};
						for(int j=0;j<3;j++)
						{
							smoothed_vertices[ids[j]].center[0]+=vsum[j][0];
							smoothed_vertices[ids[j]].center[1]+=vsum[j][1];
							smoothed_vertices[ids[j]].center[2]+=vsum[j][2];
							smoothed_vertices[ids[j]].center_weight+=2.0;
						}
					}
					{
						const double* va0=&(surface.vertices[ids[0]].normal[0]);
						const double* va1=&(surface.vertices[ids[1]].normal[0]);
						const double* va2=&(surface.vertices[ids[2]].normal[0]);
						const double vsum[3][3]={
								{va1[0]+va2[0], va1[1]+va2[1], va1[2]+va2[2]},
								{va0[0]+va2[0], va0[1]+va2[1], va0[2]+va2[2]},
								{va0[0]+va1[0], va0[1]+va1[1], va0[2]+va1[2]}};
						for(int j=0;j<3;j++)
						{
							smoothed_vertices[ids[j]].normal[0]+=vsum[j][0];
							smoothed_vertices[ids[j]].normal[1]+=vsum[j][1];
							smoothed_vertices[ids[j]].normal[2]+=vsum[j][2];
							smoothed_vertices[ids[j]].normal_weight+=2.0;
						}
					}
				}

				for(std::size_t i=0;i<smoothed_vertices.size();i++)
				{
					for(int j=0;j<3;j++)
					{
						smoothed_vertices[i].center[j]/=(smoothed_vertices[i].center_weight);
						smoothed_vertices[i].normal[j]/=(smoothed_vertices[i].normal_weight);
						if(smoothing_self_proportion>0.0)
						{
							smoothed_vertices[i].center[j]=smoothed_vertices[i].center[j]*(1.0-smoothing_self_proportion)+(surface.vertices[i].center[j]*smoothing_self_proportion);
							smoothed_vertices[i].normal[j]=smoothed_vertices[i].normal[j]*(1.0-smoothing_self_proportion)+(surface.vertices[i].normal[j]*smoothing_self_proportion);
						}
					}
					smoothed_vertices[i].center_weight=1.0;
					smoothed_vertices[i].normal_weight=1.0;
				}

				surface.vertices.swap(smoothed_vertices);
			}

			if(correct_burried_vertices)
			{
				for(std::size_t i=0;i<surface.vertices.size();i++)
				{
					Surface::Vertex& vertex=surface.vertices[i];
					const Ball& ball=balls_[vertex.ball_num];
					const double ref_radius=(solvent_excluded ? ball.r : (ball.r+probe_));
					double r[3]={vertex.center[0]-ball.xyz[0], vertex.center[1]-ball.xyz[1], vertex.center[2]-ball.xyz[2]};
					const double dist_squared=r[0]*r[0]+r[1]*r[1]+r[2]*r[2];
					if(dist_squared<(ref_radius*ref_radius))
					{
						const double scale=ref_radius/sqrt(dist_squared);
						vertex.center[0]=ball.xyz[0]+(r[0]*scale);
						vertex.center[1]=ball.xyz[1]+(r[1]*scale);
						vertex.center[2]=ball.xyz[2]+(r[2]*scale);
					}
				}
			}

			if(split_mode>0)
			{
				Surface split_surface;
				split_surface.vertices=surface.vertices;
				split_surface.triples.reserve(surface.triples.size()+balls_.size()*30);
				for(std::size_t i=0;i<surface.triples.size()/3;i++)
				{
					const int ids[3]={surface.triples[i*3+0], surface.triples[i*3+1], surface.triples[i*3+2]};
					int v_ball_nums[3]={split_surface.vertices[ids[0]].ball_num, split_surface.vertices[ids[1]].ball_num, split_surface.vertices[ids[2]].ball_num};
					if(v_ball_nums[0]==v_ball_nums[1] && v_ball_nums[0]==v_ball_nums[2])
					{
						split_surface.triples.push_back(ids[0]);
						split_surface.triples.push_back(ids[1]);
						split_surface.triples.push_back(ids[2]);
					}
					else
					{
						int new_ids[9];
						for(int j=0;j<9;j++)
						{
							new_ids[j]=split_surface.vertices.size();
							split_surface.vertices.push_back(Surface::Vertex());
						}

						Surface::Vertex* new_v[9];
						for(int j=0;j<9;j++)
						{
							new_v[j]=&(split_surface.vertices[new_ids[j]]);
							new_v[j]->ball_num=v_ball_nums[j/3];
						}

						Surface::Vertex* v[3]={&(split_surface.vertices[ids[0]]), &(split_surface.vertices[ids[1]]), &(split_surface.vertices[ids[2]])};

						{
							calc_line_strip_middle_point(v[0]->center, v[1]->center, new_v[0]->center);
							calc_triangle_middle_point(v[0]->center, v[1]->center, v[2]->center, new_v[1]->center);
							calc_line_strip_middle_point(v[0]->center, v[2]->center, new_v[2]->center);

							calc_line_strip_middle_point(v[1]->center, v[0]->center, new_v[3]->center);
							calc_triangle_middle_point(v[0]->center, v[1]->center, v[2]->center, new_v[4]->center);
							calc_line_strip_middle_point(v[1]->center, v[2]->center, new_v[5]->center);

							calc_line_strip_middle_point(v[2]->center, v[0]->center, new_v[6]->center);
							calc_triangle_middle_point(v[0]->center, v[1]->center, v[2]->center, new_v[7]->center);
							calc_line_strip_middle_point(v[2]->center, v[1]->center, new_v[8]->center);
						}

						{
							calc_line_strip_middle_point(v[0]->normal, v[1]->normal, new_v[0]->normal);
							calc_triangle_middle_point(v[0]->normal, v[1]->normal, v[2]->normal, new_v[1]->normal);
							calc_line_strip_middle_point(v[0]->normal, v[2]->normal, new_v[2]->normal);

							calc_line_strip_middle_point(v[1]->normal, v[0]->normal, new_v[3]->normal);
							calc_triangle_middle_point(v[0]->normal, v[1]->normal, v[2]->normal, new_v[4]->normal);
							calc_line_strip_middle_point(v[1]->normal, v[2]->normal, new_v[5]->normal);

							calc_line_strip_middle_point(v[2]->normal, v[0]->normal, new_v[6]->normal);
							calc_triangle_middle_point(v[0]->normal, v[1]->normal, v[2]->normal, new_v[7]->normal);
							calc_line_strip_middle_point(v[2]->normal, v[1]->normal, new_v[8]->normal);
						}

						{
							split_surface.triples.push_back(ids[0]);
							split_surface.triples.push_back(new_ids[0]);
							split_surface.triples.push_back(new_ids[1]);
							split_surface.triples.push_back(ids[0]);
							split_surface.triples.push_back(new_ids[1]);
							split_surface.triples.push_back(new_ids[2]);
						}

						{
							split_surface.triples.push_back(ids[1]);
							split_surface.triples.push_back(new_ids[3]);
							split_surface.triples.push_back(new_ids[4]);
							split_surface.triples.push_back(ids[1]);
							split_surface.triples.push_back(new_ids[4]);
							split_surface.triples.push_back(new_ids[5]);
						}

						{
							split_surface.triples.push_back(ids[2]);
							split_surface.triples.push_back(new_ids[6]);
							split_surface.triples.push_back(new_ids[7]);
							split_surface.triples.push_back(ids[2]);
							split_surface.triples.push_back(new_ids[7]);
							split_surface.triples.push_back(new_ids[8]);
						}
					}
				}
				surface.vertices.swap(split_surface.vertices);
				surface.triples.swap(split_surface.triples);
			}

			surface.map_of_ball_nums_to_triple_nums.resize(balls_.size());
			surface.map_of_triple_nums_to_ball_nums.resize(surface.triples.size()/3);

			for(int i=0;i<static_cast<int>(surface.triples.size())/3;i++)
			{
				int recorded_ball_num=-1;
				for(int j=0;j<3 && recorded_ball_num<0;j++)
				{
					recorded_ball_num=surface.vertices[surface.triples[i*3+j]].ball_num;
				}
				if(recorded_ball_num>=0)
				{
					surface.map_of_ball_nums_to_triple_nums[recorded_ball_num].push_back(i);
					surface.map_of_triple_nums_to_ball_nums[i]=recorded_ball_num;
				}
			}
		}

	private:
		void propagate()
		{
			const int grid_size=static_cast<int>(grid_vector_.size());
			const int balls_count=static_cast<int>(balls_.size());

			std::queue<int> queue;

			for(int id=0;id<grid_size;id++)
			{
				Voxel& v=grid_vector_[id];
				if(v.voxel_type==Voxel::VOXEL_TYPE_CENTER)
				{
					v.source_voxel_id=id;
					v.distance_to_source=0.0;
					queue.push(id);
				}
			}

			while(!queue.empty())
			{
				const int center_id=queue.front();
				queue.pop();
				Voxel& center_v=grid_vector_[center_id];
				int center_gc[3];
				get_grid_coords_from_id(center_id, center_gc);
				for(int cell_shift_id=0;cell_shift_id<number_of_cell_shifts_to_use_;cell_shift_id++)
				{
					int neighbor_gc[3];
					get_shifted_grid_coords_from_grid_coords_and_cell_shift_id(center_gc, cell_shift_id, neighbor_gc);
					if(check_grid_coords_in_range(neighbor_gc))
					{
						const int neighbor_id=get_id_from_grid_coords(neighbor_gc);
						Voxel& neighbor_v=grid_vector_[neighbor_id];
						if(neighbor_v.source_voxel_id!=center_v.source_voxel_id)
						{
							const double candidate_distance=calc_real_distance_between_grid_coords_and_real_ball(neighbor_gc, center_v.ball_num);
							if(!(neighbor_v.ball_num<balls_count) || neighbor_v.distance_to_source>candidate_distance)
							{
								neighbor_v.ball_num=center_v.ball_num;
								neighbor_v.source_voxel_id=center_v.source_voxel_id;
								neighbor_v.distance_to_source=candidate_distance;
								if(candidate_distance<=probe_)
								{
									neighbor_v.voxel_type=Voxel::VOXEL_TYPE_STAGE1_IN_SAS;
									queue.push(neighbor_id);
								}
								else
								{
									neighbor_v.voxel_type=Voxel::VOXEL_TYPE_STAGE1_OUT_SAS;
								}
							}
						}
					}
				}
			}

			for(int id=0;id<grid_size;id++)
			{
				grid_vector_[id].source_voxel_id=std::numeric_limits<int>::max();
				grid_vector_[id].distance_to_source=std::numeric_limits<double>::max();
			}

			for(int center_id=0;center_id<grid_size;center_id++)
			{
				if(grid_vector_[center_id].voxel_type==Voxel::VOXEL_TYPE_STAGE1_OUT_SAS)
				{
					int center_gc[3];
					get_grid_coords_from_id(center_id, center_gc);
					for(int cell_shift_id=0;cell_shift_id<number_of_cell_shifts_to_use_;cell_shift_id++)
					{
						int neighbor_gc[3];
						get_shifted_grid_coords_from_grid_coords_and_cell_shift_id(center_gc, cell_shift_id, neighbor_gc);
						if(check_grid_coords_in_range(neighbor_gc))
						{
							const int neighbor_id=get_id_from_grid_coords(neighbor_gc);
							Voxel& neighbor_v=grid_vector_[neighbor_id];
							if(neighbor_v.voxel_type==Voxel::VOXEL_TYPE_STAGE1_IN_SAS)
							{
								neighbor_v.voxel_type=Voxel::VOXEL_TYPE_STAGE1_SURFACE_SAS;
								neighbor_v.source_voxel_id=neighbor_id;
								neighbor_v.distance_to_source=0.0;
								queue.push(neighbor_id);
							}
						}
					}
				}
			}

			while(!queue.empty())
			{
				const int center_id=queue.front();
				queue.pop();
				const Voxel& center_v=grid_vector_[center_id];
				int center_gc[3];
				get_grid_coords_from_id(center_id, center_gc);
				for(int cell_shift_id=0;cell_shift_id<number_of_cell_shifts_to_use_;cell_shift_id++)
				{
					int neighbor_gc[3];
					get_shifted_grid_coords_from_grid_coords_and_cell_shift_id(center_gc, cell_shift_id, neighbor_gc);
					if(check_grid_coords_in_range(neighbor_gc))
					{
						const int neighbor_id=get_id_from_grid_coords(neighbor_gc);
						Voxel& neighbor_v=grid_vector_[neighbor_id];
						if((neighbor_v.voxel_type==Voxel::VOXEL_TYPE_STAGE1_IN_SAS || neighbor_v.voxel_type==Voxel::VOXEL_TYPE_STAGE2_IN_SAS_OUT_SES || neighbor_v.voxel_type==Voxel::VOXEL_TYPE_STAGE2_IN_SAS_IN_SES) && neighbor_v.source_voxel_id!=center_v.source_voxel_id)
						{
							int candidate_source_gc[3];
							get_grid_coords_from_id(center_v.source_voxel_id, candidate_source_gc);
							const double candidate_squared_distance=calc_real_squared_distance_between_grid_coords_and_grid_coords(neighbor_gc, candidate_source_gc);
							if(neighbor_v.distance_to_source>candidate_squared_distance)
							{
								neighbor_v.source_voxel_id=center_v.source_voxel_id;
								neighbor_v.distance_to_source=candidate_squared_distance;
								if(candidate_squared_distance<=(probe_*probe_))
								{
									neighbor_v.voxel_type=Voxel::VOXEL_TYPE_STAGE2_IN_SAS_OUT_SES;
									queue.push(neighbor_id);
								}
								else
								{
									neighbor_v.voxel_type=Voxel::VOXEL_TYPE_STAGE2_IN_SAS_IN_SES;
								}
							}
						}
					}
				}
			}

			for(int center_id=0;center_id<grid_size;center_id++)
			{
				if(grid_vector_[center_id].voxel_type==Voxel::VOXEL_TYPE_STAGE2_IN_SAS_IN_SES)
				{
					int center_gc[3];
					get_grid_coords_from_id(center_id, center_gc);
					for(int cell_shift_id=0;cell_shift_id<number_of_cell_shifts_to_use_;cell_shift_id++)
					{
						int neighbor_gc[3];
						get_shifted_grid_coords_from_grid_coords_and_cell_shift_id(center_gc, cell_shift_id, neighbor_gc);
						if(check_grid_coords_in_range(neighbor_gc))
						{
							const int neighbor_id=get_id_from_grid_coords(neighbor_gc);
							Voxel& neighbor_v=grid_vector_[neighbor_id];
							if(neighbor_v.voxel_type==Voxel::VOXEL_TYPE_STAGE2_IN_SAS_OUT_SES)
							{
								neighbor_v.voxel_type=Voxel::VOXEL_TYPE_STAGE2_SURFACE_SES;
							}
						}
					}
				}
			}
		}

		inline int get_id_from_grid_coords(const int* gc) const
		{
			return (gc[2]*(dimensions_[0]*dimensions_[1])+gc[1]*dimensions_[0]+gc[0]);
		}

		inline void get_grid_coords_from_id(const int id, int* gc) const
		{
			gc[2]=id/(dimensions_[0]*dimensions_[1]);
			gc[1]=(id-gc[2]*(dimensions_[0]*dimensions_[1]))/dimensions_[0];
			gc[0]=(id-gc[2]*(dimensions_[0]*dimensions_[1])-gc[1]*dimensions_[0]);
		}

		inline void get_shifted_grid_coords_from_grid_coords_and_cell_shift_id(const int* gc, const int cell_shift_id, int* shifted_gc) const
		{
			shifted_gc[0]=gc[0]+cell_shifts_[cell_shift_id][0];
			shifted_gc[1]=gc[1]+cell_shifts_[cell_shift_id][1];
			shifted_gc[2]=gc[2]+cell_shifts_[cell_shift_id][2];
		}

		inline void get_grid_coords_from_real_coords(const double* rc, int* gc) const
		{
			gc[0]=static_cast<int>((rc[0]+init_shift_[0])/step_+0.5);
			gc[1]=static_cast<int>((rc[1]+init_shift_[1])/step_+0.5);
			gc[2]=static_cast<int>((rc[2]+init_shift_[2])/step_+0.5);
		}

		inline void get_real_coords_from_grid_coords(const int* gc, double* rc) const
		{
			rc[0]=(static_cast<double>(gc[0])*step_)-init_shift_[0];
			rc[1]=(static_cast<double>(gc[1])*step_)-init_shift_[1];
			rc[2]=(static_cast<double>(gc[2])*step_)-init_shift_[2];
		}

		inline double calc_real_distance_between_grid_coords_and_real_ball(const int* gc, const int ball_num) const
		{
			const double* p=balls_[ball_num].xyz;
			const double r=balls_[ball_num].r;
			double rc[3]={0.0 ,0.0, 0.0};
			get_real_coords_from_grid_coords(gc, rc);
			return (std::sqrt((rc[0]-p[0])*(rc[0]-p[0])+(rc[1]-p[1])*(rc[1]-p[1])+(rc[2]-p[2])*(rc[2]-p[2]))-r);
		}

		inline double calc_real_squared_distance_between_grid_coords_and_grid_coords(const int* gc_a, const int* gc_b) const
		{
			double rc_a[3]={0.0 ,0.0, 0.0};
			double rc_b[3]={0.0 ,0.0, 0.0};
			get_real_coords_from_grid_coords(gc_a, rc_a);
			get_real_coords_from_grid_coords(gc_b, rc_b);
			return ((rc_a[0]-rc_b[0])*(rc_a[0]-rc_b[0])+(rc_a[1]-rc_b[1])*(rc_a[1]-rc_b[1])+(rc_a[2]-rc_b[2])*(rc_a[2]-rc_b[2]));
		}

		inline void calc_line_strip_middle_point(const double* p1, const double* p2, double* pm) const
		{
			pm[0]=(p1[0]+p2[0])/2.0;
			pm[1]=(p1[1]+p2[1])/2.0;
			pm[2]=(p1[2]+p2[2])/2.0;
		}

		inline void calc_triangle_middle_point(const double* p1, const double* p2, const double* p3, double* pm) const
		{
			pm[0]=(p1[0]+p2[0]+p3[0])/3.0;
			pm[1]=(p1[1]+p2[1]+p3[1])/3.0;
			pm[2]=(p1[2]+p2[2]+p3[2])/3.0;
		}

		inline bool check_grid_coords_in_range(const int* gc) const
		{
			return (gc[0]>=0 && gc[0]<dimensions_[0] && gc[1]>=0 && gc[1]<dimensions_[1] && gc[2]>=0 && gc[2]<dimensions_[2]);
		}

		double probe_;
		double step_;
		int number_of_cell_shifts_to_use_;
		int cell_shifts_[26][3];
		double init_shift_[3];
		int dimensions_[3];
		std::vector<Ball> balls_;
		std::vector<Voxel> grid_vector_;
	};

	struct SurfaceConstruction
	{
		struct Parameters
		{
			bool solvent_excluded;
			double probe;
			double grid_step;
			int grid_max_allowed_voxels_count;
			bool grid_reduced_step_directions;
			int smoothing_iterations;
			double smoothing_self_proportion;
			bool correct_burried_vertices;
			int split_mode;

			Parameters() :
				solvent_excluded(true),
				probe(1.4),
				grid_step(0.4),
				grid_max_allowed_voxels_count(20*1000*1000),
				grid_reduced_step_directions(true),
				smoothing_iterations(1),
				smoothing_self_proportion(0.0),
				correct_burried_vertices(true),
				split_mode(1)
			{
			}
		};

		static void construct_surface(const Parameters& parameters, const std::vector<Ball>& balls, Surface& surface)
		{
			Grid grid;
			grid.init(parameters.probe, parameters.grid_step, parameters.grid_max_allowed_voxels_count, parameters.grid_reduced_step_directions, balls);
			grid.construct_surface(parameters.solvent_excluded, parameters.smoothing_iterations, parameters.smoothing_self_proportion, parameters.correct_burried_vertices, parameters.split_mode, surface);
		}

		template<typename T>
		static Surface construct_surface(const Parameters& parameters, const T& balls)
		{
			std::vector<Ball> plain_balls;
			plain_balls.reserve(balls.size());
			for(std::size_t i=0;i<balls.size();i++)
			{
				Ball b;
				b.xyz[0]=balls[i].x;
				b.xyz[1]=balls[i].y;
				b.xyz[2]=balls[i].z;
				b.r=balls[i].r;
				plain_balls.push_back(b);
			}
			Surface surface;
			construct_surface(parameters, plain_balls, surface);
			return surface;
		}
	};

private:
	struct VertexConnectedMarchingCubesMapping
	{
		static std::vector< std::pair< std::vector<int>, std::vector<int> > > generate_basic_patterns()
		{
			typedef std::pair< std::vector<int>, std::vector<int> > BasicPattern;
			std::vector<BasicPattern> basic_patterns;
			basic_patterns.reserve(23);

			{
				//a
				BasicPattern p;
				basic_patterns.push_back(p);
			}
			{
				//b
				BasicPattern p;
				p.first.push_back(0);
				basic_patterns.push_back(p);
			}
			{
				//c
				BasicPattern p;
				p.first.push_back(0); p.first.push_back(6);
				basic_patterns.push_back(p);
			}
			{
				//d
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(3);
				basic_patterns.push_back(p);
			}
			{
				//e
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(2);
				basic_patterns.push_back(p);
			}
			{
				//f
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(2);p.first.push_back(3);
				p.second.push_back(0);p.second.push_back(3);p.second.push_back(2);
				p.second.push_back(7);
				basic_patterns.push_back(p);
			}
			{
				//g
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(3);p.first.push_back(6);
				basic_patterns.push_back(p);
			}
			{
				//h
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(2);p.first.push_back(7);
				basic_patterns.push_back(p);
			}
			{
				//i
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(1);p.first.push_back(2);p.first.push_back(3);
				p.second.push_back(0);p.second.push_back(2);p.second.push_back(1);
				p.second.push_back(5);
				p.second.push_back(0);p.second.push_back(3);p.second.push_back(2);
				p.second.push_back(7);
				basic_patterns.push_back(p);
			}
			{
				//j
				BasicPattern p;
				p.first.push_back(1);p.first.push_back(2);p.first.push_back(3);p.first.push_back(6);
				p.second.push_back(1);p.second.push_back(3);p.second.push_back(6);
				p.second.push_back(4);
				basic_patterns.push_back(p);
			}
			{
				//k
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(1);p.first.push_back(3);p.first.push_back(5);
				p.second.push_back(0);p.second.push_back(3);p.second.push_back(1);
				p.second.push_back(7);
				p.second.push_back(0);p.second.push_back(1);p.second.push_back(5);
				p.second.push_back(7);
				basic_patterns.push_back(p);
			}
			{
				//l
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(2);p.first.push_back(3);p.first.push_back(5);
				p.second.push_back(0);p.second.push_back(3);p.second.push_back(2);
				p.second.push_back(7);
				basic_patterns.push_back(p);
			}
			{
				//m
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(1);p.first.push_back(2);p.first.push_back(4);
				p.second.push_back(0);p.second.push_back(2);p.second.push_back(1);
				p.second.push_back(7);
				p.second.push_back(0);p.second.push_back(1);p.second.push_back(4);
				p.second.push_back(7);
				basic_patterns.push_back(p);
			}
			{
				//n
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(2);p.first.push_back(4);p.first.push_back(6);
				basic_patterns.push_back(p);
			}
			{
				//o
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(2);p.first.push_back(5);p.first.push_back(7);
				basic_patterns.push_back(p);
			}
			{
				//p
				BasicPattern p;
				p.first.push_back(1);p.first.push_back(2);p.first.push_back(3);p.first.push_back(4);p.first.push_back(6);
				p.second.push_back(1);p.second.push_back(3);p.second.push_back(6);
				p.second.push_back(4);
				basic_patterns.push_back(p);
			}
			{
				//q
				BasicPattern p;
				p.first.push_back(1);p.first.push_back(2);p.first.push_back(3);p.first.push_back(4);p.first.push_back(5);
				p.second.push_back(1);p.second.push_back(3);p.second.push_back(4);
				p.second.push_back(0);
				p.second.push_back(2);p.second.push_back(4);p.second.push_back(3);
				p.second.push_back(7);
				p.second.push_back(2);p.second.push_back(5);p.second.push_back(4);
				p.second.push_back(6);
				basic_patterns.push_back(p);
			}
			{
				//r
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(1);p.first.push_back(2);p.first.push_back(3);p.first.push_back(5);
				p.second.push_back(0);p.second.push_back(3);p.second.push_back(5);
				p.second.push_back(4);
				p.second.push_back(2);p.second.push_back(5);p.second.push_back(3);
				p.second.push_back(6);
				basic_patterns.push_back(p);
			}
			{
				//s
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(1);p.first.push_back(2);p.first.push_back(3);p.first.push_back(5);p.first.push_back(7);
				p.second.push_back(0);p.second.push_back(7);p.second.push_back(5);
				p.second.push_back(4);
				p.second.push_back(2);p.second.push_back(5);p.second.push_back(7);
				p.second.push_back(6);
				basic_patterns.push_back(p);
			}
			{
				//t
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(1);p.first.push_back(2);p.first.push_back(3);p.first.push_back(4);p.first.push_back(5);
				p.second.push_back(2);p.second.push_back(4);p.second.push_back(3);
				p.second.push_back(7);
				p.second.push_back(2);p.second.push_back(5);p.second.push_back(4);
				p.second.push_back(6);
				basic_patterns.push_back(p);
			}
			{
				//u
				BasicPattern p;
				p.first.push_back(1);p.first.push_back(2);p.first.push_back(3);p.first.push_back(4);p.first.push_back(5);p.first.push_back(7);
				p.second.push_back(1);p.second.push_back(3);p.second.push_back(4);
				p.second.push_back(0);
				p.second.push_back(2);p.second.push_back(5);p.second.push_back(7);
				p.second.push_back(6);
				basic_patterns.push_back(p);
			}
			{
				//v
				BasicPattern p;
				p.first.push_back(1);p.first.push_back(2);p.first.push_back(3);p.first.push_back(4);p.first.push_back(5);p.first.push_back(6);p.first.push_back(7);
				p.second.push_back(1);p.second.push_back(3);p.second.push_back(4);
				p.second.push_back(0);
				basic_patterns.push_back(p);
			}
			{
				//w
				BasicPattern p;
				p.first.push_back(0);p.first.push_back(1);p.first.push_back(2);p.first.push_back(3);p.first.push_back(4);p.first.push_back(5);p.first.push_back(6);p.first.push_back(7);
				basic_patterns.push_back(p);
			}

			return basic_patterns;
		}

		static std::vector< std::vector<int> > generate_cube_vertex_ids_permutations(std::ostream* printing_output)
		{
			int quadruples[6][4]={{0,1,2,3},{4,5,6,7},{0,4,5,1},{3,7,6,2},{0,4,7,3},{1,5,6,2}};
			std::set< std::vector<int> > set_of_basic_permutations;
			{
				for(int j=0;j<3;j++)
				{
					std::vector<int> order(8, 0);
					for(int e=0;e<4;e++)
					{
						order[e]=quadruples[j*2][e];
						order[e+4]=quadruples[j*2+1][e];
					}
					{
						{
							std::vector<int> transformed_order(8, 0);
							for(int e=0;e<4;e++)
							{
								transformed_order[e]=order[e+4];
								transformed_order[e+4]=order[e];
							}
							std::vector<int> permutation(8, 0);
							for(int e=0;e<8;e++)
							{
								permutation[order[e]]=transformed_order[e];
							}
							set_of_basic_permutations.insert(permutation);
						}
						{
							std::vector<int> transformed_order=order;
							for(int k=0;k<4;k++)
							{
								if(k>0)
								{
									std::swap(transformed_order[2],transformed_order[3]);
									std::swap(transformed_order[1],transformed_order[2]);
									std::swap(transformed_order[0],transformed_order[1]);
									std::swap(transformed_order[6],transformed_order[7]);
									std::swap(transformed_order[5],transformed_order[6]);
									std::swap(transformed_order[4],transformed_order[5]);
								}
								std::vector<int> permutation(8, 0);
								for(int e=0;e<8;e++)
								{
									permutation[order[e]]=transformed_order[e];
								}
								set_of_basic_permutations.insert(permutation);
							}
						}
					}
				}
			}
			std::vector< std::vector<int> > list_of_basic_permutations;
			for(std::set< std::vector<int> >::const_iterator it=set_of_basic_permutations.begin();it!=set_of_basic_permutations.end();++it)
			{
				list_of_basic_permutations.push_back(*it);
			}
			std::set< std::vector<int> > set_of_all_permutations;
			for(std::size_t i=0;i<list_of_basic_permutations.size();i++)
			{
				for(std::size_t j=0;j<list_of_basic_permutations.size();j++)
				{
					for(std::size_t k=0;k<list_of_basic_permutations.size();k++)
					{
						std::vector<int> permutation(8, 0);
						for(int e=0;e<8;e++)
						{
							permutation[e]=list_of_basic_permutations[k][list_of_basic_permutations[j][list_of_basic_permutations[i][e]]];
						}
						set_of_all_permutations.insert(permutation);
					}
				}
			}
			std::vector< std::vector<int> > list_of_all_permutations;
			for(std::set< std::vector<int> >::const_iterator it=set_of_all_permutations.begin();it!=set_of_all_permutations.end();++it)
			{
				list_of_all_permutations.push_back(*it);
			}
			if(printing_output!=0)
			{
				std::ostream& output=(*printing_output);
				output << "int cube_vertex_ids_permutations[" << list_of_all_permutations.size() << "][8]={\n";
				for(std::size_t i=0;i<list_of_all_permutations.size();i++)
				{
					for(std::size_t j=0;j<list_of_all_permutations[i].size();j++)
					{
						output << (j==0 ? "{" : ",") << list_of_all_permutations[i][j];
					}
					output << ((i+1)<list_of_all_permutations.size() ? "},\n" : "}};\n");
				}
			}
			return list_of_all_permutations;
		}

		int basic_cube_cordinates[8][3];
		int map_of_triangles[256][13];
		double map_of_normals[256][9];

		VertexConnectedMarchingCubesMapping()
		{
			const int permutations[48][8]={
											{0,1,2,3,4,5,6,7},
											{0,1,5,4,3,2,6,7},
											{0,3,2,1,4,7,6,5},
											{0,3,7,4,1,2,6,5},
											{0,4,5,1,3,7,6,2},
											{0,4,7,3,1,5,6,2},
											{1,0,3,2,5,4,7,6},
											{1,0,4,5,2,3,7,6},
											{1,2,3,0,5,6,7,4},
											{1,2,6,5,0,3,7,4},
											{1,5,4,0,2,6,7,3},
											{1,5,6,2,0,4,7,3},
											{2,1,0,3,6,5,4,7},
											{2,1,5,6,3,0,4,7},
											{2,3,0,1,6,7,4,5},
											{2,3,7,6,1,0,4,5},
											{2,6,5,1,3,7,4,0},
											{2,6,7,3,1,5,4,0},
											{3,0,1,2,7,4,5,6},
											{3,0,4,7,2,1,5,6},
											{3,2,1,0,7,6,5,4},
											{3,2,6,7,0,1,5,4},
											{3,7,4,0,2,6,5,1},
											{3,7,6,2,0,4,5,1},
											{4,0,1,5,7,3,2,6},
											{4,0,3,7,5,1,2,6},
											{4,5,1,0,7,6,2,3},
											{4,5,6,7,0,1,2,3},
											{4,7,3,0,5,6,2,1},
											{4,7,6,5,0,3,2,1},
											{5,1,0,4,6,2,3,7},
											{5,1,2,6,4,0,3,7},
											{5,4,0,1,6,7,3,2},
											{5,4,7,6,1,0,3,2},
											{5,6,2,1,4,7,3,0},
											{5,6,7,4,1,2,3,0},
											{6,2,1,5,7,3,0,4},
											{6,2,3,7,5,1,0,4},
											{6,5,1,2,7,4,0,3},
											{6,5,4,7,2,1,0,3},
											{6,7,3,2,5,4,0,1},
											{6,7,4,5,2,3,0,1},
											{7,3,0,4,6,2,1,5},
											{7,3,2,6,4,0,1,5},
											{7,4,0,3,6,5,1,2},
											{7,4,5,6,3,0,1,2},
											{7,6,2,3,4,5,1,0},
											{7,6,5,4,3,2,1,0}};

			{
				const int s[8][3]={{0,0,0},{0,1,0},{1,1,0},{1,0,0},{0,0,1},{0,1,1},{1,1,1},{1,0,1}};
				for(int i=0;i<8;i++)
				{
					for(int j=0;j<3;j++)
					{
						basic_cube_cordinates[i][j]=s[i][j];
					}
				}
			}

			for(int i=0;i<256;i++)
			{
				map_of_triangles[i][0]=-1;
				for(int j=1;j<13;j++)
				{
					map_of_triangles[i][j]=0;
				}
				for(int j=0;j<9;j++)
				{
					map_of_normals[i][j]=0.0;
				}
			}

			const std::vector< std::pair< std::vector<int>, std::vector<int> > > basic_patterns=generate_basic_patterns();

			for(std::size_t i=0;i<basic_patterns.size();i++)
			{
				for(int j=0;j<48;j++)
				{
					int code[8]={0,0,0,0,0,0,0,0};
					for(std::size_t k=0;k<basic_patterns[i].first.size();k++)
					{
						code[permutations[j][basic_patterns[i].first[k]]]=1;
					}
					const int code_num=(code[0]+code[1]*2+code[2]*4+code[3]*8+code[4]*16+code[5]*32+code[6]*64+code[7]*128);
					if(map_of_triangles[code_num][0]<0)
					{
						map_of_triangles[code_num][0]=static_cast<int>(basic_patterns[i].second.size())/4;
						for(std::size_t k=0;k<basic_patterns[i].second.size();k++)
						{
							map_of_triangles[code_num][k+1]=permutations[j][basic_patterns[i].second[k]];
						}
						for(int k=0;k<map_of_triangles[code_num][0];k++)
						{
							const int* v0=&(basic_cube_cordinates[map_of_triangles[code_num][1+k*4+0]][0]);
							const int* v1=&(basic_cube_cordinates[map_of_triangles[code_num][1+k*4+1]][0]);
							const int* v2=&(basic_cube_cordinates[map_of_triangles[code_num][1+k*4+2]][0]);
							const int* v3=&(basic_cube_cordinates[map_of_triangles[code_num][1+k*4+3]][0]);
							const double a[3]={static_cast<double>(v1[0]-v0[0]), static_cast<double>(v1[1]-v0[1]), static_cast<double>(v1[2]-v0[2])};
							const double b[3]={static_cast<double>(v2[0]-v0[0]), static_cast<double>(v2[1]-v0[1]), static_cast<double>(v2[2]-v0[2])};
							const double c[3]={static_cast<double>(v3[0]-v0[0]), static_cast<double>(v3[1]-v0[1]), static_cast<double>(v3[2]-v0[2])};
							double n[3]={a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]};
							double nlength=sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
							if((n[0]*c[0]+n[1]*c[1]+n[2]*c[2])<0)
							{
								nlength=0.0-nlength;
							}
							map_of_normals[code_num][k*3+0]=n[0]/nlength;
							map_of_normals[code_num][k*3+1]=n[1]/nlength;
							map_of_normals[code_num][k*3+2]=n[2]/nlength;
						}
					}
				}
			}
		}

		void print_map_of_triangles(std::ostream& output) const
		{
			for(int i=0;i<256;i++)
			{
				output << i << "    " << map_of_triangles[i][0] << "   ";
				for(int j=0;j<(map_of_triangles[i][0]*4);j++)
				{
					output << " " << map_of_triangles[i][j+1];
				}
				output << "   ";
				for(int j=0;j<(map_of_triangles[i][0]*3);j++)
				{
					output << " " << map_of_normals[i][j];
				}
				output << "\n";
			}
		}
	};
};

}

}

#endif /* AUXILIARIES_GRID_EDT_TOOLS_H_ */

