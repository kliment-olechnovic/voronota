#include <list>

#include "apollota/basic_operations_on_points.h"
#include "apollota/tuple.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

#include "modescommon/svg_writer.h"

namespace
{

template<typename Iter>
Iter get_prev_iter_in_cycle(Iter begin, Iter end, Iter it)
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
Iter get_next_iter_in_cycle(Iter begin, Iter end, Iter it)
{
	Iter result=it;
	++result;
	if(result==end)
	{
		result=begin;
	}
	return result;
}

double calc_convexity(const apollota::SimplePoint& normal, const apollota::SimplePoint& a, const apollota::SimplePoint& b, const apollota::SimplePoint& c)
{
	const bool convex=((((b-a)&(c-b))*normal)>=0.0);
	return (apollota::min_angle(b, a, c)*(convex ? 1.0 : -1.0));
}

std::vector<double> calc_convexity(const std::vector<apollota::SimplePoint>& points, const apollota::SimplePoint& normal, bool& inverted)
{
	std::vector<double> convexity;
	convexity.reserve(points.size());
	double sum_of_angles=0.0;
	for(std::vector<apollota::SimplePoint>::const_iterator it=points.begin();it!=points.end();++it)
	{
		const double v=calc_convexity(
				normal,
				*get_prev_iter_in_cycle<std::vector<apollota::SimplePoint>::const_iterator>(points.begin(), points.end(), it),
				*it,
				*get_next_iter_in_cycle<std::vector<apollota::SimplePoint>::const_iterator>(points.begin(), points.end(), it));
		convexity.push_back(v);
		sum_of_angles+=v;
	}
	inverted=(sum_of_angles<0.0);
	if(inverted)
	{
		for(std::size_t i=0;i<convexity.size();i++)
		{
			convexity[i]=(0.0-convexity[i]);
		}
	}
	return convexity;
}

bool check_point_between_rays(const apollota::SimplePoint& o, const apollota::SimplePoint& a, const apollota::SimplePoint& b, const apollota::SimplePoint& p)
{
	const apollota::SimplePoint oa=(a-o).unit();
	const apollota::SimplePoint ob=(b-o).unit();
	const apollota::SimplePoint op=(p-o).unit();
	const double cos_aob=oa*ob;
	const double cos_aop=oa*op;
	const double cos_pob=op*ob;
	return (cos_aop>cos_aob && cos_pob>cos_aob);
}

bool check_point_in_triangle(const apollota::SimplePoint& a, const apollota::SimplePoint& b, const apollota::SimplePoint& c, const apollota::SimplePoint& p)
{
	return (check_point_between_rays(a, b, c, p) && check_point_between_rays(b, a, c, p) && check_point_between_rays(c, a, b, p));
}

std::vector<apollota::Triple> triangulate_simple_polygon(const std::vector<apollota::SimplePoint>& points, const apollota::SimplePoint& unchecked_normal)
{
	bool convexity_inverted=false;
	std::vector<double> convexity=calc_convexity(points, unchecked_normal, convexity_inverted);
	const apollota::SimplePoint checked_normal=(convexity_inverted ? unchecked_normal.inverted() : unchecked_normal);
	std::list<std::size_t> polygon_ids;
	for(std::size_t i=0;i<points.size();i++)
	{
		polygon_ids.push_back(i);
	}
	std::vector<apollota::Triple> results;
	bool ended=false;
	while(!ended)
	{
		bool ear_found=false;
		for(std::list<std::size_t>::iterator it=polygon_ids.begin();it!=polygon_ids.end() && !ear_found;++it)
		{
			if(convexity[*it]>0.0)
			{
				std::list<std::size_t>::iterator prev_it=get_prev_iter_in_cycle<std::list<std::size_t>::iterator>(polygon_ids.begin(), polygon_ids.end(), it);
				std::list<std::size_t>::iterator next_it=get_next_iter_in_cycle<std::list<std::size_t>::iterator>(polygon_ids.begin(), polygon_ids.end(), it);
				bool empty=true;
				for(std::size_t i=0;i<convexity.size() && empty;i++)
				{
					if(convexity[i]<=0.0)
					{
						empty=!check_point_in_triangle(points[*prev_it], points[*it], points[*next_it], points[i]);
					}
				}
				if(empty)
				{
					ear_found=true;
					results.push_back(apollota::Triple(*prev_it, *it, *next_it));
					polygon_ids.erase(it);
					std::list<std::size_t>::iterator prev_prev_it=get_prev_iter_in_cycle<std::list<std::size_t>::iterator>(polygon_ids.begin(), polygon_ids.end(), prev_it);
					std::list<std::size_t>::iterator next_next_it=get_next_iter_in_cycle<std::list<std::size_t>::iterator>(polygon_ids.begin(), polygon_ids.end(), next_it);
					convexity[*prev_it]=calc_convexity(checked_normal, points[*prev_prev_it], points[*prev_it], points[*next_it]);
					convexity[*next_it]=calc_convexity(checked_normal, points[*prev_it], points[*next_it], points[*next_next_it]);
				}
			}
		}
		ended=!ear_found;
	}
	return results;
}

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

	const apollota::SimplePoint unchecked_normal(0, 0, 1);
	bool convexity_inverted=false;
	const std::vector<double> convexity=calc_convexity(points, unchecked_normal, convexity_inverted);

	const std::vector<apollota::Triple> triangulation=triangulate_simple_polygon(points, unchecked_normal);

	SVGWriter svg(p_max.x-p_min.x, p_max.y-p_min.y);
	for(std::size_t i=0;i<points.size();i++)
	{
		const apollota::SimplePoint& p=points[i];
		svg.add_circle(p.x, p.y, 10, (convexity[i]<0.0 ? "rgb(255,0,0)" : "rgb(0,255,0)"));
	}
	for(std::size_t i=0;i<points.size();i++)
	{
		const apollota::SimplePoint& a=points[i];
		const apollota::SimplePoint& b=points[i+1<points.size() ? i+1 : 0];
		svg.add_line(a.x, a.y, b.x, b.y, "rgb(0,0,255)");
	}
	for(std::size_t i=0;i<triangulation.size();i++)
	{
		const apollota::Triple& t=triangulation[i];
		const apollota::SimplePoint& a=points[t.get(0)];
		const apollota::SimplePoint& b=points[t.get(1)];
		const apollota::SimplePoint& c=points[t.get(2)];
		svg.add_line(a.x, a.y, b.x, b.y, "rgb(255,0,255)");
		svg.add_line(a.x, a.y, c.x, c.y, "rgb(255,0,255)");
		svg.add_line(b.x, b.y, c.x, c.y, "rgb(255,0,255)");
	}
	svg.write(std::cout);
}
