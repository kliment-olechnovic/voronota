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
			const Triangulation::QuadruplesMap& quadruples_map,
			const std::set<Pair>& set_of_ab_ids,
			const double probe,
			const double step,
			const int projections,
			const bool only_largest_component)
	{
		const Triangulation::VerticesVector vertices_vector=Triangulation::collect_vertices_vector_from_quadruples_map(quadruples_map);
		const TriangulationQueries::PairsMap pairs_vertices=TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector);

		std::vector<Pair> ordered_ab_ids;
		if(!set_of_ab_ids.empty())
		{
			ordered_ab_ids.reserve(set_of_ab_ids.size());
			std::map<Pair, std::size_t> visited_pairs;
			std::size_t current_component_id=0;
			while(visited_pairs.size()<set_of_ab_ids.size())
			{
				std::vector<Pair> stack;
				for(std::set<Pair>::const_iterator set_of_ab_ids_it=set_of_ab_ids.begin();set_of_ab_ids_it!=set_of_ab_ids.end() && stack.empty();++set_of_ab_ids_it)
				{
					if(visited_pairs.count(*set_of_ab_ids_it)==0)
					{
						current_component_id++;
						stack.push_back(*set_of_ab_ids_it);
					}
				}
				while(!stack.empty())
				{
					const Pair current_pair=stack.back();
					stack.pop_back();
					if(visited_pairs.count(current_pair)==0)
					{
						ordered_ab_ids.push_back(current_pair);
						visited_pairs[current_pair]=current_component_id;
						TriangulationQueries::PairsMap::const_iterator pairs_vertices_it=pairs_vertices.find(current_pair);
						if(pairs_vertices_it!=pairs_vertices.end())
						{
							std::vector< std::pair<double, std::size_t> > weighted_pair_vertices;
							weighted_pair_vertices.reserve(pairs_vertices_it->second.size());
							for(std::set<std::size_t>::const_iterator pair_vertices_it=pairs_vertices_it->second.begin();pair_vertices_it!=pairs_vertices_it->second.end();++pair_vertices_it)
							{
								const double r=vertices_vector[*pair_vertices_it].second.r;
								weighted_pair_vertices.push_back(std::make_pair(0.0-r, (*pair_vertices_it)));
							}
							std::sort(weighted_pair_vertices.begin(), weighted_pair_vertices.end());
							for(std::vector< std::pair<double, std::size_t> >::const_iterator weighted_pair_vertices_it=weighted_pair_vertices.begin();weighted_pair_vertices_it!=weighted_pair_vertices.end();++weighted_pair_vertices_it)
							{
								const Quadruple q=vertices_vector[weighted_pair_vertices_it->second].first;
								for(int i=0;i<3;i++)
								{
									for(int j=(i+1);j<4;j++)
									{
										const Pair neighbor_pair(q.get(i), q.get(j));
										if(!(neighbor_pair==current_pair) && (neighbor_pair.contains(current_pair.get(0)) || neighbor_pair.contains(current_pair.get(1))))
										{
											double max_dist=minimal_distance_from_sphere_to_sphere(spheres[current_pair.get(0)], spheres[neighbor_pair.get(0)]);
											max_dist=std::max(max_dist, minimal_distance_from_sphere_to_sphere(spheres[current_pair.get(0)], spheres[neighbor_pair.get(1)]));
											max_dist=std::max(max_dist, minimal_distance_from_sphere_to_sphere(spheres[current_pair.get(1)], spheres[neighbor_pair.get(0)]));
											max_dist=std::max(max_dist, minimal_distance_from_sphere_to_sphere(spheres[current_pair.get(1)], spheres[neighbor_pair.get(1)]));
											if(max_dist<(probe*2.0))
											{
												if(set_of_ab_ids.count(neighbor_pair)>0 && visited_pairs.count(neighbor_pair)==0)
												{
													stack.push_back(neighbor_pair);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}

			if(only_largest_component)
			{
				std::map<std::size_t, unsigned int> map_of_component_to_count;
				for(std::map<Pair, std::size_t>::const_iterator visited_pairs_it=visited_pairs.begin();visited_pairs_it!=visited_pairs.end();++visited_pairs_it)
				{
					map_of_component_to_count[visited_pairs_it->second]++;
				}
				std::map<unsigned int, std::size_t> map_of_count_to_component;
				for(std::map<std::size_t, unsigned int>::const_iterator map_of_component_to_count_it=map_of_component_to_count.begin();map_of_component_to_count_it!=map_of_component_to_count.end();++map_of_component_to_count_it)
				{
					map_of_count_to_component[map_of_component_to_count_it->second]=map_of_component_to_count_it->first;
				}
				const std::size_t largest_component_id=map_of_count_to_component.rbegin()->second;
				std::vector<Pair> selected_ordered_ab_ids;
				selected_ordered_ab_ids.reserve(map_of_count_to_component.rbegin()->first);
				for(std::size_t i=0;i<ordered_ab_ids.size();i++)
				{
					if(visited_pairs[ordered_ab_ids[i]]==largest_component_id)
					{
						selected_ordered_ab_ids.push_back(ordered_ab_ids[i]);
					}
				}
				ordered_ab_ids.swap(selected_ordered_ab_ids);
			}
		}

		typedef std::pair< Pair, std::vector<std::size_t> > MeshVertexIDs;
		typedef std::map< Quadruple, MeshVertexIDs > MapOfMeshVertexIDs;
		MapOfMeshVertexIDs map_of_mesh_vertex_ids;
		std::map< Pair, std::pair<std::size_t, std::size_t> > map_of_mesh_links;
		for(std::vector<Pair>::const_iterator ab_ids_it=ordered_ab_ids.begin();ab_ids_it!=ordered_ab_ids.end();++ab_ids_it)
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
									if(contour_mesh_vertex_ids.empty() || contour_mesh_vertex_ids.back()!=contour_mesh_vertex_id)
									{
										contour_mesh_vertex_ids.push_back(contour_mesh_vertex_id);
									}
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
								{
									bool need_reverse=false;
									for(std::size_t i=0;i<contour_mesh_vertex_ids.size() && !need_reverse;i++)
									{
										const std::pair<std::size_t, std::size_t> ordered_pair(contour_mesh_vertex_ids[i], contour_mesh_vertex_ids[(i+1)<contour_mesh_vertex_ids.size() ? (i+1) : 0]);
										std::map< Pair, std::pair<std::size_t, std::size_t> >::const_iterator map_of_mesh_links_it=map_of_mesh_links.find(Pair(ordered_pair.first, ordered_pair.second));
										if(map_of_mesh_links_it!=map_of_mesh_links.end())
										{
											if(map_of_mesh_links_it->second==ordered_pair)
											{
												need_reverse=true;
											}
										}
									}
									if(need_reverse)
									{
										std::reverse(contour_mesh_vertex_ids.begin(), contour_mesh_vertex_ids.end());
									}
								}
								for(std::size_t i=0;i<contour_mesh_vertex_ids.size();i++)
								{
									if(i+1<contour_mesh_vertex_ids.size())
									{
										mesh_faces_.push_back(MeshFace(pair_id, contour_mesh_vertex_ids[i], contour_mesh_vertex_ids[i+1], central_point_mesh_vertex_id));
										map_of_mesh_links[Pair(contour_mesh_vertex_ids[i], contour_mesh_vertex_ids[(i+1)])]=std::make_pair(contour_mesh_vertex_ids[i], contour_mesh_vertex_ids[(i+1)]);
									}
									else
									{
										mesh_faces_.push_back(MeshFace(pair_id, contour_mesh_vertex_ids[i], contour_mesh_vertex_ids[0], central_point_mesh_vertex_id));
										map_of_mesh_links[Pair(contour_mesh_vertex_ids[i], contour_mesh_vertex_ids[0])]=std::make_pair(contour_mesh_vertex_ids[i], contour_mesh_vertex_ids[0]);
									}
								}
							}
						}
					}
				}
			}
		}

		collect_mesh_links();

		if(only_largest_component)
		{
			restrict_to_largest_connected_component();
		}
	}

	void restrict_to_largest_connected_component()
	{
		if(mesh_links_.empty())
		{
			return;
		}

		std::vector< std::set<std::size_t> > graph(mesh_vertices_.size());
		for(std::size_t i=0;i<mesh_links_.size();i++)
		{
			graph[mesh_links_[i].get(0)].insert(mesh_links_[i].get(1));
			graph[mesh_links_[i].get(1)].insert(mesh_links_[i].get(0));
		}

		std::map<std::size_t, int> visited_ids;
		{
			std::size_t current_component_id=0;
			while(visited_ids.size()<mesh_vertices_.size())
			{
				std::vector<std::size_t> stack;
				for(std::size_t i=0;i<mesh_vertices_.size() && stack.empty();i++)
				{
					if(visited_ids.count(i)==0)
					{
						current_component_id++;
						stack.push_back(i);
					}
				}
				while(!stack.empty())
				{
					const std::size_t current_id=stack.back();
					stack.pop_back();
					if(visited_ids.count(current_id)==0)
					{
						visited_ids[current_id]=current_component_id;
						const std::set<std::size_t>& neighbor_ids=graph[current_id];
						for(std::set<std::size_t>::const_iterator neighbor_ids_it=neighbor_ids.begin();neighbor_ids_it!=neighbor_ids.end();++neighbor_ids_it)
						{
							const std::size_t neighbor_id=(*neighbor_ids_it);
							if(visited_ids.count(neighbor_id)==0)
							{
								stack.push_back(neighbor_id);
							}
						}
					}
				}
			}
		}

		{
			std::map<int, unsigned int> map_of_component_to_count;
			for(std::map<std::size_t, int>::const_iterator visited_ids_it=visited_ids.begin();visited_ids_it!=visited_ids.end();++visited_ids_it)
			{
				map_of_component_to_count[visited_ids_it->second]++;
			}
			if(map_of_component_to_count.size()>1)
			{
				std::map<unsigned int, int> map_of_count_to_component;
				for(std::map<int, unsigned int>::const_iterator map_of_component_to_count_it=map_of_component_to_count.begin();map_of_component_to_count_it!=map_of_component_to_count.end();++map_of_component_to_count_it)
				{
					map_of_count_to_component[map_of_component_to_count_it->second]=map_of_component_to_count_it->first;
				}
				const int largest_component_id=map_of_count_to_component.rbegin()->second;

				std::vector<std::size_t> remapping_of_vertices(mesh_vertices_.size(), null_id());
				std::vector<MeshVertex> selected_mesh_vertices;
				selected_mesh_vertices.reserve(mesh_vertices_.size());
				for(std::map<std::size_t, int>::const_iterator visited_ids_it=visited_ids.begin();visited_ids_it!=visited_ids.end();++visited_ids_it)
				{
					const std::size_t current_vertex_id=visited_ids_it->first;
					const int current_component_id=visited_ids_it->second;
					if(current_component_id==largest_component_id)
					{
						remapping_of_vertices[current_vertex_id]=selected_mesh_vertices.size();
						selected_mesh_vertices.push_back(mesh_vertices_[current_vertex_id]);
					}
				}

				for(std::size_t i=0;i<mesh_faces_.size();i++)
				{
					const MeshFace& mf=mesh_faces_[i];
					for(int a=0;a<3;a++)
					{
						const int b=(a==0 ? 1 : 0);
						const int c=(a==0 ? 2 : (a==1 ? 2 : 1));
						if(remapping_of_vertices[mf.triple_of_mesh_vertex_ids[a]]==null_id() &&
								remapping_of_vertices[mf.triple_of_mesh_vertex_ids[b]]!=null_id() &&
								remapping_of_vertices[mf.triple_of_mesh_vertex_ids[c]]!=null_id())
						{
							remapping_of_vertices[mf.triple_of_mesh_vertex_ids[a]]=selected_mesh_vertices.size();
							selected_mesh_vertices.push_back(mesh_vertices_[mf.triple_of_mesh_vertex_ids[a]]);
						}
					}
				}

				std::vector<MeshFace> selected_mesh_faces;
				selected_mesh_faces.reserve(mesh_faces_.size());
				for(std::size_t i=0;i<mesh_faces_.size();i++)
				{
					const MeshFace& mf=mesh_faces_[i];
					std::size_t remapped_ids[3]={remapping_of_vertices[mf.triple_of_mesh_vertex_ids[0]],
							remapping_of_vertices[mf.triple_of_mesh_vertex_ids[1]], remapping_of_vertices[mf.triple_of_mesh_vertex_ids[2]]};
					if(remapped_ids[0]!=null_id() && remapped_ids[1]!=null_id() && remapped_ids[2]!=null_id())
					{
						MeshFace nmf=mf;
						nmf.triple_of_mesh_vertex_ids[0]=remapped_ids[0];
						nmf.triple_of_mesh_vertex_ids[1]=remapped_ids[1];
						nmf.triple_of_mesh_vertex_ids[2]=remapped_ids[2];
						selected_mesh_faces.push_back(nmf);
					}
				}

				mesh_vertices_.swap(selected_mesh_vertices);
				mesh_faces_.swap(selected_mesh_faces);
				collect_mesh_links();
			}
		}
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

	bool check_manifold() const
	{
		std::map< Pair, std::set<Triple> > map_of_edges_to_faces;
		for(std::size_t i=0;i<mesh_faces_.size();i++)
		{
			const MeshFace& mf=mesh_faces_[i];
			const Triple t(mf.triple_of_mesh_vertex_ids[0], mf.triple_of_mesh_vertex_ids[1], mf.triple_of_mesh_vertex_ids[2]);
			for(int a=0;(a+1)<3;a++)
			{
				for(int b=(a+1);b<3;b++)
				{
					std::set<Triple>& set_ab=map_of_edges_to_faces[Pair(mf.triple_of_mesh_vertex_ids[a], mf.triple_of_mesh_vertex_ids[b])];
					set_ab.insert(t);
					if(set_ab.size()>2)
					{
						return false;
					}
				}
			}
		}

		std::map< std::size_t, std::set<Triple> > map_of_vertices_to_faces;
		for(std::size_t i=0;i<mesh_faces_.size();i++)
		{
			const MeshFace& mf=mesh_faces_[i];
			for(int a=0;a<3;a++)
			{
				map_of_vertices_to_faces[mf.triple_of_mesh_vertex_ids[a]].insert(Triple(mf.triple_of_mesh_vertex_ids[0], mf.triple_of_mesh_vertex_ids[1], mf.triple_of_mesh_vertex_ids[2]));
			}
		}

		for(std::map< std::size_t, std::set<Triple> >::const_iterator mvf_it=map_of_vertices_to_faces.begin();mvf_it!=map_of_vertices_to_faces.end();++mvf_it)
		{
			const std::size_t vid=mvf_it->first;
			std::set<Triple> visited;
			std::vector<Triple> stack(1, *(mvf_it->second.begin()));
			while(!stack.empty())
			{
				const Triple t=stack.back();
				stack.pop_back();
				if(visited.count(t)==0)
				{
					visited.insert(t);
					for(unsigned int i=0;i<3;i++)
					{
						const std::size_t nvid=t.get(i);
						if(nvid!=vid)
						{
							std::map< Pair, std::set<Triple> >::const_iterator mef_it=map_of_edges_to_faces.find(Pair(vid, nvid));
							if(mef_it!=map_of_edges_to_faces.end())
							{
								const std::set<Triple>& faces=mef_it->second;
								for(std::set<Triple>::const_iterator faces_it=faces.begin();faces_it!=faces.end();++faces_it)
								{
									const Triple& face=(*faces_it);
									if(visited.count(face)==0)
									{
										stack.push_back(face);
									}
								}
							}
						}
					}
				}
			}
			if(visited.size()<mvf_it->second.size())
			{
				return false;
			}
		}

		return true;
	}
private:
	std::vector<MeshVertex> mesh_vertices_;
	std::vector<MeshFace> mesh_faces_;
	std::vector<Pair> mesh_links_;

	void collect_mesh_links()
	{
		std::set<Pair> set_of_mesh_links;
		for(std::size_t i=0;i<mesh_faces_.size();i++)
		{
			const MeshFace& mf=mesh_faces_[i];
			for(int a=0;(a+1)<3;a++)
			{
				if(mesh_vertices_[mf.triple_of_mesh_vertex_ids[a]].origin!=MeshVertex::VoronoiFaceInside)
				{
					for(int b=(a+1);b<3;b++)
					{
						if(mesh_vertices_[mf.triple_of_mesh_vertex_ids[b]].origin!=MeshVertex::VoronoiFaceInside)
						{
							set_of_mesh_links.insert(Pair(mf.triple_of_mesh_vertex_ids[a], mf.triple_of_mesh_vertex_ids[b]));
						}
					}
				}
			}
		}
		mesh_links_.clear();
		mesh_links_.reserve(set_of_mesh_links.size());
		mesh_links_.insert(mesh_links_.end(), set_of_mesh_links.begin(), set_of_mesh_links.end());
	}
};

}

}

#endif /* APOLLOTA_CONSTRAINED_CONTACTS_INTERFACE_MESH_H_ */
