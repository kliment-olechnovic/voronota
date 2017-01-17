#include <list>

#include "apollota/basic_operations_on_points.h"
#include "apollota/tuple.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

#include "modescommon/svg_writer.h"

namespace
{

class SimplePolygonUtilities
{
public:
	struct ConvexityInfo
	{
		std::vector<apollota::SimplePoint> points;
		std::vector<double> convexity;
		apollota::SimplePoint normal;
	};

	struct TriangulationInfo
	{
		ConvexityInfo convexity_info;
		std::vector<apollota::Triple> triangulation;
	};

	static ConvexityInfo calc_polygon_convexity(const std::vector<apollota::SimplePoint>& points, const apollota::SimplePoint& normal)
	{
		ConvexityInfo result;
		if(points.size()<3)
		{
			return result;
		}
		result.normal=normal.unit();
		result.points=flatten_points(points, result.normal);
		result.convexity.reserve(result.points.size());
		for(std::vector<apollota::SimplePoint>::const_iterator it=result.points.begin();it!=result.points.end();++it)
		{
			const double v=calc_convexity(
					result.normal,
					*get_prev_iter_in_cycle<std::vector<apollota::SimplePoint>::const_iterator>(result.points.begin(), result.points.end(), it),
					*it,
					*get_next_iter_in_cycle<std::vector<apollota::SimplePoint>::const_iterator>(result.points.begin(), result.points.end(), it));
			result.convexity.push_back(v);
		}
		std::size_t min_x_id=0;
		std::size_t min_y_id=0;
		std::size_t min_z_id=0;
		for(std::size_t i=0;i<result.points.size();i++)
		{
			if(result.points[i].x<result.points[min_x_id].x)
			{
				min_x_id=i;
			}
			if(result.points[i].y<result.points[min_y_id].y)
			{
				min_y_id=i;
			}
			if(result.points[i].z<result.points[min_z_id].z)
			{
				min_z_id=i;
			}
		}
		if(result.convexity[min_x_id]<0.0 || result.convexity[min_y_id]<0.0 || result.convexity[min_z_id]<0.0)
		{
			result.normal=result.normal.inverted();
			for(std::size_t i=0;i<result.convexity.size();i++)
			{
				result.convexity[i]=(0.0-result.convexity[i]);
			}
		}
		return result;
	}

	static TriangulationInfo triangulate_simple_polygon(const std::vector<apollota::SimplePoint>& points, const apollota::SimplePoint& normal)
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

		const std::vector<apollota::SimplePoint>& polygon_points=result.convexity_info.points;
		const apollota::SimplePoint& polygon_normal=result.convexity_info.normal;
		std::vector<double> polygon_convexity_vector=result.convexity_info.convexity;
		std::list<std::size_t> polygon_ids;
		for(std::size_t i=0;i<polygon_points.size();i++)
		{
			polygon_ids.push_back(i);
		}

		bool triangulation_ended=false;
		while(!triangulation_ended)
		{
			bool ear_found=false;
			for(std::list<std::size_t>::iterator it=polygon_ids.begin();it!=polygon_ids.end() && !ear_found;++it)
			{
				if(polygon_convexity_vector[*it]>0.0)
				{
					std::list<std::size_t>::iterator prev_it=get_prev_iter_in_cycle<std::list<std::size_t>::iterator>(polygon_ids.begin(), polygon_ids.end(), it);
					std::list<std::size_t>::iterator next_it=get_next_iter_in_cycle<std::list<std::size_t>::iterator>(polygon_ids.begin(), polygon_ids.end(), it);
					bool triangle_is_empty=true;
					for(std::size_t i=0;i<polygon_convexity_vector.size() && triangle_is_empty;i++)
					{
						if(polygon_convexity_vector[i]<=0.0)
						{
							triangle_is_empty=!check_point_in_triangle(polygon_points[*prev_it], polygon_points[*it], polygon_points[*next_it], polygon_points[i]);
						}
					}
					if(triangle_is_empty)
					{
						ear_found=true;
						result.triangulation.push_back(apollota::Triple(*prev_it, *it, *next_it));
						polygon_ids.erase(it);
						std::list<std::size_t>::iterator prev_prev_it=get_prev_iter_in_cycle<std::list<std::size_t>::iterator>(polygon_ids.begin(), polygon_ids.end(), prev_it);
						std::list<std::size_t>::iterator next_next_it=get_next_iter_in_cycle<std::list<std::size_t>::iterator>(polygon_ids.begin(), polygon_ids.end(), next_it);
						polygon_convexity_vector[*prev_it]=calc_convexity(polygon_normal, polygon_points[*prev_prev_it], polygon_points[*prev_it], polygon_points[*next_it]);
						polygon_convexity_vector[*next_it]=calc_convexity(polygon_normal, polygon_points[*prev_it], polygon_points[*next_it], polygon_points[*next_next_it]);
					}
				}
			}
			triangulation_ended=!ear_found;
		}

