#include "apollota/basic_operations_on_spheres.h"
#include "apollota/rotation.h"
#include "apollota/search_for_spherical_collisions.h"
#include "apollota/interpolation.h"

#include "auxiliaries/program_options_handler.h"

#include "modescommon/ball_value.h"
#include "modescommon/drawing_utilities.h"

namespace
{

void draw_cylinder(
		const apollota::SimpleSphere& a,
		const apollota::SimpleSphere& b,
		const int sides,
		auxiliaries::OpenGLPrinter& opengl_printer)
{
	apollota::SimplePoint pa(a);
	apollota::SimplePoint pb(b);
	const apollota::SimplePoint axis=(pb-pa);
	apollota::Rotation rotation(axis, 0);
	const apollota::SimplePoint first_point=apollota::any_normal_of_vector<apollota::SimplePoint>(rotation.axis);
	const double angle_step=360.0/static_cast<double>(std::min(std::max(sides, 6), 30));
	std::vector<apollota::SimplePoint> contour;
	contour.reserve(sides+1);
	contour.push_back(first_point);
	for(rotation.angle=angle_step;rotation.angle<360;rotation.angle+=angle_step)
	{
		contour.push_back(rotation.rotate<apollota::SimplePoint>(first_point));
	}
	contour.push_back(first_point);
	std::vector<apollota::SimplePoint> vertices;
	std::vector<apollota::SimplePoint> normals;
	vertices.reserve(contour.size()*2);
	normals.reserve(vertices.size());
	for(std::size_t i=0;i<contour.size();i++)
	{
		const apollota::SimplePoint& p=contour[i];
		vertices.push_back(pa+(p*a.r));
		vertices.push_back(pb+(p*b.r));
		normals.push_back(p);
		normals.push_back(p);
	}
	opengl_printer.add_triangle_strip(vertices, normals);
}

void draw_links(
		const std::vector< std::pair<CRAD, BallValue> >& list_of_balls,
		const double ball_collision_radius,
		const double bsh_initial_radius,
		const double ball_drawing_radius,
		const double cylinder_drawing_radius,
		const int cylinder_sides,
		const bool check_sequence,
		const DrawingParametersWrapper& drawing_parameters_wrapper,
		auxiliaries::OpenGLPrinter& opengl_printer)
{
	std::vector<apollota::SimpleSphere> spheres(list_of_balls.size());
	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		spheres[i]=apollota::SimpleSphere(list_of_balls[i].second, ball_collision_radius);
	}
	apollota::BoundingSpheresHierarchy bsh(spheres, bsh_initial_radius, 1);
	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		const apollota::SimpleSphere& a=spheres[i];
		const CRAD& a_crad=list_of_balls[i].first;
		drawing_parameters_wrapper.process(a_crad, list_of_balls[i].second.props.adjuncts, opengl_printer);
		opengl_printer.add_sphere(apollota::SimpleSphere(a, ball_drawing_radius));
		std::vector<std::size_t> collisions=apollota::SearchForSphericalCollisions::find_all_collisions(bsh, a);
		for(std::size_t j=0;j<collisions.size();j++)
		{
			const std::size_t collision_id=collisions[j];
			if(i!=collision_id)
			{
				const apollota::SimpleSphere& b=spheres[collision_id];
				const CRAD& b_crad=list_of_balls[collision_id].first;
				if(!check_sequence || (a_crad.chainID==b_crad.chainID && abs(a_crad.resSeq-b_crad.resSeq)<=1))
				draw_cylinder(
						apollota::SimpleSphere(a, cylinder_drawing_radius),
						apollota::SimpleSphere(apollota::sum_of_points<apollota::SimplePoint>(a, b)*0.5, cylinder_drawing_radius),
						cylinder_sides,
						opengl_printer);
			}
		}
	}
}



class ResidueOrientation
{
public:
	CRAD crad;
	apollota::SimplePoint CA;
	apollota::SimplePoint C;
	apollota::SimplePoint N;
	apollota::SimplePoint O;
	apollota::SimplePoint up;
	apollota::SimplePoint right;
	bool CA_flag;
	bool C_flag;
	bool N_flag;
	bool O_flag;
	int ss_type;

	ResidueOrientation() : CA_flag(false), C_flag(false), N_flag(false), O_flag(false), ss_type(0)
	{
	}

