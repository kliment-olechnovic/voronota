#include "apollota/spheres_boundary_construction.h"
#include "apollota/triangulation_queries.h"
#include "apollota/rolling_topology.h"
#include "apollota/constrained_contact_remainder.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/opengl_printer.h"
#include "auxiliaries/program_options_handler.h"

namespace
{

class SubdividedSphericalTriangulation
{
public:
	SubdividedSphericalTriangulation(
			const apollota::SimpleSphere& tangent,
			const apollota::SimpleSphere& a,
			const apollota::SimpleSphere& b,
			const apollota::SimpleSphere& c,
			const std::vector<apollota::SimplePoint>& breaks,
			const int steps) : center(tangent), probe(tangent.r)
	{
		Triangle t;
		t.p[0]=(center+((apollota::SimplePoint(a)-center).unit()*probe));
		t.p[1]=(center+((apollota::SimplePoint(b)-center).unit()*probe));
		t.p[2]=(center+((apollota::SimplePoint(c)-center).unit()*probe));
		apollota::SimplePoint mp=(t.p[0]+t.p[1]+t.p[2])*(1/3.0);
		mp=apollota::SimplePoint(tangent)+(apollota::sub_of_points<apollota::SimplePoint>(mp, center).unit()*tangent.r);
		if(breaks.size()==2)
		{
			triangles.push_back(Triangle(t.p[0], breaks[0], mp));
			triangles.push_back(Triangle(breaks[0], breaks[1], mp));
			triangles.push_back(Triangle(breaks[1], t.p[1], mp));
		}
		else
		{
			triangles.push_back(Triangle(t.p[0], t.p[1], mp));
		}
		subdivide(steps);
	}

	template<typename ContainerOfSpheres>
	void cut(const ContainerOfSpheres& cutting_spheres)
	{
		if(!cutting_spheres.empty())
		{
			const apollota::SimpleSphere center_sphere(center, probe);
			for(typename ContainerOfSpheres::const_iterator cutting_sphere_it=cutting_spheres.begin();cutting_sphere_it!=cutting_spheres.end();++cutting_sphere_it)
			{
				const apollota::SimpleSphere& cutting_sphere=(*cutting_sphere_it);
				if(!(center_sphere==cutting_sphere) && used_cutting_spheres.count(cutting_sphere)==0 && apollota::sphere_intersects_sphere(center_sphere, cutting_sphere))
				{
					used_cutting_spheres.insert(cutting_sphere);
					const apollota::SimpleSphere intersection_circle=apollota::intersection_circle_of_two_spheres<apollota::SimpleSphere>(center_sphere, cutting_sphere);
					const apollota::SimplePoint plane_normal=apollota::sub_of_points<apollota::SimplePoint>(center_sphere, cutting_sphere).unit();
					std::list<Triangle>::iterator triangle_it=triangles.begin();
					while(triangle_it!=triangles.end())
					{
						const Triangle t=(*triangle_it);
						int h[3]={0,0,0};
						for(int i=0;i<3;i++)
						{
							h[i]=apollota::halfspace_of_point(intersection_circle, plane_normal, t.p[i]);
						}
						if(h[0]>=0 && h[1]>=0 && h[2]>=0)
						{
							++triangle_it;
						}
						else
						{
							{
								std::list<Triangle>::iterator deletion_triangle_it=triangle_it;
								++triangle_it;
								triangles.erase(deletion_triangle_it);
							}
							if(h[0]>0 || h[1]>0 || h[2]>0)
							{
								std::vector<apollota::SimplePoint> points_in;
								std::vector<apollota::SimplePoint> points_on;
								for(int i=0;i<3;i++)
								{
									if(h[i]>0)
									{
										points_in.push_back(t.p[i]);
									}
									else if(h[i]==0)
									{
										points_on.push_back(t.p[i]);
									}
								}
								for(int i=0;i<3;i++)
								{
									for(int j=0;j<3;j++)
									{
										if(i!=j && h[i]>0 && h[j]<0)
										{
											points_on.push_back(apollota::intersection_of_plane_and_segment<apollota::SimplePoint>(intersection_circle, plane_normal, t.p[i], t.p[j]));
										}
									}
								}
								if((points_in.size()==1 || points_in.size()==2) && points_on.size()==2)
								{
									for(std::size_t i=0;i<points_on.size();i++)
									{
										points_on[i]=apollota::sum_of_points<apollota::SimplePoint>(intersection_circle, apollota::sub_of_points<apollota::SimplePoint>(points_on[i], intersection_circle).unit()*intersection_circle.r);
									}
									if(points_in.size()==1)
									{
										triangles.insert(triangle_it, Triangle(points_in[0], points_on[0], points_on[1]));
									}
									else if(points_in.size()==2)
									{
										triangles.insert(triangle_it, Triangle(points_in[0], points_on[0], points_on[1]));
										triangles.insert(triangle_it, Triangle(points_in[0], points_in[1], points_on[1]));
									}
								}
							}
						}
					}
				}
			}
		}
	}

