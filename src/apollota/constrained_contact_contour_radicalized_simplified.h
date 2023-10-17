#ifndef APOLLOTA_CONSTRAINED_CONTACT_CONTOUR_RADICALIZED_SIMPLIFIED_H_
#define APOLLOTA_CONSTRAINED_CONTACT_CONTOUR_RADICALIZED_SIMPLIFIED_H_

#include <vector>
#include <algorithm>

#include "basic_operations_on_spheres.h"
#include "rotation.h"

namespace voronota
{

namespace apollota
{

class ConstrainedContactContourRadicalizedSimplified
{
public:
	struct ContourPoint
	{
		SimplePoint p;
		std::size_t left_id;
		std::size_t right_id;
		int indicator;
		double angle;

		ContourPoint(const SimplePoint& p, const std::size_t left_id, const std::size_t right_id) : p(p), left_id(left_id), right_id(right_id), indicator(0), angle(0.0)
		{
		}
	};

	typedef std::vector<ContourPoint> Contour;

	struct ContactDescriptor
	{
		SimpleSphere intersection_circle_sphere;
		SimplePoint intersection_circle_axis;
		Contour contour;
		SimplePoint contour_barycenter;
		double sum_of_arc_angles;
		double area;
		bool valid;

		ContactDescriptor() : sum_of_arc_angles(0.0), area(0.0), valid(false)
		{
		}

		void clear()
		{
			contour.clear();
			sum_of_arc_angles=0.0;
			area=0.0;
			valid=false;
		}
	};

