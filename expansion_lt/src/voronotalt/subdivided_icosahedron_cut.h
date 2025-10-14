#ifndef VORONOTALT_SUBDIVIDED_ICOSAHEDRON_CUT_H_
#define VORONOTALT_SUBDIVIDED_ICOSAHEDRON_CUT_H_

#include <algorithm>

#include "subdivided_icosahedron.h"

namespace voronotalt
{

class SubdividedIcosahedronCut
{
public:
	struct CuttingPlane
	{
		SimplePoint point;
		SimplePoint normal;
	};

	struct GraphicsBundle
	{
		std::vector<SimplePoint> vertices;
		std::vector<SubdividedIcosahedron::Triple> triples;

		bool empty() const noexcept
		{
			return triples.empty();
		}

		bool collect_pairs(std::vector<SubdividedIcosahedron::Pair>& pairs) const noexcept
		{
			pairs.clear();
			for(std::size_t i=0;i<triples.size();i++)
			{
				const SubdividedIcosahedron::Triple& t=triples[i];
				pairs.push_back(SubdividedIcosahedron::Pair(t.ids[0], t.ids[1]));
				pairs.push_back(SubdividedIcosahedron::Pair(t.ids[0], t.ids[2]));
				pairs.push_back(SubdividedIcosahedron::Pair(t.ids[1], t.ids[2]));
			}
			std::sort(pairs.begin(), pairs.end());
			pairs.erase(std::unique(pairs.begin(), pairs.end()), pairs.end());
			return (!pairs.empty());
		}
	};

	GraphicsBundle graphics_bundle;

	SubdividedIcosahedronCut() noexcept
	{
	}

	void init(const SubdividedIcosahedron& sih, const SimpleSphere& sphere, const std::vector<CuttingPlane>& cutting_planes) noexcept
	{
		graphics_bundle.vertices.clear();
		graphics_bundle.triples.clear();
		vertices_mask_.clear();
		vertices_renumbering_.clear();
		filtered_vertices_.clear();

		graphics_bundle.vertices.resize(sih.vertices.size());
		vertices_mask_.resize(sih.vertices.size(), 1);

		const Float max_edge_length=sih.max_edge_length*sphere.r*FLOATCONST(1.01);

		for(std::size_t i=0;i<graphics_bundle.vertices.size();i++)
		{
			graphics_bundle.vertices[i]=sih.get_point_on_sphere(i, sphere);
			for(std::size_t j=0;j<cutting_planes.size() && vertices_mask_[i]>0;j++)
			{
				const Float sd=signed_distance_from_point_to_plane(cutting_planes[j].point, cutting_planes[j].normal, graphics_bundle.vertices[i]);
				if(sd<=FLOATCONST(0.0))
				{
					vertices_mask_[i]=((FLOATCONST(0.0)-sd)<max_edge_length) ? 2 : 0;
				}
			}
		}

		for(std::size_t i=0;i<sih.triples.size();i++)
		{
			const SubdividedIcosahedron::Triple& ot=sih.triples[i];
			if(vertices_mask_[ot.ids[0]]==1 && vertices_mask_[ot.ids[1]]==1 && vertices_mask_[ot.ids[2]]==1)
			{
				graphics_bundle.triples.push_back(ot);
			}
			else if(vertices_mask_[ot.ids[0]]>0 && vertices_mask_[ot.ids[1]]>0 && vertices_mask_[ot.ids[2]]>0)
			{
				std::vector<SubdividedIcosahedron::Triple> ts1(1, ot);
				std::vector<SubdividedIcosahedron::Triple> ts2;
				for(std::size_t k=0;k<cutting_planes.size() && !ts1.empty();k++)
				{
					for(std::size_t j=0;j<ts1.size();j++)
					{
						const SubdividedIcosahedron::Triple& t=ts1[j];
						int hs[3]={halfspace_of_point(cutting_planes[k].point, cutting_planes[k].normal, graphics_bundle.vertices[t.ids[0]]),
								halfspace_of_point(cutting_planes[k].point, cutting_planes[k].normal, graphics_bundle.vertices[t.ids[1]]),
								halfspace_of_point(cutting_planes[k].point, cutting_planes[k].normal, graphics_bundle.vertices[t.ids[2]])};
						if(hs[0]==1 || hs[1]==1 || hs[2]==1)
						{
							bool left1right2=false;
							bool left2right1=false;
							UnsignedInt tids[3]={t.ids[0], t.ids[1], t.ids[2]};
							if(hs[0]==-1 && hs[1]==1 && hs[2]==1)
							{
								left1right2=true;
								tids[0]=t.ids[0];
								tids[1]=t.ids[1];
								tids[2]=t.ids[2];
							}
							else if(hs[0]==-1 && hs[1]==-1 && hs[2]==1)
							{
								left2right1=true;
								tids[0]=t.ids[0];
								tids[1]=t.ids[1];
								tids[2]=t.ids[2];
							}
							else if(hs[0]==-1 && hs[1]==1 && hs[2]==-1)
							{
								left2right1=true;
								tids[0]=t.ids[0];
								tids[1]=t.ids[2];
								tids[2]=t.ids[1];
							}
							else if(hs[0]==1 && hs[1]==-1 && hs[2]==1)
							{
								left1right2=true;
								tids[0]=t.ids[1];
								tids[1]=t.ids[0];
								tids[2]=t.ids[2];
							}
							else if(hs[0]==1 && hs[1]==-1 && hs[2]==-1)
							{
								left2right1=true;
								tids[0]=t.ids[1];
								tids[1]=t.ids[2];
								tids[2]=t.ids[0];
							}
							else if(hs[0]==1 && hs[1]==1 && hs[2]==-1)
							{
								left1right2=true;
								tids[0]=t.ids[2];
								tids[1]=t.ids[0];
								tids[2]=t.ids[1];
							}
							if(left1right2)
							{
								SimplePoint i01=intersection_of_plane_and_segment(cutting_planes[k].point, cutting_planes[k].normal, graphics_bundle.vertices[tids[0]], graphics_bundle.vertices[tids[1]]);
								project_point_on_sphere_and_cutting_plane(sphere, cutting_planes[k], i01);
								SimplePoint i02=intersection_of_plane_and_segment(cutting_planes[k].point, cutting_planes[k].normal, graphics_bundle.vertices[tids[0]], graphics_bundle.vertices[tids[2]]);
								project_point_on_sphere_and_cutting_plane(sphere, cutting_planes[k], i02);
								UnsignedInt nid01=graphics_bundle.vertices.size();
								graphics_bundle.vertices.push_back(i01);
								UnsignedInt nid02=graphics_bundle.vertices.size();
								graphics_bundle.vertices.push_back(i02);
								ts2.push_back(SubdividedIcosahedron::Triple(nid01, nid02, tids[1]));
								ts2.push_back(SubdividedIcosahedron::Triple(nid02, tids[1], tids[2]));
							}
							else if(left2right1)
							{
								SimplePoint i02=intersection_of_plane_and_segment(cutting_planes[k].point, cutting_planes[k].normal, graphics_bundle.vertices[tids[0]], graphics_bundle.vertices[tids[2]]);
								project_point_on_sphere_and_cutting_plane(sphere, cutting_planes[k], i02);
								SimplePoint i12=intersection_of_plane_and_segment(cutting_planes[k].point, cutting_planes[k].normal, graphics_bundle.vertices[tids[1]], graphics_bundle.vertices[tids[2]]);
								project_point_on_sphere_and_cutting_plane(sphere, cutting_planes[k], i12);
								UnsignedInt nid02=graphics_bundle.vertices.size();
								graphics_bundle.vertices.push_back(i02);
								UnsignedInt nid12=graphics_bundle.vertices.size();
								graphics_bundle.vertices.push_back(i12);
								ts2.push_back(SubdividedIcosahedron::Triple(nid02, nid12, tids[2]));
							}
							else
							{
								ts2.push_back(t);
							}
						}
					}
					ts1.swap(ts2);
					ts2.clear();
				}
				graphics_bundle.triples.insert(graphics_bundle.triples.end(), ts1.begin(), ts1.end());
			}
		}

		vertices_renumbering_.resize(graphics_bundle.vertices.size(), 0);
		for(std::size_t i=0;i<graphics_bundle.triples.size();i++)
		{
			const SubdividedIcosahedron::Triple& t=graphics_bundle.triples[i];
			vertices_renumbering_[t.ids[0]]=1;
			vertices_renumbering_[t.ids[1]]=1;
			vertices_renumbering_[t.ids[2]]=1;
		}
		filtered_vertices_.reserve(graphics_bundle.triples.size()*2+40);
		for(std::size_t i=0;i<graphics_bundle.vertices.size();i++)
		{
			if(vertices_renumbering_[i]>0)
			{
				vertices_renumbering_[i]=filtered_vertices_.size();
				filtered_vertices_.push_back(graphics_bundle.vertices[i]);
			}
		}
		graphics_bundle.vertices.swap(filtered_vertices_);
		for(std::size_t i=0;i<graphics_bundle.triples.size();i++)
		{
			const SubdividedIcosahedron::Triple& t=graphics_bundle.triples[i];
			graphics_bundle.triples[i]=SubdividedIcosahedron::Triple(vertices_renumbering_[t.ids[0]], vertices_renumbering_[t.ids[1]], vertices_renumbering_[t.ids[2]]);
		}

		vertices_mask_.clear();
		vertices_renumbering_.clear();
		filtered_vertices_.clear();
	}