	static void orient_sequence(std::vector<ResidueOrientation>& seq)
	{
		if(seq.size()>1)
		{
			for(std::size_t i=0;(i+1)<seq.size();i++)
			{
				seq[i].orient(seq[i+1].CA-seq[i].CA);
			}
			seq[seq.size()-1].orient(seq[seq.size()-1].CA-seq[seq.size()-2].CA);
			for(std::size_t i=1;i<seq.size();i++)
			{
				seq[i].reorient(seq[i-1]);
			}
		}
	}

	bool orientable() const
	{
		return (CA_flag && C_flag && N_flag && O_flag);
	}

	void orient(const apollota::SimplePoint& forward)
	{
		if(orientable())
		{
			up=(forward&((O-C).unit()+(N-CA).unit()*0.5)).unit();
			right=(forward&up).unit();
		}
	}

	void reorient(const ResidueOrientation& reference)
	{
		const double v_plus=(right*reference.right);
		const double v_minus=(right.inverted()*reference.right);
		if(v_minus>v_plus)
		{
			up=up.inverted();
			right=right.inverted();
		}
	}
};

int ss_type_from_ball_value(const BallValue& ball_value)
{
	if(ball_value.props.tags.count("dssp=H")>0 || ball_value.props.tags.count("dssp=G")>0 || ball_value.props.tags.count("dssp=I")>0)
	{
		return 1;
	}
	else if(ball_value.props.tags.count("dssp=B")>0 || ball_value.props.tags.count("dssp=E")>0)
	{
		return 2;
	}
	return 0;
}

std::vector< std::vector<ResidueOrientation> > collect_residue_orientations(const std::vector< std::pair<CRAD, BallValue> >& list_of_balls)
{
	std::map<CRAD, ResidueOrientation> map_of_residue_orientations;
	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		const CRAD& crad=list_of_balls[i].first;
		const BallValue& ball_value=list_of_balls[i].second;
		const apollota::SimplePoint ball_center(ball_value);
		ResidueOrientation& ro=map_of_residue_orientations[crad.without_atom()];
		ro.crad=crad.without_atom();
		if(crad.name=="CA")
		{
			ro.CA=ball_center;
			ro.CA_flag=true;
		}
		else if(crad.name=="C")
		{
			ro.C=ball_center;
			ro.C_flag=true;
		}
		else if(crad.name=="N")
		{
			ro.N=ball_center;
			ro.N_flag=true;
		}
		else if(crad.name=="O")
		{
			ro.O=ball_center;
			ro.O_flag=true;
		}
		ro.ss_type=ss_type_from_ball_value(ball_value);
	}
	std::vector< std::vector<ResidueOrientation> > result;
	for(std::map<CRAD, ResidueOrientation>::const_iterator it=map_of_residue_orientations.begin();it!=map_of_residue_orientations.end();++it)
	{
		const ResidueOrientation& ro=it->second;
		if(ro.orientable())
		{
			if(result.empty())
			{
				result.push_back(std::vector<ResidueOrientation>(1, ro));
			}
			else
			{
				const ResidueOrientation& prev_ro=result.back().back();
				if(prev_ro.crad.chainID==ro.crad.chainID && apollota::distance_from_point_to_point(prev_ro.C, ro.N)<1.6)
				{
					result.back().push_back(ro);
				}
				else
				{
					result.push_back(std::vector<ResidueOrientation>(1, ro));
				}
			}
		}
	}
	for(std::size_t i=0;i<result.size();i++)
	{
		ResidueOrientation::orient_sequence(result[i]);
	}
	return result;
}

struct RibbonVertebra
{
	apollota::SimplePoint center;
	apollota::SimplePoint up;
	apollota::SimplePoint right;
};

