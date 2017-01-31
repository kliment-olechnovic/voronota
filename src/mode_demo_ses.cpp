#include "apollota/spheres_boundary_construction.h"
#include "apollota/triangulation_queries.h"
#include "apollota/rolling_topology.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/opengl_printer.h"
#include "auxiliaries/program_options_handler.h"

namespace
{

class SubdividedSphericalTriangulation
{
public:
	SubdividedSphericalTriangulation(const int steps) : center_sphere_(0, 0, 0, 1)
	{
		const double t=(1+sqrt(5.0))/2.0;

		std::vector<apollota::SimplePoint> vertices;
		vertices.reserve(12);
		vertices.push_back(apollota::SimplePoint( t, 1, 0).unit());
		vertices.push_back(apollota::SimplePoint(-t, 1, 0).unit());
		vertices.push_back(apollota::SimplePoint( t,-1, 0).unit());
		vertices.push_back(apollota::SimplePoint(-t,-1, 0).unit());
		vertices.push_back(apollota::SimplePoint( 1, 0, t).unit());
		vertices.push_back(apollota::SimplePoint( 1, 0,-t).unit());
		vertices.push_back(apollota::SimplePoint(-1, 0, t).unit());
		vertices.push_back(apollota::SimplePoint(-1, 0,-t).unit());
		vertices.push_back(apollota::SimplePoint( 0, t, 1).unit());
		vertices.push_back(apollota::SimplePoint( 0,-t, 1).unit());
		vertices.push_back(apollota::SimplePoint( 0, t,-1).unit());
		vertices.push_back(apollota::SimplePoint( 0,-t,-1).unit());

		triangles_.push_back(Triangle(vertices[0], vertices[8], vertices[4]));
		triangles_.push_back(Triangle(vertices[1], vertices[10], vertices[7]));
		triangles_.push_back(Triangle(vertices[2], vertices[9], vertices[11]));
		triangles_.push_back(Triangle(vertices[7], vertices[3], vertices[1]));
		triangles_.push_back(Triangle(vertices[0], vertices[5], vertices[10]));
		triangles_.push_back(Triangle(vertices[3], vertices[9], vertices[6]));
		triangles_.push_back(Triangle(vertices[3], vertices[11], vertices[9]));
		triangles_.push_back(Triangle(vertices[8], vertices[6], vertices[4]));
		triangles_.push_back(Triangle(vertices[2], vertices[4], vertices[9]));
		triangles_.push_back(Triangle(vertices[3], vertices[7], vertices[11]));
		triangles_.push_back(Triangle(vertices[4], vertices[2], vertices[0]));
		triangles_.push_back(Triangle(vertices[9], vertices[4], vertices[6]));
		triangles_.push_back(Triangle(vertices[2], vertices[11], vertices[5]));
		triangles_.push_back(Triangle(vertices[0], vertices[10], vertices[8]));
		triangles_.push_back(Triangle(vertices[5], vertices[0], vertices[2]));
		triangles_.push_back(Triangle(vertices[10], vertices[5], vertices[7]));
		triangles_.push_back(Triangle(vertices[1], vertices[6], vertices[8]));
		triangles_.push_back(Triangle(vertices[1], vertices[8], vertices[10]));
		triangles_.push_back(Triangle(vertices[6], vertices[1], vertices[3]));
		triangles_.push_back(Triangle(vertices[11], vertices[7], vertices[5]));

		subdivide(steps);
	}

