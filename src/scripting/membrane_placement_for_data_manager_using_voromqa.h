#ifndef SCRIPTING_MEMBRANE_PLACEMENT_FOR_DATA_MANAGER_USING_VOROMQA_H_
#define SCRIPTING_MEMBRANE_PLACEMENT_FOR_DATA_MANAGER_USING_VOROMQA_H_

#include "data_manager.h"

namespace voronota
{

namespace scripting
{

class MembranePlacementForDataManagerUsingVoroMQA
{
public:
	struct AtomDescriptor
	{
		std::size_t atom_id;
		std::size_t solvent_contact_id;
		double area;
		bool exposure_recorded;
		double exposure;
		double frustration;
		double projection;
		double membrane_place_value;
		apollota::SimplePoint point;

		AtomDescriptor() :
			atom_id(0),
			solvent_contact_id(0),
			area(0),
			exposure_recorded(false),
			exposure(0),
			frustration(0),
			projection(0),
			membrane_place_value(0)
		{
		}

		bool operator<(const AtomDescriptor& v) const
		{
			return (projection<v.projection);
		}

		double calc_weight() const
		{
			return area*(1.0-std::max(0.0, std::min(exposure, 1.0)));
		}
	};

	struct OrientationScore
	{
		bool assigned;
		double correlation;
		double projection_center;
		int number_of_checks;
		apollota::SimplePoint direction;

		OrientationScore() :
			assigned(false),
			correlation(0.0),
			projection_center(0),
			number_of_checks(0)
		{
		}

		double value() const
		{
			return correlation;
		}
	};

	static std::vector<AtomDescriptor> init_atom_descriptors(
			DataManager& data_manager,
			const std::string& adjunct_contact_frustration_value,
			const std::string& adjunct_atom_exposure_value)
	{
		const std::set<std::size_t> solvent_contact_ids=data_manager.selection_manager().select_contacts(
				SelectionManager::Query(std::string("[--solvent --adjuncts ")+adjunct_contact_frustration_value+"]", false));

		if(solvent_contact_ids.empty())
		{
			throw std::runtime_error(std::string("No solvent contacts with energy values."));
		}

		const std::set<std::size_t> exterior_atom_ids=data_manager.selection_manager().select_atoms_by_contacts(solvent_contact_ids, false);

		if(exterior_atom_ids.empty())
		{
			throw std::runtime_error(std::string("No relevant atoms."));
		}

		std::vector<AtomDescriptor> atom_descriptors(exterior_atom_ids.size());

		{
			std::size_t i=0;
			for(std::set<std::size_t>::const_iterator it=exterior_atom_ids.begin();it!=exterior_atom_ids.end();++it)
			{
				atom_descriptors[i++].atom_id=(*it);
			}
		}

		{
			std::vector<std::size_t> atom_solvent_contact_ids(data_manager.atoms().size(), 0);

			for(std::set<std::size_t>::const_iterator it=solvent_contact_ids.begin();it!=solvent_contact_ids.end();++it)
			{
				atom_solvent_contact_ids[data_manager.contacts()[*it].ids[0]]=(*it);
			}

			for(std::size_t i=0;i<atom_descriptors.size();i++)
			{
				atom_descriptors[i].solvent_contact_id=atom_solvent_contact_ids[atom_descriptors[i].atom_id];
			}
		}

		for(std::size_t i=0;i<atom_descriptors.size();i++)
		{
			const Contact& contact=data_manager.contacts()[atom_descriptors[i].solvent_contact_id];
			const Atom& atom=data_manager.atoms()[atom_descriptors[i].atom_id];
			atom_descriptors[i].area=contact.value.area;
			atom_descriptors[i].frustration=contact.value.props.adjuncts.find(adjunct_contact_frustration_value)->second;
			atom_descriptors[i].point=apollota::SimplePoint(atom.value);
			if(!adjunct_atom_exposure_value.empty() && atom.value.props.adjuncts.count(adjunct_atom_exposure_value)>0)
			{
				atom_descriptors[i].exposure_recorded=true;
				atom_descriptors[i].exposure=atom.value.props.adjuncts.find(adjunct_atom_exposure_value)->second;
			}
		}

		return atom_descriptors;
	}

