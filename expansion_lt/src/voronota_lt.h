#ifndef VORONOTA_LT_H_
#define VORONOTA_LT_H_

#include <vector>
#include <cmath>
#include <algorithm>

namespace voronotalt
{

inline double default_comparison_epsilon()
{
	static const double e=0.0000000001;
	return e;
}

inline bool equal(const double a, const double b, const double e)
{
	return (((a-b)<=e) && ((b-a)<=e));
}

inline bool equal(const double a, const double b)
{
	return equal(a, b, default_comparison_epsilon());
}

inline bool less(const double a, const double b)
{
	return ((a+default_comparison_epsilon())<b);
}

inline bool greater(const double a, const double b)
{
	return ((a-default_comparison_epsilon())>b);
}

inline bool less_or_equal(const double a, const double b)
{
	return (less(a, b) || equal(a, b));
}

inline bool greater_or_equal(const double a, const double b)
{
	return (greater(a, b) || equal(a, b));
}

struct SimplePoint
{
	double x;
	double y;
	double z;

	SimplePoint() : x(0.0), y(0.0), z(0.0)
	{
	}

	SimplePoint(const double x, const double y, const double z) : x(x), y(y), z(z)
	{
	}
};

struct SimpleSphere
{
	SimplePoint p;
	double r;

	SimpleSphere() : r(0.0)
	{
	}

	SimpleSphere(const SimplePoint& p, const double r) : p(p), r(r)
	{
	}
};

struct SimpleQuaternion
{
	double a;
	double b;
	double c;
	double d;

	SimpleQuaternion(const double a, const double b, const double c, const double d) : a(a), b(b), c(c), d(d)
	{
	}

