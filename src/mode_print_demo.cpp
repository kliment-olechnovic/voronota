#include <iostream>
#include <deque>

#include "apollota/triangulation.h"
#include "apollota/opengl_printer.h"

#include "modes_commons.h"

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

void print_demo_face()
{
	std::vector<apollota::SimpleSphere> generators;
	generators.push_back(apollota::SimpleSphere(2, 1, 0, 0.7));
	generators.push_back(apollota::SimpleSphere(-1, 2, 0, 1.0));
	generators.push_back(apollota::SimpleSphere(-1, -1, 0, 1.3));

	apollota::OpenGLPrinter::print_setup(std::cout);

	apollota::OpenGLPrinter opengl_printer1(std::cout, "obj1", "cgo1");
	apollota::OpenGLPrinter opengl_printer2(std::cout, "obj2", "cgo2");
	apollota::OpenGLPrinter opengl_printer3(std::cout, "obj3", "cgo3");
	apollota::OpenGLPrinter opengl_printer4(std::cout, "obj4", "cgo4");

	for(std::size_t i=0;i<generators.size();i++)
	{
		opengl_printer1.print_color(0x36BBCE);
		opengl_printer1.print_sphere(apollota::SimpleSphere(generators[i], generators[i].r-0.01));
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
		opengl_printer1.print_color(0xA61700);
		opengl_printer1.print_sphere(apollota::SimpleSphere(min_tangent, 0.1));

		std::deque<apollota::SimplePoint> curve;
		std::deque<double> radii;
		{
			const double r_mult=1.01;
			const double r_max=7;
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
			}
		}

		opengl_printer1.print_line_strip(std::vector<apollota::SimplePoint>(curve.begin(), curve.end()));

		std::vector< std::vector<apollota::SimplePoint> > circles_vertices;
		std::vector< std::vector<apollota::SimplePoint> > circles_normals;
		for(std::size_t i=0;i<curve.size();i++)
		{
			std::vector<apollota::SimpleSphere> touches;
			if(i==0 || i+1==curve.size())
			{
				for(std::size_t j=0;j<generators.size();j++)
				{
					touches.push_back(apollota::SimpleSphere(apollota::SimplePoint(generators[j])+(tangent_planes[i==0 ? 0 : 1].second*generators[j].r), 0.0));
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
					touches.push_back(apollota::SimpleSphere(a+ab, 0.0));
				}
			}
			const std::vector<apollota::SimpleSphere> circles=apollota::TangentSphereOfThreeSpheres::calculate(touches[0], touches[1], touches[2]);
			if(circles.size()==1)
			{
				const apollota::SimpleSphere& circle=circles.front();
				const apollota::SimplePoint orientation=apollota::plane_normal_from_three_points<apollota::SimplePoint>(touches[0], touches[1], touches[2]);
				const apollota::SimplePoint first_point(apollota::SimplePoint(circle)-apollota::SimplePoint(touches[0]));
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
				circles_vertices.push_back(circle_vertices);
				circles_normals.push_back(circle_normals);
			}
		}

		opengl_printer2.print_color(0xFF5A40);
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
				opengl_printer2.print_triangle_strip(vertices, normals);
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
			opengl_printer3.print_color(i==0 ? 0xFFB673 : 0x64DE89);
			opengl_printer3.print_triangle_strip(vertices, normals);
		}

		std::size_t i=0;
		while(i<(curve.size()/2))
		{
			opengl_printer4.print_alpha(0.2);
			opengl_printer4.print_color(0xFF9C40);
			opengl_printer4.print_sphere(apollota::SimpleSphere(curve[i], radii[i]-0.01));
			opengl_printer4.print_color(0x37DE6A);
			opengl_printer4.print_sphere(apollota::SimpleSphere(curve[curve.size()-1-i], radii[curve.size()-1-i]-0.01));
			i+=30;
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

	apollota::OpenGLPrinter::print_setup(std::cout);

	for(std::size_t j=0;j<generators_sets.size();j++)
	{
		const std::vector<apollota::SimpleSphere>& generators=generators_sets[j];

		std::ostringstream obj_name;
		obj_name << "obj" << j;
		std::ostringstream cgo_name;
		cgo_name << "cgo" << j;
		apollota::OpenGLPrinter opengl_printer(std::cout, obj_name.str(), cgo_name.str());

		opengl_printer.print_color(0x36BBCE);
		opengl_printer.print_alpha(1.0);
		for(std::size_t i=0;i<generators.size();i++)
		{
			opengl_printer.print_sphere(generators[i]);
		}

		const std::vector<apollota::SimpleSphere> tangents=apollota::TangentSphereOfFourSpheres::calculate(generators[0], generators[1], generators[2], generators[3]);

		opengl_printer.print_color(0xFF5A40);
		opengl_printer.print_alpha(0.7);
		for(std::size_t i=0;i<tangents.size();i++)
		{
			opengl_printer.print_sphere(tangents[i]);
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

void print_demo_edges(const auxiliaries::ProgramOptionsHandler& poh)
{
	const double max_dist=poh.argument<double>("--max-dist", std::numeric_limits<double>::max());

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, true, false);
	apollota::Triangulation::NeighborsGraph graph=apollota::Triangulation::collect_neighbors_graph_from_neighbors_map(apollota::Triangulation::collect_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map), spheres.size());

	apollota::OpenGLPrinter::print_setup(std::cout);
	apollota::OpenGLPrinter opengl_printer(std::cout, "obj_edges", "cgo_edges");
	opengl_printer.print_color(0x36BBCE);
	for(std::size_t i=0;i<graph.size();i++)
	{
		for(std::size_t j=0;j<graph[i].size();j++)
		{
			std::vector<apollota::SimpleSphere> neighbors(2);
			neighbors[0]=(spheres[i]);
			neighbors[1]=(spheres[graph[i][j]]);
			const double dist=apollota::minimal_distance_from_sphere_to_sphere(neighbors[0], neighbors[1]);
			if(dist<max_dist)
			{
				opengl_printer.print_line_strip(neighbors);
			}
		}
	}
}

void print_demo_splitting(const auxiliaries::ProgramOptionsHandler& poh)
{
	const int parts=poh.argument<double>("--parts", 2);

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
		print_demo_face();
	}
	else if(scene=="tangent-spheres")
	{
		print_demo_tangent_spheres();
	}
	else if(scene=="tangent-planes")
	{
		print_demo_tangent_planes();
	}
	else if(scene=="edges")
	{
		print_demo_edges(poh);
	}
	else if(scene=="splitting")
	{
		print_demo_splitting(poh);
	}
	else
	{
		throw std::runtime_error("Invalid scene name.");
	}
}
