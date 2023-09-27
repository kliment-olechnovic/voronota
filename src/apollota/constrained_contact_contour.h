#ifndef APOLLOTA_CONSTRAINED_CONTACT_CONTOUR_H_
#define APOLLOTA_CONSTRAINED_CONTACT_CONTOUR_H_

#include <vector>
#include <list>

#include "triangulation.h"
#include "hyperboloid_between_two_spheres.h"
#include "rotation.h"
#include "simple_polygon_utilities.h"

namespace voronota
{

namespace apollota
{

class ConstrainedContactContour
{
public:
	struct PointRecord
	{
		SimplePoint p;
		std::size_t left_id;
		std::size_t right_id;

		PointRecord(const SimplePoint& p, const std::size_t left_id, const std::size_t right_id) : p(p), left_id(left_id), right_id(right_id)
		{
		}
	};

	typedef std::list<PointRecord> Contour;

	struct ContourAreaDescriptor
	{
		std::vector<SimplePoint> outline;
		SimplePoint center;
		bool star_domain;
		std::vector<Triple> simple_polygon_triangulation;

		ContourAreaDescriptor() : star_domain(false)
		{
		}
	};

	static std::list<Contour> construct_contact_contours(
			const std::vector<SimpleSphere>& spheres,
			const Triangulation::VerticesVector& vertices_vector,
			const std::set<std::size_t>& vertices_ids,
			const std::size_t a_id,
			const std::size_t b_id,
			const double probe,
			const double step,
			const int projections,
			const bool simplify)
	{
		std::list<Contour> result;
		if(a_id<spheres.size() && b_id<spheres.size())
		{
			const SimpleSphere& a=spheres[a_id];
			const SimpleSphere& b=spheres[b_id];
			if(minimal_distance_from_sphere_to_sphere(a, b)<(probe*2.0))
			{
				const std::multimap<double, std::size_t> neighbor_ids=collect_pair_neighbors_with_distances(a, spheres, collect_pair_neighbors_from_pair_vertices(a_id, b_id, vertices_vector, vertices_ids));
				const Contour initial_contour=construct_circular_contour(a, b, a_id, vertices_vector, vertices_ids, probe, step);
				if(!initial_contour.empty())
				{
					result.push_back(initial_contour);
					if(!vertices_ids.empty())
					{
						const SimpleSphere bounding_sphere_of_vertices_centers=construct_bounding_sphere_of_vertices_centers(vertices_vector, vertices_ids, step);
						for(std::multimap<double, std::size_t>::const_iterator it=neighbor_ids.begin();it!=neighbor_ids.end();++it)
						{
							const std::size_t c_id=it->second;
							if(c_id<spheres.size())
							{
								const SimpleSphere& c=spheres[c_id];
								std::list<Contour>::iterator jt=result.begin();
								while(jt!=result.end())
								{
									Contour& contour=(*jt);
									std::list<Contour> segments;
									if(cut_and_split_contour(a, c, c_id, contour, segments))
									{
										if(!contour.empty())
										{
											mend_contour(a, b, c, c_id, step, projections, contour);
											if(check_contour_intersects_sphere(bounding_sphere_of_vertices_centers, contour))
											{
												++jt;
											}
											else
											{
												jt=result.erase(jt);
											}
										}
										else
										{
											if(!segments.empty())
											{
												for(std::list<Contour>::iterator st=segments.begin();st!=segments.end();++st)
												{
													mend_contour(a, b, c, c_id, step, projections, (*st));
												}
												filter_contours_intersecting_sphere(bounding_sphere_of_vertices_centers, segments);
												if(!segments.empty())
												{
													result.splice(jt, segments);
												}
											}
											jt=result.erase(jt);
										}
									}
									else
									{
										++jt;
									}
								}
							}
						}
					}
				}
			}
		}
		if(simplify)
		{
			std::list<Contour> simplified_result;
			for(std::list<Contour>::const_iterator it=result.begin();it!=result.end();++it)
			{
				Contour simplified_contour;
				const Contour& contour=(*it);
				for(Contour::const_iterator jt=contour.begin();jt!=contour.end();++jt)
				{
					const PointRecord& pr=(*jt);
					if(pr.left_id!=pr.right_id || pr.left_id==a_id)
					{
						simplified_contour.push_back(pr);
					}
				}
				if(simplified_contour.size()>2)
				{
					simplified_result.push_back(simplified_contour);
				}
				else
				{
					simplified_result.push_back(contour);
				}
			}
			if(!simplified_result.empty())
			{
				result.swap(simplified_result);
			}
		}

		if(!result.empty() && a_id<spheres.size() && b_id<spheres.size())
		{
			const double tolerated_deviation=(probe*1.5);
			bool strangely_extended=false;
			for(std::list<Contour>::const_iterator it=result.begin();it!=result.end() && !strangely_extended;++it)
			{
				const Contour& contour=(*it);
				for(Contour::const_iterator jt=contour.begin();jt!=contour.end() && !strangely_extended;++jt)
				{
					strangely_extended=strangely_extended || (minimal_distance_from_point_to_sphere(jt->p, spheres[a_id])>tolerated_deviation);
					strangely_extended=strangely_extended || (minimal_distance_from_point_to_sphere(jt->p, spheres[b_id])>tolerated_deviation);
				}
			}
			if(strangely_extended)
			{
				SimplePoint safe_center=(SimplePoint(spheres[a_id])+SimplePoint(spheres[b_id]))*0.5;
				std::list<Contour> forcibly_shrunk_result=result;
				for(std::list<Contour>::iterator it=forcibly_shrunk_result.begin();it!=forcibly_shrunk_result.end();++it)
				{
					Contour& contour=(*it);
					for(Contour::iterator jt=contour.begin();jt!=contour.end();++jt)
					{
						if((minimal_distance_from_point_to_sphere(jt->p, spheres[a_id])>tolerated_deviation) || (minimal_distance_from_point_to_sphere(jt->p, spheres[b_id])>tolerated_deviation))
						{
							jt->p=safe_center+(((jt->p)-safe_center).unit()*std::min(spheres[a_id].r, spheres[b_id].r));
						}
					}
				}
				result.swap(forcibly_shrunk_result);
			}
		}

		return result;
	}

