#ifndef VORONOTALT_TESSELLATION_CONTACT_CONSTRUCTION_H_
#define VORONOTALT_TESSELLATION_CONTACT_CONSTRUCTION_H_

#include <vector>
#include <algorithm>

#include "basic_types_and_functions.h"

namespace voronotalt
{

struct SpheresCollisionDescriptor
{
	std::size_t id_a;
	std::size_t id_b;
	SimpleSphere intersection_circle_sphere;
	SimplePoint intersection_circle_axis;
	bool valid;

	SpheresCollisionDescriptor() : id_a(0), id_b(0), valid(false)
	{
	}

	bool set(const std::vector<SimpleSphere>& spheres, const std::size_t a_id, const std::size_t b_id)
	{
		if(a_id<spheres.size() && b_id<spheres.size() && a_id!=b_id)
		{
			const SimpleSphere& a=spheres[a_id];
			const SimpleSphere& b=spheres[b_id];
			if(sphere_intersects_sphere(a, b) && !sphere_contains_sphere(a, b) && !sphere_contains_sphere(b, a))
			{
				id_a=a_id;
				id_b=b_id;
				intersection_circle_sphere=intersection_circle_of_two_spheres(a, b);
				intersection_circle_axis=unit_point(sub_of_points(b.p, a.p));
				valid=true;
			}
		}
		return valid;
	}
};

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

struct ContactDescriptor
{
	SimpleSphere intersection_circle_sphere;
	SimplePoint intersection_circle_axis;
	std::vector<NeighborDescriptor> neighbor_descriptors;
	Contour contour;
	SimplePoint contour_barycenter;
	std::size_t id_a;
	std::size_t id_b;
	double sum_of_arc_angles;
	double area;
	double solid_angle_a;
	double solid_angle_b;
	double pyramid_volume_a;
	double pyramid_volume_b;
	bool valid;

	ContactDescriptor() : id_a(0), id_b(0), sum_of_arc_angles(0.0), area(0.0), solid_angle_a(0.0), solid_angle_b(0.0), pyramid_volume_a(0.0), pyramid_volume_b(0.0), valid(false)
	{
	}

	void clear()
	{
		id_a=0;
		id_b=0;
		neighbor_descriptors.clear();
		contour.clear();
		sum_of_arc_angles=0.0;
		area=0.0;
		solid_angle_a=0.0;
		solid_angle_b=0.0;
		pyramid_volume_a=0.0;
		pyramid_volume_b=0.0;
		valid=false;
	}
};

struct ContactDescriptorsGraphics
{
	std::vector<SimplePoint> outer_points;
	SimplePoint barycenter;
	SimplePoint normal;
	bool valid;

	ContactDescriptorsGraphics() : valid(false)
	{
	}