	void init(const SubdividedIcosahedron& sih, const SimpleSphere& sphere, const std::vector<SimpleSphere>& cutting_spheres) noexcept
	{
		std::vector<CuttingPlane> cutting_planes;
		cutting_planes.reserve(cutting_spheres.size());
		for(std::size_t i=0;i<cutting_spheres.size();i++)
		{
			if(sphere_intersects_sphere(sphere, cutting_spheres[i]))
			{
				CuttingPlane cutting_plane;
				cutting_plane.point=center_of_intersection_circle_of_two_spheres(sphere, cutting_spheres[i]);
				cutting_plane.normal=unit_point(sub_of_points(sphere.p, cutting_spheres[i].p));
				cutting_planes.push_back(cutting_plane);
			}
		}
		init(sih, sphere, cutting_planes);
	}

private:
	static inline void project_point_on_sphere_and_cutting_plane(const SimpleSphere& sphere, const CuttingPlane& cutting_plane, SimplePoint& p) noexcept
	{
		p=sum_of_points(sphere.p, point_and_number_product(unit_point(sub_of_points(p, sphere.p)), sphere.r));
		Float d=signed_distance_from_point_to_plane(cutting_plane.point, cutting_plane.normal, p);
		for(int i=0;i<10 && d>FLOATEPSILON;i++)
		{
			p=sub_of_points(p, point_and_number_product(cutting_plane.normal, d));
			p=sum_of_points(sphere.p, point_and_number_product(unit_point(sub_of_points(p, sphere.p)), sphere.r));
			d=signed_distance_from_point_to_plane(cutting_plane.point, cutting_plane.normal, p);
		}
	}

	std::vector<int> vertices_mask_;
	std::vector<UnsignedInt> vertices_renumbering_;
	std::vector<SimplePoint> filtered_vertices_;
};

}

#endif /* VORONOTALT_SUBDIVIDED_ICOSAHEDRON_CUT_H_ */
