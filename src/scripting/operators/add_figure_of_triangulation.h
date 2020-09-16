#ifndef SCRIPTING_OPERATORS_ADD_FIGURE_OF_TRIANGULATION_H_
#define SCRIPTING_OPERATORS_ADD_FIGURE_OF_TRIANGULATION_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class AddFigureOfTriangulation : public OperatorBase<AddFigureOfTriangulation>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	SelectionManager::Query parameters_for_selecting_atoms;
	FilteringOfTriangulation::Query filtering_query_without_ids;
	std::vector<std::string> figure_name;
	bool as_spheres;

	AddFigureOfTriangulation() : as_spheres(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_atoms=OperatorsUtilities::read_generic_selecting_query(input);
		filtering_query_without_ids=OperatorsUtilities::read_filtering_of_triangulation_query(input);
		figure_name=input.get_value_vector<std::string>("figure-name");
		as_spheres=input.get_flag("as-spheres");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		OperatorsUtilities::document_read_filtering_of_triangulation_query(doc);
		doc.set_option_decription(CDOD("figure-name", CDOD::DATATYPE_STRING_ARRAY, "figure name"));
		doc.set_option_decription(CDOD("as-spheres", CDOD::DATATYPE_BOOL, "flag to add figure of tangent spheres"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_triangulation_info_availability();

		FilteringOfTriangulation::Query filtering_query=filtering_query_without_ids;
		filtering_query.atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

		if(filtering_query.atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		const FilteringOfTriangulation::MatchingResult filtering_result=FilteringOfTriangulation::match_vertices(data_manager.triangulation_info(), filtering_query);

		if(filtering_result.vertices_info.empty())
		{
			throw std::runtime_error(std::string("No triangulation parts selected."));
		}

		Figure figure;
		figure.name=LongName(figure_name);

		if(as_spheres)
		{
			const apollota::SubdividedIcosahedron sih0(0);
			const apollota::SubdividedIcosahedron sih1(1);
			const apollota::SubdividedIcosahedron sih2(2);
			for(std::size_t i=0;i<filtering_result.vertices_info.size();i++)
			{
				const apollota::SimpleSphere& sphere=filtering_result.vertices_info[i].sphere;
				if(sphere.r>0.0)
				{
					figure.add_sphere((sphere.r<0.5 ? sih0 : (sphere.r<1.5 ? sih1 : sih2)), apollota::SimplePoint(sphere), sphere.r);
				}
			}
		}
		else
		{
			const std::vector<apollota::SimpleSphere>& balls=data_manager.triangulation_info().spheres;

			for(std::size_t i=0;i<filtering_result.vertices_info.size();i++)
			{
				const apollota::Quadruple& quadruple=filtering_result.vertices_info[i].quadruple;

				for(unsigned int j=0;j<4;j++)
				{
					apollota::Triple triple=quadruple.exclude(j);
					const apollota::SimplePoint normal=apollota::plane_normal_from_three_points<apollota::SimplePoint>(balls[triple.get(0)], balls[triple.get(1)], balls[triple.get(2)]);
					figure.add_triangle(balls[triple.get(0)], balls[triple.get(1)], balls[triple.get(2)], normal);
				}
			}
		}

		figure.props.adjuncts["total_relevant_tetrahedron_volume"]=filtering_result.total_relevant_tetrahedron_volume;

		data_manager.add_figure(figure);

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_ADD_FIGURE_OF_TRIANGULATION_H_ */
