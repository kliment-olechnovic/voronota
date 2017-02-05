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
	Triangle n;
	for(int i=0;i<3;i++)
	{
		const apollota::SimplePoint oa=(apollota::SimplePoint(ab.first)-t.p[i]).unit();
		const apollota::SimplePoint ob=(apollota::SimplePoint(ab.second)-t.p[i]).unit();
		const apollota::SimplePoint x=(oa&ob);
		if(apollota::equal(x.module(), 0.0))
		{
			n.p[i]=(apollota::SimplePoint(ab.second)-apollota::SimplePoint(ab.first)).unit();
		}
		else
		{
			n.p[i]=(x.unit()&((oa+ob)*0.5).unit()).unit();
		}
	}
	draw_triangle(opengl_printer, t, n);
}

inline void draw_triangle(auxiliaries::OpenGLPrinter& opengl_printer, const Triangle& t, const bool n)
{
	draw_triangle(opengl_printer, t, ((t.p[1]-t.p[0])&(t.p[2]-t.p[0])).unit()*(n ? 1.0 : -1.0));
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
	pohw.describe_io("stdin", true, false, "nothing");
	pohw.describe_io("stdout", false, true, "nothing");

	const double probe=poh.restrict_value_in_range(0.01, 100.0, poh.argument<double>(pohw.describe_option("--probe", "number", "probe radius"), 1.5));
	const double big_probe=poh.restrict_value_in_range(probe+0.01, 100.0, poh.argument<double>(pohw.describe_option("--big-probe", "number", "big probe radius"), 3.0));
	const int depth=poh.restrict_value_in_range(0, 6, poh.argument<int>(pohw.describe_option("--depth", "number", "probe radius"), 3));
	const int balls_count=poh.restrict_value_in_range(3, 4, poh.argument<int>(pohw.describe_option("--balls-count", "number", "number of balls (3 or 4)"), 4));
	const std::string name_prefix=poh.argument<std::string>(pohw.describe_option("--name-prefix", "string", "name prefix"), "");
	const std::string output_prefix=poh.argument<std::string>(pohw.describe_option("--output-prefix", "string", "output prefix"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::vector<apollota::SimpleSphere> spheres;
	spheres.push_back(apollota::SimpleSphere(0, 0, 0, 1));
	spheres.push_back(apollota::SimpleSphere(2, 0, 0, 0.5));
	spheres.push_back(apollota::SimpleSphere(1.5, -2, 0, 0.4));
	spheres.push_back(apollota::SimpleSphere(0.5, 0, 2, 0.75));

	int colors_of_singles[4]={0xFF0000, 0x00FF00, 0x0000FF, 0x888888};

	int colors_of_pairs[4][4];
	colors_of_pairs[0][1]=0xFFFF00;
	colors_of_pairs[0][2]=0xFF00FF;
	colors_of_pairs[0][3]=0xFF8888;
	colors_of_pairs[1][2]=0x00FFFF;
	colors_of_pairs[1][3]=0x88FF88;
	colors_of_pairs[2][3]=0x8888FF;
	for(int i=0;i<4;i++)
	{
		for(int j=i+1;j<4;j++)
		{
			colors_of_pairs[j][i]=colors_of_pairs[i][j];
		}
		colors_of_pairs[i][i]=colors_of_singles[i];
	}

	for(int i=0;i<balls_count;i++)
	{
		auxiliaries::OpenGLPrinter opengl_printer;
		opengl_printer.add_color(colors_of_singles[i]);
		opengl_printer.add_sphere(spheres[i]);

		std::ostringstream name;
		name << name_prefix << "ball_" << i;
		std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
		opengl_printer.print_pymol_script(name.str(), true, foutput);
	}

	for(int i=0;i<balls_count;i++)
	{
		auxiliaries::OpenGLPrinter opengl_printer;
		opengl_printer.add_color(colors_of_singles[i]);
		opengl_printer.add_sphere(apollota::SimpleSphere(spheres[i], spheres[i].r+probe-0.01));

		std::ostringstream name;
		name << name_prefix << "sball_" << i;
		std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
		opengl_printer.print_pymol_script(name.str(), true, foutput);
	}

	for(int i=0;i<balls_count;i++)
	{
		for(int j=i+1;j<balls_count;j++)
		{
			const TriangleList big_full_face=init_spheres_intersection_hyperboloid_triangles(
					apollota::SimpleSphere(spheres[i], spheres[i].r+big_probe),
					apollota::SimpleSphere(spheres[j], spheres[j].r+big_probe),
					depth);

			{
				auxiliaries::OpenGLPrinter opengl_printer;
				opengl_printer.add_color(colors_of_pairs[i][j]);
				multiple_draw_triangle(opengl_printer, big_full_face, std::make_pair(spheres[i], spheres[j]));

				std::ostringstream name;
				name << name_prefix << "big_face_" << i << "_" << j;
				std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
				opengl_printer.print_pymol_script(name.str(), true, foutput);
			}

			if(i==0 && j==1)
			{
				auxiliaries::OpenGLPrinter opengl_printer;
				opengl_printer.add_color(0x111111);
				multiple_draw_triangle_lines(opengl_printer, big_full_face);

				std::ostringstream name;
				name << name_prefix << "zbig_face_" << i << "_" << j;
				std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
				opengl_printer.print_pymol_script(name.str(), true, foutput);
			}

			const TriangleList full_face=init_spheres_intersection_hyperboloid_triangles(
					apollota::SimpleSphere(spheres[i], spheres[i].r+probe),
					apollota::SimpleSphere(spheres[j], spheres[j].r+probe),
					depth);

			{
				auxiliaries::OpenGLPrinter opengl_printer;
				opengl_printer.add_color(colors_of_pairs[i][j]);
				multiple_draw_triangle(opengl_printer, full_face, std::make_pair(spheres[i], spheres[j]));

				std::ostringstream name;
				name << name_prefix << "face_" << i << "_" << j;
				std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
				opengl_printer.print_pymol_script(name.str(), true, foutput);
			}

			if(i==0 && j==1)
			{
				auxiliaries::OpenGLPrinter opengl_printer;
				opengl_printer.add_color(0x111111);
				multiple_draw_triangle_lines(opengl_printer, full_face);

				std::ostringstream name;
				name << name_prefix << "zface_" << i << "_" << j;
				std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
				opengl_printer.print_pymol_script(name.str(), true, foutput);
			}

			for(int m=0;m<balls_count;m++)
			{
				if(m!=i && m!=j)
				{
					const TriangleList cut_face_m=multiple_cut_triangle_with_hyperboloid(full_face, spheres[i], spheres[m]).first;

					{
						auxiliaries::OpenGLPrinter opengl_printer;
						opengl_printer.add_color(colors_of_pairs[i][j]);
						multiple_draw_triangle(opengl_printer, cut_face_m, std::make_pair(spheres[i], spheres[j]));

						std::ostringstream name;
						name << name_prefix << "cut_a_" << i << "_" << j << "_" << m;
						std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
						opengl_printer.print_pymol_script(name.str(), true, foutput);
					}

					if(i==0 && j==1)
					{
						auxiliaries::OpenGLPrinter opengl_printer;
						opengl_printer.add_color(0x111111);
						multiple_draw_triangle_lines(opengl_printer, cut_face_m);

						std::ostringstream name;
						name << name_prefix << "zcut_a_" << i << "_" << j << "_" << m;
						std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
						opengl_printer.print_pymol_script(name.str(), true, foutput);
					}

					for(int n=0;n<balls_count;n++)
					{
						if(n!=i && n!=j && n>m)
						{
							const TriangleList cut_face_m_n=multiple_cut_triangle_with_hyperboloid(cut_face_m, spheres[i], spheres[n]).first;

							{
								auxiliaries::OpenGLPrinter opengl_printer;
								opengl_printer.add_color(colors_of_pairs[i][j]);
								multiple_draw_triangle(opengl_printer, cut_face_m_n, std::make_pair(spheres[i], spheres[j]));

								std::ostringstream name;
								name << name_prefix << "cut_b_" << i << "_" << j << "_" << m << "_" << n;
								std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
								opengl_printer.print_pymol_script(name.str(), true, foutput);
							}

							{
								auxiliaries::OpenGLPrinter opengl_printer;
								opengl_printer.add_color(0x111111);
								multiple_draw_triangle_lines(opengl_printer, cut_face_m_n);

								std::ostringstream name;
								name << name_prefix << "zcut_b_" << i << "_" << j << "_" << m << "_" << n;
								std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
								opengl_printer.print_pymol_script(name.str(), true, foutput);
							}
						}
					}
				}
			}
		}
	}

	for(int i=0;i<balls_count;i++)
	{
		TriangleList sface=init_sphere_triangles(apollota::SimpleSphere(spheres[i], spheres[i].r+probe), depth);

		{
			auxiliaries::OpenGLPrinter opengl_printer;
			opengl_printer.add_color(0x777777);
			multiple_draw_triangle_lines(opengl_printer, sface);

			std::ostringstream name;
			name << name_prefix << "wball_" << i;
			std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
			opengl_printer.print_pymol_script(name.str(), true, foutput);
		}

		for(int j=0;j<balls_count;j++)
		{
			if(j!=i)
			{
				sface=multiple_cut_triangle_with_hyperboloid(sface, spheres[i], spheres[j]).first;
			}
		}

		{
			auxiliaries::OpenGLPrinter opengl_printer;
			opengl_printer.add_color(colors_of_singles[i]);
			multiple_draw_triangle(opengl_printer, sface, spheres[i]);

			std::ostringstream name;
			name << name_prefix << "sas_" << i;
			std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
			opengl_printer.print_pymol_script(name.str(), true, foutput);
		}

		{
			auxiliaries::OpenGLPrinter opengl_printer;
			opengl_printer.add_color(0x111111);
			multiple_draw_triangle_lines(opengl_printer, sface);

			std::ostringstream name;
			name << name_prefix << "zsas_" << i;
			std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
			opengl_printer.print_pymol_script(name.str(), true, foutput);
		}
	}

	for(int i=0;i<balls_count;i++)
	{
		for(int j=0;j<balls_count;j++)
		{
			if(i!=j)
			{
				const TriangleList full_face=multiple_cut_triangle_with_hyperboloid(init_sphere_triangles(apollota::SimpleSphere(spheres[i], spheres[i].r+probe), depth), spheres[i], spheres[j]).second;

				{
					auxiliaries::OpenGLPrinter opengl_printer;
					opengl_printer.add_color(colors_of_singles[j]);
					multiple_draw_triangle(opengl_printer, full_face, spheres[i]);

					std::ostringstream name;
					name << name_prefix << "sf_" << i << "_" << j;
					std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
					opengl_printer.print_pymol_script(name.str(), true, foutput);
				}

				if(i==0 && j==1)
				{
					auxiliaries::OpenGLPrinter opengl_printer;
					opengl_printer.add_color(0x111111);
					multiple_draw_triangle_lines(opengl_printer, full_face);

					std::ostringstream name;
					name << name_prefix << "zsf_" << i << "_" << j;
					std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
					opengl_printer.print_pymol_script(name.str(), true, foutput);
				}

				for(int m=0;m<balls_count;m++)
				{
					if(m!=i && m!=j)
					{
						const TriangleList cut_face_m=multiple_cut_triangle_with_hyperboloid(full_face, spheres[j], spheres[m]).first;

						{
							auxiliaries::OpenGLPrinter opengl_printer;
							opengl_printer.add_color(colors_of_singles[j]);
							multiple_draw_triangle(opengl_printer, cut_face_m, spheres[i]);

							std::ostringstream name;
							name << name_prefix << "sc_a_" << i << "_" << j << "_" << m;
							std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
							opengl_printer.print_pymol_script(name.str(), true, foutput);
						}

						if(i==0 && j==1)
						{
							auxiliaries::OpenGLPrinter opengl_printer;
							opengl_printer.add_color(0x111111);
							multiple_draw_triangle_lines(opengl_printer, cut_face_m);

							std::ostringstream name;
							name << name_prefix << "zsc_a_" << i << "_" << j << "_" << m;
							std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
							opengl_printer.print_pymol_script(name.str(), true, foutput);
						}

						for(int n=0;n<balls_count;n++)
						{
							if(n!=i && n!=j && n>m)
							{
								const TriangleList cut_face_m_n=multiple_cut_triangle_with_hyperboloid(cut_face_m, spheres[j], spheres[n]).first;

								{
									auxiliaries::OpenGLPrinter opengl_printer;
									opengl_printer.add_color(colors_of_singles[j]);
									multiple_draw_triangle(opengl_printer, cut_face_m_n, spheres[i]);

									std::ostringstream name;
									name << name_prefix << "sc_b_" << i << "_" << j << "_" << m << "_" << n;
									std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
									opengl_printer.print_pymol_script(name.str(), true, foutput);
								}

								if(i==0 && j==1)
								{
									auxiliaries::OpenGLPrinter opengl_printer;
									opengl_printer.add_color(0x111111);
									multiple_draw_triangle_lines(opengl_printer, cut_face_m_n);

									std::ostringstream name;
									name << name_prefix << "zsc_b_" << i << "_" << j << "_" << m << "_" << n;
									std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
									opengl_printer.print_pymol_script(name.str(), true, foutput);
								}
							}
						}
					}
				}
			}
		}
	}

	for(int i=0;i<=4;i++)
	{
		TriangleList sface=init_sphere_triangles(apollota::SimpleSphere(0, 0, 0, 1), i);

		{
			auxiliaries::OpenGLPrinter opengl_printer;
			opengl_printer.add_color(0xAAAAAA);
			multiple_draw_triangle(opengl_printer, sface, true);

			std::ostringstream name;
			name << name_prefix << "ico_" << i;
			std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
			opengl_printer.print_pymol_script(name.str(), true, foutput);
		}

		{
			auxiliaries::OpenGLPrinter opengl_printer;
			opengl_printer.add_color(0x111111);
			multiple_draw_triangle_lines(opengl_printer, sface);

			std::ostringstream name;
			name << name_prefix << "ico_w_" << i;
			std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
			opengl_printer.print_pymol_script(name.str(), true, foutput);
		}

		{
			auxiliaries::OpenGLPrinter opengl_printer;
			opengl_printer.add_color(0xAAAAAA);
			multiple_draw_triangle(opengl_printer, sface, true);
			opengl_printer.add_color(0x111111);
			multiple_draw_triangle_lines(opengl_printer, sface);

			std::ostringstream name;
			name << name_prefix << "ico_f_" << i;
			std::ofstream foutput((output_prefix+name.str()+".py").c_str(), std::ios::out);
			opengl_printer.print_pymol_script(name.str(), true, foutput);
		}
	}
}
