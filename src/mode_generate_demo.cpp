#include "apollota/spheres_boundary_construction.h"
#include "apollota/triangulation_queries.h"
#include "apollota/rolling_topology.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/opengl_printer.h"
#include "auxiliaries/program_options_handler.h"

namespace
{

void draw_rolling_triangle(
		const apollota::SimpleSphere& a,
		const apollota::SimpleSphere& b,
		const apollota::SimpleSphere& c,
		const apollota::SimpleSphere& tangent,
		auxiliaries::OpenGLPrinter& opengl_printer)
{
	std::vector<apollota::SimplePoint> vertices(3);
	std::vector<apollota::SimplePoint> normals(3);
	normals[0]=(apollota::SimplePoint(tangent)-apollota::SimplePoint(a)).unit();
	normals[1]=(apollota::SimplePoint(tangent)-apollota::SimplePoint(b)).unit();
	normals[2]=(apollota::SimplePoint(tangent)-apollota::SimplePoint(c)).unit();
	for(int i=0;i<3;i++)
	{
		vertices[i]=apollota::SimplePoint(tangent)-(normals[i]*tangent.r);
	}
	opengl_printer.add_triangle_strip(vertices, normals);
}

void draw_rolling_quadrangle(
		const apollota::SimpleSphere& a,
		const apollota::SimpleSphere& b,
		const apollota::SimpleSphere& tangent1,
		const apollota::SimpleSphere& tangent2,
		auxiliaries::OpenGLPrinter& opengl_printer)
{
	std::vector<apollota::SimplePoint> vertices(4);
	std::vector<apollota::SimplePoint> normals(4);
	normals[0]=(apollota::SimplePoint(tangent1)-apollota::SimplePoint(a)).unit();
	normals[1]=(apollota::SimplePoint(tangent1)-apollota::SimplePoint(b)).unit();
	normals[2]=(apollota::SimplePoint(tangent2)-apollota::SimplePoint(a)).unit();
	normals[3]=(apollota::SimplePoint(tangent2)-apollota::SimplePoint(b)).unit();
	for(int i=0;i<2;i++)
	{
		vertices[i]=apollota::SimplePoint(tangent1)-(normals[i]*tangent1.r);
	}
	for(int i=2;i<4;i++)
	{
		vertices[i]=apollota::SimplePoint(tangent2)-(normals[i]*tangent2.r);
	}
	opengl_printer.add_triangle_strip(vertices, normals);
}

}

void generate_demo(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'x y z r')");
	pohw.describe_io("stdout", false, true, "drawing script fo PyMol");

	const double probe=poh.restrict_value_in_range(0.01, 14.0, poh.argument<double>(pohw.describe_option("--probe", "number", "probe radius"), 1.4));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::IOUtilities().read_lines_to_set(std::cin, spheres);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(spheres, probe*2.0);
	spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, false, false);

	apollota::TriangulationQueries::IDsMap singles_map=apollota::TriangulationQueries::collect_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map);
	const apollota::TriangulationQueries::PairsMap pairs_map=apollota::TriangulationQueries::collect_pairs_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map);

	auxiliaries::OpenGLPrinter opengl_printer;
	opengl_printer.add_color(0xFFFF00);

	for(apollota::TriangulationQueries::PairsMap::const_iterator pairs_map_it=pairs_map.begin();pairs_map_it!=pairs_map.end();++pairs_map_it)
	{
		const apollota::Pair& pair=pairs_map_it->first;
		const std::set<std::size_t>& neighbor_ids=pairs_map_it->second;
		const apollota::RollingTopology::RollingDescriptor rolling_descriptor=
				apollota::RollingTopology::calculate_rolling_descriptor(spheres, pair.get(0), pair.get(1), neighbor_ids, singles_map[pair.get(0)], singles_map[pair.get(1)], probe);
		if(rolling_descriptor.possible)
		{
			if(!rolling_descriptor.strips.empty())
			{
				for(std::list<apollota::RollingTopology::RollingStrip>::const_iterator strip_it=rolling_descriptor.strips.begin();strip_it!=rolling_descriptor.strips.end();++strip_it)
				{
					opengl_printer.add_color(rolling_descriptor.breaks.empty() ? 0xFFFF00 : 0xFF0000);
					{
						const std::vector<apollota::SimplePoint> points=apollota::RollingTopology::construct_rolling_strip_approximation(rolling_descriptor, (*strip_it), 0.05);
						for(std::size_t i=0;i+1<points.size();i++)
						{
							draw_rolling_quadrangle(spheres[pair.get(0)], spheres[pair.get(1)], apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), opengl_printer);
						}
					}

					opengl_printer.add_color(0x00FFFF);
					if(strip_it->start.generator<pair.get_min_max().first)
					{
						draw_rolling_triangle(spheres[pair.get(0)], spheres[pair.get(1)], spheres[strip_it->start.generator], strip_it->start.tangent, opengl_printer);
					}
					if(strip_it->end.generator<pair.get_min_max().first)
					{
						draw_rolling_triangle(spheres[pair.get(0)], spheres[pair.get(1)], spheres[strip_it->end.generator], strip_it->end.tangent, opengl_printer);
					}
				}
			}
			else if(rolling_descriptor.detached)
			{
				const std::vector<apollota::SimplePoint> points=apollota::RollingTopology::construct_rolling_circle_approximation(rolling_descriptor, 0.05);
				opengl_printer.add_color(0xFF00FF);
				opengl_printer.add_line_strip(points);
			}
		}
	}

	opengl_printer.print_pymol_script("presurface", false, std::cout);
}