	static bool construct_contact_descriptor_for_expanded_spheres_without_tessellation(
			const std::vector<SimpleSphere>& spheres,
			const std::size_t a_id,
			const std::size_t b_id,
			const std::vector<std::size_t>& a_neighbor_ids,
			const std::vector<std::size_t>& b_neighbor_ids,
			ContactDescriptor& result_contact_descriptor)
	{
		result_contact_descriptor.clear();
		if(a_id<spheres.size() && b_id<spheres.size())
		{
			const SimpleSphere& a=spheres[a_id];
			const SimpleSphere& b=spheres[b_id];
			if(sphere_intersects_sphere(a, b))
			{
				result_contact_descriptor.intersection_circle_sphere=intersection_circle_of_two_spheres<SimpleSphere>(a, b);
				if(result_contact_descriptor.intersection_circle_sphere.r>0.0)
				{
					bool discarded=false;
					std::vector<NeighborDescriptor> neighbor_descriptors;
					{
						const std::vector<std::size_t>& j_neighbor_ids=(a_neighbor_ids.size()<b_neighbor_ids.size() ? a_neighbor_ids : b_neighbor_ids);
						const SimpleSphere& j_alt_sphere=(a_neighbor_ids.size()<b_neighbor_ids.size() ? b : a);
						neighbor_descriptors.reserve(j_neighbor_ids.size());
						for(std::size_t i=0;i<j_neighbor_ids.size() && !discarded;i++)
						{
							const std::size_t neighbor_id=j_neighbor_ids[i];
							if(neighbor_id<spheres.size() && neighbor_id!=a_id && neighbor_id!=b_id)
							{
								const SimpleSphere& c=spheres[neighbor_id];
								if(sphere_intersects_sphere(result_contact_descriptor.intersection_circle_sphere, c) && sphere_intersects_sphere(j_alt_sphere, c))
								{
									NeighborDescriptor nd(neighbor_id, a, c);
									const double cos_val=dot_product(unit_point<PODPoint>(sub_of_points<PODPoint>(result_contact_descriptor.intersection_circle_sphere, a)), unit_point<PODPoint>(sub_of_points<PODPoint>(nd.ac_plane_center, a)));
									if(cos_val<1.0)
									{
										const double l=std::abs(signed_distance_from_point_to_plane(nd.ac_plane_center, nd.ac_plane_normal, result_contact_descriptor.intersection_circle_sphere));
										const double xl=l/sqrt(1-(cos_val*cos_val));
										const int hsi=halfspace_of_point(nd.ac_plane_center, nd.ac_plane_normal, result_contact_descriptor.intersection_circle_sphere);
										if(xl>=result_contact_descriptor.intersection_circle_sphere.r)
										{
											if(hsi>0)
											{
												discarded=true;
											}
										}
										else
										{
											nd.sort_value=(hsi>0 ? (0.0-xl) : xl);
											neighbor_descriptors.push_back(nd);
										}
									}
								}
							}
						}
					}
					if(!discarded)
					{
						result_contact_descriptor.intersection_circle_axis=sub_of_points<SimplePoint>(b, a).unit();
						if(neighbor_descriptors.empty())
						{
							result_contact_descriptor.contour_barycenter=custom_point_from_object<SimplePoint>(result_contact_descriptor.intersection_circle_sphere);
							result_contact_descriptor.sum_of_arc_angles=2.0*pi_value();
							result_contact_descriptor.area=result_contact_descriptor.intersection_circle_sphere.r*result_contact_descriptor.intersection_circle_sphere.r*pi_value();
							result_contact_descriptor.valid=true;
						}
						else
						{
							init_contour_from_base_and_axis(a_id, result_contact_descriptor.intersection_circle_sphere, result_contact_descriptor.intersection_circle_axis, result_contact_descriptor.contour);
							if(!result_contact_descriptor.contour.empty() && !neighbor_descriptors.empty())
							{
								std::sort(neighbor_descriptors.begin(), neighbor_descriptors.end());
								for(std::size_t i=0;i<neighbor_descriptors.size();i++)
								{
									const NeighborDescriptor& nd=neighbor_descriptors[i];
									mark_and_cut_contour(nd.ac_plane_center, nd.ac_plane_normal, nd.c_id, result_contact_descriptor.contour);
								}
							}
							if(!result_contact_descriptor.contour.empty())
							{
								restrict_contour_to_circle(result_contact_descriptor.intersection_circle_sphere, result_contact_descriptor.intersection_circle_axis, a_id, result_contact_descriptor.contour, result_contact_descriptor.sum_of_arc_angles);
							}
							if(!result_contact_descriptor.contour.empty())
							{
								for(std::size_t i=0;i<result_contact_descriptor.contour.size();i++)
								{
									result_contact_descriptor.contour_barycenter.x+=result_contact_descriptor.contour[i].p.x;
									result_contact_descriptor.contour_barycenter.y+=result_contact_descriptor.contour[i].p.y;
									result_contact_descriptor.contour_barycenter.z+=result_contact_descriptor.contour[i].p.z;
								}
								result_contact_descriptor.contour_barycenter.x/=static_cast<double>(result_contact_descriptor.contour.size());
								result_contact_descriptor.contour_barycenter.y/=static_cast<double>(result_contact_descriptor.contour.size());
								result_contact_descriptor.contour_barycenter.z/=static_cast<double>(result_contact_descriptor.contour.size());

								for(std::size_t i=0;i<result_contact_descriptor.contour.size();i++)
								{
									ContourPoint& pr1=result_contact_descriptor.contour[i];
									ContourPoint& pr2=result_contact_descriptor.contour[(i+1)<result_contact_descriptor.contour.size() ? (i+1) : 0];
									result_contact_descriptor.area+=triangle_area(result_contact_descriptor.contour_barycenter, pr1.p, pr2.p);
									if(pr1.angle>0.0)
									{
										result_contact_descriptor.area+=result_contact_descriptor.intersection_circle_sphere.r*result_contact_descriptor.intersection_circle_sphere.r*(pr1.angle-std::sin(pr1.angle))*0.5;
									}
								}
							}
							result_contact_descriptor.valid=(!result_contact_descriptor.contour.empty());
						}
					}
				}
			}
		}
		return result_contact_descriptor.valid;
	}