	void clear()
	{
		outer_points.clear();
		valid=false;
	}
};

class TessellationContactsConstruction
{
public:
	static bool construct_contact_descriptor(
			const std::vector<SimpleSphere>& spheres,
			const SpheresCollisionDescriptor& collision,
			const std::vector<SpheresCollisionDescriptor>& a_neighbor_collisions,
			ContactDescriptor& result_contact_descriptor)
	{
		result_contact_descriptor.clear();
		if(collision.valid)
		{
			result_contact_descriptor.id_a=collision.id_a;
			result_contact_descriptor.id_b=collision.id_b;
			const std::size_t a_id=collision.id_a;
			const std::size_t b_id=collision.id_b;
			const SimpleSphere& a=spheres[a_id];
			const SimpleSphere& b=spheres[b_id];
			{
				result_contact_descriptor.intersection_circle_sphere=collision.intersection_circle_sphere;
				if(result_contact_descriptor.intersection_circle_sphere.r>0.0)
				{
					bool discarded=false;
					{
						result_contact_descriptor.neighbor_descriptors.reserve(a_neighbor_collisions.size());
						for(std::size_t i=0;i<a_neighbor_collisions.size() && !discarded;i++)
						{
							const SpheresCollisionDescriptor& neighbor_collision=a_neighbor_collisions[i];
							if(neighbor_collision.id_b!=b_id)
							{
								const SimpleSphere& c=spheres[neighbor_collision.id_b];
								if(sphere_intersects_sphere(result_contact_descriptor.intersection_circle_sphere, c) && sphere_intersects_sphere(b, c))
								{
									if(sphere_contains_sphere(c, a) || sphere_contains_sphere(c, b))
									{
										discarded=true;
									}
									else
									{
										const double cos_val=dot_product(unit_point(sub_of_points(result_contact_descriptor.intersection_circle_sphere.p, a.p)), unit_point(sub_of_points(neighbor_collision.intersection_circle_sphere.p, a.p)));
										const int hsi=halfspace_of_point(neighbor_collision.intersection_circle_sphere.p, neighbor_collision.intersection_circle_axis, result_contact_descriptor.intersection_circle_sphere.p);
										if(std::abs(cos_val)<1.0)
										{
											const double l=std::abs(signed_distance_from_point_to_plane(neighbor_collision.intersection_circle_sphere.p, neighbor_collision.intersection_circle_axis, result_contact_descriptor.intersection_circle_sphere.p));
											const double xl=l/std::sqrt(1-(cos_val*cos_val));
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
												nd.neighbor_id=i;
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

						if(result_contact_descriptor.neighbor_descriptors.empty())
						{
							result_contact_descriptor.contour_barycenter=result_contact_descriptor.intersection_circle_sphere.p;
							result_contact_descriptor.sum_of_arc_angles=2.0*pi_value();
							result_contact_descriptor.area=result_contact_descriptor.intersection_circle_sphere.r*result_contact_descriptor.intersection_circle_sphere.r*pi_value();
							result_contact_descriptor.valid=true;
						}
						else
						{
							init_contour_from_base_and_axis(a_id, result_contact_descriptor.intersection_circle_sphere, result_contact_descriptor.intersection_circle_axis, result_contact_descriptor.contour);
							if(!result_contact_descriptor.contour.empty() && !result_contact_descriptor.neighbor_descriptors.empty())
							{
								std::sort(result_contact_descriptor.neighbor_descriptors.begin(), result_contact_descriptor.neighbor_descriptors.end());
								for(std::size_t i=0;i<result_contact_descriptor.neighbor_descriptors.size();i++)
								{
									const NeighborDescriptor& nd=result_contact_descriptor.neighbor_descriptors[i];
									const SpheresCollisionDescriptor& neighbor_collision=a_neighbor_collisions[nd.neighbor_id];
									mark_and_cut_contour(neighbor_collision.intersection_circle_sphere.p, neighbor_collision.intersection_circle_axis, neighbor_collision.id_b, result_contact_descriptor.contour);
								}
							}
							if(!result_contact_descriptor.contour.empty())
							{
								restrict_contour_to_circle(result_contact_descriptor.intersection_circle_sphere, result_contact_descriptor.intersection_circle_axis, a_id, result_contact_descriptor.contour, result_contact_descriptor.sum_of_arc_angles);
								if(!result_contact_descriptor.contour.empty())
								{
									result_contact_descriptor.area=calculate_contour_area(result_contact_descriptor.intersection_circle_sphere, result_contact_descriptor.contour, result_contact_descriptor.contour_barycenter);
									result_contact_descriptor.valid=true;
								}
							}
						}

						if(result_contact_descriptor.valid)
						{
							result_contact_descriptor.solid_angle_a=calculate_contour_solid_angle(a, b, result_contact_descriptor.intersection_circle_sphere, result_contact_descriptor.contour);
							result_contact_descriptor.solid_angle_b=calculate_contour_solid_angle(b, a, result_contact_descriptor.intersection_circle_sphere, result_contact_descriptor.contour);
							result_contact_descriptor.pyramid_volume_a=distance_from_point_to_point(a.p, result_contact_descriptor.intersection_circle_sphere.p)*result_contact_descriptor.area/3.0*(result_contact_descriptor.solid_angle_a<0.0 ? -1.0 : 1.0);
							result_contact_descriptor.pyramid_volume_b=distance_from_point_to_point(b.p, result_contact_descriptor.intersection_circle_sphere.p)*result_contact_descriptor.area/3.0*(result_contact_descriptor.solid_angle_b<0.0 ? -1.0 : 1.0);
						}
					}
				}
			}
		}
		return result_contact_descriptor.valid;
	}

	static bool construct_contact_descriptor_graphics(const ContactDescriptor& contact_descriptor, const double length_step, ContactDescriptorsGraphics& result_contact_descriptor_graphics)
	{
		result_contact_descriptor_graphics.clear();
		if(contact_descriptor.valid)
		{
			const double angle_step=std::max(std::min(length_step/contact_descriptor.intersection_circle_sphere.r, pi_value()/3.0), pi_value()/36.0);
			if(contact_descriptor.contour.empty())
			{
				const SimplePoint first_point=point_and_number_product(any_normal_of_vector(contact_descriptor.intersection_circle_axis), contact_descriptor.intersection_circle_sphere.r);
				result_contact_descriptor_graphics.outer_points.reserve(static_cast<int>((2.0*pi_value())/angle_step)+2);
				result_contact_descriptor_graphics.outer_points.push_back(sum_of_points(contact_descriptor.intersection_circle_sphere.p, first_point));
				for(double rotation_angle=angle_step;rotation_angle<(2.0*pi_value());rotation_angle+=angle_step)
				{
					result_contact_descriptor_graphics.outer_points.push_back(sum_of_points(contact_descriptor.intersection_circle_sphere.p, rotate_point_around_axis(contact_descriptor.intersection_circle_axis, rotation_angle, first_point)));
				}
				result_contact_descriptor_graphics.barycenter=contact_descriptor.intersection_circle_sphere.p;
			}
			else
			{
				if(contact_descriptor.sum_of_arc_angles>0.0)
				{
					result_contact_descriptor_graphics.outer_points.reserve(static_cast<std::size_t>(contact_descriptor.sum_of_arc_angles/angle_step)+contact_descriptor.contour.size()+4);
				}
				else
				{
					result_contact_descriptor_graphics.outer_points.reserve(contact_descriptor.contour.size());
				}
				for(std::size_t i=0;i<contact_descriptor.contour.size();i++)
				{
					const ContourPoint& pr=contact_descriptor.contour[i];
					result_contact_descriptor_graphics.outer_points.push_back(pr.p);
					if(pr.angle>0.0)
					{
						if(pr.angle>angle_step)
						{
							const SimplePoint first_v=sub_of_points(pr.p, contact_descriptor.intersection_circle_sphere.p);
							for(double rotation_angle=angle_step;rotation_angle<pr.angle;rotation_angle+=angle_step)
							{
								result_contact_descriptor_graphics.outer_points.push_back(sum_of_points(contact_descriptor.intersection_circle_sphere.p, rotate_point_around_axis(contact_descriptor.intersection_circle_axis, rotation_angle, first_v)));
							}
						}
					}
				}
				result_contact_descriptor_graphics.barycenter.x=0.0;
				result_contact_descriptor_graphics.barycenter.y=0.0;
				result_contact_descriptor_graphics.barycenter.z=0.0;
				if(!result_contact_descriptor_graphics.outer_points.empty())
				{
					for(std::size_t i=0;i<result_contact_descriptor_graphics.outer_points.size();i++)
					{
						result_contact_descriptor_graphics.barycenter.x+=result_contact_descriptor_graphics.outer_points[i].x;
						result_contact_descriptor_graphics.barycenter.y+=result_contact_descriptor_graphics.outer_points[i].y;
						result_contact_descriptor_graphics.barycenter.z+=result_contact_descriptor_graphics.outer_points[i].z;
					}
					result_contact_descriptor_graphics.barycenter.x/=static_cast<double>(result_contact_descriptor_graphics.outer_points.size());
					result_contact_descriptor_graphics.barycenter.y/=static_cast<double>(result_contact_descriptor_graphics.outer_points.size());
					result_contact_descriptor_graphics.barycenter.z/=static_cast<double>(result_contact_descriptor_graphics.outer_points.size());
				}
			}
			result_contact_descriptor_graphics.normal=contact_descriptor.intersection_circle_axis;
			result_contact_descriptor_graphics.valid=!result_contact_descriptor_graphics.outer_points.empty();
		}
		return result_contact_descriptor_graphics.valid;
	}

private:
	static void init_contour_from_base_and_axis(
			const std::size_t a_id,
			const SimpleSphere& base,
			const SimplePoint& axis,
			Contour& result)
	{
		const SimplePoint first_point=point_and_number_product(any_normal_of_vector(axis), base.r*1.19);
		const double angle_step=pi_value()/3.0;
		result.reserve(12);
		result.push_back(ContourPoint(sum_of_points(base.p, first_point), a_id, a_id));
		for(double rotation_angle=angle_step;rotation_angle<(2.0*pi_value());rotation_angle+=angle_step)
		{
			result.push_back(ContourPoint(sum_of_points(base.p, rotate_point_around_axis(axis, rotation_angle, first_point)), a_id, a_id));
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
			if(halfspace_of_point(ac_plane_center, ac_plane_normal, it->p)>=0)
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
			contour[i_start]=ContourPoint(intersection_of_plane_and_segment(ac_plane_center, ac_plane_normal, contour[i_start].p, contour[i_left].p), contour[i_left].right_id, contour[i_start].left_id);
		}

		{
			const std::size_t i_right=((i_end+1)<contour.size() ? (i_end+1) : 0);
			contour[i_end]=ContourPoint(intersection_of_plane_and_segment(ac_plane_center, ac_plane_normal, contour[i_end].p, contour[i_right].p), contour[i_end].right_id, contour[i_right].left_id);
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
			if(squared_distance_from_point_to_point(contour[i].p, ic_sphere.p)<=(ic_sphere.r*ic_sphere.r))
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
							if(project_point_inside_line(ic_sphere.p, pr1.p, pr2.p, mp))
							{
								if(squared_distance_from_point_to_point(mp, ic_sphere.p)<=(ic_sphere.r*ic_sphere.r))
								{
									SimplePoint ip1;
									SimplePoint ip2;
									if(intersect_segment_with_circle(ic_sphere, mp, pr1.p, ip1) && intersect_segment_with_circle(ic_sphere, mp, pr2.p, ip2))
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
								if(intersect_segment_with_circle(ic_sphere, pr2.p, pr1.p, ip))
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
								if(intersect_segment_with_circle(ic_sphere, pr1.p, pr2.p, ip))
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
							pr1.angle=directed_angle(ic_sphere.p, pr1.p, pr2.p, sum_of_points(ic_sphere.p, ic_axis));
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

	static double calculate_contour_area(const SimpleSphere& ic_sphere, const Contour& contour, SimplePoint& contour_barycenter)
	{
		double area=0.0;

		contour_barycenter.x=0.0;
		contour_barycenter.y=0.0;
		contour_barycenter.z=0.0;
		for(std::size_t i=0;i<contour.size();i++)
		{
			contour_barycenter.x+=contour[i].p.x;
			contour_barycenter.y+=contour[i].p.y;
			contour_barycenter.z+=contour[i].p.z;
		}
		contour_barycenter.x/=static_cast<double>(contour.size());
		contour_barycenter.y/=static_cast<double>(contour.size());
		contour_barycenter.z/=static_cast<double>(contour.size());

		for(std::size_t i=0;i<contour.size();i++)
		{
			const ContourPoint& pr1=contour[i];
			const ContourPoint& pr2=contour[(i+1)<contour.size() ? (i+1) : 0];
			area+=triangle_area(contour_barycenter, pr1.p, pr2.p);
			if(pr1.angle>0.0)
			{
				area+=ic_sphere.r*ic_sphere.r*(pr1.angle-std::sin(pr1.angle))*0.5;
			}
		}

		return area;
	}

	static double calculate_contour_solid_angle(const SimpleSphere& a, const SimpleSphere& b, const SimpleSphere& ic_sphere, const Contour& contour)
	{
		double turn_angle=0.0;

		if(!contour.empty())
		{
			for(std::size_t i=0;i<contour.size();i++)
			{
				const ContourPoint& pr0=contour[(i>0) ? (i-1) : (contour.size()-1)];
				const ContourPoint& pr1=contour[i];
				const ContourPoint& pr2=contour[(i+1)<contour.size() ? (i+1) : 0];

				if(pr0.angle>0.0)
				{
					SimplePoint d=cross_product(sub_of_points(b.p, a.p), sub_of_points(pr1.p, ic_sphere.p));
					if((pr0.angle<pi_value() && dot_product(d, sub_of_points(pr0.p, pr1.p))<0.0) || (pr0.angle>pi_value() && dot_product(d, sub_of_points(pr0.p, pr1.p))>0.0))
					{
						d=point_and_number_product(d, -1.0);
					}
					turn_angle+=(pi_value()-min_dihedral_angle(a.p, pr1.p, sum_of_points(pr1.p, d), pr2.p));
				}
				else if(pr1.angle>0.0)
				{
					SimplePoint d=cross_product(sub_of_points(b.p, a.p), sub_of_points(pr1.p, ic_sphere.p));
					if((pr1.angle<pi_value() && dot_product(d, sub_of_points(pr2.p, pr1.p))<0.0) || (pr1.angle>pi_value() && dot_product(d, sub_of_points(pr2.p, pr1.p))>0.0))
					{
						d=point_and_number_product(d, -1.0);
					}
					turn_angle+=(pi_value()-min_dihedral_angle(a.p, pr1.p, pr0.p, sum_of_points(pr1.p, d)));
					turn_angle+=pr1.angle*(distance_from_point_to_point(a.p, ic_sphere.p)/a.r);
				}
				else
				{
					turn_angle+=(pi_value()-min_dihedral_angle(a.p, pr1.p, pr0.p, pr2.p));
				}
			}
		}
		else
		{
			turn_angle=(2.0*pi_value())*(distance_from_point_to_point(a.p, ic_sphere.p)/a.r);
		}

		double solid_angle=((2.0*pi_value())-turn_angle);

		if(dot_product(sub_of_points(ic_sphere.p, a.p), sub_of_points(ic_sphere.p, b.p))>0.0 && squared_distance_from_point_to_point(ic_sphere.p, a.p)<squared_distance_from_point_to_point(ic_sphere.p, b.p))
		{
			solid_angle=(0.0-solid_angle);
		}

		return solid_angle;
	}
};

}

#endif /* VORONOTALT_TESSELLATION_CONTACT_CONSTRUCTION_H_ */
