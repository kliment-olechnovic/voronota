#ifndef SCRIPTING_OPERATORS_ADD_FIGURES_OF_PRIMITIVE_DIRECTIONS_H_
#define SCRIPTING_OPERATORS_ADD_FIGURES_OF_PRIMITIVE_DIRECTIONS_H_

#include "../operators_common.h"

#include "../primitive_atom_directions_assignment.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class AddFiguresOfPrimitiveDirections : public OperatorBase<AddFiguresOfPrimitiveDirections>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		Result()
		{
		}

		void store(HeterogeneousStorage&) const
		{
		}
	};

	SelectionManager::Query parameters_for_selecting_atoms;
	std::vector<std::string> figure_name_start;

	AddFiguresOfPrimitiveDirections()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_atoms=OperatorsUtilities::read_generic_selecting_query(input);
		figure_name_start=input.get_value_vector_or_default<std::string>("figure-name-start", std::vector<std::string>(1, std::string("pdir")));
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("figure-name", CDOD::DATATYPE_STRING_ARRAY, "figure name", "'label' 'mode value'"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(figure_name_start.empty())
		{
			throw std::runtime_error(std::string("No figure name start provided."));
		}

		const std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		PrimitiveAtomDirectionsAssignment::Result atom_directions_assignment_result;
		PrimitiveAtomDirectionsAssignment::construct_result(data_manager, atom_directions_assignment_result);

		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			const std::size_t id=(*it);
			if(!atom_directions_assignment_result.basic_directions[id].empty())
			{
				const Atom& atom=data_manager.atoms()[id];

				std::string category="undefined";
				{
					std::ostringstream output;
					output << "bonds" << atom_directions_assignment_result.directional_neighbors[id].size();
					category=output.str();
				}

				Figure figure;
				figure.name=LongName(figure_name_start, category, atom.crad.str());

				const apollota::SimplePoint o(atom.value);

				const apollota::SimplePoint& d1=atom_directions_assignment_result.basic_directions[id][0];
				const apollota::SimplePoint d2=apollota::any_normal_of_vector<apollota::SimplePoint>(d1);
				const apollota::SimplePoint d3=(d1&d2).unit();

				const apollota::SimplePoint p1=o+(d1*1.8);
				const apollota::SimplePoint p2=o+(d2*0.4);
				const apollota::SimplePoint p3=o+(d3*0.4);

				figure.add_triangle(o, p1, p2, d3);
				figure.add_triangle(o, p1, p3, d2);
				figure.add_triangle(o, p2, p3, d1);
				figure.add_triangle(p1, p2, p3, ((p2-p1)&(p3-p1)).unit());

				data_manager.add_figure(figure);
			}
		}

		Result result;
		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_ADD_FIGURES_OF_PRIMITIVE_DIRECTIONS_H_ */
