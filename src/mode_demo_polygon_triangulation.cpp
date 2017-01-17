#include "apollota/simple_polygon_utilities.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

#include "modescommon/svg_writer.h"

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

	apollota::SimplePolygonUtilities::TriangulationInfo result=apollota::SimplePolygonUtilities::triangulate_simple_polygon(points, apollota::SimplePoint(0, 0, 1));
	if(result.triangulation.empty())
	{
		throw std::runtime_error("Failed to produce triangulation.");
	}

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