	static bool discretize_contact_descriptor_contour(const ContactDescriptor& contact_descriptor, const double length_step, std::vector<SimplePoint>& result_points, SimplePoint& result_barycenter)
	{
		result_points.clear();
		if(contact_descriptor.valid)
		{
			const double angle_step=std::max(std::min(360*(length_step/(2*pi_value()*contact_descriptor.intersection_circle_sphere.r)), 60.0), 5.0);
			Rotation rotation(contact_descriptor.intersection_circle_axis, 0);
			if(contact_descriptor.contour.empty())
			{
				const SimplePoint first_point=point_and_number_product<SimplePoint>(any_normal_of_vector<SimplePoint>(rotation.axis), contact_descriptor.intersection_circle_sphere.r);
				result_points.reserve(static_cast<int>(360.0/angle_step)+2);
				result_points.push_back(sum_of_points<SimplePoint>(contact_descriptor.intersection_circle_sphere, first_point));
				for(rotation.angle=angle_step;rotation.angle<360.0;rotation.angle+=angle_step)
				{
					result_points.push_back(sum_of_points<SimplePoint>(contact_descriptor.intersection_circle_sphere, rotation.rotate<SimplePoint>(first_point)));
				}
				result_barycenter=custom_point_from_object<SimplePoint>(contact_descriptor.intersection_circle_sphere);
			}
			else
			{
				if(contact_descriptor.sum_of_arc_angles>0.0)
				{
					const double sum_of_arc_angles_in_degrees=std::max(0.0, contact_descriptor.sum_of_arc_angles*(180.0/pi_value()));
					result_points.reserve(static_cast<std::size_t>(sum_of_arc_angles_in_degrees/angle_step)+contact_descriptor.contour.size()+4);
				}
				else
				{
					result_points.reserve(contact_descriptor.contour.size());
				}
				for(std::size_t i=0;i<contact_descriptor.contour.size();i++)
				{
					const ContourPoint& pr=contact_descriptor.contour[i];
					result_points.push_back(pr.p);
					if(pr.angle>0.0)
					{
						const double angle_in_degrees=pr.angle*(180.0/pi_value());
						if(angle_in_degrees>angle_step)
						{
							const  SimplePoint first_v=sub_of_points<SimplePoint>(pr.p, contact_descriptor.intersection_circle_sphere);
							for(rotation.angle=angle_step;rotation.angle<angle_in_degrees;rotation.angle+=angle_step)
							{
								result_points.push_back(sum_of_points<SimplePoint>(contact_descriptor.intersection_circle_sphere, rotation.rotate<SimplePoint>(first_v)));
							}
						}
					}
				}
				result_barycenter.x=0.0;
				result_barycenter.y=0.0;
				result_barycenter.z=0.0;
				if(!result_points.empty())
				{
					for(std::size_t i=0;i<result_points.size();i++)
					{
						result_barycenter.x+=result_points[i].x;
						result_barycenter.y+=result_points[i].y;
						result_barycenter.z+=result_points[i].z;
					}
					result_barycenter.x/=static_cast<double>(result_points.size());
					result_barycenter.y/=static_cast<double>(result_points.size());
					result_barycenter.z/=static_cast<double>(result_points.size());
				}
			}
		}
		return (contact_descriptor.valid && !result_points.empty());
	}

private:
	struct NeighborDescriptor
	{
		double sort_value;
		std::size_t c_id;
		SimplePoint ac_plane_center;
		SimplePoint ac_plane_normal;

		NeighborDescriptor(const std::size_t c_id, const SimpleSphere& a, const SimpleSphere& c) : sort_value(0.0), c_id(c_id), ac_plane_center(intersection_circle_of_two_spheres<SimpleSphere>(a, c)), ac_plane_normal(sub_of_points<SimplePoint>(c, a).unit())
		{
		}

		bool operator<(const NeighborDescriptor& d) const
		{
			return (sort_value<d.sort_value || (sort_value==d.sort_value && c_id<d.c_id));
		}
	};

	static void init_contour_from_base_and_axis(
			const std::size_t a_id,
			const SimpleSphere& base,
			const SimplePoint& axis,
			Contour& result)
	{
		Rotation rotation(axis, 0);
		const SimplePoint first_point=point_and_number_product<SimplePoint>(any_normal_of_vector<SimplePoint>(rotation.axis), base.r*1.19);
		const double angle_step=60.0;
		result.reserve(12);
		result.push_back(ContourPoint(sum_of_points<SimplePoint>(base, first_point), a_id, a_id));
		for(rotation.angle=angle_step;rotation.angle<360.0;rotation.angle+=angle_step)
		{
			result.push_back(ContourPoint(sum_of_points<SimplePoint>(base, rotation.rotate<SimplePoint>(first_point)), a_id, a_id));
		}
	}