	static std::list<Contour> collect_subcontours_from_contour(const Contour& contour)
	{
		std::list<Contour> result;
		Contour::const_iterator start_it=contour.begin();
		while(start_it!=contour.end() && start_it->left_id==start_it->right_id)
		{
			++start_it;
		}
		if(start_it==contour.end())
		{
			result.push_back(contour);
		}
		else
		{
			Contour::const_iterator it=start_it;
			result.push_back(Contour());
			result.back().push_back(*it);
			it=get_right_iterator(contour, it);
			while(it!=start_it)
			{
				if(it->left_id!=it->right_id)
				{
					result.back().push_back(*it);
					result.push_back(Contour());
				}
				result.back().push_back(*it);
				it=get_right_iterator(contour, it);
			}
			result.back().push_back(*it);
		}
		return result;
	}

	static ContourAreaDescriptor construct_contour_area_descriptor(const Contour& contour, const SimpleSphere& sphere1, const SimpleSphere& sphere2, const bool check_and_handle_non_star_domain)
	{
		ContourAreaDescriptor d;
		d.outline=collect_points_from_contour(contour);
		if(!d.outline.empty())
		{
			d.center=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(mass_center<SimplePoint>(d.outline.begin(), d.outline.end()), sphere1, sphere2);
			if(check_and_handle_non_star_domain)
			{
				d.star_domain=check_star_domain(d.outline, d.center);
				if(!d.star_domain)
				{
					d.simple_polygon_triangulation=SimplePolygonUtilities::triangulate_simple_polygon(d.outline, sub_of_points<SimplePoint>(sphere1, sphere2)).triangulation;
				}
			}
			else
			{
				d.star_domain=true;
			}
		}
		return d;
	}