	SubdividedSphericalTriangulation(
			const apollota::SimpleSphere& tangent,
			const apollota::SimpleSphere& a,
			const apollota::SimpleSphere& b,
			const apollota::SimpleSphere& c,
			const std::vector<apollota::SimplePoint>& breaks,
			const int steps) : center_sphere_(tangent)
	{
		const apollota::SimplePoint center_point(center_sphere_);

		Triangle t;
		t.p[0]=(center_point+((apollota::SimplePoint(a)-center_point).unit()*tangent.r));
		t.p[1]=(center_point+((apollota::SimplePoint(b)-center_point).unit()*tangent.r));
		t.p[2]=(center_point+((apollota::SimplePoint(c)-center_point).unit()*tangent.r));

		apollota::SimplePoint t_mp=(t.p[0]+t.p[1]+t.p[2])*(1/3.0);
		t_mp=apollota::SimplePoint(tangent)+((t_mp-center_point).unit()*tangent.r);

		const apollota::SimplePoint t_side_mp=apollota::RollingTopology::construct_circular_arc_approximation_from_start_and_end(center_point, t.p[0]-center_point, t.p[1]-center_point, 2)[1];

		if(breaks.size()==2)
		{
			triangles_.push_back(Triangle(t.p[0], breaks[0], t_mp));
			triangles_.push_back(Triangle(breaks[0], t_side_mp, t_mp));
			triangles_.push_back(Triangle(t_side_mp, breaks[1], t_mp));
			triangles_.push_back(Triangle(breaks[1], t.p[1], t_mp));
		}
		else
		{
			triangles_.push_back(Triangle(t.p[0], t_side_mp, t_mp));
			triangles_.push_back(Triangle(t_side_mp, t.p[1], t_mp));
		}

		subdivide(steps-1);
	}

	const apollota::SimpleSphere& center_sphere() const
	{
		return center_sphere_;
	}

	template<typename InputPointType>
	void transform(const InputPointType& new_center_point, const double scale)
	{
		const apollota::SimplePoint old_center_point(center_sphere_);
		center_sphere_=apollota::SimpleSphere(new_center_point, center_sphere_.r*scale);
		for(std::list<Triangle>::iterator triangle_it=triangles_.begin();triangle_it!=triangles_.end();++triangle_it)
		{
			Triangle& t=(*triangle_it);
			for(int i=0;i<3;i++)
			{
				t.p[i]=apollota::sum_of_points<apollota::SimplePoint>(new_center_point, ((t.p[i]-old_center_point)*scale));
			}
		}
	}

