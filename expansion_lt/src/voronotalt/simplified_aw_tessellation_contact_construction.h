#ifndef VORONOTALT_SIMPLIFIED_AW_TESSELLATION_CONTACT_CONSTRUCTION_H_
#define VORONOTALT_SIMPLIFIED_AW_TESSELLATION_CONTACT_CONSTRUCTION_H_

#include <vector>
#include <list>
#include <algorithm>

#include "basic_types_and_functions.h"

namespace voronotalt
{

class SimplifiedAWTessellationContactConstruction
{
public:
	struct ContourPoint
	{
		SimplePoint p;
		std::size_t left_id;
		std::size_t right_id;

		ContourPoint(const SimplePoint& p, const std::size_t left_id, const std::size_t right_id) : p(p), left_id(left_id), right_id(right_id)
		{
		}
	};

	typedef std::list<ContourPoint> Contour;

	struct NeighborDescriptor
	{
		double sort_value;
		std::size_t neighbor_id;

		NeighborDescriptor() : sort_value(0.0), neighbor_id(0)
		{
		}

		bool operator<(const NeighborDescriptor& d) const
		{
			return (sort_value<d.sort_value || (sort_value==d.sort_value && neighbor_id<d.neighbor_id));
		}
	};

	struct ContourGraphics
	{
		std::vector<SimplePoint> outer_points;
		SimplePoint barycenter;

		ContourGraphics()
		{
		}

		void clear()
		{
			outer_points.clear();
		}
	};

	struct ContactDescriptor
	{
		std::list<Contour> contours;
		std::vector<NeighborDescriptor> neighbor_descriptors;
		std::vector<ContourGraphics> graphics;
		SimpleSphere intersection_circle_sphere;
		SimplePoint intersection_circle_axis;
		Float area;
		UnsignedInt id_a;
		UnsignedInt id_b;

		ContactDescriptor() :
			area(FLOATCONST(0.0)),
			id_a(0),
			id_b(0)
		{
		}

		void clear()
		{
			id_a=0;
			id_b=0;
			neighbor_descriptors.clear();
			contours.clear();
			graphics.clear();
			area=FLOATCONST(0.0);
		}
	};

