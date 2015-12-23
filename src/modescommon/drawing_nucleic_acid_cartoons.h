#ifndef DRAWING_NUCLEIC_ACID_CARTOONS_H_
#define DRAWING_NUCLEIC_ACID_CARTOONS_H_

#include "../apollota/polynomial_curves.h"

#include "drawing_utilities.h"
#include "drawing_cylinder.h"

namespace
{

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
			const std::vector< std::pair<CRAD, BallValue> >& list_of_balls,
			const DrawingParametersWrapper& drawing_parameters_wrapper,
			auxiliaries::OpenGLPrinter& opengl_printer)
	{
		std::map<CRAD, BallValue> map_of_crad_values;
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
				draw_cylinder(apollota::SimpleSphere(rv.center, main_radius), apollota::SimpleSphere(next_rv.center, main_radius), cylinder_quality, opengl_printer);
			}
			for(std::size_t i=0;i<subspine.size();i++)
			{
				const RibbonVertebra& rv=subspine[i];
				opengl_printer.add_sphere(apollota::SimpleSphere(rv.center, main_radius));
			}
		}
		for(std::size_t i=0;i<residue_orientations.size();i++)
		{
			for(std::size_t j=0;j<residue_orientations[i].size();j++)
			{
				const ResidueOrientation& ro=residue_orientations[i][j];
				const CRAD& crad=ro.crad;
				const apollota::SimplePoint p=(ro.C3+((ro.C2-ro.C3).unit()*6.0));
				drawing_parameters_wrapper.process(crad, map_of_crad_values[crad].props.adjuncts, opengl_printer);
				draw_cylinder(apollota::SimpleSphere(ro.C3, side_radius), apollota::SimpleSphere(p, side_radius), cylinder_quality, opengl_printer);
				opengl_printer.add_sphere(apollota::SimpleSphere(p, side_radius));
			}
		}
	}

private:
	class ResidueOrientation
	{
	public:
		CRAD crad;
		apollota::SimplePoint C3;
		apollota::SimplePoint C2;
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
		apollota::SimplePoint center;
	};

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
					if(prev_ro.crad.chainID==ro.crad.chainID && apollota::distance_from_point_to_point(prev_ro.C3, ro.C3)<8.0)
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
				std::vector<apollota::SimplePoint> strip_center;
				if(i==0)
				{
					strip_center=apollota::interpolate_using_cubic_hermite_spline(controls[i].center+(controls[i].center-controls[i+1].center), controls[i].center, controls[i+1].center, controls[i+2].center, k, steps);
				}
				else if(i+2==controls.size())
				{
					strip_center=apollota::interpolate_using_cubic_hermite_spline(controls[i-1].center, controls[i].center, controls[i+1].center, controls[i+1].center+(controls[i+1].center-controls[i].center), k, steps);
				}
				else
				{
					strip_center=apollota::interpolate_using_cubic_hermite_spline(controls[i-1].center, controls[i].center, controls[i+1].center, controls[i+2].center, k, steps);
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

#endif /* DRAWING_NUCLEIC_ACID_CARTOONS_H_ */
