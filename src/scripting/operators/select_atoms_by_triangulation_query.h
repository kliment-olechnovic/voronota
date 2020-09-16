#ifndef SCRIPTING_OPERATORS_SELECT_ATOMS_BY_TRIANGULATION_QUERY_H_
#define SCRIPTING_OPERATORS_SELECT_ATOMS_BY_TRIANGULATION_QUERY_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SelectAtomsByTriangulationQuery : public OperatorBase<SelectAtomsByTriangulationQuery>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms initial_atoms_summary;
		std::size_t number_of_relevant_voronoi_vertices;
		double total_relevant_tetrahedron_volume;
		SummaryOfAtoms selected_atoms_summary;
		std::string selection_name;

		Result() : number_of_relevant_voronoi_vertices(0), total_relevant_tetrahedron_volume(0.0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(initial_atoms_summary, heterostorage.variant_object.object("initial_atoms_summary"));
			heterostorage.variant_object.value("number_of_relevant_voronoi_vertices")=number_of_relevant_voronoi_vertices;
			heterostorage.variant_object.value("total_relevant_tetrahedron_volume")=total_relevant_tetrahedron_volume;
			VariantSerialization::write(selected_atoms_summary, heterostorage.variant_object.object("selected_atoms_summary"));
			if(selection_name.empty())
			{
				heterostorage.variant_object.value("selection_name").set_null();
			}
			else
			{
				heterostorage.variant_object.value("selection_name")=selection_name;
			}
		}
	};

	SelectionManager::Query parameters_for_selecting_atoms;
	FilteringOfTriangulation::Query filtering_query_without_ids;
	std::string name;

	SelectAtomsByTriangulationQuery()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_atoms=OperatorsUtilities::read_generic_selecting_query(input);
		filtering_query_without_ids=OperatorsUtilities::read_filtering_of_triangulation_query(input);
		name=input.get_value_or_default<std::string>("name", "");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		OperatorsUtilities::document_read_filtering_of_triangulation_query(doc);
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "atom selection name", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_triangulation_info_availability();

		FilteringOfTriangulation::Query filtering_query=filtering_query_without_ids;
		filtering_query.atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

		if(filtering_query.atom_ids.empty())
		{
		    throw std::runtime_error(std::string("No initial atoms selected."));
		}

		const FilteringOfTriangulation::MatchingResult filtering_result=FilteringOfTriangulation::match_vertices(data_manager.triangulation_info(), filtering_query);

		if(filtering_result.vertices_info.empty())
		{
		    throw std::runtime_error(std::string("No triangulation parts selected."));
		}

		std::set<std::size_t> result_ids;

		for(std::size_t i=0;i<filtering_result.vertices_info.size();i++)
		{
			const FilteringOfTriangulation::VertexInfo& vi=filtering_result.vertices_info[i];
			for(std::size_t j=0;j<4;j++)
			{
				result_ids.insert(vi.quadruple.get(j));
			}
		}

		if(result_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		if(!name.empty())
		{
			data_manager.selection_manager().set_atoms_selection(name, result_ids);
		}

		Result result;
		result.initial_atoms_summary=SummaryOfAtoms(data_manager.atoms(), filtering_query.atom_ids);
		result.number_of_relevant_voronoi_vertices=filtering_result.vertices_info.size();
		result.total_relevant_tetrahedron_volume=filtering_result.total_relevant_tetrahedron_volume;
		result.selected_atoms_summary=SummaryOfAtoms(data_manager.atoms(), result_ids);
		result.selection_name=name;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SELECT_ATOMS_BY_TRIANGULATION_QUERY_H_ */
