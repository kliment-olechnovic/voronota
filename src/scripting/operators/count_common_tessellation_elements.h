#ifndef SCRIPTING_OPERATORS_COUNT_COMMON_TESSELLATION_ELEMENTS_H_
#define SCRIPTING_OPERATORS_COUNT_COMMON_TESSELLATION_ELEMENTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class CountCommonTessellationElements : public OperatorBase<CountCommonTessellationElements>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		int pairs_total;
		int pairs_common;
		int triples_total;
		int triples_common;
		int quadruples_total;
		int quadruples_common;
		int expanded_pairs_total;
		int expanded_pairs_common;
		int pairs_common_in_expanded_pairs_common;
		int contact_pairs_total;
		int contact_pairs_common;
		int inter_residue_contact_pairs_total;
		int inter_residue_contact_pairs_common;

		Result() :
			pairs_total(0),
			pairs_common(0),
			triples_total(0),
			triples_common(0),
			quadruples_total(0),
			quadruples_common(0),
			expanded_pairs_total(0),
			expanded_pairs_common(0),
			pairs_common_in_expanded_pairs_common(0),
			contact_pairs_total(0),
			contact_pairs_common(0),
			inter_residue_contact_pairs_total(0),
			inter_residue_contact_pairs_common(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("pairs_total")=pairs_total;
			heterostorage.variant_object.value("pairs_common")=pairs_common;
			heterostorage.variant_object.value("triples_total")=triples_total;
			heterostorage.variant_object.value("triples_common")=triples_common;
			heterostorage.variant_object.value("quadruples_total")=quadruples_total;
			heterostorage.variant_object.value("quadruples_common")=quadruples_common;
			heterostorage.variant_object.value("expanded_pairs_total")=expanded_pairs_total;
			heterostorage.variant_object.value("expanded_pairs_common")=expanded_pairs_common;
			heterostorage.variant_object.value("pairs_common_in_expanded_pairs_common")=pairs_common_in_expanded_pairs_common;
			heterostorage.variant_object.value("contact_pairs_total")=contact_pairs_total;
			heterostorage.variant_object.value("contact_pairs_common")=contact_pairs_common;
			heterostorage.variant_object.value("inter_residue_contact_pairs_total")=inter_residue_contact_pairs_total;
			heterostorage.variant_object.value("inter_residue_contact_pairs_common")=inter_residue_contact_pairs_common;
		}
	};

	CongregationOfDataManagers::ObjectQuery query;
	double expansion;
	double max_tangent_radius;

	CountCommonTessellationElements() : expansion(1.0), max_tangent_radius(std::numeric_limits<double>::max())
	{
	}

	void initialize(CommandInput& input)
	{
		query=OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		expansion=input.get_value_or_default<double>("expansion", 1.0);
		max_tangent_radius=input.get_value_or_default<double>("max-tangent-radius", std::numeric_limits<double>::max());
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		doc.set_option_decription(CDOD("expansion", CDOD::DATATYPE_FLOAT, "tangent sphere radius expansion", 1.0));
		doc.set_option_decription(CDOD("max-tangent-radius", CDOD::DATATYPE_FLOAT, "max tangent sphere radius to consider", 1.0));
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
#if USE_TR1 > 0
typedef std::tr1::unordered_map<apollota::Pair, std::set<int>, apollota::Pair::HashFunctor> PairsMap;
typedef std::tr1::unordered_map<apollota::Triple, std::set<int>, apollota::Triple::HashFunctor> TriplesMap;
typedef std::tr1::unordered_map<apollota::Quadruple, std::set<int>, apollota::Quadruple::HashFunctor> QuadruplesMap;
#else
typedef std::unordered_map<apollota::Pair, std::set<int>, apollota::Pair::HashFunctor> PairsMap;
typedef std::unordered_map<apollota::Triple, std::set<int>, apollota::Triple::HashFunctor> TriplesMap;
typedef std::unordered_map<apollota::Quadruple, std::set<int>, apollota::Quadruple::HashFunctor> QuadruplesMap;
#endif

		congregation_of_data_managers.assert_objects_availability();

		const std::vector<DataManager*> objects=congregation_of_data_managers.get_objects(query);
		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}
		if(objects.size()==1)
		{
			throw std::runtime_error(std::string("Only one object selected."));
		}

		for(std::size_t i=0;i<objects.size();i++)
		{
			objects[i]->assert_triangulation_info_availability();
		}

		PairsMap pairs_map;
		PairsMap contact_pairs_map;
		PairsMap inter_residue_contact_pairs_map;
		TriplesMap triples_map;
		QuadruplesMap quadruples_map;

		for(std::size_t i=0;i<objects.size();i++)
		{
			const DataManager& data_manager=(*(objects[i]));
			for(apollota::Triangulation::QuadruplesMap::const_iterator it=data_manager.triangulation_info().quadruples_map.begin();it!=data_manager.triangulation_info().quadruples_map.end();++it)
			{
				if((it->second.size()>0 && it->second[0].r<=max_tangent_radius) || (it->second.size()>1 && it->second[1].r<=max_tangent_radius))
				{
					const apollota::Quadruple& q=it->first;
					for(int a=0;a<4;a++)
					{
						for(int b=a+1;b<4;b++)
						{
							const apollota::Pair p(q.get(a), q.get(b));
							if(p.get_min_max().second<data_manager.atoms().size())
							{
								pairs_map[p].insert(i);
								if(apollota::minimal_distance_from_sphere_to_sphere(data_manager.atoms()[p.get(0)].value, data_manager.atoms()[p.get(1)].value)<1.4)
								{
									contact_pairs_map[p].insert(i);
									const std::size_t ra=data_manager.primary_structure_info().map_of_atoms_to_residues[p.get(0)];
									const std::size_t rb=data_manager.primary_structure_info().map_of_atoms_to_residues[p.get(1)];
									if(ra!=rb)
									{
										inter_residue_contact_pairs_map[apollota::Pair(ra, rb)].insert(i);
									}
								}
							}
						}
						const apollota::Triple t=q.exclude(a);
						if(t.get_min_max().second<data_manager.atoms().size())
						{
							triples_map[t].insert(i);
						}
					}
					if(q.get_min_max().second<data_manager.atoms().size())
					{
						quadruples_map[q].insert(i);
					}
				}
			}
		}


		PairsMap expanded_pairs_map;

		if(expansion>0.0)
		{
			for(std::size_t i=0;i<objects.size();i++)
			{
				const DataManager& data_manager=(*(objects[i]));
				const std::vector<apollota::SimpleSphere>& balls=common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(data_manager.atoms());

				const apollota::BoundingSpheresHierarchy bsh(balls, 3.5, 1);

				const apollota::Triangulation::VerticesVector t_vertices=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(data_manager.triangulation_info().quadruples_map);

				for(std::size_t j=0;j<t_vertices.size();j++)
				{
					const apollota::SimpleSphere& s=t_vertices[j].second;
					if(s.r<=max_tangent_radius)
					{
						const std::vector<std::size_t> near_ids=apollota::SearchForSphericalCollisions::find_all_collisions(bsh, apollota::SimpleSphere(s, s.r+expansion));
						for(std::size_t a=0;a<near_ids.size();a++)
						{
							for(std::size_t b=a+1;b<near_ids.size();b++)
							{
								const apollota::Pair p(near_ids[a], near_ids[b]);
								if(p.get_min_max().second<data_manager.atoms().size())
								{
									expanded_pairs_map[p].insert(i);
								}
							}
						}
					}
				}
			}
		}

		Result result;

		for(PairsMap::const_iterator it=pairs_map.begin();it!=pairs_map.end();++it)
		{
			result.pairs_total++;
			if(it->second.size()==objects.size())
			{
				result.pairs_common++;
			}
		}

		for(PairsMap::const_iterator it=contact_pairs_map.begin();it!=contact_pairs_map.end();++it)
		{
			result.contact_pairs_total++;
			if(it->second.size()==objects.size())
			{
				result.contact_pairs_common++;
			}
		}

		for(PairsMap::const_iterator it=inter_residue_contact_pairs_map.begin();it!=inter_residue_contact_pairs_map.end();++it)
		{
			result.inter_residue_contact_pairs_total++;
			if(it->second.size()==objects.size())
			{
				result.inter_residue_contact_pairs_common++;
			}
		}

		for(TriplesMap::const_iterator it=triples_map.begin();it!=triples_map.end();++it)
		{
			result.triples_total++;
			if(it->second.size()==objects.size())
			{
				result.triples_common++;
			}
		}

		for(QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			result.quadruples_total++;
			if(it->second.size()==objects.size())
			{
				result.quadruples_common++;
			}
		}

		for(PairsMap::const_iterator it=expanded_pairs_map.begin();it!=expanded_pairs_map.end();++it)
		{
			result.expanded_pairs_total++;
			if(it->second.size()==objects.size())
			{
				result.expanded_pairs_common++;
			}
		}

		for(PairsMap::const_iterator it=pairs_map.begin();it!=pairs_map.end();++it)
		{
			if(it->second.size()==objects.size() || expanded_pairs_map[it->first].size()==objects.size())
			{
				result.pairs_common_in_expanded_pairs_common++;
			}
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_COUNT_COMMON_TESSELLATION_ELEMENTS_H_ */
