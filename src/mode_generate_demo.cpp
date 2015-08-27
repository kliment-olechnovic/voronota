#include "apollota/spheres_boundary_construction.h"
#include "apollota/triangulation_queries.h"
#include "apollota/rolling_topology.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/opengl_printer.h"
#include "auxiliaries/program_options_handler.h"

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

//	const std::size_t input_spheres_count=spheres.size();
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
					const std::vector<apollota::SimplePoint> points=apollota::RollingTopology::construct_rolling_strip_approximation(rolling_descriptor, (*strip_it), 0.05);
					opengl_printer.add_color(0xFFFF00);
					opengl_printer.add_line_strip(points);
				}
			}
			else if(rolling_descriptor.detached)
			{
				const std::vector<apollota::SimplePoint> points=apollota::RollingTopology::construct_rolling_circle_approximation(rolling_descriptor, 0.05);
				opengl_printer.add_color(0xFFFF00);
				opengl_printer.add_line_strip(points);
			}
		}
	}

	opengl_printer.print_pymol_script("presurface", false, std::cout);
}