	static bool construct_contact_descriptor(
			const std::vector<SimpleSphere>& spheres,
			const UnsignedInt a_id,
			const UnsignedInt b_id,
			const std::vector<UnsignedInt>& a_neighbor_collisions,
			const Float step,
			const int projections,
			ContactDescriptor& result_contact_descriptor)
	{
		result_contact_descriptor.clear();
		if(a_id<spheres.size() && b_id<spheres.size())
		{
			result_contact_descriptor.id_a=a_id;
			result_contact_descriptor.id_b=b_id;
			const SimpleSphere& a=spheres[a_id];
			const SimpleSphere& b=spheres[b_id];
			if(sphere_intersects_sphere(a, b) && !sphere_contains_sphere(a, b) && !sphere_contains_sphere(b, a))
			{
				result_contact_descriptor.intersection_circle_sphere=intersection_circle_of_two_spheres(a, b);
				if(result_contact_descriptor.intersection_circle_sphere.r>FLOATCONST(0.0))
				{
					bool discarded=false;
					{
						for(UnsignedInt i=0;i<a_neighbor_collisions.size() && !discarded;i++)
						{
							const UnsignedInt neighbor_id=a_neighbor_collisions[i];
							if(neighbor_id!=b_id)
							{
								const SimpleSphere& c=spheres[neighbor_id];
								if(sphere_intersects_sphere(result_contact_descriptor.intersection_circle_sphere, c) && sphere_intersects_sphere(b, c))
								{
									if(sphere_contains_sphere(c, a) || sphere_contains_sphere(c, b))
									{
										discarded=true;
									}
									else
									{
										const SimplePoint nd_ac_plane_center=center_of_intersection_circle_of_two_spheres(a, c);
										const SimplePoint nd_ac_plane_normal=unit_point(sub_of_points(c.p, a.p));
										const Float cos_val=dot_product(unit_point(sub_of_points(result_contact_descriptor.intersection_circle_sphere.p, a.p)), unit_point(sub_of_points(nd_ac_plane_center, a.p)));
										const int hsi=halfspace_of_point(nd_ac_plane_center, nd_ac_plane_normal, result_contact_descriptor.intersection_circle_sphere.p);
										if(std::abs(cos_val)<FLOATCONST(1.0))
										{
											const Float l=std::abs(signed_distance_from_point_to_plane(nd_ac_plane_center, nd_ac_plane_normal, result_contact_descriptor.intersection_circle_sphere.p));
											const Float xl=l/std::sqrt(1-(cos_val*cos_val));
											if(xl>=result_contact_descriptor.intersection_circle_sphere.r)
											{
												if(hsi>=0)
												{
													discarded=true;
												}
											}
											else
											{
												NeighborDescriptor nd;
												nd.neighbor_id=neighbor_id;
												nd.sort_value=(hsi>0 ? (0.0-xl) : xl);
												result_contact_descriptor.neighbor_descriptors.push_back(nd);
											}
										}
										else
										{
											if(hsi>0)
											{
												discarded=true;
											}
										}
									}
								}
							}
						}
					}
					if(!discarded)
					{
						result_contact_descriptor.intersection_circle_axis=unit_point(sub_of_points(b.p, a.p));

						{
							Contour initial_contour;
							init_contour_from_base_and_axis(a_id, result_contact_descriptor.intersection_circle_sphere, result_contact_descriptor.intersection_circle_axis, step, initial_contour);
							if(!initial_contour.empty())
							{
								result_contact_descriptor.contours.push_back(initial_contour);
							}
						}

						if(!result_contact_descriptor.neighbor_descriptors.empty())
						{
							if(!result_contact_descriptor.contours.empty())
							{
								std::sort(result_contact_descriptor.neighbor_descriptors.begin(), result_contact_descriptor.neighbor_descriptors.end());
								for(UnsignedInt i=0;i<result_contact_descriptor.neighbor_descriptors.size();i++)
								{
									const std::size_t c_id=result_contact_descriptor.neighbor_descriptors[i].neighbor_id;
									const SimpleSphere& c=spheres[c_id];
									std::list<Contour>::iterator jt=result_contact_descriptor.contours.begin();
									while(jt!=result_contact_descriptor.contours.end())
									{
										Contour& contour=(*jt);
										std::list<Contour> segments;
										if(cut_and_split_contour(a, c, c_id, contour, segments))
										{
											if(!contour.empty())
											{
												mend_contour(a, b, c, c_id, step, projections, contour);
												if(check_contour_intersects_sphere(result_contact_descriptor.intersection_circle_sphere, contour))
												{
													++jt;
												}
												else
												{
													jt=result_contact_descriptor.contours.erase(jt);
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
													filter_contours_intersecting_sphere(result_contact_descriptor.intersection_circle_sphere, segments);
													if(!segments.empty())
													{
														result_contact_descriptor.contours.splice(jt, segments);
													}
												}
												jt=result_contact_descriptor.contours.erase(jt);
											}
										}
										else
										{
											++jt;
										}
									}
								}
							}

							if(!result_contact_descriptor.contours.empty())
							{
								const double tolerated_deviation=0.5;
								bool strangely_extended=false;
								for(std::list<Contour>::const_iterator it=result_contact_descriptor.contours.begin();it!=result_contact_descriptor.contours.end() && !strangely_extended;++it)
								{
									const Contour& contour=(*it);
									for(Contour::const_iterator jt=contour.begin();jt!=contour.end() && !strangely_extended;++jt)
									{
										strangely_extended=strangely_extended || (distance_from_point_to_point(jt->p, a.p)-a.r>tolerated_deviation);
										strangely_extended=strangely_extended || (distance_from_point_to_point(jt->p, b.p)-b.r>tolerated_deviation);
									}
								}
								if(strangely_extended)
								{
									SimplePoint safe_center=point_and_number_product(sum_of_points(a.p, b.p), 0.5);
									std::list<Contour> forcibly_shrunk_result=result_contact_descriptor.contours;
									for(std::list<Contour>::iterator it=forcibly_shrunk_result.begin();it!=forcibly_shrunk_result.end();++it)
									{
										Contour& contour=(*it);
										for(Contour::iterator jt=contour.begin();jt!=contour.end();++jt)
										{
											if((distance_from_point_to_point(jt->p, a.p)-a.r>tolerated_deviation) || (distance_from_point_to_point(jt->p, b.p)-b.r>tolerated_deviation))
											{
												jt->p=sum_of_points(safe_center, point_and_number_product(unit_point(sub_of_points(jt->p, safe_center)), std::min(a.r, b.r)));
											}
										}
									}
									result_contact_descriptor.contours.swap(forcibly_shrunk_result);
								}
							}
						}

						if(!result_contact_descriptor.contours.empty())
						{
							for(std::list<Contour>::const_iterator it=result_contact_descriptor.contours.begin();it!=result_contact_descriptor.contours.end();++it)
							{
								const Contour& contour=(*it);
								ContourGraphics graphics;
								result_contact_descriptor.area+=calculate_area_from_contour(contour, a, b, graphics.outer_points, graphics.barycenter);
								if(!graphics.outer_points.empty())
								{
									result_contact_descriptor.graphics.push_back(graphics);
								}
							}
						}
					}
				}
			}
		}
		return (result_contact_descriptor.area>FLOATCONST(0.0));
	}

private:
	class HyperboloidBetweenTwoSpheres
	{
	public:
		static inline SimplePoint project_point_on_hyperboloid(const SimplePoint& p, const SimpleSphere& s1, const SimpleSphere& s2)
		{
			if(s1.r>s2.r)
			{
				return project_point_on_hyperboloid(p, s2, s1);
			}
			else
			{
				const SimplePoint dv=point_and_number_product(sub_of_points(s1.p, s2.p), 0.5);
				const SimplePoint dv_unit=unit_point(dv);
				const SimplePoint c=sum_of_points(s2.p, dv);
				const SimplePoint cp=sub_of_points(p, c);
				const double lz=dot_product(dv_unit, cp);
				const double lx=std::sqrt(std::max(squared_point_module(cp)-(lz*lz), 0.0));
				const double z=project_point_on_simple_hyperboloid(lx, 0, point_module(dv), s1.r, s2.r);
				return sum_of_points(sub_of_points(p, point_and_number_product(dv_unit, lz)), point_and_number_product(dv_unit, z));
			}
		}

