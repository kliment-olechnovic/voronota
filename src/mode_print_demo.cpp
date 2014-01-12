#include <iostream>
#include <deque>

#include "apollota/triangulation.h"
#include "apollota/opengl_printer.h"
#include "apollota/contact_contour.h"
#include "apollota/contact_remainder.h"

#include "modes_commons.h"

namespace apollota
{

class UtilitiesForTriangulation
{
public:
	typedef std::tr1::unordered_map<std::size_t, std::set<std::size_t> > NeighborsMap;
	typedef std::vector< std::vector<std::size_t> > NeighborsGraph;

	template<typename T>
	static std::vector<SimpleSphere> collect_simple_spheres(const T& spheres)
	{
		std::vector<SimpleSphere> result;
		result.reserve(spheres.size());
		for(typename T::const_iterator it=spheres.begin();it!=spheres.end();++it)
		{
			result.push_back(SimpleSphere(*it));
		}
		return result;
	}

	static NeighborsMap collect_neighbors_map_from_quadruples_map(const Triangulation::QuadruplesMap& quadruples_map)
	{
		NeighborsMap neighbors_map;
		for(Triangulation::QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			const Quadruple& quadruple=it->first;
			for(int a=0;a<4;a++)
			{
				for(int b=a+1;b<4;b++)
				{
					neighbors_map[quadruple.get(a)].insert(quadruple.get(b));
					neighbors_map[quadruple.get(b)].insert(quadruple.get(a));
				}
			}
		}
		return neighbors_map;
	}

	static NeighborsGraph collect_neighbors_graph_from_neighbors_map(const NeighborsMap& neighbors_map, const std::size_t number_of_vertices)
	{
		NeighborsGraph neighbors_graph(number_of_vertices);
		for(NeighborsMap::const_iterator it=neighbors_map.begin();it!=neighbors_map.end();++it)
		{
			if((it->first)<neighbors_graph.size())
			{
				neighbors_graph[it->first].insert(neighbors_graph[it->first].end(), it->second.begin(), it->second.end());
			}
		}
		return neighbors_graph;
	}
};

}

