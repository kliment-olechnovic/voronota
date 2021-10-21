#ifndef APOLLOTA_CONSTRAINED_CONTACTS_INTERFACE_MESH_H_
#define APOLLOTA_CONSTRAINED_CONTACTS_INTERFACE_MESH_H_

#include "constrained_contacts_construction.h"

namespace voronota
{

namespace apollota
{

class ConstrainedContactsInterfaceMesh
{
public:
	struct MeshVertex
	{
		enum Origin
		{
			VoronoiVertex,
			VoronoiEdgeCutBySolvent,
			VoronoiFaceCutBySolvent,
			VoronoiFaceInside
		};

		Origin origin;
		Pair pair_of_generator_ids;
		Quadruple quadruple_of_generator_ids;
		SimplePoint point;

		MeshVertex(const Origin origin, const Pair& pair_of_generator_ids, const Quadruple& quadruple_of_generator_ids, const SimplePoint& point) :
			origin(origin),
			pair_of_generator_ids(pair_of_generator_ids),
			quadruple_of_generator_ids(quadruple_of_generator_ids),
			point(point)
		{
		}
	};

	struct MeshFace
	{
		Pair pair_of_generator_ids;
		std::size_t triple_of_mesh_vertex_ids[3];

		MeshFace(const Pair& pair_of_generator_ids, const std::size_t a, const std::size_t b, const std::size_t c) :
			pair_of_generator_ids(pair_of_generator_ids)
		{
			triple_of_mesh_vertex_ids[0]=a;
			triple_of_mesh_vertex_ids[1]=b;
			triple_of_mesh_vertex_ids[2]=c;
		}
	};

	static std::size_t null_id()
	{
		return std::numeric_limits<std::size_t>::max();
	}

