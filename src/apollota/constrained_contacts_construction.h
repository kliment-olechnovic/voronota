#ifndef APOLLOTA_CONSTRAINED_CONTACTS_CONSTRUCTION_H_
#define APOLLOTA_CONSTRAINED_CONTACTS_CONSTRUCTION_H_

#include <deque>

#include "../compatability/tr1_usage.h"

#include "constrained_contact_contour.h"
#include "constrained_contact_remainder.h"
#include "triangulation_queries.h"

namespace voronota
{

namespace apollota
{

class ConstrainedContactsConstruction
{
public:
	static std::map<Pair, double> construct_contacts(
			const std::vector<SimpleSphere>& spheres,
			const Triangulation::VerticesVector& vertices_vector,
			const double probe,
			const double step,
			const int projections,
			const std::set<std::size_t>& mock_solvent_ids,
			const std::vector<int>& lookup_groups,
			std::pair< bool, std::map<std::size_t, double> >& volumes_bundle,
			std::pair< bool, std::map<Pair, double> >& bounding_arcs_bundle,
			std::pair< bool, std::map<Triple, double> >& edge_strips_bundle)
	{
		std::map<Pair, double> result;

		const TriangulationQueries::PairsMap pairs_vertices=TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector);
		for(TriangulationQueries::PairsMap::const_iterator pairs_vertices_it=pairs_vertices.begin();pairs_vertices_it!=pairs_vertices.end();++pairs_vertices_it)
		{
			const std::size_t a=pairs_vertices_it->first.get(0);
			const std::size_t b=pairs_vertices_it->first.get(1);
			const bool a_is_not_mock_solvent=(mock_solvent_ids.count(a)==0);
			const bool b_is_not_mock_solvent=(mock_solvent_ids.count(b)==0);
			if((a_is_not_mock_solvent || b_is_not_mock_solvent) && (a>=lookup_groups.size() || b>=lookup_groups.size() || lookup_groups[a]!=lookup_groups[b]) && minimal_distance_from_sphere_to_sphere(spheres[a], spheres[b])<(probe*2))
			{
				double sum=0.0;
				double bounding_arcs_sum=0.0;
				const std::list<ConstrainedContactContour::Contour> contours=ConstrainedContactContour::construct_contact_contours(spheres, vertices_vector, pairs_vertices_it->second, a, b, probe, step, projections, false);
				for(std::list<ConstrainedContactContour::Contour>::const_iterator contours_it=contours.begin();contours_it!=contours.end();++contours_it)
				{
					const ConstrainedContactContour::Contour& contour=(*contours_it);
					if(!contour.empty())
					{
						const ConstrainedContactContour::ContourAreaDescriptor d=ConstrainedContactContour::construct_contour_area_descriptor(contour, spheres[a], spheres[b], false);
						if(d.star_domain || d.simple_polygon_triangulation.empty())
						{
							for(std::size_t i=0;i<d.outline.size();i++)
							{
								const std::size_t second_index=((i+1<d.outline.size()) ? (i+1) : 0);
								sum+=triangle_area(d.center, d.outline[i], d.outline[second_index]);
								if(volumes_bundle.first)
								{
									if(a_is_not_mock_solvent)
									{
										volumes_bundle.second[a]+=fabs(signed_volume_of_tetrahedron(spheres[a], d.center, d.outline[i], d.outline[second_index]));
									}
									if(b_is_not_mock_solvent)
									{
										volumes_bundle.second[b]+=fabs(signed_volume_of_tetrahedron(spheres[b], d.center, d.outline[i], d.outline[second_index]));
									}
								}
							}
						}
						else
						{
							for(std::size_t i=0;i<d.simple_polygon_triangulation.size();i++)
							{
								const Triple& t=d.simple_polygon_triangulation[i];
								sum+=triangle_area(d.outline[t.get(0)], d.outline[t.get(1)], d.outline[t.get(2)]);
								if(volumes_bundle.first)
								{
									if(a_is_not_mock_solvent)
									{
										volumes_bundle.second[a]+=fabs(signed_volume_of_tetrahedron(spheres[a], d.outline[t.get(0)], d.outline[t.get(1)], d.outline[t.get(2)]));
									}
									if(b_is_not_mock_solvent)
									{
										volumes_bundle.second[b]+=fabs(signed_volume_of_tetrahedron(spheres[b], d.outline[t.get(0)], d.outline[t.get(1)], d.outline[t.get(2)]));
									}
								}
							}
						}
						if(bounding_arcs_bundle.first)
						{
							for(ConstrainedContactContour::Contour::const_iterator point_record_it=contour.begin();point_record_it!=contour.end();++point_record_it)
							{
								ConstrainedContactContour::Contour::const_iterator next_point_record_it=point_record_it;
								++next_point_record_it;
								if(next_point_record_it==contour.end())
								{
									next_point_record_it=contour.begin();
								}
								if(point_record_it->right_id==a && next_point_record_it->left_id==a)
								{
									bounding_arcs_sum+=distance_from_point_to_point(point_record_it->p, next_point_record_it->p);
								}
							}
						}
						if(edge_strips_bundle.first)
						{
							std::set<Triple> encountered_triples;
							for(ConstrainedContactContour::Contour::const_iterator point_record_it=contour.begin();point_record_it!=contour.end();++point_record_it)
							{
								ConstrainedContactContour::Contour::const_iterator next_point_record_it=point_record_it;
								++next_point_record_it;
								if(next_point_record_it==contour.end())
								{
									next_point_record_it=contour.begin();
								}
								if(!(point_record_it->right_id==a && next_point_record_it->left_id==a))
								{
									encountered_triples.insert(Triple(a, b, (point_record_it->right_id!=a ? point_record_it->right_id : next_point_record_it->left_id)));
								}
							}
							std::set<Triple> new_encountered_triples;
							for(std::set<Triple>::const_iterator encountered_triples_it=encountered_triples.begin();encountered_triples_it!=encountered_triples.end();++encountered_triples_it)
							{
								const Triple& t=(*encountered_triples_it);
								if(edge_strips_bundle.second.count(t)==0)
								{
									new_encountered_triples.insert(t);
								}
							}
							std::map<Triple, double>::iterator edge_strips_map_it=edge_strips_bundle.second.end();
							for(ConstrainedContactContour::Contour::const_iterator point_record_it=contour.begin();point_record_it!=contour.end();++point_record_it)
							{
								ConstrainedContactContour::Contour::const_iterator next_point_record_it=point_record_it;
								++next_point_record_it;
								if(next_point_record_it==contour.end())
								{
									next_point_record_it=contour.begin();
								}
								if(!(point_record_it->right_id==a && next_point_record_it->left_id==a))
								{
									Triple t(a, b, (point_record_it->right_id!=a ? point_record_it->right_id : next_point_record_it->left_id));
									if(new_encountered_triples.count(t)>0)
									{
										if(edge_strips_map_it==edge_strips_bundle.second.end() || !(edge_strips_map_it->first==t))
										{
											edge_strips_map_it=edge_strips_bundle.second.insert(std::make_pair(t, 0.0)).first;
										}
										edge_strips_map_it->second+=distance_from_point_to_point(point_record_it->p, next_point_record_it->p);
									}
								}
							}
						}
					}
				}
				if(sum>0.0)
				{
					result[Pair(a,b)]=sum;
				}
				if(bounding_arcs_bundle.first && bounding_arcs_sum>0.0)
				{
					bounding_arcs_bundle.second[Pair(a,b)]=bounding_arcs_sum;
				}
			}
		}