	static std::list<Contour> construct_contact_contours_for_expanded_spheres_without_tessellation(
			const std::vector<SimpleSphere>& spheres,
			const std::size_t a_id,
			const std::size_t b_id,
			const std::vector<std::size_t>& a_neighbor_ids,
			const std::vector<std::size_t>& b_neighbor_ids,
			const double step,
			const int projections,
			const bool simplify)
	{
		std::list<Contour> result;
		if(a_id<spheres.size() && b_id<spheres.size())
		{
			const SimpleSphere& a=spheres[a_id];
			const SimpleSphere& b=spheres[b_id];
			if(sphere_intersects_sphere(a, b))
			{
				const SimpleSphere ic_sphere=intersection_circle_of_two_spheres<SimpleSphere>(a, b);
				if(ic_sphere.r>0.0)
				{
					std::set< std::pair<double, std::size_t> > neighbor_ids;
					for(int j=0;j<2;j++)
					{
						const std::vector<std::size_t>& j_neighbor_ids=(j==0 ? a_neighbor_ids : b_neighbor_ids);
						for(std::size_t i=0;i<j_neighbor_ids.size();i++)
						{
							const std::size_t neighbor_id=j_neighbor_ids[i];
							if(neighbor_id<spheres.size() && neighbor_id!=a_id && neighbor_id!=b_id)
							{
								const SimpleSphere& c=spheres[neighbor_id];
								const double dist_to_ic_sphere=minimal_distance_from_sphere_to_sphere(c, ic_sphere);
								if(dist_to_ic_sphere<0.0)
								{
									neighbor_ids.insert(std::make_pair(dist_to_ic_sphere, neighbor_id));
								}
							}
						}
					}
					{
						const SimplePoint axis=sub_of_points<SimplePoint>(b, a).unit();
						Contour initial_contour;
						construct_circular_contour_from_base_and_axis(a_id, ic_sphere, axis, step, initial_contour);
						if(!initial_contour.empty())
						{
							result.push_back(initial_contour);
						}
					}
					if(!result.empty() && !neighbor_ids.empty())
					{
						for(std::set< std::pair<double, std::size_t> >::const_iterator it=neighbor_ids.begin();it!=neighbor_ids.end();++it)
						{
							const std::size_t c_id=it->second;
							if(c_id<spheres.size())
							{
								const SimpleSphere& c=spheres[c_id];
								std::list<Contour>::iterator jt=result.begin();
								while(jt!=result.end())
								{
									Contour& contour=(*jt);
									std::list<Contour> segments;
									if(cut_and_split_contour(a, c, c_id, contour, segments))
									{
										if(!contour.empty())
										{
											mend_contour(a, b, c, c_id, step, projections, contour);
											if(check_contour_intersects_sphere(ic_sphere, contour))
											{
												++jt;
											}
											else
											{
												jt=result.erase(jt);
											}
										}
										else
										{
											if(!segments.empty())
											{
												for(std::list<Contour>::iterator st=segments.begin();st!=segments.end();++st)
												{
													mend_contour(a, b, c, c_id, step, projections, (*st));
												}
												filter_contours_intersecting_sphere(ic_sphere, segments);
												if(!segments.empty())
												{
													result.splice(jt, segments);
												}
											}
											jt=result.erase(jt);
										}
									}
									else
									{
										++jt;
									}
								}
							}
						}
					}
				}
			}
		}
		if(simplify)
		{
			std::list<Contour> simplified_result;
			for(std::list<Contour>::const_iterator it=result.begin();it!=result.end();++it)
			{
				Contour simplified_contour;
				const Contour& contour=(*it);
				for(Contour::const_iterator jt=contour.begin();jt!=contour.end();++jt)
				{
					const PointRecord& pr=(*jt);
					if(pr.left_id!=pr.right_id || pr.left_id==a_id)
					{
						simplified_contour.push_back(pr);
					}
				}
				if(simplified_contour.size()>2)
				{
					simplified_result.push_back(simplified_contour);
				}
				else
				{
					simplified_result.push_back(contour);
				}
			}
			if(!simplified_result.empty())
			{
				result.swap(simplified_result);
			}
		}

		if(!result.empty() && a_id<spheres.size() && b_id<spheres.size())
		{
			const double tolerated_deviation=0.5;
			bool strangely_extended=false;
			for(std::list<Contour>::const_iterator it=result.begin();it!=result.end() && !strangely_extended;++it)
			{
				const Contour& contour=(*it);
				for(Contour::const_iterator jt=contour.begin();jt!=contour.end() && !strangely_extended;++jt)
				{
					strangely_extended=strangely_extended || (minimal_distance_from_point_to_sphere(jt->p, spheres[a_id])>tolerated_deviation);
					strangely_extended=strangely_extended || (minimal_distance_from_point_to_sphere(jt->p, spheres[b_id])>tolerated_deviation);
				}
			}
			if(strangely_extended)
			{
				SimplePoint safe_center=(SimplePoint(spheres[a_id])+SimplePoint(spheres[b_id]))*0.5;
				std::list<Contour> forcibly_shrunk_result=result;
				for(std::list<Contour>::iterator it=forcibly_shrunk_result.begin();it!=forcibly_shrunk_result.end();++it)
				{
					Contour& contour=(*it);
					for(Contour::iterator jt=contour.begin();jt!=contour.end();++jt)
					{
						if((minimal_distance_from_point_to_sphere(jt->p, spheres[a_id])>tolerated_deviation) || (minimal_distance_from_point_to_sphere(jt->p, spheres[b_id])>tolerated_deviation))
						{
							jt->p=safe_center+(((jt->p)-safe_center).unit()*std::min(spheres[a_id].r, spheres[b_id].r));
						}
					}
				}
				result.swap(forcibly_shrunk_result);
			}
		}

		return result;
	}

private:
	static std::set<std::size_t> collect_pair_neighbors_from_pair_vertices(
			const std::size_t a_id,
			const std::size_t b_id,
			const Triangulation::VerticesVector& vertices_vector,
			const std::set<std::size_t>& vertices_ids)
	{
		std::set<std::size_t> neighbors_ids;
		for(std::set<std::size_t>::const_iterator it=vertices_ids.begin();it!=vertices_ids.end();++it)
		{
			const std::size_t vertex_id=(*it);
			if(vertex_id<vertices_vector.size())
			{
				const Quadruple& quadruple=vertices_vector[vertex_id].first;
				for(int i=0;i<4;i++)
				{
					const std::size_t neighbor_id=quadruple.get(i);
					if(neighbor_id!=a_id && neighbor_id!=b_id)
					{
						neighbors_ids.insert(neighbor_id);
					}
				}
			}
		}
		return neighbors_ids;
	}

