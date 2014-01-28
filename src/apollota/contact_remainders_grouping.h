#ifndef APOLLOTA_CONTACT_REMAINDERS_GROUPING_H_
#define APOLLOTA_CONTACT_REMAINDERS_GROUPING_H_

#include "contact_contour.h"
#include "contact_remainder.h"

namespace apollota
{

class ContactRemaindersGrouping
{
public:
	typedef std::map< int, std::map<std::size_t, ContactRemainder::Remainder> > ContactRemaindersGroupsMap;

	static ContactRemaindersGroupsMap construct_grouped_remainders(
			const std::vector<SimpleSphere>& spheres,
			const Triangulation::VerticesVector& vertices_vector,
			const double probe,
			const double step,
			const int projections,
			const std::size_t sih_depth)
	{
		ContactRemaindersGrouping::SurfaceContoursVector surface_contours_vector;
		std::vector<int> marks;
		ContactRemaindersGrouping::construct_surface_contours(spheres, vertices_vector, probe, step, projections, surface_contours_vector, marks);

		std::vector< std::map<int, std::list<std::size_t> > > spheres_exposures(spheres.size());
		for(std::size_t i=0;i<surface_contours_vector.size();i++)
		{
			const std::size_t a=surface_contours_vector[i].first.get(0);
			const std::size_t b=surface_contours_vector[i].first.get(1);
			const int group_id=marks[i];
			spheres_exposures[a][group_id].push_back(i);
			spheres_exposures[b][group_id].push_back(i);
		}

		const SubdividedIcosahedron sih(sih_depth);
		const TriangulationQueries::IDsMap ids_vertices=TriangulationQueries::collect_vertices_map_from_vertices_vector(vertices_vector);
		ContactRemaindersGroupsMap result;
		for(std::size_t sphere_id=0;sphere_id<spheres_exposures.size();sphere_id++)
		{
			if(!spheres_exposures[sphere_id].empty())
			{
				TriangulationQueries::IDsMap::const_iterator ids_vertices_it=ids_vertices.find(sphere_id);
				if(ids_vertices_it!=ids_vertices.end())
				{
					const ContactRemainder::Remainder full_remainder=ContactRemainder::construct_contact_remainder(spheres, vertices_vector, ids_vertices_it->second, sphere_id, probe, sih);
					if(!full_remainder.empty())
					{
						const std::map<int, std::list<std::size_t> >& sphere_exposure=spheres_exposures[sphere_id];
						if(sphere_exposure.size()==1)
						{
							result[sphere_exposure.begin()->first][sphere_id]=full_remainder;
						}
						else
						{
							std::map<int, ContactRemainder::Remainder> split_remainders;
							for(ContactRemainder::Remainder::const_iterator full_remainder_it=full_remainder.begin();full_remainder_it!=full_remainder.end();++full_remainder_it)
							{
								const SimplePoint p=(full_remainder_it->p[0]+full_remainder_it->p[1]+full_remainder_it->p[2])*(1.0/3.0);
								std::pair<double, int> minimal_distance_to_group(std::numeric_limits<double>::max(), -1);
								for(std::map<int, std::list<std::size_t> >::const_iterator sphere_exposure_it=sphere_exposure.begin();sphere_exposure_it!=sphere_exposure.end();++sphere_exposure_it)
								{
									const int group_id=sphere_exposure_it->first;
									const std::list<std::size_t>& contours_ids=sphere_exposure_it->second;
									for(std::list<std::size_t>::const_iterator contours_ids_it=contours_ids.begin();contours_ids_it!=contours_ids.end();++contours_ids_it)
									{
										const ContactContour::Contour& contour=surface_contours_vector[*contours_ids_it].second;
										for(ContactContour::Contour::const_iterator contour_it=contour.begin();contour_it!=contour.end();++contour_it)
										{
											minimal_distance_to_group=std::min(minimal_distance_to_group, std::make_pair(distance_from_point_to_point(p, contour_it->p), group_id));
										}
									}
								}
								split_remainders[minimal_distance_to_group.second].push_back(*full_remainder_it);
							}
							for(std::map<int, ContactRemainder::Remainder>::const_iterator split_remainders_it=split_remainders.begin();split_remainders_it!=split_remainders.end();split_remainders_it++)
							{
								result[split_remainders_it->first][sphere_id]=split_remainders_it->second;
							}
						}
					}
				}
			}
		}

		return result;
	}

private:
	typedef std::vector< std::pair<Pair, ContactContour::Contour> > SurfaceContoursVector;

	static int construct_surface_contours(
			const std::vector<SimpleSphere>& spheres,
			const Triangulation::VerticesVector& vertices_vector,
			const double probe,
			const double step,
			const int projections,
			SurfaceContoursVector& surface_contours_vector,
			std::vector<int>& marks)
	{
		typedef std::tr1::unordered_map< Pair, std::list<std::size_t>, Pair::HashFunctor > PairsIDsMap;
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
					const std::list<ContactContour::Contour> contours=ContactContour::construct_contact_contours(spheres, vertices_vector, pair_vertices_list, a, b, probe, step, projections);
					for(std::list<ContactContour::Contour>::const_iterator contours_it=contours.begin();contours_it!=contours.end();++contours_it)
					{
						const ContactContour::Contour& contour=(*contours_it);
						const std::list<ContactContour::Contour> subcontours=ContactContour::collect_subcontours_from_contour(contour);
						for(std::list<ContactContour::Contour>::const_iterator subcontours_it=subcontours.begin();subcontours_it!=subcontours.end();++subcontours_it)
						{
							const ContactContour::Contour& subcontour=(*subcontours_it);
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
					const ContactContour::Contour& contour=surface_contours_vector[j].second;
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
									const ContactContour::Contour& related_contour=surface_contours_vector[related_contour_id].second;
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
};

}

#endif /* APOLLOTA_CONTACT_REMAINDERS_GROUPING_H_ */
