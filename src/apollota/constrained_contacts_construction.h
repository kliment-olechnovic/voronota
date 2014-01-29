#ifndef APOLLOTA_CONSTRAINED_CONTACT_REMAINDERS_GROUPING_H_
#define APOLLOTA_CONSTRAINED_CONTACT_REMAINDERS_GROUPING_H_

#include "constrained_contact_contour.h"
#include "constrained_contact_remainder.h"

namespace apollota
{

class ConstrainedContactsConstruction
{
public:
	struct ContactRemainderDescriptorFull
	{
		ConstrainedContactRemainder::Remainder remainder;

		void feed(const ConstrainedContactRemainder::Remainder& input)
		{
			remainder.insert(remainder.end(), input.begin(), input.end());
		}
	};

	struct ContactRemainderDescriptorSummary
	{
		double area;

		ContactRemainderDescriptorSummary() : area(0.0)
		{
		}

		void feed(const ConstrainedContactRemainder::Remainder& input)
		{
			for(ConstrainedContactRemainder::Remainder::const_iterator it=input.begin();it!=input.end();++it)
			{
				area+=triangle_area(it->p[0], it->p[1], it->p[2]);
			}
		}
	};

	static std::vector<SimpleSphere> construct_artificial_boundary(const std::vector<SimpleSphere>& spheres, const double coordinate_shift)
	{
		std::vector<SimpleSphere> result;
		if(!spheres.empty())
		{
			SimpleSphere a(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
			SimpleSphere b(std::numeric_limits<double>::min(), std::numeric_limits<double>::min(), std::numeric_limits<double>::min(), std::numeric_limits<double>::min());
			for(std::vector<SimpleSphere>::const_iterator it=spheres.begin();it!=spheres.end();++it)
			{
				a.x=std::min(a.x, it->x);
				a.y=std::min(a.y, it->y);
				a.z=std::min(a.z, it->z);
				a.r=std::min(a.r, it->r);
				b.x=std::max(b.x, it->x);
				b.y=std::max(b.y, it->y);
				b.z=std::max(b.z, it->z);
				b.r=std::max(b.r, it->r);
			}
			const double r=std::max(b.r, 0.0);
			const double shift=std::max(coordinate_shift, 0.0)+(r*2.0)+1.0;
			result.reserve(8);
			result.push_back(SimpleSphere(a.x-shift, a.y-shift, a.z-shift, r));
			result.push_back(SimpleSphere(a.x-shift, a.y-shift, b.z+shift, r));
			result.push_back(SimpleSphere(a.x-shift, b.y+shift, a.z-shift, r));
			result.push_back(SimpleSphere(a.x-shift, b.y+shift, b.z+shift, r));
			result.push_back(SimpleSphere(b.x+shift, a.y-shift, a.z-shift, r));
			result.push_back(SimpleSphere(b.x+shift, a.y-shift, b.z+shift, r));
			result.push_back(SimpleSphere(b.x+shift, b.y+shift, a.z-shift, r));
			result.push_back(SimpleSphere(b.x+shift, b.y+shift, b.z+shift, r));
		}
		return result;
	}

	template<typename ContactRemainderDescriptor>
	static std::map< int, std::map<std::size_t, ContactRemainderDescriptor> > construct_grouped_remainders(
			const std::vector<SimpleSphere>& spheres,
			const Triangulation::VerticesVector& vertices_vector,
			const double probe,
			const double step,
			const int projections,
			const std::size_t sih_depth)
	{
		std::vector< std::pair<Pair, ConstrainedContactContour::Contour> > surface_contours_vector;
		std::vector<int> marks;
		ConstrainedContactsConstruction::construct_surface_contours(spheres, vertices_vector, probe, step, projections, surface_contours_vector, marks);

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
		std::map< int, std::map<std::size_t, ContactRemainderDescriptor> > result;
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
								std::pair<double, int> minimal_distance_to_group(std::numeric_limits<double>::max(), -1);
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
							for(std::map<int, ConstrainedContactRemainder::Remainder>::const_iterator split_remainders_it=split_remainders.begin();split_remainders_it!=split_remainders.end();split_remainders_it++)
							{
								result[split_remainders_it->first][sphere_id].feed(split_remainders_it->second);
							}
						}
					}
				}
			}
		}

		return result;
	}

private:
	static int construct_surface_contours(
			const std::vector<SimpleSphere>& spheres,
			const Triangulation::VerticesVector& vertices_vector,
			const double probe,
			const double step,
			const int projections,
			std::vector< std::pair<Pair, ConstrainedContactContour::Contour> >& surface_contours_vector,
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
					const std::list<ConstrainedContactContour::Contour> contours=ConstrainedContactContour::construct_contact_contours(spheres, vertices_vector, pair_vertices_list, a, b, probe, step, projections);
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
};

}

#endif /* APOLLOTA_CONSTRAINED_CONTACT_REMAINDERS_GROUPING_H_ */