		return result;
	}

	static std::map<std::size_t, double> construct_contact_remainders(
			const std::vector<SimpleSphere>& spheres,
			const Triangulation::VerticesVector& vertices_vector,
			const double probe,
			const int sih_depth,
			const std::vector<int>& sas_mask,
			std::pair< bool, std::map<std::size_t, double> >& volumes_bundle)
	{
		std::map<std::size_t, double> result;

		SubdividedIcosahedron sih(sih_depth);
		const TriangulationQueries::IDsMap ids_vertices=TriangulationQueries::collect_vertices_map_from_vertices_vector(vertices_vector);
		for(TriangulationQueries::IDsMap::const_iterator ids_vertices_it=ids_vertices.begin();ids_vertices_it!=ids_vertices.end();++ids_vertices_it)
		{
			const std::size_t a=ids_vertices_it->first;
			if(sas_mask.empty() || (a<sas_mask.size() && sas_mask[a]>0))
			{
				const ConstrainedContactRemainder::Remainder remainder=ConstrainedContactRemainder::construct_contact_remainder(spheres, vertices_vector, ids_vertices_it->second, a, probe, sih);
				if(!remainder.empty())
				{
					const SimpleSphere surface_sphere(spheres[a], spheres[a].r+probe);
					double sum=0.0;
					for(ConstrainedContactRemainder::Remainder::const_iterator remainder_it=remainder.begin();remainder_it!=remainder.end();++remainder_it)
					{
						sum+=spherical_triangle_area(surface_sphere, remainder_it->p[0], remainder_it->p[1], remainder_it->p[2]);
					}
					if(sum>0.0)
					{
						result[a]=sum;
						if(volumes_bundle.first)
						{
							volumes_bundle.second[a]+=((sum*surface_sphere.r)/3.0);
						}
					}
				}
			}
		}

		return result;
	}