namespace
{

void print_demo_bsh(const auxiliaries::ProgramOptionsHandler& poh)
{
	const double init_radius=poh.argument<double>("--init-radius", 3.5);
	if(init_radius<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy initial radius should be greater than 1.");
	}

	const unsigned int max_level=poh.argument<unsigned int>("--max-level", 0);

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	apollota::BoundingSpheresHierarchy bsh(spheres, init_radius, 1);

	if(bsh.levels()>0)
	{
		apollota::OpenGLPrinter::print_setup(std::cout);

		{
			std::ostringstream obj_name;
			obj_name << "obj_as";
			std::ostringstream cgo_name;
			cgo_name << "cgo_as";
			apollota::OpenGLPrinter opengl_printer(std::cout, obj_name.str(), cgo_name.str());
			for(std::size_t i=0;i<spheres.size();i++)
			{
				opengl_printer.print_color(0x36BBCE);
				opengl_printer.print_sphere(spheres[i]);
			}
		}

		for(std::size_t l=0;l<bsh.levels() && l<=max_level;l++)
		{
			const std::vector<apollota::SimpleSphere> bs=bsh.collect_bounding_spheres(l);
			std::ostringstream obj_name;
			obj_name << "obj_bs" << l;
			std::ostringstream cgo_name;
			cgo_name << "cgo_bs" << l;
			apollota::OpenGLPrinter opengl_printer(std::cout, obj_name.str(), cgo_name.str());
			for(std::size_t i=0;i<bs.size();i++)
			{
				opengl_printer.print_color(0x37DE6A);
				opengl_printer.print_sphere(bs[i]);
			}
		}

		std::cout << "cmd.set('bg_rgb', [1,1,1])\n\n";
		std::cout << "cmd.set('ambient', 0.3)\n\n";
	}
}

void print_demo_face(const auxiliaries::ProgramOptionsHandler& poh)
{
	const std::string cyclide_type=poh.argument<std::string>("--cyclide-type", "");
	std::vector<apollota::SimpleSphere> generators;
	if(cyclide_type=="spindle")
	{
		generators.push_back(apollota::SimpleSphere(2, 1, 0, 2.0));
		generators.push_back(apollota::SimpleSphere(-1, 2, 0, 2.3));
		generators.push_back(apollota::SimpleSphere(-1, -1, 0, 2.6));
	}
	if(cyclide_type=="horn")
	{
		generators.push_back(apollota::SimpleSphere(0.0, -0.1, 0, 0.2));
		generators.push_back(apollota::SimpleSphere(-1, 2, 0, 0.3));
		generators.push_back(apollota::SimpleSphere(-2.3, 0.2, 0, 1.0));
	}
	else
	{
		generators.push_back(apollota::SimpleSphere(2, 1, 0, 0.7));
		generators.push_back(apollota::SimpleSphere(-1, 2, 0, 1.0));
		generators.push_back(apollota::SimpleSphere(-1, -1, 0, 1.3));
	}


	apollota::OpenGLPrinter::print_setup(std::cout);

	apollota::OpenGLPrinter opengl_printer_curve(std::cout, "obj_curve", "cgo_curve");
	apollota::OpenGLPrinter opengl_printer_generators(std::cout, "obj_generators", "cgo_generators");
	apollota::OpenGLPrinter opengl_printer_m_surface(std::cout, "obj_m_surface", "cgo_m_surface");
	apollota::OpenGLPrinter opengl_printer_tangent_planes(std::cout, "obj_tangent_planes", "cgo_tangent_planes");
	apollota::OpenGLPrinter opengl_printer_tangent_spheres(std::cout, "obj_tangent_spheres", "cgo_tangent_spheres");
	apollota::OpenGLPrinter opengl_printer_m_contour(std::cout, "obj_m_contour", "cgo_m_contour");
	apollota::OpenGLPrinter opengl_printer_m_touch_points(std::cout, "obj_m_touch_points", "cgo_m_touch_points");
	apollota::OpenGLPrinter opengl_printer_cyclide_contour(std::cout, "obj_cyclide_contour", "cgo_cyclide_contour");
	apollota::OpenGLPrinter opengl_printer_cyclide_surface(std::cout, "obj_cyclide_surface", "cgo_cyclide_surface");
	apollota::OpenGLPrinter opengl_printer_cylinder(std::cout, "obj_cylinder", "cgo_cylinder");
	apollota::OpenGLPrinter opengl_printer_trans_tangent_planes(std::cout, "obj_trans_tangent_planes", "cgo_trans_tangent_planes");

	for(std::size_t i=0;i<generators.size();i++)
	{
		opengl_printer_generators.print_color(0x36BBCE);
		opengl_printer_generators.print_sphere(apollota::SimpleSphere(generators[i], generators[i].r-0.01));
	}

	std::vector< std::pair<apollota::SimplePoint, apollota::SimplePoint> > tangent_planes=apollota::TangentPlaneOfThreeSpheres::calculate(generators[0], generators[1], generators[2]);
	const std::vector<apollota::SimpleSphere> min_tangents=apollota::TangentSphereOfThreeSpheres::calculate(generators[0], generators[1], generators[2]);
	if(tangent_planes.size()==2 && min_tangents.size()==1)
	{
		if(apollota::halfspace_of_point(generators[0], apollota::plane_normal_from_three_points<apollota::SimplePoint>(generators[0], generators[1], generators[2]), tangent_planes[0].first+tangent_planes[0].second)>0)
		{
			std::swap(tangent_planes[0], tangent_planes[1]);
		}

		const apollota::SimpleSphere min_tangent=min_tangents.front();
		opengl_printer_curve.print_color(0xA61700);
		opengl_printer_curve.print_sphere(apollota::SimpleSphere(min_tangent, 0.1));

		std::deque<apollota::SimplePoint> curve;
		std::deque<double> radii;
		{
			double r_mult=(min_tangent.r>0.0 ? 1.01 : 1/1.01);
			const double r_max=14.0;
			for(double r=min_tangent.r;r<r_max;r=r*r_mult)
			{
				const std::vector<apollota::SimpleSphere> tangent_spheres=apollota::TangentSphereOfThreeSpheres::calculate(generators[0], generators[1], generators[2], r);
				if(tangent_spheres.size()==2)
				{
					for(std::size_t i=0;i<tangent_spheres.size();i++)
					{
						if(apollota::halfspace_of_point(generators[0], apollota::plane_normal_from_three_points<apollota::SimplePoint>(generators[0], generators[1], generators[2]), tangent_spheres[i])<0)
						{
							curve.push_front(apollota::SimplePoint(tangent_spheres[i]));
							radii.push_front(r);
						}
						else
						{
							curve.push_back(apollota::SimplePoint(tangent_spheres[i]));
							radii.push_back(r);
						}
					}
				}
				if(r>-0.01 && r_mult<1.0)
				{
					r=0.01;
					r_mult=1.0/r_mult;
				}
			}
		}

		opengl_printer_curve.print_line_strip(std::vector<apollota::SimplePoint>(curve.begin(), curve.end()));

		std::vector<apollota::SimpleSphere> circles_spheres;
		std::vector< std::vector<apollota::SimplePoint> > circles_touches;
		std::vector< std::vector<apollota::SimplePoint> > circles_vertices;
		std::vector< std::vector<apollota::SimplePoint> > circles_normals;
		for(std::size_t i=0;i<curve.size();i++)
		{
			std::vector<apollota::SimplePoint> touches;
			if(i==0 || i+1==curve.size())
			{
				for(std::size_t j=0;j<generators.size();j++)
				{
					touches.push_back(apollota::SimplePoint(generators[j])+(tangent_planes[i==0 ? 0 : 1].second*generators[j].r));
				}
			}
			else
			{
				const apollota::SimpleSphere tangent(curve[i], radii[i]);
				for(std::size_t j=0;j<generators.size();j++)
				{
					apollota::SimplePoint a(tangent);
					apollota::SimplePoint b(generators[j]);
					apollota::SimplePoint ab=((b-a).unit())*tangent.r;
					touches.push_back(a+ab);
				}
			}
			const std::vector<apollota::SimpleSphere> circles=apollota::TangentSphereOfThreeSpheres::calculate(apollota::SimpleSphere(touches[0], 0.0), apollota::SimpleSphere(touches[1], 0.0), apollota::SimpleSphere(touches[2], 0.0));
			if(circles.size()==1)
			{
				const apollota::SimpleSphere& circle=circles.front();
				if(i==0 || i+1==curve.size())
				{
					circles_spheres.push_back(circle);
				}
				const apollota::SimplePoint orientation=apollota::plane_normal_from_three_points<apollota::SimplePoint>(touches[0], touches[1], touches[2]);
				const apollota::SimplePoint first_point(apollota::SimplePoint(circle)-touches[0]);
				apollota::Rotation rotation(apollota::custom_point_from_object<apollota::SimplePoint>(orientation), 0);
				const double angle_step=10;
				std::vector<apollota::SimplePoint> circle_vertices;
				for(rotation.angle=0;rotation.angle<=360;rotation.angle+=angle_step)
				{
					circle_vertices.push_back(apollota::SimplePoint(circle)+rotation.rotate<apollota::SimplePoint>(first_point));
				}
				std::vector<apollota::SimplePoint> circle_normals;
				for(std::size_t j=0;j<circle_vertices.size();j++)
				{
					circle_normals.push_back((circle_vertices[j]-apollota::SimplePoint(circle)).unit());
				}
				circles_touches.push_back(touches);
				circles_vertices.push_back(circle_vertices);
				circles_normals.push_back(circle_normals);
			}
		}

		opengl_printer_m_surface.print_color(0xFF5A40);
		for(std::size_t i=0;i+1<circles_vertices.size();i++)
		{
			std::size_t j=i+1;
			std::vector<apollota::SimplePoint> vertices;
			std::vector<apollota::SimplePoint> normals;
			if(circles_vertices[i].size()==circles_vertices[j].size())
			{
				for(std::size_t e=0;e<circles_vertices[i].size();e++)
				{
					vertices.push_back(circles_vertices[i][e]);
					vertices.push_back(circles_vertices[j][e]);
					normals.push_back(circles_normals[i][e]);
					normals.push_back(circles_normals[j][e]);
				}
				opengl_printer_m_surface.print_triangle_strip(vertices, normals);
			}
		}

		for(int i=0;i<2;i++)
		{
			std::vector<apollota::SimplePoint> vertices_inner=circles_vertices[i==0 ? 0 : circles_vertices.size()-1];
			std::vector<apollota::SimplePoint> normals_inner=circles_normals[i==0 ? 0 : circles_normals.size()-1];
			std::vector<apollota::SimplePoint> vertices;
			std::vector<apollota::SimplePoint> normals;
			for(std::size_t j=0;j<vertices_inner.size();j++)
			{
				vertices.push_back(vertices_inner[j]);
				vertices.push_back(vertices_inner[j]+(normals_inner[j]*2.5));
				normals.push_back(tangent_planes[i].second);
				normals.push_back(tangent_planes[i].second);
			}
			opengl_printer_tangent_planes.print_color(i==0 ? 0xFFB673 : 0x64DE89);
			opengl_printer_tangent_planes.print_triangle_strip(vertices, normals);
		}

		{
			std::size_t i=0;
			while(i<(curve.size()/2))
			{
				opengl_printer_tangent_spheres.print_alpha(0.2);
				opengl_printer_tangent_spheres.print_color(0xFF9C40);
				opengl_printer_tangent_spheres.print_sphere(apollota::SimpleSphere(curve[i], radii[i]-0.01));
				opengl_printer_tangent_spheres.print_color(0x37DE6A);
				opengl_printer_tangent_spheres.print_sphere(apollota::SimpleSphere(curve[curve.size()-1-i], radii[curve.size()-1-i]-0.01));
				i+=30;
			}
		}

		if(!circles_vertices.empty())
		{
			const double gap_distance_threshold=0.3;
			double gap_distance=0.0;
			for(std::size_t i=0;i<circles_vertices.size()/2;i++)
			{
				bool draw_on=false;
				if(i==0)
				{
					draw_on=true;
				}
				else
				{
					gap_distance+=apollota::distance_from_point_to_point(circles_vertices[i].at(0), circles_vertices[i+1].at(0));
					if(gap_distance>gap_distance_threshold)
					{
						gap_distance=0.0;
						draw_on=true;
					}
				}
				if(draw_on)
				{
					opengl_printer_m_contour.print_color(0x111111);
					opengl_printer_m_contour.print_line_strip(circles_vertices[i]);
					opengl_printer_m_contour.print_line_strip(circles_vertices[circles_vertices.size()-1-i]);

					const double touch_point_size=((cyclide_type=="horn") ? 0.03 : 0.05);
					opengl_printer_m_touch_points.print_color(0x111111);
					for(std::size_t j=0;j<circles_touches[i].size() && j<circles_touches[circles_vertices.size()-1-i].size() && j<3;j++)
					{
						opengl_printer_m_touch_points.print_sphere(apollota::SimpleSphere(circles_touches[i][j], touch_point_size));
						opengl_printer_m_touch_points.print_sphere(apollota::SimpleSphere(circles_touches[circles_vertices.size()-1-i][j], touch_point_size));
					}
				}
			}
		}
		
		if(circles_vertices.size()>=3)
		{
			std::vector< std::vector<apollota::SimplePoint> > cyclide_vertices;
			std::vector< std::vector<apollota::SimplePoint> > cyclide_normals;
			for(std::size_t i=0;i<circles_vertices.front().size();i++)
			{
				const std::vector<apollota::SimpleSphere> circles=apollota::TangentSphereOfThreeSpheres::calculate(apollota::SimpleSphere(circles_vertices.front()[i], 0.0), min_tangent, apollota::SimpleSphere(circles_vertices.back()[i], 0.0));
				if(circles.size()==1)
				{
					apollota::SimpleSphere circle=circles.front();
					circle.r*=1.05;
					const apollota::SimplePoint orientation=apollota::plane_normal_from_three_points<apollota::SimplePoint>(circles_vertices.front()[i], apollota::SimplePoint(min_tangent), circles_vertices.back()[i]);
					const apollota::SimplePoint first_point=(apollota::SimplePoint(circle)-circles_vertices.front()[i]).unit()*circle.r;
					apollota::Rotation rotation(apollota::custom_point_from_object<apollota::SimplePoint>(orientation), 0);
					const double angle_step=10;
					std::vector<apollota::SimplePoint> circle_vertices;
					for(rotation.angle=0;rotation.angle<=360;rotation.angle+=angle_step)
					{
						circle_vertices.push_back(apollota::SimplePoint(circle)+rotation.rotate<apollota::SimplePoint>(first_point));
					}
					std::vector<apollota::SimplePoint> circle_normals;
					for(std::size_t j=0;j<circle_vertices.size();j++)
					{
						circle_normals.push_back((circle_vertices[j]-apollota::SimplePoint(circle)).unit());
					}
					cyclide_vertices.push_back(circle_vertices);
					cyclide_normals.push_back(circle_normals);
				}
			}

			opengl_printer_cyclide_surface.print_color(0xFF5A40);
			opengl_printer_cyclide_surface.print_alpha(0.5);
			for(std::size_t i=0;i<cyclide_vertices.size();i++)
			{
				std::size_t j=((i+1<cyclide_vertices.size()) ? (i+1) : 0);
				std::vector<apollota::SimplePoint> vertices;
				std::vector<apollota::SimplePoint> normals;
				if(cyclide_vertices[i].size()==cyclide_vertices[j].size())
				{
					for(std::size_t e=0;e<cyclide_vertices[i].size();e++)
					{
						vertices.push_back(cyclide_vertices[i][e]);
						vertices.push_back(cyclide_vertices[j][e]);
						normals.push_back(cyclide_normals[i][e]);
						normals.push_back(cyclide_normals[j][e]);
					}
					opengl_printer_cyclide_surface.print_triangle_strip(vertices, normals);
				}
			}
			opengl_printer_cyclide_contour.print_color(0x111111);
			for(std::size_t i=0;i<cyclide_vertices.size();i++)
			{
				opengl_printer_cyclide_contour.print_line_strip(cyclide_vertices[i], true);
			}
		}

		if(circles_spheres.size()==2)
		{
			{
				const apollota::SimplePoint a(circles_spheres.front());
				const apollota::SimplePoint b(circles_spheres.back());
				const apollota::SimplePoint c=(a+b)*0.5;
				const apollota::SimplePoint p1=c+(a-c).unit()*((a-c).module()+circles_spheres.front().r);
				const apollota::SimplePoint p2=c+(b-c).unit()*((b-c).module()+circles_spheres.back().r);
				opengl_printer_cylinder.print_alpha(0.5);
				opengl_printer_cylinder.print_cylinder(p1, p2, std::max(circles_spheres.front().r, circles_spheres.back().r), 0x64DE89, 0x64DE89);
			}

			{
				opengl_printer_cylinder.print_alpha(1.0);
				opengl_printer_cylinder.print_color(0x111111);
				opengl_printer_cylinder.print_line_strip(circles_vertices.front(), true);
				opengl_printer_cylinder.print_line_strip(circles_vertices.back(), true);
			}

			if(circles_vertices.front().size()>10 && circles_vertices.back().size()>10)
			{
				opengl_printer_trans_tangent_planes.print_alpha(0.5);
				opengl_printer_trans_tangent_planes.print_color(0xFFB673);
				opengl_printer_trans_tangent_planes.print_triangle_strip(circles_vertices.front(), std::vector<apollota::SimplePoint>(circles_vertices.front().size(), apollota::plane_normal_from_three_points<apollota::SimplePoint>(circles_vertices.front()[0], circles_vertices.front()[5], circles_vertices.front()[10])), true);
				opengl_printer_trans_tangent_planes.print_triangle_strip(circles_vertices.back(), std::vector<apollota::SimplePoint>(circles_vertices.front().size(), apollota::plane_normal_from_three_points<apollota::SimplePoint>(circles_vertices.back()[0], circles_vertices.back()[5], circles_vertices.back()[10])), true);
			}
		}
	}

	std::cout << "cmd.set('ray_shadows', 'off')\n\n";
	std::cout << "cmd.set('ray_shadow', 'off')\n\n";
	std::cout << "cmd.set('two_sided_lighting', 'on')\n\n";
	std::cout << "cmd.set('cgo_line_width', 3)\n\n";
	std::cout << "cmd.set('bg_rgb', [1,1,1])\n\n";
	std::cout << "cmd.set('ambient', 0.3)\n\n";
}

void print_demo_tangent_spheres()
{
	std::vector< std::vector<apollota::SimpleSphere> > generators_sets;

	{
		std::vector<apollota::SimpleSphere> generators;
		generators.push_back(apollota::SimpleSphere(2.0, 1.0, 0.1, 0.9));
		generators.push_back(apollota::SimpleSphere(-1.0, 2.0, -0.1, 1.2));
		generators.push_back(apollota::SimpleSphere(-1.0, -1.0, 0.1, 1.5));
		generators.push_back(apollota::SimpleSphere(1.0, 1.0, 4.5, 1.0));
		generators_sets.push_back(generators);
	}

	{
		std::vector<apollota::SimpleSphere> generators;
		generators.push_back(apollota::SimpleSphere(2.0, 1.0, 0.1, 0.9));
		generators.push_back(apollota::SimpleSphere(-1.0, 2.0, -0.1, 1.2));
		generators.push_back(apollota::SimpleSphere(-1.0, -1.0, 0.1, 1.5));
		generators.push_back(apollota::SimpleSphere(0.4, 0.4, 0.0, 0.5));
		generators_sets.push_back(generators);
	}

	{
		std::vector<apollota::SimpleSphere> generators;
		generators.push_back(apollota::SimpleSphere(2.0, 1.0, 0.1, 3.4));
		generators.push_back(apollota::SimpleSphere(-1.0, 2.0, -0.1, 3.7));
		generators.push_back(apollota::SimpleSphere(-1.0, -1.0, 0.1, 4.0));
		generators.push_back(apollota::SimpleSphere(1.0, 1.0, 4.5, 3.5));
		generators_sets.push_back(generators);
	}

	apollota::OpenGLPrinter::print_setup(std::cout);

	for(std::size_t j=0;j<generators_sets.size();j++)
	{
		const std::vector<apollota::SimpleSphere>& generators=generators_sets[j];

		std::ostringstream obj_name;
		obj_name << "obj" << j;
		std::ostringstream cgo_name;
		cgo_name << "cgo" << j;
		apollota::OpenGLPrinter opengl_printer(std::cout, obj_name.str(), cgo_name.str());

		opengl_printer.print_alpha(j==2 ? 0.25 : 1.0);
		opengl_printer.print_color(0x36BBCE);
		for(std::size_t i=0;i<generators.size();i++)
		{
			opengl_printer.print_sphere(generators[i]);
		}

		const std::vector<apollota::SimpleSphere> tangents=apollota::TangentSphereOfFourSpheres::calculate(generators[0], generators[1], generators[2], generators[3]);

		opengl_printer.print_alpha(j==2 ? 1.0 : 0.7);
		opengl_printer.print_color(0xFF5A40);
		for(std::size_t i=0;i<tangents.size();i++)
		{
			opengl_printer.print_sphere(apollota::SimpleSphere(tangents[i], fabs(tangents[i].r)));
		}
	}

	std::cout << "cmd.set('bg_rgb', [1,1,1])\n\n";
	std::cout << "cmd.set('ambient', 0.3)\n\n";
}

void print_demo_tangent_planes()
{
	std::vector<apollota::SimpleSphere> generators;
	generators.push_back(apollota::SimpleSphere(2.0, 1.0, 0.1, 0.9));
	generators.push_back(apollota::SimpleSphere(-1.0, 2.0, -0.1, 1.2));
	generators.push_back(apollota::SimpleSphere(-1.0, -1.0, 0.1, 1.5));

	apollota::OpenGLPrinter::print_setup(std::cout);
	apollota::OpenGLPrinter opengl_printer(std::cout, "obj", "cgo");

	opengl_printer.print_color(0x36BBCE);
	opengl_printer.print_alpha(1.0);
	for(std::size_t i=0;i<generators.size();i++)
	{
		opengl_printer.print_sphere(generators[i]);
	}

	const std::vector< std::pair<apollota::SimplePoint, apollota::SimplePoint> > tangent_planes=apollota::TangentPlaneOfThreeSpheres::calculate(generators[0], generators[1], generators[2]);
	for(std::size_t i=0;i<tangent_planes.size();i++)
	{
		opengl_printer.print_color(0xFF5A40);
		opengl_printer.print_alpha(0.7);
		std::vector<apollota::SimplePoint> vertices(generators.size());
		std::vector<apollota::SimplePoint> normals(generators.size());
		for(std::size_t j=0;j<generators.size();j++)
		{
			vertices[j]=(apollota::SimplePoint(generators[j])+(tangent_planes[i].second.unit()*generators[j].r));
			normals[j]=tangent_planes[i].second;
		}
		{
			const std::vector<apollota::SimpleSphere> circles=apollota::TangentSphereOfThreeSpheres::calculate(apollota::SimpleSphere(vertices[0], 0.0), apollota::SimpleSphere(vertices[1], 0.0), apollota::SimpleSphere(vertices[2], 0.0));
			if(circles.size()==1)
			{
				const apollota::SimplePoint center(circles.front());
				for(std::size_t j=0;j<vertices.size();j++)
				{
					vertices[j]=vertices[j]+((vertices[j]-center)*1.5);
				}
			}
		}
		opengl_printer.print_triangle_strip(vertices, normals);
	}

	std::cout << "cmd.set('two_sided_lighting', 'on')\n\n";
	std::cout << "cmd.set('bg_rgb', [1,1,1])\n\n";
	std::cout << "cmd.set('ambient', 0.3)\n\n";
}

void print_demo_splitting(const auxiliaries::ProgramOptionsHandler& poh)
{
	const int parts=poh.argument<int>("--parts", 2);

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);