	static OrientationScore score_orientation(
			std::vector<AtomDescriptor>& atom_descriptors,
			const double window_width,
			const double window_width_extended)
	{
		apollota::SubdividedIcosahedron sih(2);
		sih.fit_into_sphere(apollota::SimplePoint(0, 0, 0), 1);

		std::size_t best_id=0;
		OrientationScore prev_best_score;

		OrientationScore best_score;
		int number_of_checks=0;

		while(number_of_checks<1000 && (!best_score.assigned || ((best_score.value()-prev_best_score.value())>fabs(prev_best_score.value()*0.01))))
		{
			std::size_t start_id=0;
			if(number_of_checks>0)
			{
				start_id=sih.vertices().size();
				sih.grow(best_id, true);
			}
			for(std::size_t i=start_id;i<sih.vertices().size();i++)
			{
				OrientationScore score=score_orientation(atom_descriptors, sih.vertices()[i], window_width, window_width_extended);
				if(!best_score.assigned || score.value()>best_score.value())
				{
					best_id=i;
					prev_best_score=best_score;
					best_score=score;
				}
				number_of_checks++;
			}
		}

		score_orientation(atom_descriptors, best_score.direction, window_width, window_width_extended);

		best_score.number_of_checks=number_of_checks;

		return best_score;
	}

private:
	static double calc_window_value(const double window_center, const double window_width, const double window_width_extended, const double x)
	{
		if(window_width==window_width_extended)
		{
			return ((fabs(window_center-x)<(window_width*0.5)) ? 1.0 : 0.0);
		}
		const double pi=3.14159265358979323846;
		const double N=window_width_extended;
		const double alpha=(window_width_extended-window_width)/window_width_extended;
		const double n=x-(window_center-(window_width_extended*0.5));
		if(n>0.0 && n<(alpha*N*0.5))
		{
			return (0.5*(1.0+cos(pi*((2*n)/(alpha*N)-1.0))));
		}
		else if(n>=(alpha*N*0.5) && n<=(N*(1.0-(alpha*0.5))))
		{
			return 1.0;
		}
		else if(n>(N*(1.0-(alpha*0.5))) && n<N)
		{
			return (0.5*(1.0+cos(pi*((2*n)/(alpha*N)-2.0/alpha+1.0))));
		}
		return 0.0;
	}

	static double calc_mean(const std::vector<double>& x, const std::vector<double>& w)
	{
		double a=0.0;
		double b=0.0;
		for(std::size_t i=0;(i<x.size() && i<w.size());i++)
		{
			a+=x[i]*w[i];
			b+=w[i];
		}
		return (a/b);
	}

	static double calc_covariance(const std::vector<double>& x, const std::vector<double>& y, const std::vector<double>& w)
	{
		const double mean_x=calc_mean(x, w);
		const double mean_y=calc_mean(y, w);
		double a=0.0;
		double b=0.0;
		for(std::size_t i=0;(i<x.size() && i<y.size() && i<w.size());i++)
		{
			a+=w[i]*(x[i]-mean_x)*(y[i]-mean_y);
			b+=w[i];
		}
		return (a/b);
	}

	static OrientationScore score_orientation(
			std::vector<AtomDescriptor>& atom_descriptors,
			const apollota::SimplePoint& direction_raw,
			const double window_width,
			const double window_width_extended)
	{
		OrientationScore best_score;

		if(atom_descriptors.empty())
		{
			return best_score;
		}

		const apollota::SimplePoint direction_unit=direction_raw.unit();

		for(std::size_t i=0;i<atom_descriptors.size();i++)
		{
			AtomDescriptor& ad_i=atom_descriptors[i];
			ad_i.projection=ad_i.point*direction_unit;
		}

		std::sort(atom_descriptors.begin(), atom_descriptors.end());

		{
			std::vector<double> x(atom_descriptors.size(), 0.0);
			std::vector<double> y(atom_descriptors.size(), 0.0);
			std::vector<double> w(atom_descriptors.size(), 0.0);

			for(std::size_t i=0;i<atom_descriptors.size();i++)
			{
				AtomDescriptor& ad_i=atom_descriptors[i];
				x[i]=ad_i.frustration;
				w[i]=ad_i.calc_weight();
			}

			const double var_x=calc_covariance(x, x, w);

			if(var_x>0.0)
			{
				double projection_start=(atom_descriptors.begin()->projection-window_width);
				double projection_end=(atom_descriptors.rbegin()->projection+window_width);
				double projection_step=window_width*0.25;

				{
					const double length=(projection_end-projection_start);
					projection_step=length/ceil(length/projection_step);
				}

				const double projection_step_deviation=0.1;

				do
				{
					for(double window_center=projection_start;window_center<=(projection_end+projection_step_deviation);window_center+=projection_step)
					{
						for(std::size_t i=0;i<atom_descriptors.size();i++)
						{
							AtomDescriptor& ad_i=atom_descriptors[i];
							y[i]=calc_window_value(window_center, window_width, window_width_extended, ad_i.projection);
						}

						const double var_y=calc_covariance(y, y, w);

						if(var_y>0.0)
						{
							OrientationScore score;
							score.correlation=calc_covariance(x, y, w)/sqrt(var_x)/sqrt(var_y);

							if(!best_score.assigned || score.value()>best_score.value())
							{
								best_score=score;
								best_score.assigned=true;
								best_score.projection_center=window_center;
							}
						}
					}

					projection_start=(best_score.projection_center-projection_step);
					projection_end=(best_score.projection_center+projection_step);
					projection_step*=0.5;
				}
				while(best_score.assigned && projection_step>projection_step_deviation);
			}
		}

		if(best_score.assigned)
		{
			for(std::size_t i=0;i<atom_descriptors.size();i++)
			{
				AtomDescriptor& ad_i=atom_descriptors[i];
				ad_i.membrane_place_value=calc_window_value(best_score.projection_center, window_width, window_width_extended, ad_i.projection);
			}

			best_score.direction=direction_unit;
		}

		return best_score;
	}
};

}

}

#endif /* SCRIPTING_MEMBRANE_PLACEMENT_FOR_DATA_MANAGER_USING_VOROMQA_H_ */