	static std::multimap<double, std::size_t> collect_pair_neighbors_with_distances(
			const SimpleSphere& a,
			const std::vector<SimpleSphere>& spheres,
			const std::set<std::size_t>& neighbor_ids)
	{
		std::multimap<double, std::size_t> result;
		for(std::set<std::size_t>::const_iterator it=neighbor_ids.begin();it!=neighbor_ids.end();++it)
		{
			const std::size_t c_id=(*it);
			if(c_id<spheres.size())
			{
				const SimpleSphere& c=spheres[c_id];
				result.insert(std::make_pair(minimal_distance_from_sphere_to_sphere(a, c), c_id));
			}
		}
		return result;
	}

	static bool check_if_radiuses_of_vertices_are_below_probe(
			const Triangulation::VerticesVector& vertices_vector,
			const std::set<std::size_t>& vertices_ids,
			const double probe)
	{
		for(std::set<std::size_t>::const_iterator it=vertices_ids.begin();it!=vertices_ids.end();++it)
		{
			if(vertices_vector[(*it)].second.r>=probe)
			{
				return false;
			}
		}
		return true;
	}

	static SimpleSphere construct_bounding_sphere_of_vertices_centers(
			const Triangulation::VerticesVector& vertices_vector,
			const std::set<std::size_t>& vertices_ids,
			const double radius_extension)
	{
		if(!vertices_ids.empty())
		{
			SimplePoint mc;
			for(std::set<std::size_t>::const_iterator it=vertices_ids.begin();it!=vertices_ids.end();++it)
			{
				mc=mc+SimplePoint(vertices_vector[(*it)].second);
			}
			mc=mc*(1.0/static_cast<double>(vertices_ids.size()));
			SimpleSphere result(mc, 0.0);
			for(std::set<std::size_t>::const_iterator it=vertices_ids.begin();it!=vertices_ids.end();++it)
			{
				result.r=std::max(result.r, distance_from_point_to_point(result, vertices_vector[(*it)].second));
			}
			result.r+=radius_extension;
			return result;
		}
		else
		{
			return SimpleSphere();
		}
	}

