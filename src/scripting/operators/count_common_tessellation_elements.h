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

		Result() : pairs_total(0), pairs_common(0), triples_total(0), triples_common(0), quadruples_total(0), quadruples_common(0)
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
		}
	};

	CongregationOfDataManagers::ObjectQuery query;

	CountCommonTessellationElements()
	{
	}

	void initialize(CommandInput& input)
	{
		query=OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
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
		TriplesMap triples_map;
		QuadruplesMap quadruples_map;

		for(std::size_t i=0;i<objects.size();i++)
		{
			const DataManager& data_manager=(*(objects[i]));
			for(apollota::Triangulation::QuadruplesMap::const_iterator it=data_manager.triangulation_info().quadruples_map.begin();it!=data_manager.triangulation_info().quadruples_map.end();++it)
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

		Result result;

		for(PairsMap::const_iterator it=pairs_map.begin();it!=pairs_map.end();++it)
		{
			result.pairs_total++;
			if(it->second.size()==objects.size())
			{
				result.pairs_common++;
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

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_COUNT_COMMON_TESSELLATION_ELEMENTS_H_ */
