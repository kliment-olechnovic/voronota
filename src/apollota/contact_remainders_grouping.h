#ifndef APOLLOTA_CONTACT_REMAINDERS_GROUPING_H_
#define APOLLOTA_CONTACT_REMAINDERS_GROUPING_H_

#include "contact_contour.h"
#include "contact_remainder.h"

namespace apollota
{

class ContactRemaindersGrouping
{
public:
	typedef std::vector< std::pair<Pair, ContactContour::Contour> > SurfaceContoursVector;

	static int construct_surface_contours(
			const std::vector<SimpleSphere>& spheres,
			const Triangulation::VerticesVector& vertices_vector,
			const double probe,
			const double step,
			const int projections,
			SurfaceContoursVector& pairs_surface_contours_vector,
			std::vector<int>& marks)
	{
		typedef std::tr1::unordered_map< Pair, std::list<std::size_t>, Pair::HashFunctor > PairsIDsMap;
		const TriangulationQueries::PairsMap pairs_vertices=TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector);
		PairsIDsMap pairs_surface_contours_map;
		pairs_surface_contours_vector.clear();
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
								pairs_surface_contours_map[it->first].push_back(pairs_surface_contours_vector.size());
								pairs_surface_contours_vector.push_back(std::make_pair(it->first, subcontour));
							}
						}
					}
				}
			}
		}
		marks.clear();
		marks.resize(pairs_surface_contours_vector.size(), 0);
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
					const std::size_t a=pairs_surface_contours_vector[j].first.get(0);
					const std::size_t b=pairs_surface_contours_vector[j].first.get(1);
					const ContactContour::Contour& contour=pairs_surface_contours_vector[j].second;
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
									const ContactContour::Contour& related_contour=pairs_surface_contours_vector[related_contour_id].second;
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
