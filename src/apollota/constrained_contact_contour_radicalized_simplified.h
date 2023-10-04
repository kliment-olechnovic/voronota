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
	struct PointRecord
	{
		SimplePoint p;
		std::size_t left_id;
		std::size_t right_id;

		PointRecord(const SimplePoint& p, const std::size_t left_id, const std::size_t right_id) : p(p), left_id(left_id), right_id(right_id)
		{
		}
	};

	typedef std::vector<PointRecord> Contour;

	static SimplePoint calc_contour_center(const Contour& contour)
	{
		SimplePoint center;
		if(!contour.empty())
		{
			for(std::size_t i=0;i<contour.size();i++)
			{
				center=center+contour[i].p;
			}
			center=center*(1.0/static_cast<double>(contour.size()));
		}
		return center;
	}

	static bool construct_contact_contours_for_expanded_spheres_without_tessellation(
			const std::vector<SimpleSphere>& spheres,
			const std::size_t a_id,
			const std::size_t b_id,
			const std::vector<std::size_t>& a_neighbor_ids,
			const std::vector<std::size_t>& b_neighbor_ids,
			const double step,
			Contour& result_contour)
	{
		if(a_id<spheres.size() && b_id<spheres.size())
		{
			const SimpleSphere& a=spheres[a_id];
			const SimpleSphere& b=spheres[b_id];
			if(sphere_intersects_sphere(a, b))
			{
				const SimpleSphere ic_sphere=intersection_circle_of_two_spheres<SimpleSphere>(a, b);
				if(ic_sphere.r>0.0)
				{
					result_contour.clear();
					std::vector< std::pair<double, std::size_t> > neighbor_ids;
					const std::vector<std::size_t>& j_neighbor_ids=(a_neighbor_ids.size()<b_neighbor_ids.size() ? a_neighbor_ids : b_neighbor_ids);
					neighbor_ids.reserve(j_neighbor_ids.size());
					for(std::size_t i=0;i<j_neighbor_ids.size();i++)
					{
						const std::size_t neighbor_id=j_neighbor_ids[i];
						if(neighbor_id<spheres.size() && neighbor_id!=a_id && neighbor_id!=b_id)
						{
							const SimpleSphere& c=spheres[neighbor_id];
							const double dist_to_ic_sphere=minimal_distance_from_sphere_to_sphere(c, ic_sphere);
							if(dist_to_ic_sphere<0.0)
							{
								neighbor_ids.push_back(std::make_pair(dist_to_ic_sphere, neighbor_id));
							}
						}
					}
					{
						const SimplePoint axis=sub_of_points<SimplePoint>(b, a).unit();
						construct_circular_contour_from_base_and_axis(a_id, ic_sphere, axis, step, result_contour);
					}
					if(!result_contour.empty() && !neighbor_ids.empty())
					{
						std::sort(neighbor_ids.begin(), neighbor_ids.end());
						for(std::size_t i=0;i<neighbor_ids.size();i++)
						{
							const std::size_t c_id=neighbor_ids[i].second;
							if((i==0 || c_id!=neighbor_ids[i-1].second) && c_id<spheres.size())
							{
								const SimpleSphere& c=spheres[c_id];
								const SimplePoint ac_plane_center(intersection_circle_of_two_spheres<SimpleSphere>(a, c));
								const SimplePoint ac_plane_normal(sub_of_points<SimplePoint>(c, a).unit());
								mark_and_cut_contour(ac_plane_center, ac_plane_normal, c_id, result_contour);
							}
						}
					}
					return (!result_contour.empty());
				}
			}
		}
		return false;
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
		result.reserve(static_cast<int>(360.0/angle_step)+2);
		result.push_back(PointRecord(sum_of_points<SimplePoint>(base, first_point), a_id, a_id));
		for(rotation.angle=angle_step;rotation.angle<360;rotation.angle+=angle_step)
		{
			result.push_back(PointRecord(sum_of_points<SimplePoint>(base, rotation.rotate<SimplePoint>(first_point)), a_id, a_id));
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
			contour[i_start]=PointRecord(intersection_of_plane_and_segment<SimplePoint>(ac_plane_center, ac_plane_normal, contour[i_start].p, contour[i_left].p), contour[i_left].right_id, contour[i_start].left_id);
		}

		{
			const std::size_t i_right=((i_end+1)<contour.size() ? (i_end+1) : 0);
			contour[i_end]=PointRecord(intersection_of_plane_and_segment<SimplePoint>(ac_plane_center, ac_plane_normal, contour[i_end].p, contour[i_right].p), contour[i_end].right_id, contour[i_right].left_id);
		}
	}
};

}

}

#endif /* APOLLOTA_CONSTRAINED_CONTACT_CONTOUR_RADICALIZED_SIMPLIFIED_H_ */

