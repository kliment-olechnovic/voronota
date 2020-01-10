#ifndef APOLLOTA_SIMPLE_POLYGON_UTILITIES_H_
#define APOLLOTA_SIMPLE_POLYGON_UTILITIES_H_

#include <list>
#include <set>

#include "basic_operations_on_points.h"
#include "tuple.h"

namespace voronota
{

namespace apollota
{

class SimplePolygonUtilities
{
public:
	struct ConvexityInfo
	{
		std::vector<SimplePoint> points;
		std::vector<double> convexity;
		SimplePoint normal;
	};

	struct TriangulationInfo
	{
		ConvexityInfo convexity_info;
		std::vector<Triple> triangulation;
	};

	static ConvexityInfo calc_polygon_convexity(const std::vector<SimplePoint>& points, const SimplePoint& normal)
	{
		ConvexityInfo result;
		if(points.size()<3)
		{
			return result;
		}
		result.normal=normal.unit();
		result.points=flatten_points(points, result.normal);
		result.convexity.reserve(result.points.size());
		for(std::vector<SimplePoint>::const_iterator it=result.points.begin();it!=result.points.end();++it)
		{
			const double v=calc_convexity(
					result.normal,
					*get_prev_iter_in_cycle<std::vector<SimplePoint>::const_iterator>(result.points.begin(), result.points.end(), it),
					*it,
					*get_next_iter_in_cycle<std::vector<SimplePoint>::const_iterator>(result.points.begin(), result.points.end(), it));
			result.convexity.push_back(v);
		}
		std::size_t max_dist_id=0;
		{
			double max_dist=distance_from_point_to_point(result.points[0], result.points[max_dist_id]);
			for(std::size_t i=0;i<result.points.size();i++)
			{
				const double dist=distance_from_point_to_point(result.points[0], result.points[i]);
				if(dist>max_dist)
				{
					max_dist_id=i;
					max_dist=dist;
				}
			}
		}
		if(result.convexity[max_dist_id]<0.0)
		{
			result.normal=result.normal.inverted();
			for(std::size_t i=0;i<result.convexity.size();i++)
			{
				result.convexity[i]=(0.0-result.convexity[i]);
			}
		}
		return result;
	}