	SimpleQuaternion(const double a, const SimplePoint& p) : a(a), b(p.x), c(p.y), d(p.z)
	{
	}
};

inline double squared_distance_from_point_to_point(const SimplePoint& a, const SimplePoint& b)
{
	const double dx=(a.x-b.x);
	const double dy=(a.y-b.y);
	const double dz=(a.z-b.z);
	return (dx*dx+dy*dy+dz*dz);
}

inline double distance_from_point_to_point(const SimplePoint& a, const SimplePoint& b)
{
	return sqrt(squared_distance_from_point_to_point(a, b));
}

inline double squared_point_module(const SimplePoint& a)
{
	return (a.x*a.x+a.y*a.y+a.z*a.z);
}

inline double point_module(const SimplePoint& a)
{
	return sqrt(squared_point_module(a));
}

inline double dot_product(const SimplePoint& a, const SimplePoint& b)
{
	return (a.x*b.x+a.y*b.y+a.z*b.z);
}

inline SimplePoint cross_product(const SimplePoint& a, const SimplePoint& b)
{
	return SimplePoint(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
}

inline SimplePoint point_and_number_product(const SimplePoint& a, const double k)
{
	return SimplePoint(a.x*k, a.y*k, a.z*k);
}

inline SimplePoint unit_point(const SimplePoint& a)
{
	return ((equal(squared_point_module(a), 1.0)) ? a : point_and_number_product(a, 1.0/point_module(a)));
}

inline SimplePoint sum_of_points(const SimplePoint& a, const SimplePoint& b)
{
	return SimplePoint(a.x+b.x, a.y+b.y, a.z+b.z);
}

inline SimplePoint sub_of_points(const SimplePoint& a, const SimplePoint& b)
{
	return SimplePoint(a.x-b.x, a.y-b.y, a.z-b.z);
}

inline double signed_distance_from_point_to_plane(const SimplePoint& plane_point, const SimplePoint& plane_normal, const SimplePoint& x)
{
	return dot_product(unit_point(plane_normal), sub_of_points(x, plane_point));
}

inline int halfspace_of_point(const SimplePoint& plane_point, const SimplePoint& plane_normal, const SimplePoint& x)
{
	const double sd=signed_distance_from_point_to_plane(plane_point, plane_normal, x);
	if(greater(sd, 0.0))
	{
		return 1;
	}
	else if(less(sd, 0.0))
	{
		return -1;
	}
	return 0;
}

inline SimplePoint intersection_of_plane_and_segment(const SimplePoint& plane_point, const SimplePoint& plane_normal, const SimplePoint& a, const SimplePoint& b)
{
	const double da=signed_distance_from_point_to_plane(plane_point, plane_normal, a);
	const double db=signed_distance_from_point_to_plane(plane_point, plane_normal, b);
	if((da-db)==0)
	{
		return a;
	}
	else
	{
		const double t=da/(da-db);
		return sum_of_points(a, point_and_number_product(sub_of_points(b, a), t));
	}
}

inline double triangle_area(const SimplePoint& a, const SimplePoint& b, const SimplePoint& c)
{
	return (point_module(cross_product(sub_of_points(b, a), sub_of_points(c, a)))/2.0);
}

inline double min_angle(const SimplePoint& o, const SimplePoint& a, const SimplePoint& b)
{
	double cos_val=dot_product(unit_point(sub_of_points(a, o)), unit_point(sub_of_points(b, o)));
	if(cos_val<-1.0)
	{
		cos_val=-1.0;
	}
	else if(cos_val>1.0)
	{
		cos_val=1.0;
	}
	return std::acos(cos_val);
}

inline double pi_value()
{
	static const double pi=std::acos(-1.0);
	return pi;
}

inline double directed_angle(const SimplePoint& o, const SimplePoint& a, const SimplePoint& b, const SimplePoint& c)
{
	const double angle=min_angle(o, a, b);
	const SimplePoint n=cross_product(unit_point(sub_of_points(a, o)), unit_point(sub_of_points(b, o)));
	if(dot_product(sub_of_points(c, o), n)>=0)
	{
		return angle;
	}
	else
	{
		return (pi_value()*2-angle);
	}
}

SimplePoint any_normal_of_vector(const SimplePoint& a)
{
	SimplePoint b=a;
	if(!equal(b.x, 0.0) && (!equal(b.y, 0.0) || !equal(b.z, 0.0)))
	{
		b.x=0.0-b.x;
		return unit_point(cross_product(a, b));
	}
	else if(!equal(b.y, 0.0) && (!equal(b.x, 0.0) || !equal(b.z, 0.0)))
	{
		b.y=0.0-b.y;
		return unit_point(cross_product(a, b));
	}
	else if(!equal(b.x, 0.0))
	{
		return SimplePoint(0.0, 1.0, 0.0);
	}
	else
	{
		return SimplePoint(1.0, 0.0, 0.0);
	}
}

inline bool sphere_intersects_sphere(const SimpleSphere& a, const SimpleSphere& b)
{
	return less(squared_distance_from_point_to_point(a.p, b.p), (a.r+b.r)*(a.r+b.r));
}

inline bool sphere_contains_sphere(const SimpleSphere& a, const SimpleSphere& b)
{
	return (greater_or_equal(a.r, b.r) && less_or_equal(squared_distance_from_point_to_point(a.p, b.p), (a.r-b.r)*(a.r-b.r)));
}

inline SimplePoint center_of_intersection_circle_of_two_spheres(const SimpleSphere& a, const SimpleSphere& b)
{
	const SimplePoint cv=sub_of_points(b.p, a.p);
	const double cm=point_module(cv);
	const double cos_g=(a.r*a.r+cm*cm-b.r*b.r)/(2*a.r*cm);
	return sum_of_points(a.p, point_and_number_product(cv, a.r*cos_g/cm));
}

inline SimpleSphere intersection_circle_of_two_spheres(const SimpleSphere& a, const SimpleSphere& b)
{
	const SimplePoint cv=sub_of_points(b.p, a.p);
	const double cm=point_module(cv);
	const double cos_g=(a.r*a.r+cm*cm-b.r*b.r)/(2*a.r*cm);
	const double sin_g=std::sqrt(1-cos_g*cos_g);
	return SimpleSphere(sum_of_points(a.p, point_and_number_product(cv, a.r*cos_g/cm)), a.r*sin_g);
}

inline bool project_point_inside_line(const SimplePoint& o, const SimplePoint& a, const SimplePoint& b, SimplePoint& result)
{
	const SimplePoint v=unit_point(sub_of_points(b, a));
	const double l=dot_product(v, sub_of_points(o, a));
	if(l>0.0 && (l*l)<=squared_distance_from_point_to_point(a, b))
	{
		result=sum_of_points(a, point_and_number_product(v, l));
		return true;
	}
	return false;
}

inline bool intersect_segment_with_circle(const SimpleSphere& circle, const SimplePoint& p_in, const SimplePoint& p_out, SimplePoint& result)
{
	const double dist_in_to_out=distance_from_point_to_point(p_in, p_out);
	if(dist_in_to_out>0.0)
	{
		const SimplePoint v=point_and_number_product(sub_of_points(p_in, p_out), 1.0/dist_in_to_out);
		const SimplePoint u=sub_of_points(circle.p, p_out);
		const SimplePoint s=sum_of_points(p_out, point_and_number_product(v, dot_product(v, u)));
		const double ll=(circle.r*circle.r)-squared_distance_from_point_to_point(circle.p, s);
		if(ll>=0.0)
		{
			result=sum_of_points(s, point_and_number_product(v, 0.0-std::sqrt(ll)));
			return true;
		}
	}
	return false;
}

inline double min_dihedral_angle(const SimplePoint& o, const SimplePoint& a, const SimplePoint& b1, const SimplePoint& b2)
{
	const SimplePoint oa=unit_point(sub_of_points(a, o));
	const SimplePoint d1=sub_of_points(b1, sum_of_points(o, point_and_number_product(oa, dot_product(oa, sub_of_points(b1, o)))));
	const SimplePoint d2=sub_of_points(b2, sum_of_points(o, point_and_number_product(oa, dot_product(oa, sub_of_points(b2, o)))));
	const double cos_val=dot_product(unit_point(d1), unit_point(d2));
	return std::acos(std::max(-1.0, std::min(cos_val, 1.0)));
}

inline SimpleQuaternion quaternion_product(const SimpleQuaternion& q1, const SimpleQuaternion& q2)
{
	return SimpleQuaternion(
			q1.a*q2.a - q1.b*q2.b - q1.c*q2.c - q1.d*q2.d,
			q1.a*q2.b + q1.b*q2.a + q1.c*q2.d - q1.d*q2.c,
			q1.a*q2.c - q1.b*q2.d + q1.c*q2.a + q1.d*q2.b,
			q1.a*q2.d + q1.b*q2.c - q1.c*q2.b + q1.d*q2.a);
}

inline SimpleQuaternion inverted_quaternion(const SimpleQuaternion& q)
{
	return SimpleQuaternion(q.a, 0-q.b, 0-q.c, 0-q.d);
}

inline SimplePoint rotate_point_around_axis(const SimplePoint axis, const double angle, const SimplePoint& p)
{
	if(squared_point_module(axis)>0)
	{
		const double radians_angle_half=(angle*0.5);
		const SimpleQuaternion q1=SimpleQuaternion(std::cos(radians_angle_half), point_and_number_product(unit_point(axis), std::sin(radians_angle_half)));
		const SimpleQuaternion q2=SimpleQuaternion(0.0, p);
		const SimpleQuaternion q3=quaternion_product(quaternion_product(q1, q2), inverted_quaternion(q1));
		return SimplePoint(q3.b, q3.c, q3.d);
	}
	else
	{
		return p;
	}
}

class SpheresSearcher
{
public:
	SpheresSearcher(const std::vector<SimpleSphere>& spheres) : spheres_(spheres), box_size_(1.0)
	{
		for(std::size_t i=0;i<spheres_.size();i++)
		{
			const SimpleSphere& s=spheres_[i];
			box_size_=std::max(box_size_, s.r*2.0+0.25);
		}

		for(std::size_t i=0;i<spheres_.size();i++)
		{
			const GridPoint gp(spheres_[i], box_size_);
			if(i==0)
			{
				grid_offset_=gp;
				grid_size_=gp;
			}
			else
			{
				grid_offset_.x=std::min(grid_offset_.x, gp.x);
				grid_offset_.y=std::min(grid_offset_.y, gp.y);
				grid_offset_.z=std::min(grid_offset_.z, gp.z);
				grid_size_.x=std::max(grid_size_.x, gp.x);
				grid_size_.y=std::max(grid_size_.y, gp.y);
				grid_size_.z=std::max(grid_size_.z, gp.z);
			}
		}

		grid_size_.x=grid_size_.x-grid_offset_.x+1;
		grid_size_.y=grid_size_.y-grid_offset_.y+1;
		grid_size_.z=grid_size_.z-grid_offset_.z+1;

		map_of_boxes_.resize(grid_size_.x*grid_size_.y*grid_size_.z, -1);

		for(std::size_t i=0;i<spheres_.size();i++)
		{
			const GridPoint gp(spheres_[i], box_size_, grid_offset_);
			const int index=gp.index(grid_size_);
			const int box_id=map_of_boxes_[index];
			if(box_id<0)
			{
				map_of_boxes_[index]=static_cast<int>(boxes_.size());
				boxes_.push_back(std::vector<std::size_t>(1, i));
			}
			else
			{
				boxes_[box_id].push_back(i);
			}
		}
	}

	const std::vector<SimpleSphere>& all_spheres() const
	{
		return spheres_;
	}

	bool find_colliding_ids(const std::size_t& central_id, std::vector<std::size_t>& colliding_ids) const
	{
		colliding_ids.clear();
		if(central_id<spheres_.size())
		{
			const SimpleSphere& central_sphere=spheres_[central_id];
			colliding_ids.reserve(20);
			const GridPoint gp(central_sphere, box_size_, grid_offset_);
			GridPoint dgp=gp;
			for(int dx=-1;dx<=1;dx++)
			{
				dgp.x=gp.x+dx;
				for(int dy=-1;dy<=1;dy++)
				{
					dgp.y=gp.y+dy;
					for(int dz=-1;dz<=1;dz++)
					{
						dgp.z=gp.z+dz;
						const int index=dgp.index(grid_size_);
						if(index>=0)
						{
							const int box_id=map_of_boxes_[index];
							if(box_id>=0)
							{
								const std::vector<std::size_t>& ids=boxes_[box_id];
								for(std::size_t i=0;i<ids.size();i++)
								{
									const std::size_t id=ids[i];
									if(id!=central_id && sphere_intersects_sphere(central_sphere, spheres_[id]))
									{
										colliding_ids.push_back(id);
									}
								}
							}
						}
					}
				}
			}
		}
		return (!colliding_ids.empty());
	}

private:
	struct GridPoint
	{
		int x;
		int y;
		int z;

		GridPoint() : x(0), y(0), z(0)
		{
		}

		GridPoint(const SimpleSphere& s, const double grid_step)
		{
			init(s, grid_step);
		}

		GridPoint(const SimpleSphere& s, const double grid_step, const GridPoint& grid_offset)
		{
			init(s, grid_step, grid_offset);
		}

		void init(const SimpleSphere& s, const double grid_step)
		{
			x=static_cast<int>(s.p.x/grid_step);
			y=static_cast<int>(s.p.y/grid_step);
			z=static_cast<int>(s.p.z/grid_step);
		}

		void init(const SimpleSphere& s, const double grid_step, const GridPoint& grid_offset)
		{
			x=static_cast<int>(s.p.x/grid_step)-grid_offset.x;
			y=static_cast<int>(s.p.y/grid_step)-grid_offset.y;
			z=static_cast<int>(s.p.z/grid_step)-grid_offset.z;
		}

		int index(const GridPoint& grid_size) const
		{
			return ((x>=0 && y>=0 && z>=0 && x<grid_size.x && y<grid_size.y &&z<grid_size.z) ? (z*grid_size.x*grid_size.y+y*grid_size.x+x) : (-1));
		}

		bool operator<(const GridPoint& gp) const
		{
			return (x<gp.x || (x==gp.x && y<gp.y) || (x==gp.x && y==gp.y && z<gp.z));
		}
	};

	std::vector<SimpleSphere> spheres_;
	GridPoint grid_offset_;
	GridPoint grid_size_;
	std::vector<int> map_of_boxes_;
	std::vector< std::vector<std::size_t> > boxes_;
	double box_size_;
};

class ConstrainedContactsConstruction
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

	struct NeighborDescriptor
	{
		double sort_value;
		std::size_t c_id;
		SimplePoint ac_plane_center;
		SimplePoint ac_plane_normal;

		NeighborDescriptor(const std::size_t c_id, const SimpleSphere& a, const SimpleSphere& c) : sort_value(0.0), c_id(c_id), ac_plane_center(center_of_intersection_circle_of_two_spheres(a, c)), ac_plane_normal(unit_point(sub_of_points(c.p, a.p)))
		{
		}

		bool operator<(const NeighborDescriptor& d) const
		{
			return (sort_value<d.sort_value || (sort_value==d.sort_value && c_id<d.c_id));
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

		void clear(const std::size_t a_id, const std::size_t b_id)
		{
			id_a=a_id;
			id_b=b_id;
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

	struct ContactDescriptorSummary
	{
		std::size_t id_a;
		std::size_t id_b;
		double area;
		double arc_length;
		int complexity;
		double solid_angle_a;
		double solid_angle_b;
		double pyramid_volume_a;
		double pyramid_volume_b;
		bool valid;

		ContactDescriptorSummary() : id_a(0), id_b(0), area(0.0), arc_length(0.0), complexity(0), solid_angle_a(0.0), solid_angle_b(0.0), pyramid_volume_a(0.0), pyramid_volume_b(0.0), valid(false)
		{
		}

		void set(const ContactDescriptor& cd)
		{
			if(cd.valid)
			{
				id_a=cd.id_a;
				id_b=cd.id_b;
				area=cd.area;
				arc_length=(cd.sum_of_arc_angles*cd.intersection_circle_sphere.r);
				complexity=cd.contour.size();
				solid_angle_a=cd.solid_angle_a;
				solid_angle_b=cd.solid_angle_b;
				pyramid_volume_a=cd.pyramid_volume_a;
				pyramid_volume_b=cd.pyramid_volume_b;
				valid=true;
			}
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

	struct CellContactDescriptorsSummary
	{
		std::size_t id;
		int count;
		double area;
		double arc_length;
		int complexity;
		double explained_solid_angle_positive;
		double explained_solid_angle_negative;
		double explained_pyramid_volume_positive;
		double explained_pyramid_volume_negative;
		bool valid;
		double sas_r;
		double sas_area;
		double sas_inside_volume;
		bool sas_computed;

		CellContactDescriptorsSummary() : id(0), count(0), area(0.0), arc_length(0.0), complexity(0), explained_solid_angle_positive(0.0), explained_solid_angle_negative(0.0), explained_pyramid_volume_positive(0.0), explained_pyramid_volume_negative(0.0), valid(false), sas_r(0.0), sas_area(0.0), sas_inside_volume(0.0), sas_computed(false)
		{
		}

		void clear(const std::size_t new_id)
		{
			id=new_id;
			count=0;
			area=0.0;
			arc_length=0.0;
			complexity=0;
			explained_solid_angle_positive=0.0;
			explained_solid_angle_negative=0.0;
			explained_pyramid_volume_positive=0.0;
			explained_pyramid_volume_negative=0.0;
			valid=false;
			sas_r=0.0;
			sas_area=0.0;
			sas_inside_volume=0.0;
			sas_computed=false;
		}

		void add(const ContactDescriptorSummary& cds)
		{
			if(cds.valid && (cds.id_a==id || cds.id_b==id))
			{
				count++;
				area+=cds.area;
				arc_length+=cds.arc_length;
				complexity+=cds.complexity;
				explained_solid_angle_positive+=std::max(0.0, (cds.id_a==id ? cds.solid_angle_a : cds.solid_angle_b));
				explained_solid_angle_negative+=0.0-std::min(0.0, (cds.id_a==id ? cds.solid_angle_a : cds.solid_angle_b));
				explained_pyramid_volume_positive+=std::max(0.0, (cds.id_a==id ? cds.pyramid_volume_a : cds.pyramid_volume_b));
				explained_pyramid_volume_negative+=0.0-std::min(0.0, (cds.id_a==id ? cds.pyramid_volume_a : cds.pyramid_volume_b));
				valid=true;
			}
		}

		void add(const std::size_t new_id, const ContactDescriptorSummary& cds)
		{
			if(cds.valid)
			{
				if(!valid)
				{
					clear(new_id);
				}
				add(cds);
			}
		}

		void compute_sas(const double r)
		{
			if(valid)
			{
				sas_r=r;
				sas_area=0.0;
				sas_inside_volume=0.0;
				if(arc_length>0.0)
				{
					if(explained_solid_angle_positive>explained_solid_angle_negative)
					{
						sas_area=((4.0*pi_value())-std::max(0.0, explained_solid_angle_positive-explained_solid_angle_negative))*(r*r);
					}
					else if(explained_solid_angle_negative>explained_solid_angle_positive)
					{
						sas_area=(std::max(0.0, explained_solid_angle_negative-explained_solid_angle_positive))*(r*r);
					}
					sas_inside_volume=(sas_area*r/3.0)+explained_pyramid_volume_positive-explained_pyramid_volume_negative;
				}
				else
				{
					sas_inside_volume=explained_pyramid_volume_positive-explained_pyramid_volume_negative;
				}
				sas_computed=true;
			}
		}
	};

	struct TotalContactDescriptorsSummary
	{
		int count;
		double area;
		double arc_length;
		int complexity;
		bool valid;

		TotalContactDescriptorsSummary() : count(0), area(0.0), arc_length(0.0), complexity(0), valid(false)
		{
		}

		void clear()
		{
			count=0;
			area=0.0;
			arc_length=0.0;
			complexity=0;
			valid=false;
		}

		void add(const ContactDescriptorSummary& cds)
		{
			if(cds.valid)
			{
				count++;
				area+=cds.area;
				arc_length+=cds.arc_length;
				complexity+=cds.complexity;
				valid=true;
			}
		}
	};

	struct TotalCellContactDescriptorsSummary
	{
		int count;
		double sas_area;
		double sas_inside_volume;
		bool valid;

		TotalCellContactDescriptorsSummary() : count(0), sas_area(0.0), sas_inside_volume(0.0), valid(false)
		{
		}

		void clear()
		{
			count=0;
			sas_area=0.0;
			sas_inside_volume=0.0;
			valid=false;
		}

		void add(const CellContactDescriptorsSummary& ccds)
		{
			if(ccds.sas_computed)
			{
				count++;
				sas_area+=ccds.sas_area;
				sas_inside_volume+=ccds.sas_inside_volume;
				valid=true;
			}
		}
	};

	static bool construct_contact_descriptor(
			const std::vector<SimpleSphere>& spheres,
			const std::size_t a_id,
			const std::size_t b_id,
			const std::vector<std::size_t>& a_neighbor_ids,
			const std::vector<std::size_t>& b_neighbor_ids,
			ContactDescriptor& result_contact_descriptor)
	{
		result_contact_descriptor.clear(a_id, b_id);
		if(a_id<spheres.size() && b_id<spheres.size())
		{
			const SimpleSphere& a=spheres[a_id];
			const SimpleSphere& b=spheres[b_id];
			if(sphere_intersects_sphere(a, b) && !sphere_contains_sphere(a, b) && !sphere_contains_sphere(b, a))
			{
				result_contact_descriptor.intersection_circle_sphere=intersection_circle_of_two_spheres(a, b);
				if(result_contact_descriptor.intersection_circle_sphere.r>0.0)
				{
					bool discarded=false;
					{
						const std::vector<std::size_t>& j_neighbor_ids=(a_neighbor_ids.size()<b_neighbor_ids.size() ? a_neighbor_ids : b_neighbor_ids);
						const SimpleSphere& j_alt_sphere=(a_neighbor_ids.size()<b_neighbor_ids.size() ? b : a);
						result_contact_descriptor.neighbor_descriptors.reserve(j_neighbor_ids.size());
						for(std::size_t i=0;i<j_neighbor_ids.size() && !discarded;i++)
						{
							const std::size_t neighbor_id=j_neighbor_ids[i];
							if(neighbor_id<spheres.size() && neighbor_id!=a_id && neighbor_id!=b_id)
							{
								const SimpleSphere& c=spheres[neighbor_id];
								if(sphere_intersects_sphere(result_contact_descriptor.intersection_circle_sphere, c) && sphere_intersects_sphere(j_alt_sphere, c))
								{
									if(sphere_contains_sphere(c, a) || sphere_contains_sphere(c, b))
									{
										discarded=true;
									}
									else
									{
										NeighborDescriptor nd(neighbor_id, a, c);
										const double cos_val=dot_product(unit_point(sub_of_points(result_contact_descriptor.intersection_circle_sphere.p, a.p)), unit_point(sub_of_points(nd.ac_plane_center, a.p)));
										const int hsi=halfspace_of_point(nd.ac_plane_center, nd.ac_plane_normal, result_contact_descriptor.intersection_circle_sphere.p);
										if(std::abs(cos_val)<1.0)
										{
											const double l=std::abs(signed_distance_from_point_to_plane(nd.ac_plane_center, nd.ac_plane_normal, result_contact_descriptor.intersection_circle_sphere.p));
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
									mark_and_cut_contour(nd.ac_plane_center, nd.ac_plane_normal, nd.c_id, result_contact_descriptor.contour);
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

#endif /* VORONOTA_LT_H_ */

