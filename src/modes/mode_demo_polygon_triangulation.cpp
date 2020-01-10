#include "../apollota/simple_polygon_utilities.h"

#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"

#include "modescommon/svg_writer.h"

namespace
{

std::pair<double, double> fit_points_to_positive_canvas(const double margin, std::vector<voronota::apollota::SimplePoint>& points)
{
	if(!points.empty())
	{
		voronota::apollota::SimplePoint p_min=points[0];
		voronota::apollota::SimplePoint p_max=points[0];
		for(std::size_t i=1;i<points.size();i++)
		{
			const voronota::apollota::SimplePoint& p=points[i];
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
		p_min=p_min-voronota::apollota::SimplePoint(margin, margin, 0);
		p_max=p_max+voronota::apollota::SimplePoint(margin, margin, 0);
		for(std::size_t i=0;i<points.size();i++)
		{
			voronota::apollota::SimplePoint& p=points[i];
			p=(p-p_min);
		}
		return std::pair<double, double>(p_max.x-p_min.x, p_max.y-p_min.y);
	}
	return std::pair<double, double>(0, 0);
}

}

void demo_polygon_triangulation(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "polygon points");
	pohw.describe_io("stdout", false, true, "picture in SVG format");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::vector<voronota::apollota::SimplePoint> points;
	voronota::auxiliaries::IOUtilities().read_lines_to_set(std::cin, points);
	if(points.size()<3)
	{
		throw std::runtime_error("Less than 3 points provided to stdin.");
	}

	const std::pair<double, double> dimensions=fit_points_to_positive_canvas(100.0, points);

	voronota::apollota::SimplePolygonUtilities::TriangulationInfo result=voronota::apollota::SimplePolygonUtilities::triangulate_simple_polygon(points, voronota::apollota::SimplePoint(0, 0, 1));
	if(result.triangulation.empty())
	{
		throw std::runtime_error("Failed to produce triangulation.");
	}

	voronota::modescommon::SVGWriter svg(dimensions.first, dimensions.second);
	for(std::size_t i=0;i<points.size();i++)
	{
		const voronota::apollota::SimplePoint& p=points[i];
		svg.add_circle(p.x, p.y, 10, (result.convexity_info.convexity[i]<0.0 ? "fill:rgb(255,0,0)" : "fill:rgb(0,0,255)"));
	}
	for(std::size_t i=0;i<points.size();i++)
	{
		const voronota::apollota::SimplePoint& a=points[i];
		const voronota::apollota::SimplePoint& b=points[i+1<points.size() ? i+1 : 0];
		svg.add_line(a.x, a.y, b.x, b.y, "stroke:rgb(255,255,0);stroke-width:5");
	}
	for(std::size_t i=0;i<result.triangulation.size();i++)
	{
		const voronota::apollota::Triple& t=result.triangulation[i];
		const voronota::apollota::SimplePoint& a=points[t.get(0)];
		const voronota::apollota::SimplePoint& b=points[t.get(1)];
		const voronota::apollota::SimplePoint& c=points[t.get(2)];
		svg.add_line(a.x, a.y, b.x, b.y, "stroke:rgb(0,0,0);stroke-width:1");
		svg.add_line(a.x, a.y, c.x, c.y, "stroke:rgb(0,0,0);stroke-width:1");
		svg.add_line(b.x, b.y, c.x, c.y, "stroke:rgb(0,0,0);stroke-width:1");
	}
	svg.write(std::cout);
}