	template<typename ContainerOfSpheres>
	void cut(const ContainerOfSpheres& cutting_spheres)
	{
		if(!cutting_spheres.empty())
		{
			for(typename ContainerOfSpheres::const_iterator cutting_sphere_it=cutting_spheres.begin();cutting_sphere_it!=cutting_spheres.end();++cutting_sphere_it)
			{
				const apollota::SimpleSphere& cutting_sphere=(*cutting_sphere_it);
				if(!(center_sphere_==cutting_sphere) && used_cutting_spheres_.count(cutting_sphere)==0 && apollota::sphere_intersects_sphere(center_sphere_, cutting_sphere))
				{
					used_cutting_spheres_.insert(cutting_sphere);
					const apollota::SimpleSphere intersection_circle=apollota::intersection_circle_of_two_spheres<apollota::SimpleSphere>(center_sphere_, cutting_sphere);
					const apollota::SimplePoint plane_normal=apollota::sub_of_points<apollota::SimplePoint>(center_sphere_, cutting_sphere).unit();
					std::list<Triangle>::iterator triangle_it=triangles_.begin();
					while(triangle_it!=triangles_.end())
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
								triangles_.erase(deletion_triangle_it);
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
										triangles_.insert(triangle_it, Triangle(points_in[0], points_on[0], points_on[1]));
									}
									else if(points_in.size()==2)
									{
										triangles_.insert(triangle_it, Triangle(points_in[0], points_on[0], points_on[1]));
										triangles_.insert(triangle_it, Triangle(points_in[0], points_in[1], points_on[1]));
									}
								}
							}
						}
					}
				}
			}
		}
	}

	void draw(auxiliaries::OpenGLPrinter& opengl_printer, const bool concave, const bool mesh)
	{
		std::vector<apollota::SimplePoint> vertices(3);
		std::vector<apollota::SimplePoint> normals(3);
		for(std::list<Triangle>::const_iterator triangle_it=triangles_.begin();triangle_it!=triangles_.end();++triangle_it)
		{
			const Triangle& t=(*triangle_it);
			vertices[0]=t.p[0];
			vertices[1]=t.p[1];
			vertices[2]=t.p[2];
			for(int i=0;i<3;i++)
			{
				normals[i]=apollota::sub_of_points<apollota::SimplePoint>(vertices[i], center_sphere_).unit();
				if(concave)
				{
					normals[i]=normals[i].inverted();
				}
			}
			if(mesh)
			{
				opengl_printer.add_line_loop(vertices);
			}
			else
			{
				opengl_printer.add_triangle_strip(vertices, normals);
			}
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
		const apollota::SimplePoint center_point(center_sphere_);
		for(int step=0;step<steps;step++)
		{
			std::list<Triangle> new_triangles;
			for(std::list<Triangle>::const_iterator triangle_it=triangles_.begin();triangle_it!=triangles_.end();++triangle_it)
			{
				const Triangle& t=(*triangle_it);
				Triangle m;
				m.p[0]=apollota::RollingTopology::construct_circular_arc_approximation_from_start_and_end(center_point, t.p[0]-center_point, t.p[1]-center_point, 2)[1];
				m.p[1]=apollota::RollingTopology::construct_circular_arc_approximation_from_start_and_end(center_point, t.p[0]-center_point, t.p[2]-center_point, 2)[1];
				m.p[2]=apollota::RollingTopology::construct_circular_arc_approximation_from_start_and_end(center_point, t.p[1]-center_point, t.p[2]-center_point, 2)[1];
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
			triangles_=new_triangles;
		}
	}

	apollota::SimpleSphere center_sphere_;
	std::list<Triangle> triangles_;
	std::set<apollota::SimpleSphere> used_cutting_spheres_;
};

class SubdividedToricQuadrangulation
{
public:
	SubdividedToricQuadrangulation(
			const apollota::SimpleSphere& tangent1,
			const apollota::SimpleSphere& tangent2,
			const apollota::SimpleSphere& a,
			const apollota::SimpleSphere& b,
			const int steps) : centers_(apollota::SimplePoint(tangent1), apollota::SimplePoint(tangent2))
	{
		points_.first=apollota::RollingTopology::construct_circular_arc_approximation_from_start_and_end(
				centers_.first,
				(apollota::SimplePoint(a)-centers_.first).unit()*tangent1.r,
				(apollota::SimplePoint(b)-centers_.first).unit()*tangent1.r,
				steps);
		points_.second=apollota::RollingTopology::construct_circular_arc_approximation_from_start_and_end(
				centers_.second,
				(apollota::SimplePoint(a)-centers_.second).unit()*tangent2.r,
				(apollota::SimplePoint(b)-centers_.second).unit()*tangent2.r,
				steps);
	}

	void draw(auxiliaries::OpenGLPrinter& opengl_printer, const bool mesh)
	{
		std::vector<apollota::SimplePoint> vertices;
		vertices.reserve(points_.first.size()*2);
		std::vector<apollota::SimplePoint> normals;
		normals.reserve(vertices.size());
		for(std::size_t i=0;i<std::min(points_.first.size(), points_.second.size());i++)
		{
			vertices.push_back(points_.first[i]);
			vertices.push_back(points_.second[i]);
			normals.push_back((centers_.first-points_.first[i]).unit());
			normals.push_back((centers_.second-points_.second[i]).unit());
		}
		if(mesh)
		{
			std::vector<apollota::SimplePoint> q(4);
			for(std::size_t i=0;i+3<vertices.size();i+=2)
			{
				q[0]=vertices[i];
				q[1]=vertices[i+1];
				q[2]=vertices[i+3];
				q[3]=vertices[i+2];
				opengl_printer.add_line_loop(q);
			}
		}
		else
		{
			opengl_printer.add_triangle_strip(vertices, normals);
		}
	}

private:
	std::pair< apollota::SimplePoint, apollota::SimplePoint > centers_;
	std::pair< std::vector<apollota::SimplePoint>, std::vector<apollota::SimplePoint> > points_;
};

}