	void draw(auxiliaries::OpenGLPrinter& opengl_printer, const bool concave)
	{
		std::vector<apollota::SimplePoint> vertices(3);
		std::vector<apollota::SimplePoint> normals(3);
		for(std::list<Triangle>::const_iterator triangle_it=triangles.begin();triangle_it!=triangles.end();++triangle_it)
		{
			const Triangle& t=(*triangle_it);
			vertices[0]=t.p[0];
			vertices[1]=t.p[1];
			vertices[2]=t.p[2];
			for(int i=0;i<3;i++)
			{
				normals[i]=(vertices[i]-center).unit();
				if(concave)
				{
					normals[i]=normals[i].inverted();
				}
			}
			opengl_printer.add_triangle_strip(vertices, normals);
		}
	}

private:
	struct Triangle
	{
		apollota::SimplePoint p[3];

		Triangle()
		{
		}

		Triangle(const apollota::SimplePoint& a, const apollota::SimplePoint& b, const apollota::SimplePoint& c)
		{
			p[0]=a;
			p[1]=b;
			p[2]=c;
		}
	};

	void subdivide(const int steps)
	{
		for(int step=0;step<steps;step++)
		{
			std::list<Triangle> new_triangles;
			for(std::list<Triangle>::const_iterator triangle_it=triangles.begin();triangle_it!=triangles.end();++triangle_it)
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
	std::list<Triangle> triangles;
	std::set<apollota::SimpleSphere> used_cutting_spheres;
};

class SubdividedToricQuadrangulation
{
public:
	SubdividedToricQuadrangulation(
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
	pohw.describe_io("stdout", false, true, "nothing");

	const double probe=poh.restrict_value_in_range(0.01, 14.0, poh.argument<double>(pohw.describe_option("--probe", "number", "probe radius"), 1.4));
	const double angle_step=poh.restrict_value_in_range(0.01, 1.0, poh.argument<double>(pohw.describe_option("--angle-step", "number", "angle step in radians for circle approximation"), 0.2));
	const int depth=poh.restrict_value_in_range(1, 4, poh.argument<int>(pohw.describe_option("--depth", "number", "triangular patches subdivision depth"), 2));
	const double alpha=poh.argument<double>(pohw.describe_option("--alpha", "number", "alpha opacity value for drawing output"), 1.0);
	const std::string drawing_for_pymol=poh.argument<std::string>(pohw.describe_option("--drawing-for-pymol", "string", "file path to output drawing as pymol script"), "");
	const std::string drawing_for_scenejs=poh.argument<std::string>(pohw.describe_option("--drawing-for-scenejs", "string", "file path to output drawing as scenejs script"), "");
	const std::string drawing_name=poh.argument<std::string>(pohw.describe_option("--drawing-name", "string", "graphics object name for drawing output"), "ses");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const int parts_from_depth=(1 << depth);

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

	std::vector< std::set<apollota::SimpleSphere> > map_of_generators_cutting_spheres(spheres.size());
	for(std::vector<apollota::RollingTopology::RollingDescriptor>::const_iterator rolling_descriptor_it=rolling_descriptors.begin();rolling_descriptor_it!=rolling_descriptors.end();++rolling_descriptor_it)
	{
		const apollota::RollingTopology::RollingDescriptor& rolling_descriptor=(*rolling_descriptor_it);
		if(!rolling_descriptor.strips.empty())
		{
			for(std::list<apollota::RollingTopology::RollingStrip>::const_iterator strip_it=rolling_descriptor.strips.begin();strip_it!=rolling_descriptor.strips.end();++strip_it)
			{
				map_of_generators_cutting_spheres[rolling_descriptor.a_id].insert(strip_it->start.tangent);
				map_of_generators_cutting_spheres[rolling_descriptor.b_id].insert(strip_it->start.tangent);
				map_of_generators_cutting_spheres[strip_it->start.generator].insert(strip_it->start.tangent);
				map_of_generators_cutting_spheres[strip_it->end.generator].insert(strip_it->start.tangent);
				map_of_generators_cutting_spheres[rolling_descriptor.a_id].insert(strip_it->end.tangent);
				map_of_generators_cutting_spheres[rolling_descriptor.b_id].insert(strip_it->end.tangent);
				map_of_generators_cutting_spheres[strip_it->start.generator].insert(strip_it->end.tangent);
				map_of_generators_cutting_spheres[strip_it->end.generator].insert(strip_it->end.tangent);
			}
		}
	}

	auxiliaries::OpenGLPrinter opengl_printer;
	opengl_printer.add_alpha(alpha);

	for(std::vector<apollota::RollingTopology::RollingDescriptor>::const_iterator rolling_descriptor_it=rolling_descriptors.begin();rolling_descriptor_it!=rolling_descriptors.end();++rolling_descriptor_it)
	{
		const apollota::RollingTopology::RollingDescriptor& rolling_descriptor=(*rolling_descriptor_it);
		if(!rolling_descriptor.strips.empty())
		{
			for(std::list<apollota::RollingTopology::RollingStrip>::const_iterator strip_it=rolling_descriptor.strips.begin();strip_it!=rolling_descriptor.strips.end();++strip_it)
			{
				opengl_printer.add_color(rolling_descriptor.breaks.empty() ? 0xFFFF00 : 0x77FF00);
				{
					const std::vector<apollota::SimplePoint> points=apollota::RollingTopology::construct_rolling_strip_approximation(rolling_descriptor, (*strip_it), angle_step);
					for(std::size_t i=0;i+1<points.size();i++)
					{
						if(rolling_descriptor.breaks.size()==2)
						{
							SubdividedToricQuadrangulation stq1(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), spheres[rolling_descriptor.a_id], apollota::SimpleSphere(rolling_descriptor.breaks[0], 0.0), parts_from_depth);
							SubdividedToricQuadrangulation stq2(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), apollota::SimpleSphere(rolling_descriptor.breaks[1], 0.0), spheres[rolling_descriptor.b_id], parts_from_depth);
							stq1.draw(opengl_printer);
							stq2.draw(opengl_printer);
						}
						else
						{
							SubdividedToricQuadrangulation stq(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), spheres[rolling_descriptor.a_id], spheres[rolling_descriptor.b_id], parts_from_depth);
							stq.draw(opengl_printer);
						}
					}
				}

				opengl_printer.add_color(0x00FFFF);
				{
					SubdividedSphericalTriangulation sst(strip_it->start.tangent, spheres[rolling_descriptor.a_id], spheres[rolling_descriptor.b_id], spheres[strip_it->start.generator], rolling_descriptor.breaks, depth);
					sst.cut(map_of_generators_cutting_spheres[strip_it->start.generator]);
					sst.cut(map_of_generators_cutting_spheres[rolling_descriptor.a_id]);
					sst.cut(map_of_generators_cutting_spheres[rolling_descriptor.b_id]);
					sst.draw(opengl_printer, true);
				}
				{
					SubdividedSphericalTriangulation sst(strip_it->end.tangent, spheres[rolling_descriptor.a_id], spheres[rolling_descriptor.b_id], spheres[strip_it->end.generator], rolling_descriptor.breaks, depth);
					sst.cut(map_of_generators_cutting_spheres[strip_it->end.generator]);
					sst.cut(map_of_generators_cutting_spheres[rolling_descriptor.a_id]);
					sst.cut(map_of_generators_cutting_spheres[rolling_descriptor.b_id]);
					sst.draw(opengl_printer, true);
				}
			}
		}
		else if(rolling_descriptor.detached)
		{
			const std::vector<apollota::SimplePoint> points=apollota::RollingTopology::construct_rolling_circle_approximation(rolling_descriptor, angle_step);
			opengl_printer.add_color(rolling_descriptor.breaks.empty() ? 0x00FF00 : 0x00FF77);
			for(std::size_t i=0;i+1<points.size();i++)
			{
				if(rolling_descriptor.breaks.size()==2)
				{
					SubdividedToricQuadrangulation stq1(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), spheres[rolling_descriptor.a_id], apollota::SimpleSphere(rolling_descriptor.breaks[0], 0.0), parts_from_depth);
					SubdividedToricQuadrangulation stq2(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), apollota::SimpleSphere(rolling_descriptor.breaks[1], 0.0), spheres[rolling_descriptor.b_id], parts_from_depth);
					stq1.draw(opengl_printer);
					stq2.draw(opengl_printer);
				}
				else
				{
					SubdividedToricQuadrangulation stq(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), spheres[rolling_descriptor.a_id], spheres[rolling_descriptor.b_id], parts_from_depth);
					stq.draw(opengl_printer);
				}
			}
		}
	}

	if(!drawing_for_pymol.empty())
	{
		std::ofstream foutput(drawing_for_pymol.c_str(), std::ios::out);
		if(foutput.good())
		{
			opengl_printer.print_pymol_script(drawing_name, false, foutput);
		}
	}

	if(!drawing_for_scenejs.empty())
	{
		std::ofstream foutput(drawing_for_scenejs.c_str(), std::ios::out);
		if(foutput.good())
		{
			opengl_printer.print_scenejs_script(drawing_name, true, foutput);
		}
	}
}
