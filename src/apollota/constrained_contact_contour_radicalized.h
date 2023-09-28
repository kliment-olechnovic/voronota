#ifndef APOLLOTA_CONSTRAINED_CONTACT_CONTOUR_RADICALIZED_H_
#define APOLLOTA_CONSTRAINED_CONTACT_CONTOUR_RADICALIZED_H_

#include <vector>
#include <list>

#include "basic_operations_on_spheres.h"
#include "rotation.h"

namespace voronota
{

namespace apollota
{

class ConstrainedContactContourRadicalized
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

		ContourAreaDescriptor()
		{
		}
	};

	static ContourAreaDescriptor construct_contour_area_descriptor(const Contour& contour)
	{
		ContourAreaDescriptor d;
		d.outline=collect_points_from_contour(contour);
		if(!d.outline.empty())
		{
			d.center=mass_center<SimplePoint>(d.outline.begin(), d.outline.end());
		}
		return d;
	}

	static std::list<Contour> construct_contact_contours_for_expanded_spheres_without_tessellation(
			const std::vector<SimpleSphere>& spheres,
			const std::size_t a_id,
			const std::size_t b_id,
			const std::vector<std::size_t>& a_neighbor_ids,
			const std::vector<std::size_t>& b_neighbor_ids,
			const double step)
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
		return result;
	}

private:
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
			if(halfspace_of_point_with_radical_plane(it->p, a, c)>0)
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
					const double l=intersect_vector_with_radical_plane(p0, p1, a, c);
					cuts.push_back(contour.insert(it, PointRecord(p0+((p1-p0).unit()*l), left_it->right_id, it->left_id)));
					cuts_count++;
				}

				if(right_it->left_id!=c_id)
				{
					const SimplePoint& p0=it->p;
					const SimplePoint& p1=right_it->p;
					const double l=intersect_vector_with_radical_plane(p0, p1, a, c);
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

	static inline double intersect_vector_with_radical_plane(const SimplePoint& a, const SimplePoint& b, const SimpleSphere& s1, const SimpleSphere& s2)
	{
		const SimpleSphere ic=intersection_circle_of_two_spheres<SimpleSphere>(s1, s2);
		const SimplePoint plane_normal=sub_of_points<SimplePoint>(s2, s1).unit();
		const double da=signed_distance_from_point_to_plane(ic, plane_normal, a);
		const double db=signed_distance_from_point_to_plane(ic, plane_normal, b);
		if(da*db>0.0)
		{
			return 0.0;
		}
		const double t=da/(da-db);
		return (point_module(sub_of_points<PODPoint>(b, a))*t);
	}

	static int halfspace_of_point_with_radical_plane(const SimplePoint& x, const SimpleSphere& s1, const SimpleSphere& s2)
	{
		const SimpleSphere ic=intersection_circle_of_two_spheres<SimpleSphere>(s1, s2);
		const PODPoint plane_normal=sub_of_points<PODPoint>(s2, s1);
		return halfspace_of_point(ic, plane_normal, x);
	}
};

}

}

#endif /* APOLLOTA_CONSTRAINED_CONTACT_CONTOUR_RADICALIZED_H_ */
