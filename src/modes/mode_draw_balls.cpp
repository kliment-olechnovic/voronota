#include "../apollota/search_for_spherical_collisions.h"
#include "../apollota/rotation.h"
#include "../apollota/polynomial_curves.h"

#include "../auxiliaries/program_options_handler.h"

#include "../common/ball_value.h"

#include "modescommon/drawing_utilities.h"

namespace
{

typedef voronota::common::ChainResidueAtomDescriptor CRAD;

void draw_cylinder(
		const voronota::apollota::SimpleSphere& a,
		const voronota::apollota::SimpleSphere& b,
		const int sides,
		voronota::auxiliaries::OpenGLPrinter& opengl_printer)
{
	voronota::apollota::SimplePoint pa(a);
	voronota::apollota::SimplePoint pb(b);
	const voronota::apollota::SimplePoint axis=(pb-pa);
	voronota::apollota::Rotation rotation(axis, 0);
	const voronota::apollota::SimplePoint first_point=voronota::apollota::any_normal_of_vector<voronota::apollota::SimplePoint>(rotation.axis);
	const double angle_step=360.0/static_cast<double>(std::min(std::max(sides, 6), 30));
	std::vector<voronota::apollota::SimplePoint> contour;
	contour.reserve(sides+1);
	contour.push_back(first_point);
	for(rotation.angle=angle_step;rotation.angle<360;rotation.angle+=angle_step)
	{
		contour.push_back(rotation.rotate<voronota::apollota::SimplePoint>(first_point));
	}
	contour.push_back(first_point);
	std::vector<voronota::apollota::SimplePoint> vertices;
	std::vector<voronota::apollota::SimplePoint> normals;
	vertices.reserve(contour.size()*2);
	normals.reserve(vertices.size());
	for(std::size_t i=0;i<contour.size();i++)
	{
		const voronota::apollota::SimplePoint& p=contour[i];
		vertices.push_back(pa+(p*a.r));
		vertices.push_back(pb+(p*b.r));
		normals.push_back(p);
		normals.push_back(p);
	}
	opengl_printer.add_triangle_strip(vertices, normals);
}

void draw_links(
		const std::vector< std::pair<CRAD, voronota::common::BallValue> >& list_of_balls,
		const double ball_collision_radius,
		const double bsh_initial_radius,
		const double ball_drawing_radius,
		const double cylinder_drawing_radius,
		const int cylinder_sides,
		const bool check_sequence,
		const voronota::modescommon::DrawingParametersWrapper& drawing_parameters_wrapper,
		voronota::auxiliaries::OpenGLPrinter& opengl_printer)
{
	std::vector<voronota::apollota::SimpleSphere> spheres(list_of_balls.size());
	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		spheres[i]=voronota::apollota::SimpleSphere(list_of_balls[i].second, ball_collision_radius);
	}
	voronota::apollota::BoundingSpheresHierarchy bsh(spheres, bsh_initial_radius, 1);
	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		const voronota::apollota::SimpleSphere& a=spheres[i];
		const CRAD& a_crad=list_of_balls[i].first;
		drawing_parameters_wrapper.process(a_crad, list_of_balls[i].second.props.adjuncts, opengl_printer);
		opengl_printer.add_sphere(voronota::apollota::SimpleSphere(a, ball_drawing_radius));
		std::vector<std::size_t> collisions=voronota::apollota::SearchForSphericalCollisions::find_all_collisions(bsh, a);
		for(std::size_t j=0;j<collisions.size();j++)
		{
			const std::size_t collision_id=collisions[j];
			if(i!=collision_id)
			{
				const voronota::apollota::SimpleSphere& b=spheres[collision_id];
				const CRAD& b_crad=list_of_balls[collision_id].first;
				if(!check_sequence || (a_crad.chainID==b_crad.chainID && abs(a_crad.resSeq-b_crad.resSeq)<=1))
				draw_cylinder(
						voronota::apollota::SimpleSphere(a, cylinder_drawing_radius),
						voronota::apollota::SimpleSphere(voronota::apollota::sum_of_points<voronota::apollota::SimplePoint>(a, b)*0.5, cylinder_drawing_radius),
						cylinder_sides,
						opengl_printer);
			}
		}
	}
}

