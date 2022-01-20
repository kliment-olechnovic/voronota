#ifndef SCRIPTING_OPERATORS_CHECK_DISTANCE_CONSTRAINT_H_
#define SCRIPTING_OPERATORS_CHECK_DISTANCE_CONSTRAINT_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class CheckDistanceConstraint : public OperatorBase<CheckDistanceConstraint>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string selection_expresion_for_atoms_a;
	std::string selection_expresion_for_atoms_b;
	double interval_min;
	double interval_max;
	bool all;

	CheckDistanceConstraint() : interval_min((-std::numeric_limits<double>::max())), interval_max(std::numeric_limits<double>::max()), all(false)
	{
	}

	void initialize(CommandInput& input)
	{
		selection_expresion_for_atoms_a=input.get_value<std::string>("atoms-first");
		selection_expresion_for_atoms_b=input.get_value<std::string>("atoms-second");
		interval_min=input.get_value_or_default<double>("interval-min", (-std::numeric_limits<double>::max()));
		interval_max=input.get_value_or_default<double>("interval-max", std::numeric_limits<double>::max());
		all=input.get_flag("all");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("atoms-first", CDOD::DATATYPE_STRING, "selection expression for the first group of atoms"));
		doc.set_option_decription(CDOD("atoms-second", CDOD::DATATYPE_STRING, "selection expression for the second group of atoms"));
		doc.set_option_decription(CDOD("interval-min", CDOD::DATATYPE_FLOAT, "distance interval minimum", "-inf"));
		doc.set_option_decription(CDOD("interval-max", CDOD::DATATYPE_FLOAT, "distance interval maximum", "+inf"));
		doc.set_option_decription(CDOD("all", CDOD::DATATYPE_BOOL, "flag to check if all selected distances are in the interval"));
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
		int count_in=0;

		for(std::set<std::size_t>::const_iterator it_a=atom_ids_a.begin();it_a!=atom_ids_a.end();++it_a)
		{
			const apollota::SimplePoint pount_a(data_manager.atoms()[*it_a].value);
			for(std::set<std::size_t>::const_iterator it_b=atom_ids_b.begin();it_b!=atom_ids_b.end();++it_b)
			{
				const apollota::SimplePoint pount_b(data_manager.atoms()[*it_b].value);
				const double distance=apollota::distance_from_point_to_point(pount_a, pount_b);
				if(distance>=interval_min && distance<=interval_max)
				{
					count_in++;
					if(!all)
					{
						return result;
					}
				}
				else
				{
					if(all)
					{
						throw std::runtime_error(std::string("Found a pair of atoms that did not satisfy the distance constraint."));
					}
				}
			}
		}

		if(count_in==0)
		{
			throw std::runtime_error(std::string("No pair of atoms satisfied the distance constraint."));
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_CHECK_DISTANCE_CONSTRAINT_H_ */