	const std::vector< std::vector<std::size_t> > ids=apollota::SplittingOfSpheres::split_for_number_of_parts(spheres, parts);

	apollota::OpenGLPrinter::print_setup(std::cout);
	apollota::OpenGLPrinter opengl_printer(std::cout, "obj_splitting", "cgo_splitting");

	for(std::size_t i=0;i<ids.size();i++)
	{
		opengl_printer.print_color(((0x36BBCE)*static_cast<int>(i+1))%(0xFFFFFF));
		for(std::size_t j=0;j<ids[i].size();j++)
		{
			opengl_printer.print_sphere(spheres[ids[i][j]]);
		}
	}
}

void print_demo_empty_tangents(const auxiliaries::ProgramOptionsHandler& poh)
{
	const double max_r=poh.argument<double>("--max-r", std::numeric_limits<double>::max());
	const double alpha=poh.argument<double>("--alpha", 0.5);
	const double reduction=poh.argument<double>("--reduction", 0.0);
	const bool selection_as_intervals=poh.contains_option("--selection-as-intervals");
	const std::vector<std::size_t> selection_vector=poh.argument_vector<std::size_t>("--selection");
	const bool no_neighbors=poh.contains_option("--no-neighbors");

	std::tr1::unordered_set<std::size_t> selection_set;
	if(!selection_vector.empty())
	{
		if(!selection_as_intervals)
		{
			selection_set.insert(selection_vector.begin(), selection_vector.end());
		}
		else if(selection_vector.size()%2==0)
		{
			for(std::size_t i=0;i<selection_vector.size();i+=2)
			{
				for(std::size_t a=selection_vector[i];a<=selection_vector[i+1];a++)
				{
					selection_set.insert(a);
				}
			}
		}
	}

	std::tr1::unordered_set<std::size_t> extended_selection_set=selection_set;

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, true, false);

	std::set<apollota::Quadruple> drawn_quadruples;

	apollota::OpenGLPrinter::print_setup(std::cout);

	{
		apollota::OpenGLPrinter opengl_printer_opaq(std::cout, "obj_opaq_empty_tangent_spheres", "cgo_opaq_empty_tangent_spheres");
		opengl_printer_opaq.print_color(0xFF5A40);
		apollota::OpenGLPrinter opengl_printer_trans(std::cout, "obj_trans_empty_tangent_spheres", "cgo_trans_empty_tangent_spheres");
		opengl_printer_trans.print_color(0xFF5A40);
		opengl_printer_trans.print_alpha(alpha);
		for(apollota::Triangulation::QuadruplesMap::const_iterator it=triangulation_result.quadruples_map.begin();it!=triangulation_result.quadruples_map.end();++it)
		{
			const apollota::Quadruple& quadruple=it->first;
			if(selection_set.empty() || (no_neighbors ? (selection_set.count(quadruple.get(0))>0 && selection_set.count(quadruple.get(1))>0 && selection_set.count(quadruple.get(2))>0 && selection_set.count(quadruple.get(3))>0) : (selection_set.count(quadruple.get(0))>0 || selection_set.count(quadruple.get(1))>0 || selection_set.count(quadruple.get(2))>0 || selection_set.count(quadruple.get(3))>0)))
			{
				for(std::size_t i=0;i<4;i++)
				{
					extended_selection_set.insert(quadruple.get(i));
				}

				const std::vector<apollota::SimpleSphere> tangents=it->second;
				for(std::size_t i=0;i<tangents.size();i++)
				{
					if(tangents[i].r<max_r)
					{
						opengl_printer_trans.print_sphere(apollota::SimpleSphere(tangents[i], fabs(tangents[i].r+reduction)));
						opengl_printer_opaq.print_sphere(apollota::SimpleSphere(tangents[i], fabs(tangents[i].r+reduction)));
						drawn_quadruples.insert(quadruple);
					}
				}
			}
		}
	}

	{
		apollota::OpenGLPrinter opengl_printer_central(std::cout, "obj_central_balls", "cgo_central_balls");
		opengl_printer_central.print_color(0x36BBCE);
		apollota::OpenGLPrinter opengl_printer_adjacent_opaq(std::cout, "obj_opaq_adjacent_balls", "cgo_opaq_adjacent_balls");
		opengl_printer_adjacent_opaq.print_color(0x36BBCE);
		apollota::OpenGLPrinter opengl_printer_adjacent_trans(std::cout, "obj_trans_adjacent_balls", "cgo_trans_adjacent_balls");
		opengl_printer_adjacent_trans.print_color(0x36BBCE);
		opengl_printer_adjacent_trans.print_alpha(alpha);
		for(std::size_t i=0;i<spheres.size();i++)
		{
			if(selection_set.empty() || selection_set.count(i)>0)
			{
				opengl_printer_central.print_sphere(apollota::SimpleSphere(spheres[i], spheres[i].r-reduction));
			}
			if(extended_selection_set.count(i)>0)
			{
				opengl_printer_adjacent_trans.print_sphere(apollota::SimpleSphere(spheres[i], spheres[i].r-reduction));
				opengl_printer_adjacent_opaq.print_sphere(apollota::SimpleSphere(spheres[i], spheres[i].r-reduction));
			}
		}
	}

	std::cout << "cmd.set('bg_rgb', [1,1,1])\n\n";
	std::cout << "cmd.set('ambient', 0.3)\n\n";
}