	ConstrainedContactsInterfaceMesh(
			const std::vector<SimpleSphere>& spheres,
			const Triangulation::VerticesVector& vertices_vector,
			const TriangulationQueries::PairsMap& pairs_vertices,
			const std::set<Pair>& ab_ids,
			const double probe,
			const double step,
			const int projections)
	{
		typedef std::pair< Pair, std::vector<std::size_t> > MeshVertexIDs;
		typedef std::map< Quadruple, MeshVertexIDs > MapOfMeshVertexIDs;
		MapOfMeshVertexIDs map_of_mesh_vertex_ids;
		std::set<Pair> set_of_mesh_links;
		for(std::set<Pair>::const_iterator ab_ids_it=ab_ids.begin();ab_ids_it!=ab_ids.end();++ab_ids_it)
		{
			const Pair& pair_id=(*ab_ids_it);
			const std::size_t a_id=pair_id.get(0);
			const std::size_t b_id=pair_id.get(1);
			if(a_id<spheres.size() && b_id<spheres.size())
			{
				TriangulationQueries::PairsMap::const_iterator pairs_vertices_it=pairs_vertices.find(pair_id);
				if(pairs_vertices_it!=pairs_vertices.end())
				{
					const std::list<ConstrainedContactContour::Contour> contours=ConstrainedContactContour::construct_contact_contours(
							spheres, vertices_vector, pairs_vertices_it->second, a_id, b_id, probe, step, projections, true);
					if(!contours.empty())
					{
						for(std::list<ConstrainedContactContour::Contour>::const_iterator contours_it=contours.begin();contours_it!=contours.end();++contours_it)
						{
							const ConstrainedContactContour::Contour& contour=(*contours_it);
							std::vector<std::size_t> contour_mesh_vertex_ids;
							contour_mesh_vertex_ids.reserve(contour.size());
							for(ConstrainedContactContour::Contour::const_iterator contour_it=contour.begin();contour_it!=contour.end();++contour_it)
							{
								const ConstrainedContactContour::PointRecord& pr=(*contour_it);
								std::size_t contour_mesh_vertex_id=null_id();
								Quadruple quadruple_id(a_id, b_id, (pr.left_id==a_id ? null_id() : pr.left_id), (pr.right_id==a_id ? null_id() : pr.right_id));
								if(quadruple_id.count(null_id())<2)
								{
									const MeshVertex::Origin origin=(quadruple_id.count(null_id())==0 ? MeshVertex::VoronoiVertex : MeshVertex::VoronoiEdgeCutBySolvent);
									MapOfMeshVertexIDs::iterator map_of_mesh_vertex_ids_it=map_of_mesh_vertex_ids.find(quadruple_id);
									if(map_of_mesh_vertex_ids_it==map_of_mesh_vertex_ids.end())
									{
										contour_mesh_vertex_id=mesh_vertices_.size();
										mesh_vertices_.push_back(MeshVertex(origin, pair_id, quadruple_id, pr.p));
										MeshVertexIDs& mesh_vertex_ids=map_of_mesh_vertex_ids[quadruple_id];
										mesh_vertex_ids.first=pair_id;
										mesh_vertex_ids.second.resize(1, contour_mesh_vertex_id);
									}
									else
									{
										MeshVertexIDs& mesh_vertex_ids=map_of_mesh_vertex_ids_it->second;
										if(mesh_vertex_ids.first==pair_id || mesh_vertex_ids.second.empty())
										{
											contour_mesh_vertex_id=mesh_vertices_.size();
											mesh_vertices_.push_back(MeshVertex(origin, pair_id, quadruple_id, pr.p));
											mesh_vertex_ids.first=pair_id;
											mesh_vertex_ids.second.push_back(contour_mesh_vertex_id);
										}
										else
										{
											contour_mesh_vertex_id=mesh_vertex_ids.second[0];
											double min_dist=distance_from_point_to_point(pr.p, mesh_vertices_[contour_mesh_vertex_id].point);
											for(std::size_t i=1;i<mesh_vertex_ids.second.size();i++)
											{
												const std::size_t candidate_id=mesh_vertex_ids.second[i];
												const double candidate_dist=distance_from_point_to_point(pr.p, mesh_vertices_[candidate_id].point);
												if(candidate_dist<min_dist)
												{
													contour_mesh_vertex_id=candidate_id;
													min_dist=candidate_dist;
												}
											}
										}
									}
								}
								else if(quadruple_id.count(null_id())==2)
								{
									contour_mesh_vertex_id=mesh_vertices_.size();
									mesh_vertices_.push_back(MeshVertex(MeshVertex::VoronoiFaceCutBySolvent, pair_id, quadruple_id, pr.p));
								}
								if(contour_mesh_vertex_id!=null_id())
								{
									contour_mesh_vertex_ids.push_back(contour_mesh_vertex_id);
								}
							}
							if(!contour_mesh_vertex_ids.empty())
							{
								SimplePoint central_point;
								for(std::size_t i=0;i<contour_mesh_vertex_ids.size();i++)
								{
									central_point=central_point+mesh_vertices_[contour_mesh_vertex_ids[i]].point;
								}
								central_point=central_point*(1.0/static_cast<double>(contour_mesh_vertex_ids.size()));
								const std::size_t central_point_mesh_vertex_id=mesh_vertices_.size();
								mesh_vertices_.push_back(MeshVertex(MeshVertex::VoronoiFaceInside, pair_id, Quadruple(a_id, b_id, null_id(), null_id()), central_point));
								for(std::size_t i=0;i<contour_mesh_vertex_ids.size();i++)
								{
									if(i+1<contour_mesh_vertex_ids.size())
									{
										mesh_faces_.push_back(MeshFace(pair_id, contour_mesh_vertex_ids[i], contour_mesh_vertex_ids[i+1], central_point_mesh_vertex_id));
										set_of_mesh_links.insert(Pair(contour_mesh_vertex_ids[i], contour_mesh_vertex_ids[(i+1)]));
									}
									else
									{
										mesh_faces_.push_back(MeshFace(pair_id, contour_mesh_vertex_ids[i], contour_mesh_vertex_ids[0], central_point_mesh_vertex_id));
										set_of_mesh_links.insert(Pair(contour_mesh_vertex_ids[i], contour_mesh_vertex_ids[0]));
									}
								}
							}
						}
					}
				}
			}
		}
		mesh_links_.insert(mesh_links_.begin(), set_of_mesh_links.begin(), set_of_mesh_links.end());
	}

	const std::vector<MeshVertex>& mesh_vertices() const
	{
		return mesh_vertices_;
	}


	const std::vector<MeshFace>& mesh_faces() const
	{
		return mesh_faces_;
	}

	const std::vector<Pair>& mesh_links() const
	{
		return mesh_links_;
	}
private:
	std::vector<MeshVertex> mesh_vertices_;
	std::vector<MeshFace> mesh_faces_;
	std::vector<Pair> mesh_links_;
};

}

}

#endif /* APOLLOTA_CONSTRAINED_CONTACTS_INTERFACE_MESH_H_ */
