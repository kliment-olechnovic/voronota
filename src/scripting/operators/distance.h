#ifndef SCRIPTING_OPERATORS_DISTANCE_H_
#define SCRIPTING_OPERATORS_DISTANCE_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class Distance : public OperatorBase<Distance>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		double distance_min;
		double distance_max;

		Result() :
			distance_min(std::numeric_limits<double>::max()),
			distance_max(std::numeric_limits<double>::min())
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("distance_min")=distance_min;
			heterostorage.variant_object.value("distance_max")=distance_max;
		}
	};

	std::string selection_expresion_for_atoms_a;
	std::string selection_expresion_for_atoms_b;

	Distance()
	{
	}

	void initialize(CommandInput& input)
	{
		selection_expresion_for_atoms_a=input.get_value<std::string>("atoms-first");
		selection_expresion_for_atoms_b=input.get_value<std::string>("atoms-second");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("atoms-first", CDOD::DATATYPE_STRING, "selection expression for the first group of atoms"));
		doc.set_option_decription(CDOD("atoms-second", CDOD::DATATYPE_STRING, "selection expression for the second group of atoms"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		const std::set<std::size_t> atom_ids_a=data_manager.selection_manager().select_atoms(SelectionManager::Query(selection_expresion_for_atoms_a, false));

		if(atom_ids_a.empty())
		{
			throw std::runtime_error(std::string("No first atoms selected."));
		}

		const std::set<std::size_t> atom_ids_b=data_manager.selection_manager().select_atoms(SelectionManager::Query(selection_expresion_for_atoms_b, false));

		if(atom_ids_b.empty())
		{
			throw std::runtime_error(std::string("No second atoms selected."));
		}

		Result result;

		for(std::set<std::size_t>::const_iterator it_a=atom_ids_a.begin();it_a!=atom_ids_a.end();++it_a)
		{
			const apollota::SimplePoint point_a(data_manager.atoms()[*it_a].value);
			for(std::set<std::size_t>::const_iterator it_b=atom_ids_b.begin();it_b!=atom_ids_b.end();++it_b)
			{
				const apollota::SimplePoint point_b(data_manager.atoms()[*it_b].value);
				const double distance=apollota::distance_from_point_to_point(point_a, point_b);
				if(distance<result.distance_min)
				{
					result.distance_min=distance;
				}
				if(distance>result.distance_max)
				{
					result.distance_max=distance;
				}
			}
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_DISTANCE_H_ */
