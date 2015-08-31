#include "apollota/spheres_boundary_construction.h"
#include "apollota/triangulation_queries.h"
#include "apollota/rolling_topology.h"
#include "apollota/constrained_contact_remainder.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/opengl_printer.h"
#include "auxiliaries/program_options_handler.h"

namespace
{

class SubdividedSphericalTriangle
{
public:
	SubdividedSphericalTriangle(
			const apollota::SimpleSphere& tangent,
			const apollota::SimpleSphere& a,
			const apollota::SimpleSphere& b,
			const apollota::SimpleSphere& c,
			const int steps) : center(tangent), probe(tangent.r)
	{
		Triangle t;
		t.p[0]=(center+((apollota::SimplePoint(a)-center).unit()*probe));
		t.p[1]=(center+((apollota::SimplePoint(b)-center).unit()*probe));
		t.p[2]=(center+((apollota::SimplePoint(c)-center).unit()*probe));
		triangles.push_back(t);
		subdivide(steps);
	}

	void cut(const std::vector<apollota::SimpleSphere>& cutting_spheres)
	{
		if(!cutting_spheres.empty())
		{
			apollota::ConstrainedContactRemainder::Remainder remainder;
			for(std::vector<Triangle>::const_iterator triangle_it=triangles.begin();triangle_it!=triangles.end();++triangle_it)
			{
				const Triangle& t=(*triangle_it);
				remainder.push_back(apollota::ConstrainedContactRemainder::TriangleRecord(t.p[0], t.p[1], t.p[2], 0, 0, 0));
			}
			for(std::vector<apollota::SimpleSphere>::const_iterator cutting_sphere_it=cutting_spheres.begin();cutting_sphere_it!=cutting_spheres.end();++cutting_sphere_it)
			{
				const apollota::SimpleSphere& cutting_sphere=(*cutting_sphere_it);
				if(apollota::greater(apollota::distance_from_point_to_point(center, cutting_sphere), 0.0))
				{

					apollota::ConstrainedContactRemainder::cut_contact_remainder(cutting_sphere, std::make_pair(1000, apollota::SimpleSphere(center, probe)), remainder);
				}
			}
			std::vector<Triangle> new_triangles;
			new_triangles.reserve(remainder.size());
			for(apollota::ConstrainedContactRemainder::Remainder::const_iterator remainder_it=remainder.begin();remainder_it!=remainder.end();++remainder_it)
			{
				const apollota::ConstrainedContactRemainder::TriangleRecord& tr=(*remainder_it);
				Triangle t;
				t.p[0]=tr.p[0];
				t.p[1]=tr.p[1];
				t.p[2]=tr.p[2];
				new_triangles.push_back(t);
			}
			triangles=new_triangles;
		}
	}

	void draw(auxiliaries::OpenGLPrinter& opengl_printer)
	{
		std::vector<apollota::SimplePoint> vertices(3);
		std::vector<apollota::SimplePoint> normals(3);
		for(std::vector<Triangle>::const_iterator triangle_it=triangles.begin();triangle_it!=triangles.end();++triangle_it)
		{
			const Triangle& t=(*triangle_it);
			vertices[0]=t.p[0];
			vertices[1]=t.p[1];
			vertices[2]=t.p[2];
			for(int i=0;i<3;i++)
			{
				normals[i]=(center-vertices[i]).unit();
			}
			opengl_printer.add_triangle_strip(vertices, normals);
		}
	}

private:
	struct Triangle
	{
		apollota::SimplePoint p[3];
	};