void demo_ses(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'x y z r')");
	pohw.describe_io("stdout", false, true, "nothing");

	const double probe=poh.restrict_value_in_range(0.01, 14.0, poh.argument<double>(pohw.describe_option("--probe", "number", "probe radius"), 1.4));
	const double angle_step=poh.restrict_value_in_range(0.01, 1.0, poh.argument<double>(pohw.describe_option("--angle-step", "number", "angle step in radians for circle approximation"), 0.2));
	const int depth=poh.restrict_value_in_range(1, 4, poh.argument<int>(pohw.describe_option("--depth", "number", "triangular patches subdivision depth"), 2));
	const int sih_depth=poh.restrict_value_in_range(0, 4, poh.argument<int>(pohw.describe_option("--sih-depth", "number", "spherical surface optimization depth"), 2));
	const double alpha=poh.argument<double>(pohw.describe_option("--alpha", "number", "alpha opacity value for drawing output"), 1.0);
	const std::string drawing_for_pymol=poh.argument<std::string>(pohw.describe_option("--drawing-for-pymol", "string", "file path to output drawing as pymol script"), "");
	const std::string drawing_for_scenejs=poh.argument<std::string>(pohw.describe_option("--drawing-for-scenejs", "string", "file path to output drawing as scenejs script"), "");
	const std::string drawing_name=poh.argument<std::string>(pohw.describe_option("--drawing-name", "string", "graphics object name for drawing output"), "ses");
	const bool mesh=poh.contains_option(pohw.describe_option("--mesh", "", "flag to draw mesh"));
	const bool trajectory=poh.contains_option(pohw.describe_option("--trajectory", "", "flag to draw rolling probe center trajectory"));
	const unsigned int solid_color=poh.convert_hex_string_to_integer<unsigned int>(poh.argument<std::string>(pohw.describe_option("--solid-color", "string", "default color for drawing output, in hex format, white is 0xFFFFFF"), "0x0"));

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

	std::set<std::size_t> set_of_all_generators;
	for(std::vector<apollota::RollingTopology::RollingDescriptor>::const_iterator rolling_descriptor_it=rolling_descriptors.begin();rolling_descriptor_it!=rolling_descriptors.end();++rolling_descriptor_it)
	{
		set_of_all_generators.insert(rolling_descriptor_it->a_id);
		set_of_all_generators.insert(rolling_descriptor_it->b_id);
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

	if(!trajectory)
	{
		const SubdividedSphericalTriangulation start_sst(sih_depth);
		for(apollota::TriangulationQueries::IDsMap::const_iterator singles_map_it=singles_map.begin();singles_map_it!=singles_map.end();++singles_map_it)
		{
			const std::set<std::size_t>& neighbors=singles_map_it->second;
			if(!neighbors.empty() && set_of_all_generators.count(singles_map_it->first)>0)
			{
				SubdividedSphericalTriangulation sst=start_sst;
				sst.transform(spheres[singles_map_it->first], spheres[singles_map_it->first].r+probe);
				std::set<apollota::SimpleSphere> cutting_spheres;
				for(std::set<std::size_t>::const_iterator neighbor_it=neighbors.begin();neighbor_it!=neighbors.end();++neighbor_it)
				{
					cutting_spheres.insert(apollota::SimpleSphere(spheres[*neighbor_it], spheres[*neighbor_it].r+probe));
				}
				sst.cut(cutting_spheres);
				sst.transform(sst.center_sphere(), (sst.center_sphere().r-probe)/sst.center_sphere().r);
				opengl_printer.add_color(solid_color>0 ? solid_color : 0xFF33FF);
				sst.draw(opengl_printer, false, mesh);
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
				const std::vector<apollota::SimplePoint> points=apollota::RollingTopology::construct_rolling_strip_approximation(rolling_descriptor, (*strip_it), angle_step);
				if(trajectory)
				{
					opengl_printer.add_color(solid_color>0 ? solid_color : 0xFF7700);
					opengl_printer.add_line_strip(points);
				}
				else
				{
					opengl_printer.add_color(solid_color>0 ? solid_color : (rolling_descriptor.breaks.empty() ? 0xFFFF00 : 0x77FF00));
					for(std::size_t i=0;i+1<points.size();i++)
					{
						if(rolling_descriptor.breaks.size()==2)
						{
							SubdividedToricQuadrangulation stq1(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), spheres[rolling_descriptor.a_id], apollota::SimpleSphere(rolling_descriptor.breaks[0], 0.0), parts_from_depth/2);
							SubdividedToricQuadrangulation stq2(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), apollota::SimpleSphere(rolling_descriptor.breaks[1], 0.0), spheres[rolling_descriptor.b_id], parts_from_depth/2);
							stq1.draw(opengl_printer, mesh);
							stq2.draw(opengl_printer, mesh);
						}
						else
						{
							SubdividedToricQuadrangulation stq(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), spheres[rolling_descriptor.a_id], spheres[rolling_descriptor.b_id], parts_from_depth);
							stq.draw(opengl_printer, mesh);
						}
					}

					opengl_printer.add_color(solid_color>0 ? solid_color : 0x00FFFF);
					{
						SubdividedSphericalTriangulation sst(strip_it->start.tangent, spheres[rolling_descriptor.a_id], spheres[rolling_descriptor.b_id], spheres[strip_it->start.generator], rolling_descriptor.breaks, depth);
						sst.cut(map_of_generators_cutting_spheres[strip_it->start.generator]);
						sst.cut(map_of_generators_cutting_spheres[rolling_descriptor.a_id]);
						sst.cut(map_of_generators_cutting_spheres[rolling_descriptor.b_id]);
						sst.draw(opengl_printer, true, mesh);
					}
					{
						SubdividedSphericalTriangulation sst(strip_it->end.tangent, spheres[rolling_descriptor.a_id], spheres[rolling_descriptor.b_id], spheres[strip_it->end.generator], rolling_descriptor.breaks, depth);
						sst.cut(map_of_generators_cutting_spheres[strip_it->end.generator]);
						sst.cut(map_of_generators_cutting_spheres[rolling_descriptor.a_id]);
						sst.cut(map_of_generators_cutting_spheres[rolling_descriptor.b_id]);
						sst.draw(opengl_printer, true, mesh);
					}
				}
			}
		}
		else if(rolling_descriptor.detached)
		{
			const std::vector<apollota::SimplePoint> points=apollota::RollingTopology::construct_rolling_circle_approximation(rolling_descriptor, angle_step);
			if(trajectory)
			{
				opengl_printer.add_color(solid_color>0 ? solid_color : 0xFF7700);
				opengl_printer.add_line_strip(points);
			}
			else
			{
				opengl_printer.add_color(solid_color>0 ? solid_color : (rolling_descriptor.breaks.empty() ? 0x00FF00 : 0x00FF77));
				for(std::size_t i=0;i+1<points.size();i++)
				{
					if(rolling_descriptor.breaks.size()==2)
					{
						SubdividedToricQuadrangulation stq1(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), spheres[rolling_descriptor.a_id], apollota::SimpleSphere(rolling_descriptor.breaks[0], 0.0), parts_from_depth/2);
						SubdividedToricQuadrangulation stq2(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), apollota::SimpleSphere(rolling_descriptor.breaks[1], 0.0), spheres[rolling_descriptor.b_id], parts_from_depth/2);
						stq1.draw(opengl_printer, mesh);
						stq2.draw(opengl_printer, mesh);
					}
					else
					{
						SubdividedToricQuadrangulation stq(apollota::SimpleSphere(points[i], probe), apollota::SimpleSphere(points[i+1], probe), spheres[rolling_descriptor.a_id], spheres[rolling_descriptor.b_id], parts_from_depth);
						stq.draw(opengl_printer, mesh);
					}
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
