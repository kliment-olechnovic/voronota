#include "apollota/basic_operations_on_spheres.h"
#include "apollota/subdivided_icosahedron.h"
#include "apollota/hyperboloid_between_two_spheres.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/opengl_printer.h"
#include "auxiliaries/program_options_handler.h"

namespace
{

struct Triangle
{
	std::vector<apollota::SimplePoint> p;

	Triangle() : p(3)
	{
	}

	Triangle(const apollota::SimplePoint& a, const apollota::SimplePoint& b, const apollota::SimplePoint& c) : p(3)
	{
		p[0]=a;
		p[1]=b;
		p[2]=c;
	}
};

typedef std::vector<Triangle> TriangleList;

typedef std::pair<TriangleList, TriangleList> TriangleListSplit;

inline TriangleList subdivide_triangle_in_four(const Triangle& t)
{
	TriangleList result(4);
	result[0]=Triangle((t.p[0]+t.p[1])*0.5, (t.p[0]+t.p[2])*0.5, (t.p[1]+t.p[2])*0.5);
	result[1]=Triangle(t.p[0], result[0].p[0], result[0].p[1]);
	result[1]=Triangle(t.p[1], result[0].p[0], result[0].p[2]);
	result[1]=Triangle(t.p[2], result[0].p[1], result[0].p[2]);
	return result;
}

inline TriangleList subdivide_triangle_in_three(const Triangle& t)
{
	const apollota::SimplePoint c=(t.p[0]+t.p[1]+t.p[2])*(1.0/3.0);
	TriangleList result(3);
	result[0]=Triangle(t.p[0], t.p[1], c);
	result[1]=Triangle(t.p[0], t.p[2], c);
	result[2]=Triangle(t.p[1], t.p[2], c);
	return result;
}

inline TriangleList subdivide_triangle(const Triangle& t, const bool in_three)
{
	return (in_three ? subdivide_triangle_in_three(t) : subdivide_triangle_in_four(t));
}

inline TriangleList multiple_subdivide_triangle(const TriangleList& tl, const bool in_three)
{
	TriangleList result;
	for(std::size_t i=0;i<tl.size();i++)
	{
		const TriangleList subresult=subdivide_triangle(tl[i], in_three);
		result.insert(result.end(), subresult.begin(), subresult.end());
	}
	return result;
}

inline Triangle project_triangle_on_hyperboloid(const Triangle& t, const apollota::SimpleSphere& a, const apollota::SimpleSphere& b)
{
	return Triangle(
			apollota::HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(t.p[0], a, b),
			apollota::HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(t.p[1], a, b),
			apollota::HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(t.p[2], a, b));
}

inline TriangleList multiple_project_triangle_on_hyperboloid(const TriangleList& tl, const apollota::SimpleSphere& a, const apollota::SimpleSphere& b)
{
	TriangleList result;
	result.reserve(tl.size());
	for(std::size_t i=0;i<tl.size();i++)
	{
		result.push_back(project_triangle_on_hyperboloid(tl[i], a, b));
	}
	return result;
}

inline TriangleListSplit cut_triangle_with_hyperboloid(const Triangle& t, const apollota::SimpleSphere& a, const apollota::SimpleSphere& b)
{
	bool mask[3];
	for(int i=0;i<3;i++)
	{
		mask[i]=(apollota::minimal_distance_from_point_to_sphere(t.p[i], a)<apollota::minimal_distance_from_point_to_sphere(t.p[i], b));
	}

	if(mask[0] && mask[1] && mask[2])
	{
		return TriangleListSplit(TriangleList(1, t), TriangleList());
	}
	else if(!mask[0] && !mask[1] && !mask[2])
	{
		return TriangleListSplit(TriangleList(), TriangleList(1, t));
	}

	int ids[3]={0, 1, 2};
	if(mask[0]!=mask[1] && mask[1]==mask[2])
	{
		ids[0]=0;
		ids[1]=1;
		ids[2]=2;
	}
	else if(mask[1]!=mask[0] && mask[0]==mask[2])
	{
		ids[0]=1;
		ids[1]=0;
		ids[2]=2;
	}
	else if(mask[2]!=mask[0] && mask[0]==mask[1])
	{
		ids[0]=2;
		ids[1]=0;
		ids[2]=1;
	}

	TriangleListSplit result;
	{
		const apollota::SimplePoint m01=t.p[ids[0]]+((t.p[ids[1]]-t.p[ids[0]]).unit()*apollota::HyperboloidBetweenTwoSpheres::intersect_vector_with_hyperboloid(t.p[ids[0]], t.p[ids[1]], a, b));
		const apollota::SimplePoint m02=t.p[ids[0]]+((t.p[ids[2]]-t.p[ids[0]]).unit()*apollota::HyperboloidBetweenTwoSpheres::intersect_vector_with_hyperboloid(t.p[ids[0]], t.p[ids[2]], a, b));
		result.first.push_back(Triangle(t.p[ids[0]], m01, m02));
		result.second.push_back(Triangle(m01, t.p[ids[1]], t.p[ids[2]]));
		result.second.push_back(Triangle(m01, m02, t.p[ids[2]]));
		if(!mask[ids[0]])
		{
			std::swap(result.first, result.second);
		}
	}
	return result;
}

inline TriangleListSplit multiple_cut_triangle_with_hyperboloid(const TriangleList& tl, const apollota::SimpleSphere& a, const apollota::SimpleSphere& b)
{
	TriangleListSplit result;
	for(std::size_t i=0;i<tl.size();i++)
	{
		const TriangleListSplit subresult=cut_triangle_with_hyperboloid(tl[i], a, b);
		result.first.insert(result.first.end(), subresult.first.begin(), subresult.first.end());
		result.second.insert(result.second.end(), subresult.second.begin(), subresult.second.end());
	}
	return result;
}

inline TriangleList init_sphere_triangles(const apollota::SimpleSphere& s, const int depth)
{
	apollota::SubdividedIcosahedron sih(depth);
	sih.fit_into_sphere(s, s.r);
	TriangleList result;
	result.reserve(sih.triples().size());
	for(std::size_t i=0;i<sih.triples().size();i++)
	{
		const apollota::Triple& t=sih.triples()[i];
		result.push_back(Triangle(sih.vertices()[t.get(0)], sih.vertices()[t.get(1)], sih.vertices()[t.get(2)]));
	}
	return result;
}

inline TriangleList init_spheres_intersection_hyperboloid_triangles(const apollota::SimpleSphere& a, const apollota::SimpleSphere& b, const int depth)
{
	if(a.r<b.r)
	{
		return init_spheres_intersection_hyperboloid_triangles(b, a, depth);
	}
	if(apollota::sphere_intersects_sphere(a, b))
	{
		const TriangleListSplit tls=multiple_cut_triangle_with_hyperboloid(init_sphere_triangles(a, depth), a, b);
		return multiple_project_triangle_on_hyperboloid((tls.first.size()<tls.second.size() ? tls.first : tls.second), a, b);
	}
	else
	{
		return TriangleList();
	}
}

inline void draw_triangle(auxiliaries::OpenGLPrinter& opengl_printer, const Triangle& t, const Triangle& n)
{
	opengl_printer.add_triangle_strip(t.p, n.p);
}

inline void draw_triangle(auxiliaries::OpenGLPrinter& opengl_printer, const Triangle& t, const apollota::SimplePoint& n)
{
	draw_triangle(opengl_printer, t, Triangle(n, n, n));
}

inline void draw_triangle(auxiliaries::OpenGLPrinter& opengl_printer, const Triangle& t, const apollota::SimpleSphere& a)
{
	draw_triangle(opengl_printer, t, Triangle((t.p[0]-apollota::SimplePoint(a)).unit(), (t.p[1]-apollota::SimplePoint(a)).unit(), (t.p[2]-apollota::SimplePoint(a)).unit()));
}

inline void draw_triangle(auxiliaries::OpenGLPrinter& opengl_printer, const Triangle& t, const std::pair<apollota::SimpleSphere, apollota::SimpleSphere>& ab)
{
	draw_triangle(opengl_printer, t, (apollota::SimplePoint(ab.second)-apollota::SimplePoint(ab.first)).unit());
}

template <typename NormalDescriptor>
inline void multiple_draw_triangle(auxiliaries::OpenGLPrinter& opengl_printer, const TriangleList& tl, const NormalDescriptor& nd)
{
	for(std::size_t i=0;i<tl.size();i++)
	{
		draw_triangle(opengl_printer, tl[i], nd);
	}
}

inline void draw_triangle_lines(auxiliaries::OpenGLPrinter& opengl_printer, const Triangle& t)
{
	opengl_printer.add_line_loop(t.p);
}

inline void multiple_draw_triangle_lines(auxiliaries::OpenGLPrinter& opengl_printer, const TriangleList& tl)
{
	for(std::size_t i=0;i<tl.size();i++)
	{
		draw_triangle_lines(opengl_printer, tl[i]);
	}
}

}