void print_min_distances_of_ignored_balls(const auxiliaries::ProgramOptionsHandler& poh)
{
	const std::string name=poh.argument<std::string>("--name");

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, false, false);

	for(std::set<std::size_t>::const_iterator it=triangulation_result.ignored_spheres_ids.begin();it!=triangulation_result.ignored_spheres_ids.end();++it)
	{
		const std::size_t j=(*it);
		std::multimap<double, std::size_t> distances_ids;
		for(std::size_t i=0;i<spheres.size();i++)
		{
			if(i!=j)
			{
				distances_ids.insert(std::make_pair(apollota::distance_from_point_to_point(spheres[i], spheres[j]), i));
			}
		}
		std::cout << name << " " << j << " " << spheres[j].r << " ";
		int n=0;
		for(std::multimap<double, std::size_t>::const_iterator jt=distances_ids.begin();jt!=distances_ids.end() && n<2;++jt)
		{
			std::cout << jt->first << " " << jt->second << " " << spheres[jt->second].r << (n+1<2 ? " " : "\n");
			n++;
		}
	}
}

void print_cavities(const auxiliaries::ProgramOptionsHandler& poh)
{
	const double probe=poh.argument<double>("--probe", 1.4);
	const double min_r=poh.argument<double>("--min-r", 0.0);

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, false, false);
	const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);
	const apollota::Triangulation::VerticesGraph vertices_graph=apollota::Triangulation::construct_vertices_graph(spheres, triangulation_result.quadruples_map);

	std::vector<int> coloring(vertices_vector.size(), 0);
	std::deque<std::size_t> stack;
	std::deque< std::pair< std::pair<std::size_t, std::size_t>, apollota::SimpleSphere > > hitting_stack;

	for(std::size_t a=0;a<vertices_vector.size();a++)
	{
		if(vertices_vector[a].second.r>=probe)
		{
			for(std::size_t i=0;i<vertices_graph[a].size() && coloring[a]==0;i++)
			{
				if(vertices_graph[a][i]>=vertices_graph.size())
				{
					coloring[a]=1;
					stack.push_back(a);
				}
			}
		}
	}

	while(!stack.empty())
	{
		const std::size_t a=stack.back();
		stack.pop_back();
		if(a<coloring.size() && coloring[a]==1 && vertices_vector[a].second.r>=probe)
		{
			for(std::size_t i=0;i<vertices_graph[a].size();i++)
			{
				const std::size_t b=vertices_graph[a][i];
				if(b<coloring.size() && coloring[b]==0)
				{
					bool connect=false;
					const apollota::SimpleSphere& sa=vertices_vector[a].second;
					const apollota::SimpleSphere& sb=vertices_vector[b].second;
					const apollota::Triple triple=vertices_vector[a].first.exclude(i);
					const std::vector<apollota::SimpleSphere> probe_tangents=apollota::TangentSphereOfThreeSpheres::calculate(spheres[triple.get(0)], spheres[triple.get(1)], spheres[triple.get(2)], probe);
					if(probe_tangents.size()<2)
					{
						if(sb.r>=probe)
						{
							connect=true;
						}
					}
					else
					{
						apollota::SimpleSphere o;
						{
							const double r0=spheres[triple.get(0)].r;
							const double r1=spheres[triple.get(1)].r;
							const double r2=spheres[triple.get(2)].r;
							o=spheres[triple.get((r0<=r1 && r0<=r2) ? 0 : ((r1<=r0 && r1<=r2) ? 1 : 2))];
						}

						const double d_a0=apollota::min_angle(o, sa, probe_tangents[0]);
						const double d_a1=apollota::min_angle(o, sa, probe_tangents[1]);
						const double d_ab=apollota::min_angle(o, sa, sb);
						const double d_b0=apollota::min_angle(o, sb, probe_tangents[0]);
						const double d_b1=apollota::min_angle(o, sb, probe_tangents[1]);

						const bool safe0=(d_a0>d_ab || d_b0>d_ab);
						const bool safe1=(d_a1>d_ab || d_b1>d_ab);

						if(safe0 && safe1 && sb.r>=probe)
						{
							connect=true;
						}
						else if(!safe0 && safe1)
						{
							hitting_stack.push_back(std::make_pair(std::make_pair(a, b), probe_tangents[0]));
						}
						else if(safe0 && !safe1)
						{
							hitting_stack.push_back(std::make_pair(std::make_pair(a, b), probe_tangents[1]));
						}
						else if(!safe0 && !safe1)
						{
							hitting_stack.push_back(std::make_pair(std::make_pair(a, b), probe_tangents[d_a0<d_a1 ? 0 : 1]));
						}
					}
					if(connect)
					{
						coloring[b]=1;
						stack.push_back(b);
					}
				}
			}
		}
	}

	std::vector<apollota::SimpleSphere> hitters;

	{
		std::vector<apollota::SimpleSphere> hitting_visits(vertices_vector.size());
		while(!hitting_stack.empty())
		{
			const std::size_t s=hitting_stack.back().first.first;
			const std::size_t a=hitting_stack.back().first.second;
			const apollota::SimpleSphere hitter=hitting_stack.back().second;
			hitting_stack.pop_back();
			if(coloring[s]==1)
			{
				hitters.push_back(hitter);
			}
			hitting_visits[a]=hitter;
			if(a<coloring.size() && coloring[a]!=1)
			{
				if(apollota::sphere_intersects_sphere(vertices_vector[a].second, hitter))
				{
					coloring[a]=2;
				}
				for(std::size_t i=0;i<vertices_graph[a].size();i++)
				{
					const std::size_t b=vertices_graph[a][i];
					if(b<coloring.size() && !(hitting_visits[b]==hitter) && apollota::sphere_intersects_sphere(vertices_vector[b].second, hitter))
					{
						hitting_stack.push_back(std::make_pair(std::make_pair(a, b), hitter));
					}
				}
			}
		}
	}

	apollota::OpenGLPrinter::print_setup(std::cout);

	{
		apollota::OpenGLPrinter opengl_printer_cavities(std::cout, "obj_cavities", "cgo_cavities");
		opengl_printer_cavities.print_color(0xFF5A40);
		for(std::size_t i=0;i<vertices_vector.size();i++)
		{
			if(coloring[i]==0 && vertices_vector[i].second.r>=min_r)
			{
				opengl_printer_cavities.print_sphere(vertices_vector[i].second);
			}
		}
	}

	{
		apollota::OpenGLPrinter opengl_printer_precavities(std::cout, "obj_precavities", "cgo_precavities");
		opengl_printer_precavities.print_color(0xFFFF40);
		for(std::size_t i=0;i<vertices_vector.size();i++)
		{
			if(coloring[i]==2)
			{
				opengl_printer_precavities.print_sphere(vertices_vector[i].second);
			}
		}
	}

	{
		apollota::OpenGLPrinter opengl_printer_hitters(std::cout, "obj_hitters", "cgo_hitters");
		opengl_printer_hitters.print_color(0xFFAAFF);
		for(std::size_t i=0;i<hitters.size();i++)
		{
			opengl_printer_hitters.print_sphere(hitters[i]);
		}
	}

	{
		apollota::OpenGLPrinter opengl_printer_balls(std::cout, "obj_balls", "cgo_balls");
		opengl_printer_balls.print_color(0x36BBCE);
		for(std::size_t i=0;i<spheres.size();i++)
		{
			opengl_printer_balls.print_sphere(spheres[i]);
		}
	}
}