void draw_trace(
		const std::vector< std::pair<CRAD, voronota::common::BallValue> >& list_of_balls,
		const std::string& atom_name,
		const double max_distance,
		const double drawing_radius,
		const voronota::modescommon::DrawingParametersWrapper& drawing_parameters_wrapper,
		voronota::auxiliaries::OpenGLPrinter& opengl_printer)
{
	std::vector< std::pair<CRAD, voronota::common::BallValue> > list_of_balls_filtered;
	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		if(list_of_balls[i].first.name==atom_name)
		{
			list_of_balls_filtered.push_back(list_of_balls[i]);
		}
	}
	draw_links(list_of_balls_filtered, max_distance*0.5, 10.0, drawing_radius, drawing_radius, 12, true, drawing_parameters_wrapper, opengl_printer);
}

class DrawingProteinCartoons
{
public:
	double loop_width;
	double loop_height;
	double nonloop_width;
	double nonloop_height;
	double arrow_width;
	double hermite_spline_k;
	double hermite_spline_steps;
	bool enable_arrows;

	DrawingProteinCartoons() :
		loop_width(0.30),
		loop_height(0.15),
		nonloop_width(1.2),
		nonloop_height(0.15),
		arrow_width(1.6),
		hermite_spline_k(0.8),
		hermite_spline_steps(10),
		enable_arrows(true)
	{
	}