	static TriangulationInfo triangulate_simple_polygon(const std::vector<SimplePoint>& points, const SimplePoint& normal)
	{
		TriangulationInfo result;
		if(points.size()<3)
		{
			return result;
		}
		result.convexity_info=calc_polygon_convexity(points, normal);
		if(result.convexity_info.convexity.size()!=points.size())
		{
			return result;
		}

		const std::vector<SimplePoint>& polygon_points=result.convexity_info.points;
		const SimplePoint& polygon_normal=result.convexity_info.normal;
		std::vector<double> polygon_convexity_vector=result.convexity_info.convexity;
		std::list<std::size_t> polygon_ids;
		for(std::size_t i=0;i<polygon_points.size();i++)
		{
			polygon_ids.push_back(i);
		}
		std::set<std::size_t> polygon_concave_set;
		for(std::size_t i=0;i<polygon_convexity_vector.size();i++)
		{
			if(polygon_convexity_vector[i]<=0.0)
			{
				polygon_concave_set.insert(i);
			}
		}

		std::vector< std::pair<double, std::list<std::size_t>::iterator> > current_convex_ids;
		current_convex_ids.reserve(polygon_convexity_vector.size());
		bool triangulation_ended=false;
		while(!triangulation_ended)
		{
			current_convex_ids.clear();
			for(std::list<std::size_t>::iterator it=polygon_ids.begin();it!=polygon_ids.end();++it)
			{
				if(polygon_convexity_vector[*it]>0.0)
				{
					current_convex_ids.push_back(std::make_pair(polygon_convexity_vector[*it], it));
				}
			}
			if(current_convex_ids.size()<=3)
			{
				if(current_convex_ids.size()==3)
				{
					result.triangulation.push_back(Triple(*(current_convex_ids[0].second), *(current_convex_ids[1].second), *(current_convex_ids[2].second)));
				}
				triangulation_ended=true;
			}
			else
			{
				std::sort(current_convex_ids.begin(), current_convex_ids.end(), custom_pair_comparison<double, std::list<std::size_t>::iterator>());
				bool ear_found=false;
				for(std::vector< std::pair<double, std::list<std::size_t>::iterator> >::const_iterator current_convex_ids_it=current_convex_ids.begin();current_convex_ids_it!=current_convex_ids.end() && !ear_found;++current_convex_ids_it)
				{
					std::list<std::size_t>::iterator it=current_convex_ids_it->second;
					std::list<std::size_t>::iterator prev_it=get_prev_iter_in_cycle<std::list<std::size_t>::iterator>(polygon_ids.begin(), polygon_ids.end(), it);
					std::list<std::size_t>::iterator next_it=get_next_iter_in_cycle<std::list<std::size_t>::iterator>(polygon_ids.begin(), polygon_ids.end(), it);
					bool triangle_is_empty=true;
					for(std::set<std::size_t>::const_iterator concave_it=polygon_concave_set.begin();concave_it!=polygon_concave_set.end() && triangle_is_empty;++concave_it)
					{
						triangle_is_empty=!check_point_in_triangle(polygon_points[*prev_it], polygon_points[*it], polygon_points[*next_it], polygon_points[*concave_it]);
					}
					if(triangle_is_empty)
					{
						ear_found=true;
						result.triangulation.push_back(Triple(*prev_it, *it, *next_it));
						polygon_concave_set.erase(*it);
						polygon_ids.erase(it);
						std::list<std::size_t>::iterator prev_prev_it=get_prev_iter_in_cycle<std::list<std::size_t>::iterator>(polygon_ids.begin(), polygon_ids.end(), prev_it);
						std::list<std::size_t>::iterator next_next_it=get_next_iter_in_cycle<std::list<std::size_t>::iterator>(polygon_ids.begin(), polygon_ids.end(), next_it);
						polygon_convexity_vector[*prev_it]=calc_convexity(polygon_normal, polygon_points[*prev_prev_it], polygon_points[*prev_it], polygon_points[*next_it]);
						polygon_convexity_vector[*next_it]=calc_convexity(polygon_normal, polygon_points[*prev_it], polygon_points[*next_it], polygon_points[*next_next_it]);
						if(polygon_convexity_vector[*prev_it]<=0.0)
						{
							polygon_concave_set.insert(*prev_it);
						}
						else
						{
							polygon_concave_set.erase(*prev_it);
						}
						if(polygon_convexity_vector[*next_it]<=0.0)
						{
							polygon_concave_set.insert(*next_it);
						}
						else
						{
							polygon_concave_set.erase(*next_it);
						}
					}
				}
				triangulation_ended=!ear_found;
			}
		}

		if(result.triangulation.size()!=(result.convexity_info.points.size()-2))
		{
			result.triangulation.clear();
		}
		else
		{
			std::set<std::size_t> included_vertices;
			for(std::size_t i=0;i<result.triangulation.size();i++)
			{
				const apollota::Triple& t=result.triangulation[i];
				included_vertices.insert(t.get(0));
				included_vertices.insert(t.get(1));
				included_vertices.insert(t.get(2));
			}
			if(included_vertices.size()!=result.convexity_info.points.size())
			{
				result.triangulation.clear();
			}
		}

		return result;
	}

private:
	static std::vector<SimplePoint> flatten_points(const std::vector<SimplePoint>& points, const SimplePoint& normal)
	{
		std::vector<SimplePoint> result(points.size());
		for(std::size_t i=0;i<points.size();i++)
		{
			const double dist_to_plane=(points[i]-points[0])*normal;
			result[i]=(points[i]-(normal*dist_to_plane));
		}
		return result;
	}

	template<typename Iter>
	static Iter get_prev_iter_in_cycle(Iter begin, Iter end, Iter it)
	{
		Iter result=it;
		if(result==begin)
		{
			result=end;
		}
		--result;
		return result;
	}

	template<typename Iter>
	static Iter get_next_iter_in_cycle(Iter begin, Iter end, Iter it)
	{
		Iter result=it;
		++result;
		if(result==end)
		{
			result=begin;
		}
		return result;
	}

	static double calc_convexity(const SimplePoint& normal, const SimplePoint& a, const SimplePoint& b, const SimplePoint& c)
	{
		const bool convex=((((b-a)&(c-b))*normal)>=0.0);
		return (min_angle(b, a, c)*(convex ? 1.0 : -1.0));
	}

	static bool check_point_between_rays(const SimplePoint& o, const SimplePoint& a, const SimplePoint& b, const SimplePoint& p)
	{
		const SimplePoint oa=(a-o).unit();
		const SimplePoint ob=(b-o).unit();
		const SimplePoint op=(p-o).unit();
		const double cos_aob=oa*ob;
		const double cos_aop=oa*op;
		const double cos_pob=op*ob;
		return (cos_aop>cos_aob && cos_pob>cos_aob);
	}

	static bool check_point_in_triangle(const SimplePoint& a, const SimplePoint& b, const SimplePoint& c, const SimplePoint& p)
	{
		return (check_point_between_rays(a, b, c, p) && check_point_between_rays(b, a, c, p));
	}

	template<typename T1, typename T2>
	struct custom_pair_comparison
	{
		bool operator()(const std::pair<T1, T2>& a, const std::pair<T1, T2>& b)
		{
			return (a.first<b.first);
		}
	};
};

}

}

#endif /* APOLLOTA_SIMPLE_POLYGON_UTILITIES_H_ */
