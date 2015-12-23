#ifndef DRAWING_CARTOONS_H_
#define DRAWING_CARTOONS_H_

#include "../apollota/polynomial_curves.h"

#include "drawing_utilities.h"

namespace
{

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
		const std::map< CRAD, std::vector<RibbonVertebra> > spine=construct_ribbon_spine(list_of_balls, hermite_spline_k, hermite_spline_steps);
		for(std::map< CRAD, std::vector<RibbonVertebra> >::const_iterator it=spine.begin();it!=spine.end();++it)
		{
			const CRAD& crad=it->first;
			const BallValue& ball_value=map_of_crad_values[crad];
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
						std::vector<apollota::SimplePoint> vertices;
						std::vector<apollota::SimplePoint> normals;
						for(std::size_t i=0;i<subspine.size();i++)
						{
							double mwk=wk;
							if(arrow)
							{
								mwk=(1.0-static_cast<double>(i)/static_cast<double>(subspine.size()-1))*arrow_width+loop_width;
							}
							const RibbonVertebra& rv=subspine[i];
							const apollota::SimplePoint c=rv.center;
							const apollota::SimplePoint u=(o==0 ? ((rv.up-c).unit()*(e==0 ? 1.0 : -1.0)*hk) : ((rv.right-c).unit()*(e==0 ? 1.0 : -1.0)*mwk));
							const apollota::SimplePoint r=(o==0 ? ((rv.right-c).unit()*mwk) : ((rv.up-c).unit()*hk));
							const apollota::SimplePoint l=r.inverted();
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
						const apollota::SimplePoint c=rv.center;
						const apollota::SimplePoint u=(rv.up-c).unit()*hk;
						const apollota::SimplePoint d=u.inverted();
						const apollota::SimplePoint r=(rv.right-c).unit()*mwk;
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

private:
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

	struct RibbonVertebra
	{
		apollota::SimplePoint center;
		apollota::SimplePoint up;
		apollota::SimplePoint right;
		int ss_type;
	};

	static int ss_type_from_ball_value(const BallValue& ball_value)
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

	static std::vector< std::vector<ResidueOrientation> > collect_residue_orientations(const std::vector< std::pair<CRAD, BallValue> >& list_of_balls)
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
		if(controls.size()>=4)
		{
			for(std::size_t i=0;i+1<controls.size();i++)
			{
				int ss_types[4]={(i>0 ? controls[i-1].ss_type : 0), controls[i].ss_type, controls[i+1].ss_type, (i+2<controls.size() ? controls[i+2].ss_type : 0)};
				std::vector<apollota::SimplePoint> strip_center;
				std::vector<apollota::SimplePoint> strip_up;
				std::vector<apollota::SimplePoint> strip_right;
				if(i==0 || (ss_types[0]!=2 && ss_types[1]==2 && ss_types[2]==2))
				{
					strip_center=apollota::interpolate_using_cubic_hermite_spline(controls[i].center+(controls[i].center-controls[i+1].center), controls[i].center, controls[i+1].center, controls[i+2].center, k, steps);
					strip_up=apollota::interpolate_using_cubic_hermite_spline(controls[i].up+(controls[i].up-controls[i+1].up), controls[i].up, controls[i+1].up, controls[i+2].up, k, steps);
					strip_right=apollota::interpolate_using_cubic_hermite_spline(controls[i].right+(controls[i].right-controls[i+1].right), controls[i].right, controls[i+1].right, controls[i+2].right, k, steps);
				}
				else if(i+2==controls.size() || (ss_types[1]==2 && ss_types[2]==2 && ss_types[3]!=2))
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

	static std::map< CRAD, std::vector<RibbonVertebra> > construct_ribbon_spine(const std::vector< std::pair<CRAD, BallValue> >& list_of_balls, const double k, const int steps)
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

}

#endif /* DRAWING_CARTOONS_H_ */