	template<typename ContactRemainderDescriptor>
	static std::map< int, std::map<std::size_t, ContactRemainderDescriptor> > construct_groups_of_contact_remainders(
			const std::vector<SimpleSphere>& spheres,
			const Triangulation::VerticesVector& vertices_vector,
			const double probe,
			const double step,
			const int projections,
			const std::size_t sih_depth,
			const bool exclude_external_remainders)
	{
		std::map< int, std::map<std::size_t, ContactRemainderDescriptor> > result;

		std::vector< std::pair<Pair, ConstrainedContactContour::Contour> > surface_contours_vector;
		std::vector<int> surface_contours_vector_marks;
		construct_surface_contours(spheres, vertices_vector, probe, step, projections, surface_contours_vector, surface_contours_vector_marks);

		const std::set<int> external_groups_set=
				select_external_contours_groups(spheres, mark_spheres_by_connected_components(spheres, vertices_vector, probe), surface_contours_vector, surface_contours_vector_marks);

		std::vector< std::map<int, std::list<std::size_t> > > spheres_exposures(spheres.size());
		for(std::size_t i=0;i<surface_contours_vector.size();i++)
		{
			const std::size_t a=surface_contours_vector[i].first.get(0);
			const std::size_t b=surface_contours_vector[i].first.get(1);
			int group_id=surface_contours_vector_marks[i];
			if(external_groups_set.count(group_id))
			{
				group_id=0-group_id;
			}
			if(group_id>0 || !exclude_external_remainders)
			{
				spheres_exposures[a][group_id].push_back(i);
				spheres_exposures[b][group_id].push_back(i);
			}
		}

		const SubdividedIcosahedron sih(sih_depth);
		const TriangulationQueries::IDsMap ids_vertices=TriangulationQueries::collect_vertices_map_from_vertices_vector(vertices_vector);

		for(std::size_t sphere_id=0;sphere_id<spheres_exposures.size();sphere_id++)
		{
			if(!spheres_exposures[sphere_id].empty())
			{
				TriangulationQueries::IDsMap::const_iterator ids_vertices_it=ids_vertices.find(sphere_id);
				if(ids_vertices_it!=ids_vertices.end())
				{
					const ConstrainedContactRemainder::Remainder full_remainder=ConstrainedContactRemainder::construct_contact_remainder(spheres, vertices_vector, ids_vertices_it->second, sphere_id, probe, sih);
					if(!full_remainder.empty())
					{
						const std::map<int, std::list<std::size_t> >& sphere_exposure=spheres_exposures[sphere_id];
						if(sphere_exposure.size()==1)
						{
							result[sphere_exposure.begin()->first][sphere_id].feed(full_remainder);
						}
						else
						{
							std::map<int, ConstrainedContactRemainder::Remainder> split_remainders;
							for(ConstrainedContactRemainder::Remainder::const_iterator full_remainder_it=full_remainder.begin();full_remainder_it!=full_remainder.end();++full_remainder_it)
							{
								const SimplePoint p=(full_remainder_it->p[0]+full_remainder_it->p[1]+full_remainder_it->p[2])*(1.0/3.0);
								std::pair<double, int> minimal_distance_to_group(std::numeric_limits<double>::max(), 0);
								for(std::map<int, std::list<std::size_t> >::const_iterator sphere_exposure_it=sphere_exposure.begin();sphere_exposure_it!=sphere_exposure.end();++sphere_exposure_it)
								{
									const int group_id=sphere_exposure_it->first;
									const std::list<std::size_t>& contours_ids=sphere_exposure_it->second;
									for(std::list<std::size_t>::const_iterator contours_ids_it=contours_ids.begin();contours_ids_it!=contours_ids.end();++contours_ids_it)
									{
										const ConstrainedContactContour::Contour& contour=surface_contours_vector[*contours_ids_it].second;
										for(ConstrainedContactContour::Contour::const_iterator contour_it=contour.begin();contour_it!=contour.end();++contour_it)
										{
											minimal_distance_to_group=std::min(minimal_distance_to_group, std::make_pair(distance_from_point_to_point(p, contour_it->p), group_id));
										}
									}
								}
								split_remainders[minimal_distance_to_group.second].push_back(*full_remainder_it);
							}
							for(std::map<int, ConstrainedContactRemainder::Remainder>::const_iterator split_remainders_it=split_remainders.begin();split_remainders_it!=split_remainders.end();++split_remainders_it)
							{
								if(split_remainders_it->first!=0)
								{
									result[split_remainders_it->first][sphere_id].feed(split_remainders_it->second);
								}
							}
						}
					}
				}
			}
		}

		return result;
	}