	static bool mark_and_cut_contour(
			const SimplePoint& ac_plane_center,
			const SimplePoint& ac_plane_normal,
			const std::size_t c_id,
			Contour& contour)
	{
		const std::size_t outsiders_count=mark_contour(ac_plane_center, ac_plane_normal, c_id, contour);
		if(outsiders_count>0)
		{
			if(outsiders_count<contour.size())
			{
				cut_contour(ac_plane_center, ac_plane_normal, c_id, contour);
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
			const SimplePoint& ac_plane_center,
			const SimplePoint& ac_plane_normal,
			const std::size_t c_id,
			Contour& contour)
	{
		std::size_t outsiders_count=0;
		for(Contour::iterator it=contour.begin();it!=contour.end();++it)
		{
			if(halfspace_of_point(ac_plane_center, ac_plane_normal, it->p)>0)
			{
				it->left_id=c_id;
				it->right_id=c_id;
				outsiders_count++;
			}
		}
		return outsiders_count;
	}

	static void cut_contour(
			const SimplePoint& ac_plane_center,
			const SimplePoint& ac_plane_normal,
			const std::size_t c_id,
			Contour& contour)
	{
		if(contour.size()<3)
		{
			return;
		}

		std::size_t i_start=0;
		while(i_start<contour.size() && !(contour[i_start].left_id==c_id && contour[i_start].right_id==c_id))
		{
			i_start++;
		}

		if(i_start>=contour.size())
		{
			return;
		}

		std::size_t i_end=(contour.size()-1);
		while(i_end>0 && !(contour[i_end].left_id==c_id && contour[i_end].right_id==c_id))
		{
			i_end--;
		}

		if(i_start==0 && i_end==(contour.size()-1))
		{
			i_end=0;
			while((i_end+1)<contour.size() && contour[i_end+1].left_id==c_id && contour[i_end+1].right_id==c_id)
			{
				i_end++;
			}

			i_start=(contour.size()-1);
			while(i_start>0 && contour[i_start-1].left_id==c_id && contour[i_start-1].right_id==c_id)
			{
				i_start--;
			}
		}

		if(i_start==i_end)
		{
			contour.insert(contour.begin()+i_start, contour[i_start]);
			i_end=i_start+1;
		}
		else if(i_start<i_end)
		{
			if(i_start+1<i_end)
			{
				contour.erase(contour.begin()+i_start+1, contour.begin()+i_end);
			}
			i_end=i_start+1;
		}
		else if(i_start>i_end)
		{
			if(i_start+1<contour.size())
			{
				contour.erase(contour.begin()+i_start+1, contour.end());
			}
			if(i_end>0)
			{
				contour.erase(contour.begin(), contour.begin()+i_end);

			}
			i_start=contour.size()-1;
			i_end=0;
		}

		{
			const std::size_t i_left=((i_start)>0 ? (i_start-1) : (contour.size()-1));
			contour[i_start]=ContourPoint(intersection_of_plane_and_segment<SimplePoint>(ac_plane_center, ac_plane_normal, contour[i_start].p, contour[i_left].p), contour[i_left].right_id, contour[i_start].left_id);
		}

		{
			const std::size_t i_right=((i_end+1)<contour.size() ? (i_end+1) : 0);
			contour[i_end]=ContourPoint(intersection_of_plane_and_segment<SimplePoint>(ac_plane_center, ac_plane_normal, contour[i_end].p, contour[i_right].p), contour[i_end].right_id, contour[i_right].left_id);
		}
	}

	static bool restrict_contour_to_circle(
			const SimpleSphere& ic_sphere,
			const SimplePoint& ic_axis,
			const std::size_t a_id,
			Contour& contour,
			double& sum_of_arc_angles)
	{
		std::size_t outsiders_count=0;
		for(std::size_t i=0;i<contour.size();i++)
		{
			if(squared_distance_from_point_to_point(contour[i].p, ic_sphere)<=(ic_sphere.r*ic_sphere.r))
			{
				contour[i].indicator=0;
			}
			else
			{
				contour[i].indicator=1;
				outsiders_count++;
			}
		}

		if(outsiders_count>0)
		{
			std::size_t insertions_count=0;
			{
				std::size_t i=0;
				while(i<contour.size())
				{
					ContourPoint& pr1=contour[i];
					ContourPoint& pr2=contour[(i+1)<contour.size() ? (i+1) : 0];
					if(pr1.indicator==1 || pr2.indicator==1)
					{
						if(pr1.indicator==1 && pr2.indicator==1)
						{
							SimplePoint mp;
							if(project_point_inside_line(SimplePoint(ic_sphere), pr1.p, pr2.p, mp))
							{
								if(squared_distance_from_point_to_point(mp, ic_sphere)<=(ic_sphere.r*ic_sphere.r))
								{
									SimplePoint ip1;
									SimplePoint ip2;
									if(intersect_seqment_with_circle(ic_sphere, mp, pr1.p, ip1) && intersect_seqment_with_circle(ic_sphere, mp, pr2.p, ip2))
									{
										const std::size_t pr2_left_id=pr2.left_id;
										contour.insert(((i+1)<contour.size() ? (contour.begin()+(i+1)) : contour.end()), 2, ContourPoint(ip1, a_id, pr1.right_id));
										contour[i+2]=ContourPoint(ip2, pr2_left_id, a_id);
										insertions_count+=2;
										i+=2;
									}
								}
							}
						}
						else
						{
							if(pr1.indicator==1 && pr2.indicator!=1)
							{
								SimplePoint ip;
								if(intersect_seqment_with_circle(ic_sphere, pr2.p, pr1.p, ip))
								{
									contour.insert(((i+1)<contour.size() ? (contour.begin()+(i+1)) : contour.end()), ContourPoint(ip, a_id, pr1.right_id));
									insertions_count++;
									i++;
								}
								else
								{
									pr2.left_id=a_id;
									pr2.right_id=pr1.right_id;
								}
							}
							else if(pr1.indicator!=1 && pr2.indicator==1)
							{
								SimplePoint ip;
								if(intersect_seqment_with_circle(ic_sphere, pr1.p, pr2.p, ip))
								{
									contour.insert(((i+1)<contour.size() ? (contour.begin()+(i+1)) : contour.end()), ContourPoint(ip, pr2.left_id, a_id));
									insertions_count++;
									i++;
								}
								else
								{
									pr1.left_id=pr2.left_id;
									pr1.right_id=a_id;
								}
							}
						}
					}
					i++;
				}
			}
			if(insertions_count==0)
			{
				contour.clear();
			}
			else if(!contour.empty())
			{
				{
					std::size_t i=(contour.size()-1);
					while(i<contour.size())
					{
						if(contour[i].indicator==1)
						{
							contour.erase(contour.begin()+i);
						}
						i=(i>0 ? (i-1) : contour.size());
					}
				}
				if(contour.size()<2)
				{
					contour.clear();
				}
				{
					std::size_t i=0;
					while(i<contour.size())
					{
						ContourPoint& pr1=contour[i];
						ContourPoint& pr2=contour[(i+1)<contour.size() ? (i+1) : 0];
						if(pr1.right_id==a_id && pr2.left_id==a_id)
						{
							pr1.angle=directed_angle(ic_sphere, pr1.p, pr2.p, sum_of_points<SimplePoint>(ic_sphere, ic_axis));
							sum_of_arc_angles+=pr1.angle;
							pr1.indicator=2;
							pr2.indicator=3;
						}
						i++;
					}
				}
			}
		}

		return (outsiders_count>0);
	}

	static bool project_point_inside_line(const SimplePoint& o, const SimplePoint& a, const SimplePoint& b, SimplePoint& result)
	{
		const SimplePoint v=unit_point<SimplePoint>(sub_of_points<SimplePoint>(b, a));
		const double l=dot_product(v, sub_of_points<SimplePoint>(o, a));
		if(l>0.0 && (l*l)<=squared_distance_from_point_to_point(a, b))
		{
			result=sum_of_points<SimplePoint>(a, point_and_number_product<SimplePoint>(v, l));
			return true;
		}
		return false;
	}

	static bool intersect_seqment_with_circle(const SimpleSphere& circle, const SimplePoint& p_in, const SimplePoint& p_out, SimplePoint& result)
	{
		const double dist_in_to_out=distance_from_point_to_point(p_in, p_out);
		if(dist_in_to_out>0.0)
		{
			const SimplePoint v=point_and_number_product<SimplePoint>(sub_of_points<SimplePoint>(p_in, p_out), 1.0/dist_in_to_out);
			const SimplePoint u=sub_of_points<SimplePoint>(circle, p_out);
			const SimplePoint s=sum_of_points<SimplePoint>(p_out, point_and_number_product<SimplePoint>(v, dot_product(v, u)));
			const double ll=(circle.r*circle.r)-squared_distance_from_point_to_point(circle, s);
			if(ll>=0.0)
			{
				result=sum_of_points<SimplePoint>(s, point_and_number_product<SimplePoint>(v, 0.0-sqrt(ll)));
				return true;
			}
		}
		return false;
	}
};

}

}

#endif /* APOLLOTA_CONSTRAINED_CONTACT_CONTOUR_RADICALIZED_SIMPLIFIED_H_ */