std::map< CRAD, std::vector<RibbonVertebra> > construct_ribbon_spine(const std::vector<ResidueOrientation>& ros)
{
	const double k=0.8;
	const int steps=10;
	std::map< CRAD, std::vector<RibbonVertebra> > result;
	std::vector<RibbonVertebra> controls(ros.size());
	std::vector<std::size_t> beta_ids;
	for(std::size_t i=0;i<ros.size();i++)
	{
		const ResidueOrientation& ro=ros[i];
		RibbonVertebra& rv=controls[i];
		rv.center=ro.CA;
		rv.up=ro.CA+(ro.up*0.2);
		rv.right=ro.CA+(ro.right*0.5);

		if(ro.ss_type==2)
		{
			beta_ids.push_back(i);
		}
		else
		{
			if(beta_ids.size()>2)
			{
				std::vector<apollota::SimplePoint> controls_center(beta_ids.size());
				std::vector<apollota::SimplePoint> controls_up(beta_ids.size());
				std::vector<apollota::SimplePoint> controls_right(beta_ids.size());
				for(std::size_t j=0;j<beta_ids.size();j++)
				{
					const RibbonVertebra& rv=controls[beta_ids[j]];
					controls_center[j]=rv.center;
					controls_up[j]=rv.up;
					controls_right[j]=rv.right;
				}
				for(std::size_t j=1;(j+1)<beta_ids.size();j++)
				{
					const double t=1.0/static_cast<double>(beta_ids.size()-1)*static_cast<double>(j);
					RibbonVertebra& rv=controls[beta_ids[j]];
					rv.center=apollota::bezier_curve_point(controls_center, t);
					rv.up=apollota::bezier_curve_point(controls_up, t);
					rv.right=apollota::bezier_curve_point(controls_right, t);
				}
			}
			beta_ids.clear();
		}
	}
	if(ros.size()>=4)
	{
		for(std::size_t i=0;i+1<controls.size();i++)
		{
			std::vector<apollota::SimplePoint> strip_center;
			std::vector<apollota::SimplePoint> strip_up;
			std::vector<apollota::SimplePoint> strip_right;
			if(i==0)
			{
				strip_center=apollota::interpolate_using_cubic_hermite_spline(controls[0].center+(controls[0].center-controls[1].center), controls[0].center, controls[1].center, controls[2].center, k, steps);
				strip_up=apollota::interpolate_using_cubic_hermite_spline(controls[0].up+(controls[0].up-controls[1].up), controls[0].up, controls[1].up, controls[2].up, k, steps);
				strip_right=apollota::interpolate_using_cubic_hermite_spline(controls[0].right+(controls[0].right-controls[1].right), controls[0].right, controls[1].right, controls[2].right, k, steps);
			}
			else if(i+2==controls.size())
			{
				strip_center=apollota::interpolate_using_cubic_hermite_spline(controls[i-1].center, controls[i].center, controls[i+1].center, controls[i+1].center+(controls[i+1].center-controls[i].center), k, steps);
				strip_up=apollota::interpolate_using_cubic_hermite_spline(controls[i-1].up, controls[i].up, controls[i+1].up, controls[i+1].up+(controls[i+1].up-controls[i].up), k, steps);
				strip_right=apollota::interpolate_using_cubic_hermite_spline(controls[i-1].right, controls[i].right, controls[i+1].right, controls[i+1].right+(controls[i+1].right-controls[i].right), k, steps);
			}
			else
			{
				strip_center=apollota::interpolate_using_cubic_hermite_spline(controls[i-1].center, controls[i].center, controls[i+1].center, controls[i+2].center, k, steps);
				strip_up=apollota::interpolate_using_cubic_hermite_spline(controls[i-1].up, controls[i].up, controls[i+1].up, controls[i+2].up, k, steps);
				strip_right=apollota::interpolate_using_cubic_hermite_spline(controls[i-1].right, controls[i].right, controls[i+1].right, controls[i+2].right, k, steps);
			}
			if(!strip_center.empty() && strip_center.size()==strip_up.size() && strip_center.size()==strip_right.size())
			{
				std::vector<RibbonVertebra>& result_a=result[ros[i].crad];
				std::vector<RibbonVertebra>& result_b=result[ros[i+1].crad];
				result_a.reserve(strip_center.size()/2+1);
				result_b.reserve(strip_center.size()/2+1);
				for(std::size_t j=0;j<strip_center.size();j++)
				{
					RibbonVertebra v;
					v.center=strip_center[j];
					v.up=strip_up[j];
					v.right=strip_right[j];
					if(j<strip_center.size()/2)
					{
						result_a.push_back(v);
					}
					else if(j==strip_center.size()/2)
					{
						result_a.push_back(v);
						result_b.push_back(v);
					}
					else
					{
						result_b.push_back(v);
					}
				}
			}
		}
	}
	return result;
}