	static std::vector<int> mark_spheres_by_connected_components(
			const std::vector<SimpleSphere>& spheres,
			const Triangulation::VerticesVector& vertices_vector,
			const double probe)
	{
		const std::vector< std::vector<std::size_t> > graph=
				TriangulationQueries::collect_ids_graph_from_ids_map(
						TriangulationQueries::collect_neighbors_map_from_quadruples_map(vertices_vector), spheres.size());

		std::vector<int> marks(spheres.size(), 0);

		int groups_count=0;
		for(std::size_t i=0;i<graph.size();i++)
		{
			if(marks[i]==0)
			{
				groups_count++;
				std::deque<std::size_t> stack;
				marks[i]=groups_count;
				stack.push_back(i);
				while(!stack.empty())
				{
					const std::size_t a=stack.back();
					stack.pop_back();
					for(std::size_t e=0;e<graph[a].size();e++)
					{
						const std::size_t b=graph[a][e];
						if(marks[b]==0 && minimal_distance_from_sphere_to_sphere(spheres[a], spheres[b])<(2*probe))
						{
							marks[b]=groups_count;
							stack.push_back(b);
						}
					}
				}
			}
		}

		return marks;
	}

	static int construct_surface_contours(
			const std::vector<SimpleSphere>& spheres,
			const Triangulation::VerticesVector& vertices_vector,
			const double probe,
			const double step,
			const int projections,
			std::vector< std::pair<Pair, ConstrainedContactContour::Contour> >& surface_contours_vector,
			std::vector<int>& marks)
	{
#if USE_TR1 > 0
typedef std::tr1::unordered_map< Pair, std::list<std::size_t>, Pair::HashFunctor > PairsIDsMap;
#else
typedef std::unordered_map< Pair, std::list<std::size_t>, Pair::HashFunctor > PairsIDsMap;
#endif

		const TriangulationQueries::PairsMap pairs_vertices=TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector);
		PairsIDsMap pairs_surface_contours_map;
		surface_contours_vector.clear();
		for(TriangulationQueries::PairsMap::const_iterator it=pairs_vertices.begin();it!=pairs_vertices.end();++it)
		{
			const std::size_t a=it->first.get(0);
			const std::size_t b=it->first.get(1);
			if(minimal_distance_from_sphere_to_sphere(spheres[a], spheres[b])<(probe*2))
			{
				const std::set<std::size_t>& pair_vertices_list=it->second;
				bool has_surface=false;
				for(std::set<std::size_t>::const_iterator list_it=pair_vertices_list.begin();list_it!=pair_vertices_list.end() && !has_surface;++list_it)
				{
					has_surface=vertices_vector[*list_it].second.r>=probe;
				}
				if(has_surface)
				{
					const std::list<ConstrainedContactContour::Contour> contours=ConstrainedContactContour::construct_contact_contours(spheres, vertices_vector, pair_vertices_list, a, b, probe, step, projections, false);
					for(std::list<ConstrainedContactContour::Contour>::const_iterator contours_it=contours.begin();contours_it!=contours.end();++contours_it)
					{
						const ConstrainedContactContour::Contour& contour=(*contours_it);
						const std::list<ConstrainedContactContour::Contour> subcontours=ConstrainedContactContour::collect_subcontours_from_contour(contour);
						for(std::list<ConstrainedContactContour::Contour>::const_iterator subcontours_it=subcontours.begin();subcontours_it!=subcontours.end();++subcontours_it)
						{
							const ConstrainedContactContour::Contour& subcontour=(*subcontours_it);
							if(!subcontour.empty() && subcontour.front().right_id==a && subcontour.front().left_id!=a)
							{
								pairs_surface_contours_map[it->first].push_back(surface_contours_vector.size());
								surface_contours_vector.push_back(std::make_pair(it->first, subcontour));
							}
						}
					}
				}
			}
		}
		marks.clear();
		marks.resize(surface_contours_vector.size(), 0);
		int groups_count=0;
		for(std::size_t i=0;i<marks.size();i++)
		{
			if(marks[i]==0)
			{
				groups_count++;
				std::deque<std::size_t> stack;
				marks[i]=groups_count;
				stack.push_back(i);
				while(!stack.empty())
				{
					const std::size_t j=stack.back();
					stack.pop_back();
					const std::size_t a=surface_contours_vector[j].first.get(0);
					const std::size_t b=surface_contours_vector[j].first.get(1);
					const ConstrainedContactContour::Contour& contour=surface_contours_vector[j].second;
					const std::size_t c[2]={contour.front().left_id, contour.back().right_id};
					const Pair related_pairs[4]={Pair(a, c[0]), Pair(a, c[1]), Pair(b, c[0]), Pair(b, c[1])};
					for(int e=0;e<4;e++)
					{
						const Pair& related_pair=related_pairs[e];
						PairsIDsMap::const_iterator it=pairs_surface_contours_map.find(related_pair);
						if(it!=pairs_surface_contours_map.end())
						{
							const std::list<std::size_t>& related_contours_ids=it->second;
							if(!related_contours_ids.empty())
							{
								std::pair<double, std::size_t> real_related_contour(std::numeric_limits<double>::max(), j);
								for(std::list<std::size_t>::const_iterator jt=related_contours_ids.begin();jt!=related_contours_ids.end();++jt)
								{
									const std::size_t related_contour_id=(*jt);
									const ConstrainedContactContour::Contour& related_contour=surface_contours_vector[related_contour_id].second;
									double shortest_dist=distance_from_point_to_point(contour.front().p, related_contour.front().p);
									shortest_dist=std::min(shortest_dist, distance_from_point_to_point(contour.front().p, related_contour.back().p));
									shortest_dist=std::min(shortest_dist, distance_from_point_to_point(contour.back().p, related_contour.front().p));
									shortest_dist=std::min(shortest_dist, distance_from_point_to_point(contour.back().p, related_contour.back().p));
									real_related_contour=std::min(real_related_contour, std::make_pair(shortest_dist, related_contour_id));
								}
								if(marks[real_related_contour.second]==0)
								{
									marks[real_related_contour.second]=groups_count;
									stack.push_back(real_related_contour.second);
								}
							}
						}
					}
				}
			}
		}
		return groups_count;
	}

private:
	static std::set<int> select_external_contours_groups(
			const std::vector<SimpleSphere>& spheres,
			const std::vector<int>& spheres_marks,
			const std::vector< std::pair<Pair, ConstrainedContactContour::Contour> >& surface_contours_vector,
			const std::vector<int>& surface_contours_vector_marks)
	{
		std::set<int> result;
		if(!spheres.empty() && spheres.size()==spheres_marks.size() && surface_contours_vector.size()==surface_contours_vector_marks.size())
		{
			SimplePoint mc;
			for(std::size_t i=0;i<spheres.size();i++)
			{
				mc=mc+SimplePoint(spheres[i]);
			}
			mc=mc*(1.0/static_cast<double>(spheres.size()));

			std::map< int, std::pair<double, int> > max_map;
			for(std::size_t i=0;i<surface_contours_vector.size();i++)
			{
				const int sphere_mark=spheres_marks[surface_contours_vector[i].first.get(0)];
				std::map< int, std::pair<double, int> >::iterator val_it=max_map.find(sphere_mark);
				if(val_it==max_map.end())
				{
					val_it=max_map.insert(std::make_pair(sphere_mark, std::make_pair(0.0, 0))).first;
				}
				const ConstrainedContactContour::Contour& contour=surface_contours_vector[i].second;
				const int group_id=surface_contours_vector_marks[i];
				for(ConstrainedContactContour::Contour::const_iterator it=contour.begin();it!=contour.end();++it)
				{
					const double dist=distance_from_point_to_point(it->p, mc);
					if(dist>(val_it->second.first))
					{
						val_it->second.first=dist;
						val_it->second.second=group_id;
					}
				}
			}

			for(std::map< int, std::pair<double, int> >::const_iterator it=max_map.begin();it!=max_map.end();++it)
			{
				result.insert(it->second.second);
			}
		}
		return result;
	}
};

}

}

#endif /* APOLLOTA_CONSTRAINED_CONTACTS_CONSTRUCTION_H_ */