	static Contour construct_circular_contour(
			const SimpleSphere& a,
			const SimpleSphere& b,
			const std::size_t a_id,
			const Triangulation::VerticesVector& vertices_vector,
			const std::set<std::size_t>& vertices_ids,
			const double probe,
			const double step)
	{
		Contour result;
		const SimpleSphere a_expanded=custom_sphere_from_point<SimpleSphere>(a, a.r+probe);
		const SimpleSphere b_expanded=custom_sphere_from_point<SimpleSphere>(b, b.r+probe);
		if(sphere_intersects_sphere(a_expanded, b_expanded))
		{
			const SimplePoint axis=sub_of_points<SimplePoint>(b, a).unit();
			if(vertices_ids.size()>1 && check_if_radiuses_of_vertices_are_below_probe(vertices_vector, vertices_ids, probe))
			{
				construct_circular_contour_from_base_and_axis(a_id, construct_bounding_sphere_of_vertices_centers(vertices_vector, vertices_ids, step), axis, step, result);
				for(Contour::iterator it=result.begin();it!=result.end();++it)
				{
					it->p=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(it->p, a, b);
				}
			}
			else
			{
				construct_circular_contour_from_base_and_axis(a_id, intersection_circle_of_two_spheres<SimpleSphere>(a_expanded, b_expanded), axis, step, result);
			}
		}
		return result;
	}

	static void construct_circular_contour_from_base_and_axis(
			const std::size_t a_id,
			const SimpleSphere& base,
			const SimplePoint& axis,
			const double step,
			Contour& result)
	{
		Rotation rotation(axis, 0);
		const SimplePoint first_point=any_normal_of_vector<SimplePoint>(rotation.axis)*base.r;
		const double angle_step=std::max(std::min(360*(step/(2*pi_value()*base.r)), 60.0), 5.0);
		result.push_back(PointRecord(sum_of_points<SimplePoint>(base, first_point), a_id, a_id));
		for(rotation.angle=angle_step;rotation.angle<360;rotation.angle+=angle_step)
		{
			result.push_back(PointRecord(sum_of_points<SimplePoint>(base, rotation.rotate<SimplePoint>(first_point)), a_id, a_id));
		}
	}

	static bool cut_and_split_contour(
			const SimpleSphere& a,
			const SimpleSphere& c,
			const std::size_t c_id,
			Contour& contour,
			std::list<Contour>& segments)
	{
		const std::size_t outsiders_count=mark_contour(a, c, c_id, contour);
		if(outsiders_count>0)
		{
			if(outsiders_count<contour.size())
			{
				std::list<Contour::iterator> cuts;
				const int cuts_count=cut_contour(a, c, c_id, contour, cuts);
				if(cuts_count>0 && cuts_count%2==0)
				{
					if(cuts_count>2)
					{
						order_cuts(cuts);
						split_contour(contour, cuts, segments);
					}
				}
			}
			else
			{
				contour.clear();
			}
			return true;
		}
		return false;
	}