std::map< CRAD, std::vector<RibbonVertebra> > construct_ribbon_spine(const std::vector< std::pair<CRAD, BallValue> >& list_of_balls)
{
	std::map< CRAD, std::vector<RibbonVertebra> > result;
	const std::vector< std::vector<ResidueOrientation> > residue_orientations=collect_residue_orientations(list_of_balls);
	for(std::size_t i=0;i<residue_orientations.size();i++)
	{
		std::map< CRAD, std::vector<RibbonVertebra> > partial_result=construct_ribbon_spine(residue_orientations[i]);
		result.insert(partial_result.begin(), partial_result.end());
	}
	return result;
}

void draw_cartoon(
		const std::vector< std::pair<CRAD, BallValue> >& list_of_balls,
		const DrawingParametersWrapper& drawing_parameters_wrapper,
		auxiliaries::OpenGLPrinter& opengl_printer)
{
	std::map<CRAD, BallValue> map_of_crad_values;
	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		const CRAD& crad=list_of_balls[i].first;
		if(crad.name=="CA")
		{
			map_of_crad_values[crad.without_atom()]=list_of_balls[i].second;
		}
	}
	const std::map< CRAD, std::vector<RibbonVertebra> > spine=construct_ribbon_spine(list_of_balls);
	for(std::map< CRAD, std::vector<RibbonVertebra> >::const_iterator it=spine.begin();it!=spine.end();++it)
	{
		const CRAD& crad=it->first;
		const BallValue& ball_value=map_of_crad_values[crad];
		const std::vector<RibbonVertebra>& subspine=it->second;

		if(subspine.size()>1)
		{
			drawing_parameters_wrapper.process(crad, ball_value.props.adjuncts, opengl_printer);

			const int ss_type=ss_type_from_ball_value(ball_value);

//			int next_ss_type=0;
//			{
//				std::map< CRAD, std::vector<RibbonVertebra> >::const_iterator next_it=it;
//				++next_it;
//				if(next_it!=spine.end())
//				{
//					next_ss_type=ss_type_from_ball_value(map_of_crad_values[next_it->first]);
//				}
//			}

			const double wk=(ss_type==0 ? 0.5 : 2.0);

			for(int e=0;e<2;e++)
			{
				{
					std::vector<apollota::SimplePoint> vertices;
					std::vector<apollota::SimplePoint> normals;
					for(std::size_t i=0;i<subspine.size();i++)
					{
						const RibbonVertebra& rv=subspine[i];
						const apollota::SimplePoint c=rv.center;
						const apollota::SimplePoint u=(rv.up-c)*(e==0 ? 1.0 : -1.0);
						const apollota::SimplePoint r=(rv.right-c)*wk;
						const apollota::SimplePoint l=r.inverted();
						vertices.push_back(c+l+u);
						vertices.push_back(c+r+u);
						normals.push_back(u.unit());
						normals.push_back(u.unit());
					}
					opengl_printer.add_triangle_strip(vertices, normals);
				}

				{
					std::vector<apollota::SimplePoint> vertices;
					std::vector<apollota::SimplePoint> normals;
					for(std::size_t i=0;i<subspine.size();i++)
					{
						const RibbonVertebra& rv=subspine[i];
						const apollota::SimplePoint c=rv.center;
						const apollota::SimplePoint u=(rv.right-c)*(e==0 ? 1.0 : -1.0)*wk;
						const apollota::SimplePoint r=(rv.up-c);
						const apollota::SimplePoint l=r.inverted();
						vertices.push_back(c+l+u);
						vertices.push_back(c+r+u);
						normals.push_back(u.unit());
						normals.push_back(u.unit());
					}
					opengl_printer.add_triangle_strip(vertices, normals);
				}

				{
					const RibbonVertebra& rv=(e==0 ? subspine.front() : subspine.back());
					const apollota::SimplePoint c=rv.center;
					const apollota::SimplePoint u=(rv.up-c);
					const apollota::SimplePoint d=u.inverted();
					const apollota::SimplePoint r=(rv.right-c)*wk;
					const apollota::SimplePoint l=r.inverted();
					const apollota::SimplePoint n=((subspine.front().center-subspine.back().center)*(e==0 ? 1.0 : -1.0)).unit();
					std::vector<apollota::SimplePoint> vertices;
					std::vector<apollota::SimplePoint> normals;
					vertices.push_back(c+d+l);
					vertices.push_back(c+d+r);
					vertices.push_back(c+u+l);
					vertices.push_back(c+u+r);
					normals.push_back(n.unit());
					normals.push_back(n.unit());
					normals.push_back(n.unit());
					normals.push_back(n.unit());
					opengl_printer.add_triangle_strip(vertices, normals);
				}
			}
		}
	}
}

}