void print_contact_contours(const auxiliaries::ProgramOptionsHandler& poh)
{
	const bool selection_as_intervals=poh.contains_option("--selection-as-intervals");
	const std::vector<std::size_t> selection_vector=poh.argument_vector<std::size_t>("--selection");
	const double probe=poh.argument<double>("--probe", 1.4);
	const double step=poh.argument<double>("--step", 0.3);
	const int projections=poh.argument<int>("--projections", 7);
	const bool draw_remainders=poh.contains_option("--draw-remainders");
	const std::size_t sih_depth=poh.argument<std::size_t>("--sih-depth", 3);

	std::set<std::size_t> selection_set;
	if(!selection_vector.empty())
	{
		if(!selection_as_intervals)
		{
			selection_set.insert(selection_vector.begin(), selection_vector.end());
		}
		else if(selection_vector.size()%2==0)
		{
			for(std::size_t i=0;i<selection_vector.size();i+=2)
			{
				for(std::size_t a=selection_vector[i];a<=selection_vector[i+1];a++)
				{
					selection_set.insert(a);
				}
			}
		}
	}

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, false, false);
	const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);

	apollota::OpenGLPrinter::print_setup(std::cout);

	{
		const apollota::UtilitiesForTriangulation::NeighborsGraph neighbors_graph=apollota::UtilitiesForTriangulation::collect_neighbors_graph_from_neighbors_map(apollota::UtilitiesForTriangulation::collect_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map), spheres.size());
		const apollota::ContactContour::PairsIDsMap pairs_vertices=apollota::ContactContour::collect_pairs_vertices_map_from_vertices_vector(vertices_vector);
		for(std::set<std::size_t>::const_iterator sel_it=selection_set.begin();sel_it!=selection_set.end();++sel_it)
		{
			const std::size_t sel=(*sel_it);
			if(sel<spheres.size())
			{
				const std::vector<std::size_t>& neighbors_list=neighbors_graph[sel];
				for(std::size_t i=0;i<neighbors_list.size();i++)
				{
					const std::size_t neighbor=neighbors_list[i];
					if(selection_set.count(neighbor)==0)
					{
						apollota::ContactContour::PairsIDsMap::const_iterator it=pairs_vertices.find(apollota::Pair(sel, neighbor));
						if(it!=pairs_vertices.end())
						{
							const std::set<std::size_t>& pair_vertices_list=it->second;
							const std::list<apollota::ContactContour::Contour> contours=apollota::ContactContour::construct_contact_contours(spheres, vertices_vector, pair_vertices_list, sel, neighbor, probe, step, projections);
							if(!contours.empty())
							{
								std::ostringstream id_string;
								id_string << "a" << sel << "b" << neighbor;
								apollota::OpenGLPrinter opengl_printer(std::cout, std::string("obj_")+id_string.str(), std::string("cgo_")+id_string.str());

								{
									opengl_printer.print_color(0xFFFF00);
									const std::vector< std::vector<apollota::SimplePoint> > line_strips=apollota::ContactContour::collect_points_from_contours(contours);
									for(std::size_t j=0;j<line_strips.size();j++)
									{
										opengl_printer.print_line_strip(line_strips[j], true);
									}
								}
								{
									opengl_printer.print_color(0x00FF00);
									const std::vector< std::vector<apollota::SimplePoint> > meshes=apollota::ContactContour::collect_meshes_from_contours(contours, spheres[sel], spheres[neighbor]);
									for(std::size_t j=0;j<meshes.size();j++)
									{
										opengl_printer.print_triangle_strip(meshes[j], std::vector<apollota::SimplePoint>(meshes[j].size(), apollota::sub_of_points<apollota::SimplePoint>(spheres[neighbor], spheres[sel]).unit()), true);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if(draw_remainders)
	{
		apollota::SubdividedIcosahedron sih(sih_depth);
		const apollota::ContactRemainder::IDsMap ids_vertices=apollota::ContactRemainder::collect_vertices_map_from_vertices_vector(vertices_vector);
		for(std::set<std::size_t>::const_iterator sel_it=selection_set.begin();sel_it!=selection_set.end();++sel_it)
		{
			const std::size_t sel=(*sel_it);
			if(sel<spheres.size())
			{
				apollota::ContactRemainder::IDsMap::const_iterator it=ids_vertices.find(sel);
				if(it!=ids_vertices.end())
				{
					const std::set<std::size_t>& vertices_set=it->second;
					const apollota::ContactRemainder::Remainder remainder=apollota::ContactRemainder::construct_contact_remainder(spheres, vertices_vector, vertices_set, sel, probe, sih);
					if(!remainder.empty())
					{
						std::ostringstream id_string;
						id_string << "s" << sel;
						apollota::OpenGLPrinter opengl_printer(std::cout, std::string("obj_")+id_string.str(), std::string("cgo_")+id_string.str());
						opengl_printer.print_color(0xFF7700);
						for(apollota::ContactRemainder::Remainder::const_iterator jt=remainder.begin();jt!=remainder.end();++jt)
						{
							std::vector<apollota::SimplePoint> ts(3);
							std::vector<apollota::SimplePoint> ns(3);
							for(int i=0;i<3;i++)
							{
								ts[i]=jt->p[i];
								ns[i]=apollota::sub_of_points<apollota::SimplePoint>(ts[i], spheres[sel]).unit();
							}
							opengl_printer.print_triangle_strip(ts, ns, false);
						}
					}
				}
			}
		}
	}

	std::cout << "cmd.center('all')\n\n";
	std::cout << "cmd.zoom('all')\n\n";
}

}

void print_demo(const auxiliaries::ProgramOptionsHandler& poh)
{
	if(poh.contains_option("--help") || poh.contains_option("--help-full"))
	{
		return;
	}

	const std::string scene=poh.argument<std::string>("--scene");

	if(scene=="bsh")
	{
		print_demo_bsh(poh);
	}
	else if(scene=="face")
	{
		print_demo_face(poh);
	}
	else if(scene=="tangent-spheres")
	{
		print_demo_tangent_spheres();
	}
	else if(scene=="tangent-planes")
	{
		print_demo_tangent_planes();
	}
	else if(scene=="splitting")
	{
		print_demo_splitting(poh);
	}
	else if(scene=="empty-tangents")
	{
		print_demo_empty_tangents(poh);
	}
	else if(scene=="print-min-distances-of-ignored-balls")
	{
		print_min_distances_of_ignored_balls(poh);
	}
	else if(scene=="cavities")
	{
		print_cavities(poh);
	}
	else if(scene=="contact-contours")
	{
		print_contact_contours(poh);
	}
	else
	{
		throw std::runtime_error("Invalid scene name.");
	}
}