		return result;
	}

private:
	static std::vector<apollota::SimplePoint> flatten_points(const std::vector<apollota::SimplePoint>& points, const apollota::SimplePoint& normal)
	{
		std::vector<apollota::SimplePoint> result(points.size());
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

	static double calc_convexity(const apollota::SimplePoint& normal, const apollota::SimplePoint& a, const apollota::SimplePoint& b, const apollota::SimplePoint& c)
	{
		const bool convex=((((b-a)&(c-b))*normal)>=0.0);
		return (apollota::min_angle(b, a, c)*(convex ? 1.0 : -1.0));
	}

	static bool check_point_between_rays(const apollota::SimplePoint& o, const apollota::SimplePoint& a, const apollota::SimplePoint& b, const apollota::SimplePoint& p)
	{
		const apollota::SimplePoint oa=(a-o).unit();
		const apollota::SimplePoint ob=(b-o).unit();
		const apollota::SimplePoint op=(p-o).unit();
		const double cos_aob=oa*ob;
		const double cos_aop=oa*op;
		const double cos_pob=op*ob;
		return (cos_aop>cos_aob && cos_pob>cos_aob);
	}

	static bool check_point_in_triangle(const apollota::SimplePoint& a, const apollota::SimplePoint& b, const apollota::SimplePoint& c, const apollota::SimplePoint& p)
	{
		return (check_point_between_rays(a, b, c, p) && check_point_between_rays(b, a, c, p));
	}
};

}

void demo_polygon_triangulation(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "polygon points");
	pohw.describe_io("stdout", false, true, "picture in SVG format");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::vector<apollota::SimplePoint> points;
	auxiliaries::IOUtilities().read_lines_to_set(std::cin, points);
	if(points.size()<3)
	{
		throw std::runtime_error("Less than 3 points provided to stdin.");
	}

	apollota::SimplePoint p_min=points[0];
	apollota::SimplePoint p_max=points[0];
	for(std::size_t i=1;i<points.size();i++)
	{
		const apollota::SimplePoint& p=points[i];
		if(p.x<p_min.x)
		{
			p_min.x=p.x;
		}
		if(p.y<p_min.y)
		{
			p_min.y=p.y;
		}
		if(p.x>p_max.x)
		{
			p_max.x=p.x;
		}
		if(p.y>p_max.y)
		{
			p_max.y=p.y;
		}
	}
	p_min=p_min-apollota::SimplePoint(100, 100, 0);
	p_max=p_max+apollota::SimplePoint(100, 100, 0);
	for(std::size_t i=0;i<points.size();i++)
	{
		apollota::SimplePoint& p=points[i];
		p=(p-p_min);
	}

	SimplePolygonUtilities::TriangulationInfo result=SimplePolygonUtilities::triangulate_simple_polygon(points, apollota::SimplePoint(0, 0, 1));

	SVGWriter svg(p_max.x-p_min.x, p_max.y-p_min.y);
	for(std::size_t i=0;i<points.size();i++)
	{
		const apollota::SimplePoint& p=points[i];
		svg.add_circle(p.x, p.y, 10, (result.convexity_info.convexity[i]<0.0 ? "rgb(255,0,0)" : "rgb(0,255,0)"));
	}
	for(std::size_t i=0;i<points.size();i++)
	{
		const apollota::SimplePoint& a=points[i];
		const apollota::SimplePoint& b=points[i+1<points.size() ? i+1 : 0];
		svg.add_line(a.x, a.y, b.x, b.y, "rgb(0,0,255)");
	}
	for(std::size_t i=0;i<result.triangulation.size();i++)
	{
		const apollota::Triple& t=result.triangulation[i];
		const apollota::SimplePoint& a=points[t.get(0)];
		const apollota::SimplePoint& b=points[t.get(1)];
		const apollota::SimplePoint& c=points[t.get(2)];
		svg.add_line(a.x, a.y, b.x, b.y, "rgb(255,0,255)");
		svg.add_line(a.x, a.y, c.x, c.y, "rgb(255,0,255)");
		svg.add_line(b.x, b.y, c.x, c.y, "rgb(255,0,255)");
	}
	svg.write(std::cout);
}