		static inline double intersect_vector_with_hyperboloid(const SimplePoint& a, const SimplePoint& b, const SimpleSphere& s1, const SimpleSphere& s2)
		{
			if(s1.r>s2.r)
			{
				return intersect_vector_with_hyperboloid(a, b, s2, s1);
			}
			else
			{
				const SimplePoint dv=point_and_number_product(sub_of_points(s1.p, s2.p), 0.5);
				const SimplePoint dv_unit=unit_point(dv);
				const SimplePoint c=sum_of_points(s2.p, dv);

				const SimplePoint ca=sub_of_points(a, c);
				const double maz=dot_product(dv_unit, ca);
				const SimplePoint cax=sub_of_points(sub_of_points(a, point_and_number_product(dv_unit, maz)), c);
				const SimplePoint cax_unit=unit_point(cax);
				const double max=dot_product(cax_unit, ca);
				const double may=0.0;

				const SimplePoint cb=sub_of_points(b, c);
				const double mbz=dot_product(dv_unit, cb);
				const double mbx=dot_product(cax_unit, cb);
				const double mby=std::sqrt(std::max(squared_point_module(cb)-mbz*mbz-mbx*mbx, 0.0));

				return intersect_vector_with_simple_hyperboloid(SimplePoint(max, may, maz), SimplePoint(mbx, mby, mbz), point_module(dv), s1.r, s2.r);
			}
		}

	private:
		static inline double project_point_on_simple_hyperboloid(const double x, const double y, const double d, const double r1, const double r2)
		{
			if(r1>r2)
			{
				return project_point_on_simple_hyperboloid(x, y, d, r2, r1);
			}
			else
			{
				const double r=r2-r1;
				return 2*r*std::sqrt(std::max((0-r*r+4*d*d)*(4*x*x+4*y*y+4*d*d-r*r), 0.0))/(0-4*r*r+16*d*d);
			}
		}

		static inline double intersect_vector_with_simple_hyperboloid(const SimplePoint& a, const SimplePoint& b, const double d, const double r1, const double r2)
		{
			if(r1>r2)
			{
				return intersect_vector_with_simple_hyperboloid(a, b, d, r2, r1);
			}
			else
			{
				const double r=r2-r1;
				SimplePoint ab=sub_of_points(b, a);
				SimplePoint v=unit_point(ab);
				const double k=(4*r*r/((0-4*r*r+16*d*d)*(0-4*r*r+16*d*d))) * (0-r*r+4*d*d) * 4;
				const double m=(4*d*d-r*r)*k/4;

				const double x0=a.x;
				const double y0=a.y;
				const double z0=a.z;
				const double vx=v.x;
				const double vy=v.y;
				const double vz=v.z;

				const double t1 =  (std::sqrt((k*vy*vy+k*vx*vx)*z0*z0+(-2*k*vy*vz*y0-2*k*vx*vz*x0)*z0+(k*vz*vz-k*k*vx*vx)*y0*y0+2*k*k*vx*vy*x0*y0+(k*vz*vz-k*k*vy*vy)*x0*x0+m*vz*vz-k*m*vy*vy-k*m*vx*vx)-vz*z0+k*vy*y0+k*vx*x0)/(vz*vz-k*vy*vy-k*vx*vx);

				const double t2 = -(std::sqrt((k*vy*vy+k*vx*vx)*z0*z0+(-2*k*vy*vz*y0-2*k*vx*vz*x0)*z0+(k*vz*vz-k*k*vx*vx)*y0*y0+2*k*k*vx*vy*x0*y0+(k*vz*vz-k*k*vy*vy)*x0*x0+m*vz*vz-k*m*vy*vy-k*m*vx*vx)+vz*z0-k*vy*y0-k*vx*x0)/(vz*vz-k*vy*vy-k*vx*vx);

				const SimplePoint tp1=sum_of_points(a, point_and_number_product(v, t1));
				const SimplePoint tp2=sum_of_points(a, point_and_number_product(v, t2));
				if(greater(t1, 0.0) && less(t1, point_module(ab)) && equal(tp1.z, std::sqrt(k*tp1.x*tp1.x+k*tp1.y*tp1.y+m), 0.000001))
				{
					return t1;
				}
				else if(greater(t2, 0.0) && less(t2, point_module(ab)) && equal(tp2.z, std::sqrt(k*tp2.x*tp2.x+k*tp2.y*tp2.y+m), 0.000001))
				{
					return t2;
				}
				else
				{
					return 0.0;
				}
			}
		}
	};