	static std::size_t mark_contour(
			const SimpleSphere& a,
			const SimpleSphere& c,
			const std::size_t c_id,
			Contour& contour)
	{
		std::size_t outsiders_count=0;
		for(Contour::iterator it=contour.begin();it!=contour.end();++it)
		{
			if(minimal_distance_from_point_to_sphere(it->p, c)<minimal_distance_from_point_to_sphere(it->p, a))
			{
				it->left_id=c_id;
				it->right_id=c_id;
				outsiders_count++;
			}
		}
		return outsiders_count;
	}

	static std::size_t cut_contour(
			const SimpleSphere& a,
			const SimpleSphere& c,
			const std::size_t c_id,
			Contour& contour,
			std::list<Contour::iterator>& cuts)
	{
		std::size_t cuts_count=0;
		Contour::iterator it=contour.begin();
		while(it!=contour.end())
		{
			if(it->left_id==c_id && it->right_id==c_id)
			{
				const Contour::iterator left_it=get_left_iterator(contour, it);
				const Contour::iterator right_it=get_right_iterator(contour, it);

				if(left_it->right_id!=c_id)
				{
					const SimplePoint& p0=it->p;
					const SimplePoint& p1=left_it->p;
					const double l=HyperboloidBetweenTwoSpheres::intersect_vector_with_hyperboloid(p0, p1, a, c);
					cuts.push_back(contour.insert(it, PointRecord(p0+((p1-p0).unit()*l), left_it->right_id, it->left_id)));
					cuts_count++;
				}

				if(right_it->left_id!=c_id)
				{
					const SimplePoint& p0=it->p;
					const SimplePoint& p1=right_it->p;
					const double l=HyperboloidBetweenTwoSpheres::intersect_vector_with_hyperboloid(p0, p1, a, c);
					cuts.push_back(contour.insert(right_it, PointRecord(p0+((p1-p0).unit()*l), it->right_id, right_it->left_id)));
					cuts_count++;
				}

				it=contour.erase(it);
			}
			else
			{
				++it;
			}
		}
		return cuts_count;
	}

	static void order_cuts(std::list<Contour::iterator>& cuts)
	{
		double sums[2]={0.0, 0.0};
		for(int i=0;i<2;i++)
		{
			if(i==1)
			{
				shift_list(cuts, false);
			}
			std::list<Contour::iterator>::const_iterator it=cuts.begin();
			while(it!=cuts.end())
			{
				std::list<Contour::iterator>::const_iterator next=it;
				++next;
				if(next!=cuts.end())
				{
					sums[i]+=distance_from_point_to_point((*it)->p, (*next)->p);
					it=next;
					++it;
				}
				else
				{
					it=cuts.end();
				}
			}
		}
		if(sums[0]<sums[1])
		{
			shift_list(cuts, true);
		}
	}

	static std::size_t split_contour(
			Contour& contour,
			const std::list<Contour::iterator>& ordered_cuts,
			std::list<Contour>& segments)
	{
		std::size_t segments_count=0;
		std::list<Contour::iterator>::const_iterator it=ordered_cuts.begin();
		while(it!=ordered_cuts.end())
		{
			std::list<Contour::iterator>::const_iterator next=it;
			++next;
			if(next!=ordered_cuts.end())
			{
				if((*next)!=get_right_iterator(contour, (*it)))
				{
					segments_count++;
					Contour segment;
					Contour::iterator jt=(*it);
					do
					{
						segment.push_back(*jt);
						jt=get_right_iterator(contour, jt);
					}
					while(jt!=(*next));
					segments.push_back(segment);
				}
				it=next;
				++it;
			}
			else
			{
				it=ordered_cuts.end();
			}
		}
		if(segments_count>0)
		{
			contour.clear();
		}
		return segments_count;
	}