void demo_hypercut(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'x y z r')");
	pohw.describe_io("stdout", false, true, "nothing");

	const double probe=poh.restrict_value_in_range(0.01, 100.0, poh.argument<double>(pohw.describe_option("--probe", "number", "probe radius"), 1.4));
	const int depth=poh.restrict_value_in_range(0, 6, poh.argument<int>(pohw.describe_option("--probe", "number", "probe radius"), 3));
	const std::string output_prefix=poh.argument<std::string>(pohw.describe_option("--output-prefix", "string", "output prefix"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::IOUtilities().read_lines_to_set(std::cin, spheres);
	if(spheres.size()<2)
	{
		throw std::runtime_error("Less than 2 balls provided to stdin.");
	}

	{
		auxiliaries::OpenGLPrinter opengl_printer;
		opengl_printer.add_color(0xFF0000);
		opengl_printer.add_sphere(spheres[0]);
		opengl_printer.add_color(0x00FF00);
		opengl_printer.add_sphere(spheres[1]);

		std::ofstream foutput((output_prefix+"balls_ab.py").c_str(), std::ios::out);
		if(foutput.good())
		{
			opengl_printer.print_pymol_script("balls_ab", true, foutput);
		}
	}

	const TriangleList full_face_ab=init_spheres_intersection_hyperboloid_triangles(
			apollota::SimpleSphere(spheres[0], spheres[0].r+probe),
			apollota::SimpleSphere(spheres[1], spheres[1].r+probe),
			depth);

	{
		auxiliaries::OpenGLPrinter opengl_printer;
		opengl_printer.add_color(0xFFFF00);
		multiple_draw_triangle(opengl_printer, full_face_ab, std::make_pair(spheres[0], spheres[1]));

		std::ofstream foutput((output_prefix+"full_face_ab.py").c_str(), std::ios::out);
		if(foutput.good())
		{
			opengl_printer.print_pymol_script("full_face_ab", true, foutput);
		}
	}

	{
		auxiliaries::OpenGLPrinter opengl_printer;
		opengl_printer.add_color(0x333333);
		multiple_draw_triangle_lines(opengl_printer, full_face_ab);

		std::ofstream foutput((output_prefix+"full_face_mesh.py").c_str(), std::ios::out);
		if(foutput.good())
		{
			opengl_printer.print_pymol_script("full_face_mesh", true, foutput);
		}
	}

	if(spheres.size()>=3)
	{
		{
			auxiliaries::OpenGLPrinter opengl_printer;
			opengl_printer.add_color(0xFF0000);
			opengl_printer.add_sphere(spheres[0]);
			opengl_printer.add_color(0x00FF00);
			opengl_printer.add_sphere(spheres[1]);
			opengl_printer.add_color(0x0000FF);
			opengl_printer.add_sphere(spheres[2]);

			std::ofstream foutput((output_prefix+"balls_abc.py").c_str(), std::ios::out);
			if(foutput.good())
			{
				opengl_printer.print_pymol_script("balls_abc", true, foutput);
			}
		}

		const TriangleList full_face_ac=init_spheres_intersection_hyperboloid_triangles(
				apollota::SimpleSphere(spheres[0], spheres[0].r+probe),
				apollota::SimpleSphere(spheres[2], spheres[2].r+probe),
				depth);

		{
			auxiliaries::OpenGLPrinter opengl_printer;
			opengl_printer.add_color(0xFF00FF);
			multiple_draw_triangle(opengl_printer, full_face_ac, std::make_pair(spheres[0], spheres[2]));

			std::ofstream foutput((output_prefix+"full_face_ac.py").c_str(), std::ios::out);
			if(foutput.good())
			{
				opengl_printer.print_pymol_script("full_face_ac", true, foutput);
			}
		}

		const TriangleList cut_face_ab_c=multiple_cut_triangle_with_hyperboloid(full_face_ab, spheres[0], spheres[2]).first;

		{
			auxiliaries::OpenGLPrinter opengl_printer;
			opengl_printer.add_color(0xFFFF00);
			multiple_draw_triangle(opengl_printer, cut_face_ab_c, std::make_pair(spheres[0], spheres[1]));

			std::ofstream foutput((output_prefix+"cut_face_ab_c.py").c_str(), std::ios::out);
			if(foutput.good())
			{
				opengl_printer.print_pymol_script("cut_face_ab_c", true, foutput);
			}
		}

		const TriangleList cut_face_ac_b=multiple_cut_triangle_with_hyperboloid(full_face_ac, spheres[0], spheres[1]).first;

		{
			auxiliaries::OpenGLPrinter opengl_printer;
			opengl_printer.add_color(0xFF00FF);
			multiple_draw_triangle(opengl_printer, cut_face_ac_b, std::make_pair(spheres[0], spheres[1]));

			std::ofstream foutput((output_prefix+"cut_face_ac_b.py").c_str(), std::ios::out);
			if(foutput.good())
			{
				opengl_printer.print_pymol_script("cut_face_ac_b", true, foutput);
			}
		}
	}
}