	static void init_contour_from_base_and_axis(
			const UnsignedInt a_id,
			const SimpleSphere& base,
			const SimplePoint& axis,
			const Float length_step,
			Contour& result)
	{
		const Float angle_step=std::max(std::min(length_step/base.r, PIVALUE/FLOATCONST(3.0)), PIVALUE/FLOATCONST(36.0));
		const SimplePoint first_point=point_and_number_product(any_normal_of_vector(axis), base.r);
		result.push_back(ContourPoint(sum_of_points(base.p, first_point), a_id, a_id));
		for(Float rotation_angle=angle_step;rotation_angle<(PIVALUE*FLOATCONST(2.0));rotation_angle+=angle_step)
		{
			result.push_back(ContourPoint(sum_of_points(base.p, rotate_point_around_axis(axis, rotation_angle, first_point)), a_id, a_id));
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
			if((distance_from_point_to_point(it->p, c.p)-c.r)<(distance_from_point_to_point(it->p, a.p)-a.r))
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
					cuts.push_back(contour.insert(it, ContourPoint(sum_of_points(p0, point_and_number_product(unit_point(sub_of_points(p1, p0)), l)), left_it->right_id, it->left_id)));
					cuts_count++;
				}

				if(right_it->left_id!=c_id)
				{
					const SimplePoint& p0=it->p;
					const SimplePoint& p1=right_it->p;
					const double l=HyperboloidBetweenTwoSpheres::intersect_vector_with_hyperboloid(p0, p1, a, c);
					cuts.push_back(contour.insert(right_it, ContourPoint(sum_of_points(p0, point_and_number_product(unit_point(sub_of_points(p1, p0)), l)), it->right_id, right_it->left_id)));
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
						const SimplePoint direction=unit_point(sub_of_points(p1, p0));
						for(int leap=1;leap<leap_distance;leap++)
						{
							SimplePoint p=sum_of_points(p0, point_and_number_product(direction, (leap_size*static_cast<double>(leap))));
							for(int e=0;e<projections;e++)
							{
								p=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p, b, c);
								p=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p, a, c);
								p=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p, a, b);
							}
							contour.insert(jt, ContourPoint(p, c_id, c_id));
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
			if(distance_from_point_to_point(shell.p, it->p)<=shell.r)
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

	static bool collect_points_from_contour(const Contour& contour, std::vector<SimplePoint>& contour_points)
	{
		contour_points.clear();
		if(!contour.empty())
		{
			contour_points.reserve(contour.size());
			for(Contour::const_iterator jt=contour.begin();jt!=contour.end();++jt)
			{
				contour_points.push_back(jt->p);
			}
		}
		return (!contour_points.empty());
	}

	static Float calculate_area_from_contour(const Contour& contour, const SimpleSphere& sphere1, const SimpleSphere& sphere2, std::vector<SimplePoint>& contour_points, SimplePoint& contour_barycenter)
	{
		Float area=0.0;
		if(collect_points_from_contour(contour, contour_points))
		{
			contour_barycenter.x=FLOATCONST(0.0);
			contour_barycenter.y=FLOATCONST(0.0);
			contour_barycenter.z=FLOATCONST(0.0);
			for(UnsignedInt i=0;i<contour_points.size();i++)
			{
				contour_barycenter.x+=contour_points[i].x;
				contour_barycenter.y+=contour_points[i].y;
				contour_barycenter.z+=contour_points[i].z;
			}
			contour_barycenter.x/=static_cast<Float>(contour.size());
			contour_barycenter.y/=static_cast<Float>(contour.size());
			contour_barycenter.z/=static_cast<Float>(contour.size());

			for(UnsignedInt i=0;i<contour_points.size();i++)
			{
				const SimplePoint& pr1=contour_points[i];
				const SimplePoint& pr2=contour_points[(i+1)<contour_points.size() ? (i+1) : 0];
				area+=triangle_area(contour_barycenter, pr1, pr2);
			}
		}
		return area;
	}
};

}

#endif /* VORONOTALT_SIMPLIFIED_AW_TESSELLATION_CONTACT_CONSTRUCTION_H_ */