	void subdivide(const int steps)
	{
		for(int step=0;step<steps;step++)
		{
			std::vector<Triangle> new_triangles;
			new_triangles.reserve(triangles.size()*4);
			for(std::vector<Triangle>::const_iterator triangle_it=triangles.begin();triangle_it!=triangles.end();++triangle_it)
			{
				const Triangle& t=(*triangle_it);
				Triangle m;
				m.p[0]=apollota::RollingTopology::construct_circular_arc_approximation_from_start_and_end(center, t.p[0]-center, t.p[1]-center, 2)[1];
				m.p[1]=apollota::RollingTopology::construct_circular_arc_approximation_from_start_and_end(center, t.p[0]-center, t.p[2]-center, 2)[1];
				m.p[2]=apollota::RollingTopology::construct_circular_arc_approximation_from_start_and_end(center, t.p[1]-center, t.p[2]-center, 2)[1];
				new_triangles.push_back(m);
				{
					Triangle s=m;
					s.p[2]=t.p[0];
					new_triangles.push_back(s);
				}
				{
					Triangle s=m;
					s.p[1]=t.p[1];
					new_triangles.push_back(s);
				}
				{
					Triangle s=m;
					s.p[0]=t.p[2];
					new_triangles.push_back(s);
				}
			}
			triangles=new_triangles;
		}
	}

	apollota::SimplePoint center;
	double probe;
	std::vector<Triangle> triangles;
};

class SubdividedToricQuadrangle
{
public:
	SubdividedToricQuadrangle(
			const apollota::SimpleSphere& tangent1,
			const apollota::SimpleSphere& tangent2,
			const apollota::SimpleSphere& a,
			const apollota::SimpleSphere& b,
			const int steps) : centers(apollota::SimplePoint(tangent1), apollota::SimplePoint(tangent2))
	{
		points.first=apollota::RollingTopology::construct_circular_arc_approximation_from_start_and_end(
				centers.first,
				(apollota::SimplePoint(a)-centers.first).unit()*tangent1.r,
				(apollota::SimplePoint(b)-centers.first).unit()*tangent1.r,
				steps);
		points.second=apollota::RollingTopology::construct_circular_arc_approximation_from_start_and_end(
				centers.second,
				(apollota::SimplePoint(a)-centers.second).unit()*tangent2.r,
				(apollota::SimplePoint(b)-centers.second).unit()*tangent2.r,
				steps);
	}

	void draw(auxiliaries::OpenGLPrinter& opengl_printer)
	{
		std::vector<apollota::SimplePoint> vertices;
		vertices.reserve(points.first.size()*2);
		std::vector<apollota::SimplePoint> normals;
		normals.reserve(vertices.size());
		for(std::size_t i=0;i<std::min(points.first.size(), points.second.size());i++)
		{
			vertices.push_back(points.first[i]);
			vertices.push_back(points.second[i]);
			normals.push_back((centers.first-points.first[i]).unit());
			normals.push_back((centers.second-points.second[i]).unit());
		}
		opengl_printer.add_triangle_strip(vertices, normals);
	}

private:
	std::pair< apollota::SimplePoint, apollota::SimplePoint > centers;
	std::pair< std::vector<apollota::SimplePoint>, std::vector<apollota::SimplePoint> > points;
};

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

	std::vector<apollota::RollingTopology::RollingDescriptor> rolling_descriptors;
	for(apollota::TriangulationQueries::PairsMap::const_iterator pairs_map_it=pairs_map.begin();pairs_map_it!=pairs_map.end();++pairs_map_it)
	{
		const apollota::Pair& pair=pairs_map_it->first;
		const std::set<std::size_t>& neighbor_ids=pairs_map_it->second;
		const apollota::RollingTopology::RollingDescriptor rolling_descriptor=
				apollota::RollingTopology::calculate_rolling_descriptor(spheres, pair.get(0), pair.get(1), neighbor_ids, singles_map[pair.get(0)], singles_map[pair.get(1)], probe);
		if(rolling_descriptor.possible && (!rolling_descriptor.strips.empty() || rolling_descriptor.detached))
		{
			rolling_descriptors.push_back(rolling_descriptor);
		}
	}