	static void mend_contour(
			const SimpleSphere& a,
			const SimpleSphere& b,
			const SimpleSphere& c,
			const std::size_t c_id,
			const double step,
			const int projections,
			Contour& contour)
	{
		Contour::iterator it=contour.begin();
		while(it!=contour.end())
		{
			if(it->left_id!=c_id && it->right_id==c_id)
			{
				const Contour::iterator jt=get_right_iterator(contour, it);
				if(jt->left_id==c_id)
				{
					SimplePoint& p0=it->p;
					SimplePoint& p1=jt->p;
					for(int e=0;e<projections;e++)
					{
						p0=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p0, b, c);
						p0=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p0, a, c);
						p0=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p0, a, b);
						p1=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p1, b, c);
						p1=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p1, a, c);
						p1=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p1, a, b);
					}
					const double distance=distance_from_point_to_point(p0, p1);
					if(distance>step)
					{
						const int leap_distance=static_cast<int>(floor(distance/step+0.5));
						const double leap_size=distance/static_cast<double>(leap_distance);
						const SimplePoint direction=(p1-p0).unit();
						for(int leap=1;leap<leap_distance;leap++)
						{
							SimplePoint p=p0+(direction*(leap_size*static_cast<double>(leap)));
							for(int e=0;e<projections;e++)
							{
								p=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p, b, c);
								p=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p, a, c);
								p=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p, a, b);
							}
							contour.insert(jt, PointRecord(p, c_id, c_id));
						}
					}
				}
			}
			++it;
		}
	}

	static bool check_contour_intersects_sphere(const SimpleSphere& shell, const Contour& contour)
	{
		for(Contour::const_iterator it=contour.begin();it!=contour.end();++it)
		{
			if(distance_from_point_to_point(shell, it->p)<=shell.r)
			{
				return true;
			}
		}
		return false;
	}

	static void filter_contours_intersecting_sphere(const SimpleSphere& shell, std::list<Contour>& contours)
	{
		std::list<Contour>::iterator it=contours.begin();
		while(it!=contours.end())
		{
			if(!check_contour_intersects_sphere(shell, (*it)))
			{
				it=contours.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	template<typename List, typename Iterator>
	static Iterator get_left_iterator(List& container, const Iterator& iterator)
	{
		Iterator left_it=iterator;
		if(left_it==container.begin())
		{
			left_it=container.end();
		}
		--left_it;
		return left_it;
	}

	template<typename List, typename Iterator>
	static Iterator get_right_iterator(List& container, const Iterator& iterator)
	{
		Iterator right_it=iterator;
		++right_it;
		if(right_it==container.end())
		{
			right_it=container.begin();
		}
		return right_it;
	}

	template<typename List>
	static void shift_list(List& list, const bool reverse)
	{
		if(!reverse)
		{
			list.push_front(*list.rbegin());
			list.pop_back();
		}
		else
		{
			list.push_back(*list.begin());
			list.pop_front();
		}
	}

	static std::vector<SimplePoint> collect_points_from_contour(const Contour& contour)
	{
		std::vector<SimplePoint> result;
		if(!contour.empty())
		{
			result.reserve(contour.size());
			for(Contour::const_iterator jt=contour.begin();jt!=contour.end();++jt)
			{
				result.push_back(jt->p);
			}
		}
		return result;
	}

	static bool check_star_domain(const std::vector<SimplePoint>& outline, const SimplePoint& center)
	{
		bool star_domain=(outline.size()>2);
		for(std::size_t i0=0;i0<outline.size() && star_domain;i0++)
		{
			std::size_t i1=(i0+1);
			std::size_t i2=(i0+2);
			if(i1>=outline.size())
			{
				i1=0;
				i2=1;
			}
			if(i2>=outline.size())
			{
				i2=0;
			}
			const SimplePoint v0=(outline[i0]-center).unit();
			const SimplePoint v1=(outline[i1]-center).unit();
			const SimplePoint v2=(outline[i2]-center).unit();
			if(dot_product(v0, v1)<dot_product(v0, v2))
			{
				star_domain=false;
			}
		}
		return star_domain;
	}
};

}

}

#endif /* APOLLOTA_CONSTRAINED_CONTACT_CONTOUR_H_ */