void draw_balls(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'annotation x y z r tags adjuncts')");
	pohw.describe_io("stdout", false, true, "list of balls (line format: 'annotation x y z r tags adjuncts')");

	const std::string representation=poh.argument<std::string>(pohw.describe_option("--representation", "string", "representation name: 'vdw', 'sticks' or 'trace'"), "vdw");
	const std::string drawing_for_pymol=poh.argument<std::string>(pohw.describe_option("--drawing-for-pymol", "string", "file path to output drawing as pymol script"), "");
	const std::string drawing_for_scenejs=poh.argument<std::string>(pohw.describe_option("--drawing-for-scenejs", "string", "file path to output drawing as scenejs script"), "");
	const std::string drawing_name=poh.argument<std::string>(pohw.describe_option("--drawing-name", "string", "graphics object name for drawing output"), "contacts");
	DrawingParametersWrapper drawing_parameters_wrapper;
	drawing_parameters_wrapper.default_color=poh.convert_hex_string_to_integer<unsigned int>(poh.argument<std::string>(pohw.describe_option("--default-color", "string", "default color for drawing output, in hex format, white is 0xFFFFFF"), "0xFFFFFF"));
	drawing_parameters_wrapper.adjunct_gradient=poh.argument<std::string>(pohw.describe_option("--adjunct-gradient", "string", "adjunct name to use for gradient-based coloring"), "");
	drawing_parameters_wrapper.adjunct_gradient_blue=poh.argument<double>(pohw.describe_option("--adjunct-gradient-blue", "number", "blue adjunct gradient value"), 0.0);
	drawing_parameters_wrapper.adjunct_gradient_red=poh.argument<double>(pohw.describe_option("--adjunct-gradient-red", "number", "red adjunct gradient value"), 1.0);
	drawing_parameters_wrapper.adjuncts_rgb=poh.contains_option(pohw.describe_option("--adjuncts-rgb", "", "flag to use RGB color values from adjuncts"));
	drawing_parameters_wrapper.random_colors=poh.contains_option(pohw.describe_option("--random-colors", "", "flag to use random color for each drawn ball"));
	drawing_parameters_wrapper.use_labels=poh.contains_option(pohw.describe_option("--use-labels", "", "flag to use labels in drawing if possible"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	typedef std::vector< std::pair<CRAD, BallValue> > ListOfBalls;
	const ListOfBalls list_of_balls=auxiliaries::IOUtilities().read_lines_to_map<ListOfBalls>(std::cin);
	if(list_of_balls.empty())
	{
		throw std::runtime_error("No input.");
	}

	auxiliaries::OpenGLPrinter opengl_printer;
	opengl_printer.add_color(drawing_parameters_wrapper.default_color);
	if(representation=="vdw")
	{
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const CRAD& crad=list_of_balls[i].first;
			const BallValue& value=list_of_balls[i].second;
			drawing_parameters_wrapper.process(crad, value.props.adjuncts, opengl_printer);
			opengl_printer.add_sphere(value);
		}
	}
	else if(representation=="sticks")
	{
		draw_links(list_of_balls, 0.8, 4.0, 0.3, 0.2, 6, false, drawing_parameters_wrapper, opengl_printer);
	}
	else if(representation=="trace")
	{
		std::vector< std::pair<CRAD, BallValue> > list_of_balls_filtered;
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			if(list_of_balls[i].first.name=="CA")
			{
				list_of_balls_filtered.push_back(list_of_balls[i]);
			}
		}
		draw_links(list_of_balls_filtered, 2.0, 10.0, 0.3, 0.3, 12, true, drawing_parameters_wrapper, opengl_printer);
	}
	else if(representation=="cartoon")
	{
		draw_cartoon(list_of_balls, drawing_parameters_wrapper, opengl_printer);
	}
	else
	{
		throw std::runtime_error("Invalid representation name.");
	}

	if(!drawing_for_pymol.empty())
	{
		std::ofstream foutput(drawing_for_pymol.c_str(), std::ios::out);
		if(foutput.good())
		{
			opengl_printer.print_pymol_script(drawing_name, true, foutput);
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

	auxiliaries::IOUtilities().write_map(list_of_balls, std::cout);
}
