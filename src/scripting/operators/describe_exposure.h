#ifndef SCRIPTING_OPERATORS_DESCRIBE_EXPOSURE_H_
#define SCRIPTING_OPERATORS_DESCRIBE_EXPOSURE_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class DescribeExposure : public OperatorBase<DescribeExposure>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string adjunct_atom_exposure_value;
	double probe_min;
	double probe_max;
	double expansion;
	unsigned int smoothing_iterations;
	unsigned int smoothing_depth;
	double weight_power;
	bool use_sas_areas;

	DescribeExposure() : probe_min(0.0), probe_max(30.0), expansion(1.0), smoothing_iterations(2), smoothing_depth(1), weight_power(3.0), use_sas_areas(false)
	{
	}

	void initialize(CommandInput& input)
	{
		adjunct_atom_exposure_value=input.get_value_or_default<std::string>("adj-atom-exposure-value", "exposure_value");
		probe_min=input.get_value_or_default<double>("probe-min", 0.0);
		probe_max=input.get_value_or_default<double>("probe-max", 30.0);
		expansion=input.get_value_or_default<double>("expansion", 1.0);
		smoothing_iterations=input.get_value_or_default<unsigned int>("smoothing-iterations", 2);
		smoothing_depth=input.get_value_or_default<unsigned int>("smoothing-depth", 1);
		weight_power=input.get_value_or_default<double>("weight-power", 3.0);
		use_sas_areas=input.get_flag("use-sas-areas");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("adj-atom-exposure-value", CDOD::DATATYPE_STRING, "adjunct name", "exposure_value"));
		doc.set_option_decription(CDOD("probe-min", CDOD::DATATYPE_FLOAT, "min probe radius", 0.0));
		doc.set_option_decription(CDOD("probe-max", CDOD::DATATYPE_FLOAT, "max probe radius", 30.0));
		doc.set_option_decription(CDOD("expansion", CDOD::DATATYPE_FLOAT, "tangent sphere radius expansion", 1.0));
		doc.set_option_decription(CDOD("smoothing-iterations", CDOD::DATATYPE_INT, "number of smoothing iterations", 2));
		doc.set_option_decription(CDOD("smoothing-depth", CDOD::DATATYPE_INT, "smoothing depth", 1));
		doc.set_option_decription(CDOD("weight-power", CDOD::DATATYPE_FLOAT, "weight power", 3.0));
		doc.set_option_decription(CDOD("use-sas-areas", CDOD::DATATYPE_BOOL, "flag to use SAS areas"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_triangulation_info_availability();
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(adjunct_atom_exposure_value, false);

		const std::vector<apollota::SimpleSphere>& balls=common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(data_manager.atoms());

		const apollota::BoundingSpheresHierarchy bsh(balls, 3.5, 1);

		const apollota::Triangulation::VerticesVector t_vertices=
				apollota::Triangulation::collect_vertices_vector_from_quadruples_map(data_manager.triangulation_info().quadruples_map);

		std::vector<double> atoms_values(data_manager.atoms().size(), 0.0);
		std::vector<double> atoms_weights(data_manager.atoms().size(), 0.0);

		for(std::size_t i=0;i<t_vertices.size();i++)
		{
			const apollota::Quadruple& q=t_vertices[i].first;
			const apollota::SimpleSphere& s=t_vertices[i].second;
			if(s.r>probe_min)
			{
				double t_vertex_value=0.0;
				double t_vertex_weight=0.0;

				const std::vector<std::size_t> near_ids=apollota::SearchForSphericalCollisions::find_all_collisions(bsh, apollota::SimpleSphere(s, s.r+expansion));
				const std::size_t N=near_ids.size();

				if(s.r>probe_max || q.get_min_max().second>=balls.size())
				{
					t_vertex_weight=std::pow(probe_max, weight_power);
				}
				else
				{
					if(N>=4)
					{
						std::vector<apollota::SimplePoint> touches(N);
						for(std::size_t j=0;j<N;j++)
						{
							touches[j]=(apollota::SimplePoint(balls[near_ids[j]])-apollota::SimplePoint(s)).unit();
						}

						double d_sum=0.0;
						for(std::size_t j=0;(j+1)<N;j++)
						{
							for(std::size_t k=j+1;k<N;k++)
							{
								d_sum+=apollota::distance_from_point_to_point(touches[j], touches[k]);
							}
						}

						const double pi=3.14159265358979323846;
						const double max_d_sum=N*(1.0/tan(pi/(2.0*N)));
						t_vertex_value=std::min(d_sum/max_d_sum, 1.0);
					}
					t_vertex_weight=std::pow(s.r, weight_power);
				}

				for(std::size_t j=0;j<N;j++)
				{
					const std::size_t id=near_ids[j];
					atoms_values[id]+=t_vertex_value*t_vertex_weight;
					atoms_weights[id]+=t_vertex_weight;
				}
			}
		}

		for(std::size_t i=0;i<atoms_values.size();i++)
		{
			if(atoms_weights[i]>0.0)
			{
				atoms_values[i]=atoms_values[i]/atoms_weights[i];
			}
		}

		{
			const std::set<std::size_t> solvent_contact_ids=data_manager.selection_manager().select_contacts(
					SelectionManager::Query(std::string("[--solvent]"), false));

			if(solvent_contact_ids.empty())
			{
				throw std::runtime_error(std::string("No solvent contacts."));
			}

			if(use_sas_areas)
			{
				for(std::set<std::size_t>::const_iterator it=solvent_contact_ids.begin();it!=solvent_contact_ids.end();++it)
				{
					const Contact& contact=data_manager.contacts()[*it];
					atoms_weights[contact.ids[0]]*=contact.value.area;
				}
			}

			const std::set<std::size_t> exterior_atom_ids=data_manager.selection_manager().select_atoms_by_contacts(solvent_contact_ids, false);

			const std::set<std::size_t> exterior_contact_ids=data_manager.selection_manager().select_contacts(
					SelectionManager::Query("[--tags peripherial]", false));

			if(exterior_contact_ids.empty())
			{
				throw std::runtime_error(std::string("No peripherial contacts."));
			}

			std::vector< std::set<std::size_t> > graph(data_manager.atoms().size());
			for(std::set<std::size_t>::const_iterator it=exterior_contact_ids.begin();it!=exterior_contact_ids.end();++it)
			{
				const Contact& contact=data_manager.contacts()[*it];
				graph[contact.ids[0]].insert(contact.ids[1]);
				graph[contact.ids[1]].insert(contact.ids[0]);
			}

			for(unsigned int iteration=0;iteration<smoothing_iterations;iteration++)
			{
				std::vector<double> updated_atoms_values=atoms_values;

				for(std::set<std::size_t>::const_iterator it=exterior_atom_ids.begin();it!=exterior_atom_ids.end();++it)
				{
					const std::size_t central_id=(*it);

					std::map<std::size_t, bool> neighbors;
					neighbors[central_id]=false;

					for(unsigned int level=0;level<smoothing_depth;level++)
					{
						std::map<std::size_t, bool> more_neighbors;
						for(std::map<std::size_t, bool>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
						{
							const std::size_t id=jt->first;
							if(!jt->second)
							{
								for(std::set<std::size_t>::const_iterator et=graph[id].begin();et!=graph[id].end();++et)
								{
									more_neighbors[*et]=(neighbors.count(*et)>0 && neighbors.find(*et)->second);
								}
							}
							more_neighbors[id]=true;
						}
						neighbors.swap(more_neighbors);
					}

					double sum_of_weights=0.0;
					double sum_of_weighted_values=0.0;

					for(std::map<std::size_t, bool>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
					{
						sum_of_weights+=atoms_weights[jt->first];
						sum_of_weighted_values+=atoms_values[jt->first]*atoms_weights[jt->first];
					}

					updated_atoms_values[central_id]=(sum_of_weights>0.0 ? (sum_of_weighted_values/sum_of_weights) : 0.0);
				}

				atoms_values.swap(updated_atoms_values);
			}

			if(!adjunct_atom_exposure_value.empty())
			{
				for(std::size_t i=0;i<data_manager.atoms().size();i++)
				{
					data_manager.atom_adjuncts_mutable(i).erase(adjunct_atom_exposure_value);
				}

				for(std::set<std::size_t>::const_iterator it=exterior_atom_ids.begin();it!=exterior_atom_ids.end();++it)
				{
					const std::size_t central_id=(*it);
					if(atoms_weights[central_id]>0.0)
					{
						data_manager.atom_adjuncts_mutable(central_id)[adjunct_atom_exposure_value]=atoms_values[central_id];
					}
				}
			}
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_DESCRIBE_EXPOSURE_H_ */