	void draw_cartoon(
			const std::vector< std::pair<CRAD, voronota::common::BallValue> >& list_of_balls,
			const voronota::modescommon::DrawingParametersWrapper& drawing_parameters_wrapper,
			voronota::auxiliaries::OpenGLPrinter& opengl_printer)
	{
		std::map<CRAD, voronota::common::BallValue> map_of_crad_values;
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const CRAD& crad=list_of_balls[i].first;
			if(crad.name=="CA")
			{
				map_of_crad_values[crad.without_atom()]=list_of_balls[i].second;
			}
		}
		const std::map< CRAD, std::vector<RibbonVertebra> > spine=construct_ribbon_spine(list_of_balls, hermite_spline_k, hermite_spline_steps);
		for(std::map< CRAD, std::vector<RibbonVertebra> >::const_iterator it=spine.begin();it!=spine.end();++it)
		{
			const CRAD& crad=it->first;
			const voronota::common::BallValue& ball_value=map_of_crad_values[crad];
			const std::vector<RibbonVertebra>& subspine=it->second;
			if(subspine.size()>1)
			{
				drawing_parameters_wrapper.process(crad, ball_value.props.adjuncts, opengl_printer);

				const int ss_type=subspine.front().ss_type;

				int prev_ss_type=0;
				if(it!=spine.begin())
				{
					std::map< CRAD, std::vector<RibbonVertebra> >::const_iterator prev_it=it;
					--prev_it;
					prev_ss_type=prev_it->second.front().ss_type;
				}

				int next_ss_type=0;
				{
					std::map< CRAD, std::vector<RibbonVertebra> >::const_iterator next_it=it;
					++next_it;
					if(next_it!=spine.end())
					{
						next_ss_type=next_it->second.front().ss_type;
					}
				}

				const bool loop=(ss_type==0);
				const double wk=(loop ? loop_width : nonloop_width);
				const double hk=(loop ? loop_height : nonloop_height);

				const bool arrow=(enable_arrows && (prev_ss_type==2 && ss_type==2 && next_ss_type!=2));

				for(int e=0;e<2;e++)
				{
					for(int o=0;o<2;o++)
					{
						std::vector<voronota::apollota::SimplePoint> vertices;
						std::vector<voronota::apollota::SimplePoint> normals;
						for(std::size_t i=0;i<subspine.size();i++)
						{
							double mwk=wk;
							if(arrow)
							{
								mwk=(1.0-static_cast<double>(i)/static_cast<double>(subspine.size()-1))*arrow_width+loop_width;
							}
							const RibbonVertebra& rv=subspine[i];
							const voronota::apollota::SimplePoint c=rv.center;
							const voronota::apollota::SimplePoint u=(o==0 ? ((rv.up-c).unit()*(e==0 ? 1.0 : -1.0)*hk) : ((rv.right-c).unit()*(e==0 ? 1.0 : -1.0)*mwk));
							const voronota::apollota::SimplePoint r=(o==0 ? ((rv.right-c).unit()*mwk) : ((rv.up-c).unit()*hk));
							const voronota::apollota::SimplePoint l=r.inverted();
							vertices.push_back(c+l+u);
							vertices.push_back(c+r+u);
							normals.push_back(u.unit());
							normals.push_back(u.unit());
						}
						opengl_printer.add_triangle_strip(vertices, normals);
					}

					{
						double mwk=wk;
						if(arrow)
						{
							mwk=(e==0 ? (arrow_width+loop_width) : loop_width);
						}
						const RibbonVertebra& rv=(e==0 ? subspine.front() : subspine.back());
						const voronota::apollota::SimplePoint c=rv.center;
						const voronota::apollota::SimplePoint u=(rv.up-c).unit()*hk;
						const voronota::apollota::SimplePoint d=u.inverted();
						const voronota::apollota::SimplePoint r=(rv.right-c).unit()*mwk;
						const voronota::apollota::SimplePoint l=r.inverted();
						const voronota::apollota::SimplePoint n=((subspine.front().center-subspine.back().center)*(e==0 ? 1.0 : -1.0)).unit();
						std::vector<voronota::apollota::SimplePoint> vertices;
						std::vector<voronota::apollota::SimplePoint> normals;
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

private:
	class ResidueOrientation
	{
	public:
		CRAD crad;
		voronota::apollota::SimplePoint CA;
		voronota::apollota::SimplePoint C;
		voronota::apollota::SimplePoint N;
		voronota::apollota::SimplePoint O;
		voronota::apollota::SimplePoint up;
		voronota::apollota::SimplePoint right;
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

		void orient(const voronota::apollota::SimplePoint& forward)
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

	struct RibbonVertebra
	{
		voronota::apollota::SimplePoint center;
		voronota::apollota::SimplePoint up;
		voronota::apollota::SimplePoint right;
		int ss_type;
	};

	static int ss_type_from_ball_value(const voronota::common::BallValue& ball_value)
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

	static std::vector< std::vector<ResidueOrientation> > collect_residue_orientations(const std::vector< std::pair<CRAD, voronota::common::BallValue> >& list_of_balls)
	{
		std::map<CRAD, ResidueOrientation> map_of_residue_orientations;
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const CRAD& crad=list_of_balls[i].first;
			const voronota::common::BallValue& ball_value=list_of_balls[i].second;
			const voronota::apollota::SimplePoint ball_center(ball_value);
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
					if(prev_ro.crad.chainID==ro.crad.chainID && voronota::apollota::distance_from_point_to_point(prev_ro.C, ro.N)<1.6)
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

	static std::map< CRAD, std::vector<RibbonVertebra> > construct_ribbon_spine(const std::vector<ResidueOrientation>& ros, const double k, const int steps)
	{
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
			rv.ss_type=ro.ss_type;
		}
		for(std::size_t i=1;i+1<controls.size();i++)
		{
			RibbonVertebra& rv=controls[i];
			const ResidueOrientation& prev_ro=ros[i-1];
			const ResidueOrientation& next_ro=ros[i+1];
			if(rv.ss_type==2 && prev_ro.ss_type==2 && next_ro.ss_type==1)
			{
				rv.ss_type=0;
			}
			else if(rv.ss_type==2 && prev_ro.ss_type==1 && next_ro.ss_type==2)
			{
				rv.ss_type=0;
			}
			else if(rv.ss_type!=0 && prev_ro.ss_type==0 && next_ro.ss_type==0)
			{
				rv.ss_type=0;
			}
		}
		for(std::size_t i=0;i<controls.size();i++)
		{
			const RibbonVertebra& rv=controls[i];
			if(rv.ss_type==2)
			{
				beta_ids.push_back(i);
			}
			if(rv.ss_type!=2 || (i+1)==controls.size())
			{
				if(beta_ids.size()>2)
				{
					std::vector<voronota::apollota::SimplePoint> controls_center(beta_ids.size());
					std::vector<voronota::apollota::SimplePoint> controls_up(beta_ids.size());
					std::vector<voronota::apollota::SimplePoint> controls_right(beta_ids.size());
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
						rv.center=voronota::apollota::bezier_curve_point(controls_center, t);
						rv.up=voronota::apollota::bezier_curve_point(controls_up, t);
						rv.right=voronota::apollota::bezier_curve_point(controls_right, t);
					}
				}
				beta_ids.clear();
			}
		}
		if(controls.size()>=4)
		{
			for(std::size_t i=0;i+1<controls.size();i++)
			{
				int ss_types[4]={(i>0 ? controls[i-1].ss_type : 0), controls[i].ss_type, controls[i+1].ss_type, (i+2<controls.size() ? controls[i+2].ss_type : 0)};
				std::vector<voronota::apollota::SimplePoint> strip_center;
				std::vector<voronota::apollota::SimplePoint> strip_up;
				std::vector<voronota::apollota::SimplePoint> strip_right;
				if(i==0 || (ss_types[0]!=2 && ss_types[1]==2 && ss_types[2]==2))
				{
					strip_center=voronota::apollota::interpolate_using_cubic_hermite_spline(controls[i].center+(controls[i].center-controls[i+1].center), controls[i].center, controls[i+1].center, controls[i+2].center, k, steps);
					strip_up=voronota::apollota::interpolate_using_cubic_hermite_spline(controls[i].up+(controls[i].up-controls[i+1].up), controls[i].up, controls[i+1].up, controls[i+2].up, k, steps);
					strip_right=voronota::apollota::interpolate_using_cubic_hermite_spline(controls[i].right+(controls[i].right-controls[i+1].right), controls[i].right, controls[i+1].right, controls[i+2].right, k, steps);
				}
				else if(i+2==controls.size() || (ss_types[1]==2 && ss_types[2]==2 && ss_types[3]!=2))
				{
					strip_center=voronota::apollota::interpolate_using_cubic_hermite_spline(controls[i-1].center, controls[i].center, controls[i+1].center, controls[i+1].center+(controls[i+1].center-controls[i].center), k, steps);
					strip_up=voronota::apollota::interpolate_using_cubic_hermite_spline(controls[i-1].up, controls[i].up, controls[i+1].up, controls[i+1].up+(controls[i+1].up-controls[i].up), k, steps);
					strip_right=voronota::apollota::interpolate_using_cubic_hermite_spline(controls[i-1].right, controls[i].right, controls[i+1].right, controls[i+1].right+(controls[i+1].right-controls[i].right), k, steps);
				}
				else
				{
					strip_center=voronota::apollota::interpolate_using_cubic_hermite_spline(controls[i-1].center, controls[i].center, controls[i+1].center, controls[i+2].center, k, steps);
					strip_up=voronota::apollota::interpolate_using_cubic_hermite_spline(controls[i-1].up, controls[i].up, controls[i+1].up, controls[i+2].up, k, steps);
					strip_right=voronota::apollota::interpolate_using_cubic_hermite_spline(controls[i-1].right, controls[i].right, controls[i+1].right, controls[i+2].right, k, steps);
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
						if(ss_types[1]==0 && ss_types[2]!=0 && ss_types[3]!=0 && ss_types[2]==ss_types[3])
						{
							v.ss_type=controls[i].ss_type;
							result_a.push_back(v);
						}
						else if(ss_types[0]!=0 && ss_types[1]!=0 && ss_types[2]==0 && ss_types[0]==ss_types[1])
						{
							v.ss_type=controls[i+1].ss_type;
							result_b.push_back(v);
						}
						else
						{
							if(j<strip_center.size()/2)
							{
								v.ss_type=controls[i].ss_type;
								result_a.push_back(v);
							}
							else if(j==strip_center.size()/2)
							{
								v.ss_type=controls[i].ss_type;
								result_a.push_back(v);
								v.ss_type=controls[i+1].ss_type;
								result_b.push_back(v);
							}
							else
							{
								v.ss_type=controls[i+1].ss_type;
								result_b.push_back(v);
							}
						}
					}
				}
			}
		}
		return result;
	}

	static std::map< CRAD, std::vector<RibbonVertebra> > construct_ribbon_spine(const std::vector< std::pair<CRAD, voronota::common::BallValue> >& list_of_balls, const double k, const int steps)
	{
		std::map< CRAD, std::vector<RibbonVertebra> > result;
		const std::vector< std::vector<ResidueOrientation> > residue_orientations=collect_residue_orientations(list_of_balls);
		for(std::size_t i=0;i<residue_orientations.size();i++)
		{
			std::map< CRAD, std::vector<RibbonVertebra> > partial_result=construct_ribbon_spine(residue_orientations[i], k, steps);
			result.insert(partial_result.begin(), partial_result.end());
		}
		return result;
	}
};

class DrawingNucleicAcidCartoons
{
public:
	double main_radius;
	double side_radius;
	double hermite_spline_k;
	double hermite_spline_steps;
	int cylinder_quality;

	DrawingNucleicAcidCartoons() :
		main_radius(0.5),
		side_radius(0.3),
		hermite_spline_k(0.5),
		hermite_spline_steps(4),
		cylinder_quality(12)
	{
	}

	void draw_cartoon(
			const std::vector< std::pair<CRAD, voronota::common::BallValue> >& list_of_balls,
			const voronota::modescommon::DrawingParametersWrapper& drawing_parameters_wrapper,
			voronota::auxiliaries::OpenGLPrinter& opengl_printer)
	{
		std::map<CRAD, voronota::common::BallValue> map_of_crad_values;
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const CRAD& crad=list_of_balls[i].first;
			if(crad.name=="C3'")
			{
				map_of_crad_values[crad.without_atom()]=list_of_balls[i].second;
			}
		}
		const std::vector< std::vector<ResidueOrientation> > residue_orientations=collect_residue_orientations(list_of_balls);
		const std::map< CRAD, std::vector<RibbonVertebra> > spine=construct_ribbon_spine(residue_orientations, hermite_spline_k, hermite_spline_steps);
		for(std::map< CRAD, std::vector<RibbonVertebra> >::const_iterator it=spine.begin();it!=spine.end();++it)
		{
			const CRAD& crad=it->first;
			const std::vector<RibbonVertebra>& subspine=it->second;
			drawing_parameters_wrapper.process(crad, map_of_crad_values[crad].props.adjuncts, opengl_printer);
			for(std::size_t i=0;i+1<subspine.size();i++)
			{
				const RibbonVertebra& rv=subspine[i];
				const RibbonVertebra& next_rv=subspine[i+1];
				draw_cylinder(voronota::apollota::SimpleSphere(rv.center, main_radius), voronota::apollota::SimpleSphere(next_rv.center, main_radius), cylinder_quality, opengl_printer);
			}
			for(std::size_t i=0;i<subspine.size();i++)
			{
				const RibbonVertebra& rv=subspine[i];
				opengl_printer.add_sphere(voronota::apollota::SimpleSphere(rv.center, main_radius));
			}
		}
		for(std::size_t i=0;i<residue_orientations.size();i++)
		{
			for(std::size_t j=0;j<residue_orientations[i].size();j++)
			{
				const ResidueOrientation& ro=residue_orientations[i][j];
				const CRAD& crad=ro.crad;
				const voronota::apollota::SimplePoint p=(ro.C3+((ro.C2-ro.C3).unit()*6.0));
				drawing_parameters_wrapper.process(crad, map_of_crad_values[crad].props.adjuncts, opengl_printer);
				draw_cylinder(voronota::apollota::SimpleSphere(ro.C3, side_radius), voronota::apollota::SimpleSphere(p, side_radius), cylinder_quality, opengl_printer);
				opengl_printer.add_sphere(voronota::apollota::SimpleSphere(p, side_radius));
			}
		}
	}

private:
	class ResidueOrientation
	{
	public:
		CRAD crad;
		voronota::apollota::SimplePoint C3;
		voronota::apollota::SimplePoint C2;
		bool C3_flag;
		bool C2_flag;

		ResidueOrientation() : C3_flag(false), C2_flag(false)
		{
		}

		bool valid() const
		{
			return (C3_flag && C2_flag);
		}
	};

	struct RibbonVertebra
	{
		voronota::apollota::SimplePoint center;
	};

	static std::vector< std::vector<ResidueOrientation> > collect_residue_orientations(const std::vector< std::pair<CRAD, voronota::common::BallValue> >& list_of_balls)
	{
		std::map<CRAD, ResidueOrientation> map_of_residue_orientations;
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const CRAD& crad=list_of_balls[i].first;
			const voronota::common::BallValue& ball_value=list_of_balls[i].second;
			const voronota::apollota::SimplePoint ball_center(ball_value);
			ResidueOrientation& ro=map_of_residue_orientations[crad.without_atom()];
			ro.crad=crad.without_atom();
			if(crad.name=="C3'")
			{
				ro.C3=ball_center;
				ro.C3_flag=true;
			}
			else if(crad.name=="C2")
			{
				ro.C2=ball_center;
				ro.C2_flag=true;
			}
		}
		std::vector< std::vector<ResidueOrientation> > result;
		for(std::map<CRAD, ResidueOrientation>::const_iterator it=map_of_residue_orientations.begin();it!=map_of_residue_orientations.end();++it)
		{
			const ResidueOrientation& ro=it->second;
			if(ro.valid())
			{
				if(result.empty())
				{
					result.push_back(std::vector<ResidueOrientation>(1, ro));
				}
				else
				{
					const ResidueOrientation& prev_ro=result.back().back();
					if(prev_ro.crad.chainID==ro.crad.chainID && voronota::apollota::distance_from_point_to_point(prev_ro.C3, ro.C3)<8.0)
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
		return result;
	}

	static std::map< CRAD, std::vector<RibbonVertebra> > construct_ribbon_spine(const std::vector<ResidueOrientation>& ros, const double k, const int steps)
	{
		std::map< CRAD, std::vector<RibbonVertebra> > result;
		std::vector<RibbonVertebra> controls(ros.size());
		for(std::size_t i=0;i<ros.size();i++)
		{
			const ResidueOrientation& ro=ros[i];
			RibbonVertebra& rv=controls[i];
			rv.center=ro.C3;
		}
		if(controls.size()>=4)
		{
			for(std::size_t i=0;i+1<controls.size();i++)
			{
				std::vector<voronota::apollota::SimplePoint> strip_center;
				if(i==0)
				{
					strip_center=voronota::apollota::interpolate_using_cubic_hermite_spline(controls[i].center+(controls[i].center-controls[i+1].center), controls[i].center, controls[i+1].center, controls[i+2].center, k, steps);
				}
				else if(i+2==controls.size())
				{
					strip_center=voronota::apollota::interpolate_using_cubic_hermite_spline(controls[i-1].center, controls[i].center, controls[i+1].center, controls[i+1].center+(controls[i+1].center-controls[i].center), k, steps);
				}
				else
				{
					strip_center=voronota::apollota::interpolate_using_cubic_hermite_spline(controls[i-1].center, controls[i].center, controls[i+1].center, controls[i+2].center, k, steps);
				}
				if(!strip_center.empty())
				{
					std::vector<RibbonVertebra>& result_a=result[ros[i].crad];
					std::vector<RibbonVertebra>& result_b=result[ros[i+1].crad];
					result_a.reserve(strip_center.size()/2+1);
					result_b.reserve(strip_center.size()/2+1);
					for(std::size_t j=0;j<strip_center.size();j++)
					{
						RibbonVertebra v;
						v.center=strip_center[j];
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

	static std::map< CRAD, std::vector<RibbonVertebra> > construct_ribbon_spine(const std::vector< std::vector<ResidueOrientation> >& residue_orientations, const double k, const int steps)
	{
		std::map< CRAD, std::vector<RibbonVertebra> > result;
		for(std::size_t i=0;i<residue_orientations.size();i++)
		{
			std::map< CRAD, std::vector<RibbonVertebra> > partial_result=construct_ribbon_spine(residue_orientations[i], k, steps);
			result.insert(partial_result.begin(), partial_result.end());
		}
		return result;
	}
};

}

void draw_balls(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'annotation x y z r tags adjuncts')");
	pohw.describe_io("stdout", false, true, "list of balls (line format: 'annotation x y z r tags adjuncts')");

	const std::string representation=poh.argument<std::string>(pohw.describe_option("--representation", "string", "representation name: 'vdw', 'sticks', 'trace' or 'cartoon'"), "vdw");
	const std::string drawing_for_pymol=poh.argument<std::string>(pohw.describe_option("--drawing-for-pymol", "string", "file path to output drawing as pymol script"), "");
	const std::string drawing_for_scenejs=poh.argument<std::string>(pohw.describe_option("--drawing-for-scenejs", "string", "file path to output drawing as scenejs script"), "");
	const std::string drawing_name=poh.argument<std::string>(pohw.describe_option("--drawing-name", "string", "graphics object name for drawing output"), "balls");
	voronota::modescommon::DrawingParametersWrapper drawing_parameters_wrapper;
	drawing_parameters_wrapper.default_color=poh.convert_hex_string_to_integer<unsigned int>(poh.argument<std::string>(pohw.describe_option("--default-color", "string", "default color for drawing output, in hex format, white is 0xFFFFFF"), "0xFFFFFF"));
	drawing_parameters_wrapper.adjunct_gradient=poh.argument<std::string>(pohw.describe_option("--adjunct-gradient", "string", "adjunct name to use for gradient-based coloring"), "");
	drawing_parameters_wrapper.adjunct_gradient_blue=poh.argument<double>(pohw.describe_option("--adjunct-gradient-blue", "number", "blue adjunct gradient value"), 0.0);
	drawing_parameters_wrapper.adjunct_gradient_red=poh.argument<double>(pohw.describe_option("--adjunct-gradient-red", "number", "red adjunct gradient value"), 1.0);
	drawing_parameters_wrapper.rainbow_gradient=poh.contains_option(pohw.describe_option("--rainbow-gradient", "", "flag to use rainbow color gradient"));
	drawing_parameters_wrapper.adjuncts_rgb=poh.contains_option(pohw.describe_option("--adjuncts-rgb", "", "flag to use RGB color values from adjuncts"));
	drawing_parameters_wrapper.random_colors=poh.contains_option(pohw.describe_option("--random-colors", "", "flag to use random color for each drawn ball"));
	drawing_parameters_wrapper.random_colors_by_chain=poh.contains_option(pohw.describe_option("--random-colors-by-chain", "", "flag to use random color for each drawn chain"));
	drawing_parameters_wrapper.use_labels=poh.contains_option(pohw.describe_option("--use-labels", "", "flag to use labels in drawing if possible"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	typedef std::vector< std::pair<CRAD, voronota::common::BallValue> > ListOfBalls;
	const ListOfBalls list_of_balls=voronota::auxiliaries::IOUtilities().read_lines_to_map<ListOfBalls>(std::cin);
	if(list_of_balls.empty())
	{
		throw std::runtime_error("No input.");
	}

	voronota::auxiliaries::OpenGLPrinter opengl_printer;
	opengl_printer.add_color(drawing_parameters_wrapper.default_color);
	if(representation=="vdw")
	{
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const CRAD& crad=list_of_balls[i].first;
			const voronota::common::BallValue& value=list_of_balls[i].second;
			drawing_parameters_wrapper.process(crad, value.props.adjuncts, opengl_printer);
			opengl_printer.add_sphere(value);
		}
	}
	else if(representation=="sticks")
	{
		draw_links(list_of_balls, 0.8, 4.0, 0.2, 0.2, 9, false, drawing_parameters_wrapper, opengl_printer);
	}
	else if(representation=="trace")
	{
		draw_trace(list_of_balls, "CA", 4.0, 0.3, drawing_parameters_wrapper, opengl_printer);
		draw_trace(list_of_balls, "C3'", 8.0, 0.3, drawing_parameters_wrapper, opengl_printer);
	}
	else if(representation=="cartoon")
	{
		DrawingProteinCartoons().draw_cartoon(list_of_balls, drawing_parameters_wrapper, opengl_printer);
		DrawingNucleicAcidCartoons().draw_cartoon(list_of_balls, drawing_parameters_wrapper, opengl_printer);
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

	voronota::auxiliaries::IOUtilities().write_map(list_of_balls, std::cout);
}