	std::map<apollota::Triple, std::vector<apollota::SimpleSphere> > map_of_triples_cutting_spheres;
	for(std::vector<apollota::RollingTopology::RollingDescriptor>::const_iterator rolling_descriptor_it=rolling_descriptors.begin();rolling_descriptor_it!=rolling_descriptors.end();++rolling_descriptor_it)
	{
		const apollota::RollingTopology::RollingDescriptor& rolling_descriptor=(*rolling_descriptor_it);
		if(!rolling_descriptor.strips.empty() && !rolling_descriptor.breaks.empty())
		{
			for(std::list<apollota::RollingTopology::RollingStrip>::const_iterator strip_it=rolling_descriptor.strips.begin();strip_it!=rolling_descriptor.strips.end();++strip_it)
			{
				map_of_triples_cutting_spheres[apollota::Triple(rolling_descriptor.a_id, rolling_descriptor.b_id, strip_it->start.generator)].push_back(strip_it->end.tangent);
				map_of_triples_cutting_spheres[apollota::Triple(rolling_descriptor.a_id, rolling_descriptor.b_id, strip_it->end.generator)].push_back(strip_it->start.tangent);
			}
		}
	}

	for(std::vector<apollota::RollingTopology::RollingDescriptor>::const_iterator rolling_descriptor_it=rolling_descriptors.begin();rolling_descriptor_it!=rolling_descriptors.end();++rolling_descriptor_it)
	{
		const apollota::RollingTopology::RollingDescriptor& rolling_descriptor=(*rolling_descriptor_it);
		if(!rolling_descriptor.strips.empty())
		{
			for(std::list<apollota::RollingTopology::RollingStrip>::const_iterator strip_it=rolling_descriptor.strips.begin();strip_it!=rolling_descriptor.strips.end();++strip_it)
			{
				opengl_printer.add_color(rolling_descriptor.breaks.empty() ? 0xFFFF00 : 0xFF0000);
				{
					const std::vector<apollota::SimplePoint> points=apollota::RollingTopology::construct_rolling_strip_approximation(rolling_descriptor, (*strip_it), 0.1);
					for(std::size_t i=0;i+1<points.size();i++)
					{
						if(rolling_descriptor.breaks.size()==2)
						{
							SubdividedToricQuadrangle stq1(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), spheres[rolling_descriptor.a_id], apollota::SimpleSphere(rolling_descriptor.breaks[0], 0.0), 4);
							SubdividedToricQuadrangle stq2(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), apollota::SimpleSphere(rolling_descriptor.breaks[1], 0.0), spheres[rolling_descriptor.b_id], 4);
							stq1.draw(opengl_printer);
							stq2.draw(opengl_printer);
						}
						else
						{
							SubdividedToricQuadrangle stq(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), spheres[rolling_descriptor.a_id], spheres[rolling_descriptor.b_id], 8);
							stq.draw(opengl_printer);
						}
					}
				}

				opengl_printer.add_color(0x00FFFF);
				if(strip_it->start.generator<std::min(rolling_descriptor.a_id, rolling_descriptor.b_id))
				{
					const std::vector<apollota::SimpleSphere>& cutting_spheres=map_of_triples_cutting_spheres[apollota::Triple(strip_it->start.generator, rolling_descriptor.a_id, rolling_descriptor.b_id)];
					SubdividedSphericalTriangle sst(strip_it->start.tangent, spheres[rolling_descriptor.a_id], spheres[rolling_descriptor.b_id], spheres[strip_it->start.generator], 3);
					sst.cut(cutting_spheres);
					sst.draw(opengl_printer);
				}
				if(strip_it->end.generator<std::min(rolling_descriptor.a_id, rolling_descriptor.b_id))
				{
					const std::vector<apollota::SimpleSphere>& cutting_spheres=map_of_triples_cutting_spheres[apollota::Triple(strip_it->end.generator, rolling_descriptor.a_id, rolling_descriptor.b_id)];
					SubdividedSphericalTriangle sst(strip_it->end.tangent, spheres[rolling_descriptor.a_id], spheres[rolling_descriptor.b_id], spheres[strip_it->end.generator], 3);
					sst.cut(cutting_spheres);
					sst.draw(opengl_printer);
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

	opengl_printer.print_pymol_script("presurface", false, std::cout);
}
