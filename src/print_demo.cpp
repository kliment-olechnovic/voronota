#include <iostream>
#include <deque>

#include "apollota/bounding_spheres_hierarchy.h"
#include "apollota/tangent_plane_of_three_spheres.h"
#include "apollota/tangent_sphere_of_three_spheres.h"
#include "apollota/rotation.h"
#include "apollota/opengl_printer.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/program_options_handler.h"

namespace
{

void add_sphere_from_stream_to_vector(std::istream& input, std::vector<apollota::SimpleSphere>& spheres)
{
	apollota::SimpleSphere sphere;
	input >> sphere.x >> sphere.y >> sphere.z >> sphere.r;
	if(!input.fail())
	{
		spheres.push_back(sphere);
	}
}

void print_demo_bsh(const auxiliaries::ProgramOptionsHandler& poh)
{
	const double init_radius=poh.argument<double>("--init-radius", 3.5);
	if(init_radius<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy initial radius should be greater than 1.");
	}

	const unsigned int max_level=poh.argument<unsigned int>("--max-level", 0);

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", add_sphere_from_stream_to_vector, spheres);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	apollota::BoundingSpheresHierarchy<apollota::SimpleSphere> bsh(spheres, init_radius, 1);

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

	apollota::OpenGLPrinter opengl_printer1(std::cout, "objradii1", "cgo1");
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
	if(scene=="face")
	{
		print_demo_face();
	}
	else
	{
		throw std::runtime_error("Invalid scene name.");
	}
}
